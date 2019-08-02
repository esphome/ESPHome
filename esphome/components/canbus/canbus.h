#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/optional.h"

namespace esphome {
namespace canbus {

/* CAN payload length and DLC definitions according to ISO 11898-1 */
static const uint8_t CAN_MAX_DLC = 8;
static const uint8_t CAN_MAX_DLEN = 8;

struct can_frame {
  uint32_t can_id; /* 32 bit CAN_ID + EFF/RTR/ERR flags */
  uint8_t can_dlc; /* frame payload length in byte (0 .. CAN_MAX_DLEN) */
  uint8_t data[CAN_MAX_DLEN] __attribute__((aligned(8)));
};
enum CAN_SPEED : uint8_t {
  CAN_5KBPS,
  CAN_10KBPS,
  CAN_20KBPS,
  CAN_31K25BPS,
  CAN_33KBPS,
  CAN_40KBPS,
  CAN_50KBPS,
  CAN_80KBPS,
  CAN_83K3BPS,
  CAN_95KBPS,
  CAN_100KBPS,
  CAN_125KBPS,
  CAN_200KBPS,
  CAN_250KBPS,
  CAN_500KBPS,
  CAN_1000KBPS
};

enum ERROR : uint8_t {
  ERROR_OK = 0,
  ERROR_FAIL = 1,
  ERROR_ALLTXBUSY = 2,
  ERROR_FAILINIT = 3,
  ERROR_FAILTX = 4,
  ERROR_NOMSG = 5
};
/* special address description flags for the CAN_ID */
static const uint32_t CAN_EFF_FLAG = 0x80000000UL; /* EFF/SFF is set in the MSB */
static const uint32_t CAN_RTR_FLAG = 0x40000000UL; /* remote transmission request */
static const uint32_t CAN_ERR_FLAG = 0x20000000UL; /* error message frame */

/* valid bits in CAN ID for frame formats */
static const uint32_t CAN_SFF_MASK = 0x000007FFUL; /* standard frame format (SFF) */
static const uint32_t CAN_EFF_MASK = 0x1FFFFFFFUL; /* extended frame format (EFF) */
static const uint32_t CAN_ERR_MASK = 0x1FFFFFFFUL; /* omit EFF, RTR, ERR flags */

class Canbus : public Component {
 public:
  Canbus(){};
  Canbus(const std::string &name){};
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }
  void loop() override;

  void send_data(uint32_t can_id, const std::vector<uint8_t> data);
  void set_sender_id(int sender_id) { this->sender_id_ = sender_id; }
  void set_bitrate(CAN_SPEED bit_rate) { this->bit_rate_ = bit_rate; }

 protected:
  uint32_t sender_id_{0};
  CAN_SPEED bit_rate_{CAN_125KBPS};

  virtual bool setup_internal_();
  virtual ERROR send_message_(const struct can_frame *frame);
};

class CanbusTrigger : public Trigger<std::uint32_t>, public Component {
 public:
  explicit CanbusTrigger(const std::uint32_t &can_id);

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;

 protected:
  uint32_t can_id_;
};

}  // namespace canbus
}  // namespace esphome