#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "bb_epaper.h"

namespace esphome {
namespace bb_epaper_iot {

class bb_epaper_iot : public display::DisplayBuffer {

 public:
  bb_epaper_iot(void) { ESP_LOGCONFIG("bb_epaper_iot class", "instantiation");}
  void set_model(std::string model) {model_name = model;}
  float get_setup_priority() const override;

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
  std::string model_name;

 protected:
  int get_height_internal() override;
  int get_width_internal() override;
  void display();

  virtual int get_width_controller() { return this->get_width_internal(); };

  virtual uint32_t idle_timeout_() { return 1000u; }  // NOLINT(readability-identifier-naming)

  void draw_absolute_pixel_internal(int x, int y, Color color) override;
};  // class

}  // namespace bb_epaper_iot
}  // namespace esphome
