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
#include "bb_epaperbase.h"
#include <SPI.h>
#include <bitset>
#include <cinttypes>
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

// Names of the pre-configured embedded devices with built-in e-paper displays
const char *szModels[] = {
"BADGER2040", "LILYGO_S3_MINI", "TRMNL_OG",
"CROWPANEL29", "CROWPANEL213", "CROWPANEL42", "CROWPANEL37",
"CROWPANEL154", "RETERMINAL_SPECTRA", "LILYGO_T_DECK_PRO",
"WAVESHARE_154", "XTEINK_X4", "WAVESHARE_397",
nullptr};
// need to have an array with the indices since they are not in order
const uint8_t u8Models[] = {
EPD_BADGER2040, EPD_LILYGO_S3_MINI, EPD_TRMNL_OG,
EPD_CROWPANEL29, EPD_CROWPANEL213, EPD_CROWPANEL42, EPD_CROWPANEL37,
EPD_CROWPANEL154, EPD_RETERMINAL_SPECTRA, EPD_LILYGO_T_DECK_PRO,
EPD_WAVESHARE_154, EPD_XTEINK_X4, EPD_WAVESHARE_397};

// Screen update/refresh options
const char *szUpdates[] = {"full", "fast", "partial"};
const int iUpdates[] = {REFRESH_FULL, REFRESH_FAST, REFRESH_PARTIAL};

namespace esphome {
namespace bb_epaper_iot {

bb_epaper_iot *pThis;

static const char *const TAG = "bb_epaper_iot";

int bb_epaper_iot::get_height_internal() {
//  ESP_LOGCONFIG(TAG, "get_height_internal() %d", _bbepaper._bbep.native_height);
  return _bbepaper._bbep.native_height;
}
int bb_epaper_iot::get_width_internal() {
//  ESP_LOGCONFIG(TAG, "get_width_internal() %d", _bbepaper._bbep.native_width);
  return _bbepaper._bbep.native_width;
}

void spi_write(const uint8_t *pData, int iLen) {
    SPI.transferBytes(pData, NULL, iLen);
} /* spi_write() */

void set_gpio(uint8_t pin, uint8_t value) {
    digitalWrite(pin, value);
}
uint8_t get_gpio(uint8_t pin) {
  return (uint8_t)digitalRead(pin);
} /* get_gpio() */

void bb_epaper_iot::setup() {
int i, iModel=-1;

  iCount = 0; // number of update counts
  LOG_DISPLAY("setup:", "bb_epaper_iot", this);
  // See if the specified model defines an IoT product or raw panel
  i = 0;
  while (szModels[i] != nullptr) {
      if (strcasecmp(szModels[i], model_name.c_str()) == 0) {
          iModel = i;
          break;
      }
      i++;
  } // while
  this->_refresh = REFRESH_FULL; // assume full refresh unless told otherwise
  if (refresh_type.length()) {
      i = 0;
      while (szUpdates[i] != nullptr) {
          if (strcasecmp(szUpdates[i], refresh_type.c_str()) == 0) {
              this->_refresh = iUpdates[i];
              break;
          }
          i++;
      } // while
  }
  if (iModel >= 0) { // found a matching model
      _bbepaper.setWritefn(spi_write);
      _bbepaper.setSetGPIOfn(set_gpio);
      _bbepaper.setGetGPIOfn(get_gpio);
      _bbepaper.begin(u8Models[iModel]); // initialize this display
      if (_bbepaper.allocBuffer() != BBEP_SUCCESS) {
          ESP_LOGCONFIG(TAG, "allocBuffer() failed!");
      }
      LOG_DISPLAY("setup:", "bb_epaper", this);
  } else { // search for raw panels
      ESP_LOGCONFIG(TAG, "setup error! no matching panel for %d", model_name.c_str());
      return;
  }
  pThis = this;
  _bbepaper.fillScreen(BBEP_WHITE);
  _bbepaper.writePlane(PLANE_DUPLICATE);
  _bbepaper.refresh(REFRESH_FULL);
} /* setup() */

float bb_epaper_iot::get_setup_priority() const { return setup_priority::PROCESSOR; }

void bb_epaper_iot::dump_config() {
  LOG_DISPLAY("dump_config()", "bb_epaper", this);
  ESP_LOGCONFIG(TAG, "  Model: %d", this->_bbepaper.getPanelType());
//  ESP_LOGCONFIG(TAG, "  Full Update Every: %" PRIu32, this->full_update_every_);
  LOG_UPDATE_INTERVAL(this);
}

void bb_epaper_iot::display() {
    if (_refresh == REFRESH_PARTIAL && (iCount & 15) == 0) { // do a fast update every 16 
        _bbepaper.writePlane();
        _bbepaper.refresh(REFRESH_FAST);
    } else { // do the users' choice of refresh type
        if (_refresh == REFRESH_PARTIAL) {
            _bbepaper.writePlane(PLANE_FALSE_DIFF);
        } else {
            _bbepaper.writePlane(); // default type
        }     
        _bbepaper.refresh(_refresh);
    }
    iCount++;
    _bbepaper.sleep(LIGHT_SLEEP);
}

void bb_epaper_iot::update() {
  LOG_DISPLAY("update()", "bb_epaper", this);
  this->do_update_();
  this->display();
}

void bb_epaper_iot::fill(Color color) {
  const uint8_t fill_color = color.is_on() ? BBEP_BLACK : BBEP_WHITE;
ESP_LOGCONFIG(TAG, "fill with %d", fill_color);
  _bbepaper.fillScreen(fill_color);
} /* fill() */

void HOT bb_epaper_iot::draw_absolute_pixel_internal(int x, int y, Color color) {
//  ESP_LOGCONFIG(TAG, "draw pixel at %d,%d, color=%d", x, y, color.is_on());
  _bbepaper.drawPixel(x, y, (color.is_on() ? BBEP_BLACK : BBEP_WHITE));
} /* draw_absolute_pixel_internal() */

void bb_epaper_iot::on_safe_shutdown() { this->deep_sleep(); }

}  // namespace bb_epaper_iot
}  // namespace esphome
