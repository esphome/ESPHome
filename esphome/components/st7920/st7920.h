#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/components/spi/spi.h"

namespace esphome {
namespace st7920 {

/// Turn the pixel ON.
const Color COLOR_OFF(0, 0, 0, 0);
const Color COLOR_ON(1, 1, 1, 1);

const uint16_t historySize = 90;

class ST7920;

using st7920_writer_t = std::function<void(ST7920 &)>;

class ST7920 : public PollingComponent,
      public display::DisplayBuffer,
      public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_HIGH, spi::CLOCK_PHASE_TRAILING, spi::DATA_RATE_1MHZ> {
 public:
  void set_writer(st7920_writer_t &&writer) { this->writer_local_ = writer; }
  void set_rs_pin(GPIOPin *rs_pin) { this->rs_pin_ = rs_pin; }
  void set_height(uint16_t height) { this->height_ = height; }
  void set_width(uint16_t width) { this->width_ = width; }
  
  // ========== INTERNAL METHODS ==========
  // (In most use cases you won't need these)
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void update() override;
  void loop() override;
  void fill(Color color) override;
  
  void write_display_data();
  
 protected:  
  void displayInit();
  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  int get_height_internal() override;
  int get_width_internal() override;
  size_t get_buffer_length_();

  GPIOPin *rs_pin_;
  int16_t width_ = 128, height_ = 64;
  optional<st7920_writer_t> writer_local_{};
 private:
  void command(uint8_t value);
  void data(uint8_t value);
  void send(uint8_t type, uint8_t value);
  void gotoXY(uint16_t x, uint16_t y);
  void startTransaction();
  void endTransaction();
};


}  // namespace st7920
}  // namespace esphome