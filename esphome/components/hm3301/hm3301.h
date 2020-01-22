#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

#include <Seeed_HM330X.h>

namespace esphome {
namespace hm3301 {

class HM3301Component : public PollingComponent, public i2c::I2CDevice {
 public:
  HM3301Component() = default;

  void set_pm_1_0_sensor(sensor::Sensor *pm_1_0_sensor) { pm_1_0_sensor_ = pm_1_0_sensor; }
  void set_pm_2_5_sensor(sensor::Sensor *pm_2_5_sensor) { pm_2_5_sensor_ = pm_2_5_sensor; }
  void set_pm_10_0_sensor(sensor::Sensor *pm_10_0_sensor) { pm_10_0_sensor_ = pm_10_0_sensor; }

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void update() override;

 protected:
  HM330X *hm3301_;

  HM330XErrorCode error_code_{NO_ERROR};

  uint8_t data_buffer_[30];

  sensor::Sensor *pm_1_0_sensor_{nullptr};
  sensor::Sensor *pm_2_5_sensor_{nullptr};
  sensor::Sensor *pm_10_0_sensor_{nullptr};

  bool read_sensor_value_(uint8_t*);
  bool validate_checksum_(const uint8_t*);
  uint16_t get_sensor_value_(const uint8_t*, uint8_t);
};

}  // namespace hm3301
}  // namespace esphome
