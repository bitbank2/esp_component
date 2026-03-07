//
// bb_epaper wrapper library for ESPHome
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2026 Larry Bank <bitbank@pobox.com>
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//===========================================================================
//
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
  void set_refresh_type(std::string type) {refresh_type = type;}
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
  std::string model_name, refresh_type;

 protected:
  int get_height_internal() override;
  int get_width_internal() override;
  void display();
  int _refresh; // refresh type
  int iCount;

  virtual int get_width_controller() { return this->get_width_internal(); };

  virtual uint32_t idle_timeout_() { return 1000u; }  // NOLINT(readability-identifier-naming)

  void draw_absolute_pixel_internal(int x, int y, Color color) override;
};  // class

}  // namespace bb_epaper_iot
}  // namespace esphome
