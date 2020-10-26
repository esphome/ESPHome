#pragma once

#include "esphome/core/log.h"
#include "esphome/components/climate_ir/climate_ir.h"

namespace esphome {
namespace hitachi_ac344 {

const uint16_t HITACHI_AC344_HDR_MARK = 3300;  // ac
const uint16_t HITACHI_AC344_HDR_SPACE = 1700; // ac
const uint16_t HITACHI_AC344_BIT_MARK = 400;
const uint16_t HITACHI_AC344_ONE_SPACE = 1250;
const uint16_t HITACHI_AC344_ZERO_SPACE = 500;
const uint32_t HITACHI_AC344_MIN_GAP = 100000;  // just a guess.
const uint16_t HITACHI_AC344_FREQ = 38000;  // Hz.

const uint8_t HITACHI_AC344_BUTTON_BYTE = 11;
const uint8_t HITACHI_AC344_BUTTON_POWER = 0x13;
const uint8_t HITACHI_AC344_BUTTON_SLEEP = 0x31;
const uint8_t HITACHI_AC344_BUTTON_MODE = 0x41;
const uint8_t HITACHI_AC344_BUTTON_FAN = 0x42;
const uint8_t HITACHI_AC344_BUTTON_TEMP_DOWN = 0x43;
const uint8_t HITACHI_AC344_BUTTON_TEMP_UP = 0x44;
const uint8_t HITACHI_AC344_BUTTON_SWINGV = 0x81;
const uint8_t HITACHI_AC344_BUTTON_SWINGH = 0x8C;
const uint8_t HITACHI_AC344_BUTTON_MILDEWPROOF = 0xE2;

const uint8_t HITACHI_AC344_TEMP_BYTE = 13;
const uint8_t HITACHI_AC344_TEMP_OFFSET = 2;
const uint8_t HITACHI_AC344_TEMP_SIZE = 6;
const uint8_t HITACHI_AC344_TEMP_MIN = 16;   // 16C
const uint8_t HITACHI_AC344_TEMP_MAX = 32;   // 32C
const uint8_t HITACHI_AC344_TEMP_FAN = 27;   // 27C

const uint8_t HITACHI_AC344_TIMER_BYTE = 15;

const uint8_t HITACHI_AC344_MODE_BYTE = 25;
const uint8_t HITACHI_AC344_MODE_FAN = 1;
const uint8_t HITACHI_AC344_MODE_COOL = 3;
const uint8_t HITACHI_AC344_MODE_DRY = 5;
const uint8_t HITACHI_AC344_MODE_HEAT = 6;
const uint8_t HITACHI_AC344_MODE_AUTO = 7;

const uint8_t HITACHI_AC344_FAN_BYTE = HITACHI_AC344_MODE_BYTE;
const uint8_t HITACHI_AC344_FAN_MIN = 1;
const uint8_t HITACHI_AC344_FAN_LOW = 2;
const uint8_t HITACHI_AC344_FAN_MEDIUM = 3;
const uint8_t HITACHI_AC344_FAN_HIGH = 4;
const uint8_t HITACHI_AC344_FAN_AUTO = 5;
const uint8_t HITACHI_AC344_FAN_MAX = 6;
const uint8_t HITACHI_AC344_FAN_MAX_DRY = 2;

const uint8_t HITACHI_AC344_POWER_BYTE = 27;
const uint8_t HITACHI_AC344_POWER_ON = 0xF1;
const uint8_t HITACHI_AC344_POWER_OFF = 0xE1;

const uint8_t HITACHI_AC344_SWINGH_BYTE = 35;
const uint8_t HITACHI_AC344_SWINGH_OFFSET = 0;  // Mask 0b00000xxx
const uint8_t HITACHI_AC344_SWINGH_SIZE = 3;    // Mask 0b00000xxx
const uint8_t HITACHI_AC344_SWINGH_AUTO = 0;              // 0b000
const uint8_t HITACHI_AC344_SWINGH_RIGHT_MAX = 1;         // 0b001
const uint8_t HITACHI_AC344_SWINGH_RIGHT = 2;             // 0b010
const uint8_t HITACHI_AC344_SWINGH_MIDDLE = 3;            // 0b011
const uint8_t HITACHI_AC344_SWINGH_LEFT = 4;              // 0b100
const uint8_t HITACHI_AC344_SWINGH_LEFT_MAX = 5;          // 0b101

const uint8_t HITACHI_AC344_SWINGV_BYTE = 37;
const uint8_t HITACHI_AC344_SWINGV_OFFSET = 5;  // Mask 0b00x00000

const uint8_t HITACHI_AC344_MILDEWPROOF_BYTE = HITACHI_AC344_SWINGV_BYTE;
const uint8_t HITACHI_AC344_MILDEWPROOF_OFFSET = 2;  // Mask 0b00000x00

const uint16_t HITACHI_AC344_STATE_LENGTH = 43;
const uint16_t HITACHI_AC344_BITS = HITACHI_AC344_STATE_LENGTH * 8;

#define GETBIT8(a, b) (a & ((uint8_t)1 << b))
#define GETBITS8(data, offset, size) \
    (((data) & (((uint8_t)UINT8_MAX >> (8 - (size))) << (offset))) >> (offset))

class HitachiClimate : public climate_ir::ClimateIR {
 public:
  HitachiClimate()
      : climate_ir::ClimateIR(
            HITACHI_AC344_TEMP_MIN, HITACHI_AC344_TEMP_MAX, 1.0f, true, true,
            std::vector<climate::ClimateFanMode>{climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW,
                                                 climate::CLIMATE_FAN_MEDIUM, climate::CLIMATE_FAN_HIGH},
            std::vector<climate::ClimateSwingMode>{climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_HORIZONTAL}) {}

 protected:
  uint8_t remote_state[HITACHI_AC344_STATE_LENGTH]{
                        0x01, 0x10, 0x00, 0x40, 0x00, 0xFF, 0x00, 0xCC, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
                        0x00, 0x01, 0x00, 0x80, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00};
  uint8_t _previous_temp{27};
  // Transmit via IR the state of this climate controller.
  void transmit_state() override;
  bool getPower(void);
  void setPower(const bool on);
  uint8_t getMode(void);
  void setMode(const uint8_t mode);
  void setTemp(const uint8_t celsius, bool setPrevious=false);
  uint8_t getFan(void);
  void setFan(const uint8_t speed);
  void setSwingVToggle(const bool on);
  bool getSwingVToggle(void);
  void setSwingV(const bool on);
  bool getSwingV(void);
  void setSwingH(const uint8_t position);
  uint8_t getSwingH(void);
  uint8_t getButton(void);
  void setButton(const uint8_t button);
  // Handle received IR Buffer
  bool on_receive(remote_base::RemoteReceiveData data) override;
  bool parse_mode_(const uint8_t remote_state[]);
  bool parse_temperature_(const uint8_t remote_state[]);
  bool parse_fan_(const uint8_t remote_state[]);
  bool parse_swing_(const uint8_t remote_state[]);
  bool parse_state_frame_(const uint8_t frame[]);
  void dump_state(const char action[], uint8_t remote_state[]);
};

}  // namespace hitachi_ac344
}  // namespace esphome
