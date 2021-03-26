#include "esp32_can.h"
#include "esphome/core/log.h"

#include <driver/can.h>

// WORKAROUND, because CAN_IO_UNUSED is just defined as (-1) in this version
// of the framework which does not work with -fpermissive
#undef CAN_IO_UNUSED
#define CAN_IO_UNUSED ((gpio_num_t) -1)

namespace esphome {
namespace esp32_can {

static const char *TAG = "esp32_can";

static bool get_bitrate(canbus::CanSpeed bitrate, can_timing_config_t *t_config) {
  switch (bitrate) {
    case canbus::CAN_50KBPS:
      *t_config = (can_timing_config_t) CAN_TIMING_CONFIG_50KBITS();
      return true;
    case canbus::CAN_100KBPS:
      *t_config = (can_timing_config_t) CAN_TIMING_CONFIG_100KBITS();
      return true;
    case canbus::CAN_125KBPS:
      *t_config = (can_timing_config_t) CAN_TIMING_CONFIG_125KBITS();
      return true;
    case canbus::CAN_250KBPS:
      *t_config = (can_timing_config_t) CAN_TIMING_CONFIG_250KBITS();
      return true;
    case canbus::CAN_500KBPS:
      *t_config = (can_timing_config_t) CAN_TIMING_CONFIG_500KBITS();
      return true;
    case canbus::CAN_1000KBPS:
      *t_config = (can_timing_config_t) CAN_TIMING_CONFIG_1MBITS();
      return true;
    default:
      return false;
  }
}

bool ESP32Can::setup_internal() {
  can_general_config_t g_config =
      CAN_GENERAL_CONFIG_DEFAULT((gpio_num_t) this->tx_->get_pin(), (gpio_num_t) this->rx_->get_pin(), CAN_MODE_NORMAL);
  can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();
  can_timing_config_t t_config;

  if (!get_bitrate(this->bit_rate_, &t_config)) {
    // invalid bit rate
    this->mark_failed();
    return false;
  }

  // Install CAN driver
  if (can_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    // Failed to install driver
    this->mark_failed();
    return false;
  }

  // Start CAN driver
  if (can_start() != ESP_OK) {
    // Failed to start driver
    this->mark_failed();
    return false;
  }
  return true;
}

canbus::Error ESP32Can::send_message(struct canbus::CanFrame *frame) {
  if (frame->can_data_length_code > canbus::CAN_MAX_DATA_LENGTH) {
    return canbus::ERROR_FAILTX;
  }

  uint32_t flags = CAN_MSG_FLAG_NONE;
  if (frame->use_extended_id) {
    flags |= CAN_MSG_FLAG_EXTD;
  }
  if (frame->remote_transmission_request) {
    flags |= CAN_MSG_FLAG_RTR;
  }

  can_message_t message = {
      .flags = flags,
      .identifier = frame->can_id,
      .data_length_code = frame->can_data_length_code,
  };
  if (!frame->remote_transmission_request) {
    memcpy(message.data, frame->data, frame->can_data_length_code);
  }

  if (can_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
    return canbus::ERROR_OK;
  } else {
    return canbus::ERROR_ALLTXBUSY;
  }
}

canbus::Error ESP32Can::read_message(struct canbus::CanFrame *frame) {
  canbus::Error rc;
  can_message_t message;

  if (can_receive(&message, 0) != ESP_OK) {
    return canbus::ERROR_NOMSG;
  }

  frame->can_id = message.identifier;
  frame->use_extended_id = message.flags & CAN_MSG_FLAG_EXTD;
  frame->remote_transmission_request = message.flags & CAN_MSG_FLAG_RTR;
  frame->can_data_length_code = message.data_length_code;

  if (!frame->remote_transmission_request) {
    size_t dlc =
        message.data_length_code < canbus::CAN_MAX_DATA_LENGTH ? message.data_length_code : canbus::CAN_MAX_DATA_LENGTH;
    memcpy(frame->data, message.data, dlc);
  }

  return canbus::ERROR_OK;
}

}  // namespace esp32_can
}  // namespace esphome
