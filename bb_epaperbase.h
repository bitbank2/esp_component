#pragma once

#include "esphome/core/component.h"
#include "esphome/components/spi/spi.h"
#include "esphome/components/display/display_buffer.h"
#include "bb_epaper.h"

namespace esphome {
namespace bb_epaper {

class bb_epaper : public display::DisplayBuffer,
                      public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW, spi::CLOCK_PHASE_LEADING,
                                            spi::DATA_RATE_8MHZ> {
 public:
  bb_epaper(void) { ESP_LOGCONFIG("bb_epaper class", "instantiation");}
  void set_dc_pin(InternalGPIOPin *dc_pin) { dc_pin_ = dc_pin; }
  void set_cs_pin(InternalGPIOPin *cs_pin) { cs_pin_ = cs_pin; }
  void set_power_pin(InternalGPIOPin *power_pin) { power_pin_ = power_pin; }
  float get_setup_priority() const override;
  void set_reset_pin(InternalGPIOPin *reset) { this->reset_pin_ = reset; }
  void set_busy_pin(InternalGPIOPin *busy) { this->busy_pin_ = busy; }
  void set_reset_duration(uint32_t reset_duration) { this->reset_duration_ = reset_duration; }

  void command(uint8_t value);
  void data(uint8_t value);
  void cmd_data(const uint8_t *data, size_t length);

  void fill(Color color) override;

  void update() override;

  void setup() override;

  void on_safe_shutdown() override;
  void dump_config() override;

  void deep_sleep() { _bbepaper.sleep(LIGHT_SLEEP); }

  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_BINARY; }
  BBEPAPER _bbepaper;
  InternalGPIOPin *reset_pin_{nullptr};
  InternalGPIOPin *dc_pin_;
  InternalGPIOPin *cs_pin_;
  InternalGPIOPin *power_pin_;
  InternalGPIOPin *busy_pin_{nullptr};

 protected:
  int get_height_internal() override;
  int get_width_internal() override;
  void setup_pins_();

  void reset_() {
    if (this->reset_pin_ != nullptr) {
      this->reset_pin_->digital_write(false);
      delay(reset_duration_);  // NOLINT
      this->reset_pin_->digital_write(true);
      delay(20);
    }
  }

  virtual int get_width_controller() { return this->get_width_internal(); };

  uint32_t reset_duration_{200};

  virtual uint32_t idle_timeout_() { return 1000u; }  // NOLINT(readability-identifier-naming)

  void draw_absolute_pixel_internal(int x, int y, Color color) override;
};  // class

}  // namespace bb_epaper
}  // namespace esphome
