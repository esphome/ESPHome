#include "max31855.h"
#include "esphome/core/log.h"

namespace esphome {
namespace max31855 {

static const char *TAG = "max31855";

void MAX31855Sensor::update() {
  this->enable();
  delay(1);
  // conversion initiated by rising edge
  this->disable();

  // Conversion time typ: 170ms, max: 220ms
  auto f = std::bind(&MAX31855Sensor::read_data_, this);
  this->set_timeout("value", 220, f);
}

void MAX31855Sensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MAX31855Sensor '%s'...", this->name_.c_str());
  this->spi_setup();
}
void MAX31855Sensor::dump_config() {
  ESP_LOGCONFIG(TAG, "MAX31855:");
  LOG_PIN("  CS Pin: ", this->cs_);
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Thermocouple", this);
  if (this->temperature_reference_) {
    LOG_SENSOR("  ", "Reference", this->temperature_reference_);
  } else {
    ESP_LOGCONFIG(TAG, "  Reference temperature disabled.");
  }
}
float MAX31855Sensor::get_setup_priority() const { return setup_priority::DATA; }
void MAX31855Sensor::read_data_() {
  this->enable();
  delay(1);
  uint8_t data[4];
  this->read_array(data, 4);
  this->disable();

  const uint32_t mem(data[0] << 24 |
                     data[1] << 16 |
                     data[2] <<  8 |
                     data[3] <<  0);

  // Verify we got data
  if (mem && mem != 0xFFFFFFFF) {
    this->status_clear_error();
  } else {
    ESP_LOGE(TAG, "No data received from MAX31855 (0x%08X). Check wiring!", mem);
    this->publish_state(NAN);
    if (this->temperature_reference_) { this->temperature_reference_->publish_state(NAN); }
    this->status_set_error();
    return;
  }

  // Internal reference temperature always works
  if (this->temperature_reference_) {
    int16_t val((mem & 0x0000FFF0) >> 4);
    if (val & 0x800) { val |= 0xF000; } // Pad out 2's complement
    const float tRef(float(val) * 0.0625f);
    ESP_LOGD(TAG, "Got reference temperature: %.4f°C", tRef);
    this->temperature_reference_->publish_state(tRef);
  }

  // Check thermocouple faults
  if (mem & 0x00000001) {
    ESP_LOGW(TAG, "Thermocouple open circuit (not connected) fault from MAX31855 (0x%08X)", mem);
    this->publish_state(NAN);
    this->status_set_warning();
    return;
  }
  if (mem & 0x00000002) {
    ESP_LOGW(TAG, "Thermocouple short circuit to ground fault from MAX31855 (0x%08X)", mem);
    this->publish_state(NAN);
    this->status_set_warning();
    return;
  }
  if (mem & 0x00000004) {
    ESP_LOGW(TAG, "Thermocouple short circuit to VCC fault from MAX31855 (0x%08X)", mem);
    this->publish_state(NAN);
    this->status_set_warning();
    return;
  }
  if (mem & 0x00010000) {
    ESP_LOGW(TAG, "Got faulty reading from MAX31855Thermocouple short circuit to VCC fault from MAX31855 (0x%08X)", mem);
    this->publish_state(NAN);
    this->status_set_warning();
    return;
  }

  // Decode thermocouple temperature
  int16_t val = (mem & 0xFFFC0000) >> 18;
  if (val & 0x2000) { val |= 0xC000; } // Pad out 2's complement
  const float tSense(float(val) * 0.25f);
  ESP_LOGD(TAG, "Got thermocouple temperature: %.2f°C", tSense);
  this->publish_state(tSense);
  this->status_clear_warning();
}

}  // namespace max31855
}  // namespace esphome
