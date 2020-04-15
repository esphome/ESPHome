#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/xiaomi_ble/xiaomi_ble.h"
#include "esphome/core/log.h"

#ifdef ARDUINO_ARCH_ESP32

namespace esphome {
namespace xiaomi_hhccjcy01 {
static const char *TAG = "xiaomi_hhccjcy01";

class XiaomiHHCCJCY01 : public Component, public esp32_ble_tracker::ESPBTDeviceListener {
 public:
  void set_address(uint64_t address) { address_ = address; }

  bool parse_device(const esp32_ble_tracker::ESPBTDevice &device) override {
    if (device.address_uint64() != this->address_)
      return false;

    auto res = xiaomi_ble::parse_xiaomi(device);
    if (!res.has_value()) {
      ESP_LOGVV(TAG, "Couldn't parse XIAOMI parse_xiaomi_header");
      return true;  // seems wrong? We have the correct address, therefore we want to stop other processing
    }

    if ((device.get_service_data().size() < 14) ||
        !res->has_data) {  // Not 100% sure on the sizing here. Might not be necessary any more
      ESP_LOGVV(TAG, "Xiaomi service data too short or missing");
      return true;  // seems wrong? We have the correct address, therefore we want to stop other processing
    }
    uint8_t *message;
    if ((*res).has_encryption) {
      ESP_LOGVV(TAG, "Decryption is currently not supported on this device. See xiaomi_lysd003mmc for detail of how to "
                     "implement bindkey_");
      return true;  // seems wrong? We have the correct address, therefore we want to stop other processing
    } else {
      message = (uint8_t *) reinterpret_cast<const uint8_t *>(device.get_service_data().data());
    }

    xiaomi_ble::parse_xiaomi_message(message, *res);
    if (!res.has_value())
      return false;

    if (res->temperature.has_value() && this->temperature_ != nullptr)
      this->temperature_->publish_state(*res->temperature);
    if (res->moisture.has_value() && this->moisture_ != nullptr)
      this->moisture_->publish_state(*res->moisture);
    if (res->conductivity.has_value() && this->conductivity_ != nullptr)
      this->conductivity_->publish_state(*res->conductivity);
    if (res->illuminance.has_value() && this->illuminance_ != nullptr)
      this->illuminance_->publish_state(*res->illuminance);
    if (res->battery_level.has_value() && this->battery_level_ != nullptr)
      this->battery_level_->publish_state(*res->battery_level);
    return true;
  }

  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void set_temperature(sensor::Sensor *temperature) { temperature_ = temperature; }
  void set_moisture(sensor::Sensor *moisture) { moisture_ = moisture; }
  void set_conductivity(sensor::Sensor *conductivity) { conductivity_ = conductivity; }
  void set_illuminance(sensor::Sensor *illuminance) { illuminance_ = illuminance; }
  void set_battery_level(sensor::Sensor *battery_level) { battery_level_ = battery_level; }

 protected:
  uint64_t address_;
  sensor::Sensor *temperature_{nullptr};
  sensor::Sensor *moisture_{nullptr};
  sensor::Sensor *conductivity_{nullptr};
  sensor::Sensor *illuminance_{nullptr};
  sensor::Sensor *battery_level_{nullptr};
};

}  // namespace xiaomi_hhccjcy01
}  // namespace esphome

#endif
