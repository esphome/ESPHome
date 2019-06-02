#pragma once

#include "sx1509.h"

#define BREATHE_OUTPUT 0x04
#define BLINK_OUTPUT 0x05

#define LINEAR		0	
#define LOGARITHMIC	1

namespace esphome {
namespace sx1509 {

/// PinModes for SX1509 pins
enum SX1509GPIOMode : uint8_t {
  SX1509_INPUT = INPUT,                    // 0x00
  SX1509_INPUT_PULLUP = INPUT_PULLUP,      // 0x02
  SX1509_OUTPUT = OUTPUT,                  // 0x01
  SX1509_BREATHE_OUTPUT = BREATHE_OUTPUT,  // 0x04
  SX1509_BLINK_OUTPUT = BLINK_OUTPUT       // 0x05
};
/// PinModes for SX1509 pins
enum SX1509FadingMode : uint8_t {
  SX1509_FADING_LINEAR = LINEAR,      // 0x00
  SX1509_FADING_LOGARITHMIC = LOGARITHMIC   // 0x01
};

class SX1509Component;

class SX1509GPIOPin : public GPIOPin {
 public:
  SX1509GPIOPin(SX1509Component *parent, uint8_t pin, uint8_t mode, bool inverted = false, uint16_t t_on = 500,
                uint16_t t_off = 500, uint16_t t_rise = 250, uint16_t t_fall = 250, uint8_t fading_mode = SX1509_FADING_LINEAR);
  void setup() override;
  void pin_mode(uint8_t mode) override;
  bool digital_read() override;
  void digital_write(bool value) override;

 protected:
  SX1509Component *parent_;
  uint16_t t_on_ = {0};
  uint16_t t_off_ = {0};
  uint16_t t_rise_ = {0};
  uint16_t t_fall_ = {0};
  uint8_t fading_mode_ = {SX1509_FADING_LINEAR};
};

}  // namespace sx1509
}  // namespace esphome