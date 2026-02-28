#include "bb_epaperbase.h"
#include <bitset>
#include <cinttypes>
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace bb_epaper {

bb_epaper *pThis;

static const char *const TAG = "bb_epaper";

int bb_epaper::get_height_internal() {
//  ESP_LOGCONFIG(TAG, "get_height_internal() %d", _bbepaper._bbep.native_height);
  return _bbepaper._bbep.native_height;
}
int bb_epaper::get_width_internal() {
//  ESP_LOGCONFIG(TAG, "get_width_internal() %d", _bbepaper._bbep.native_width);
  return _bbepaper._bbep.native_width;
}

void spi_write(const uint8_t *pData, int iLen) {
//ESP_LOGCONFIG(TAG, "Writing %d bytes to SPI", iLen); 
  pThis->write_array(pData, iLen);
} /* spi_write() */

void set_gpio(uint8_t pin, uint8_t value) {
  // Figure out which pin because ESPHome uses a pointer to a GPIO structure
  if (pin == pThis->_bbepaper._bbep.iDCPin) {
    pThis->dc_pin_->digital_write(value);
  } else if (pin == pThis->_bbepaper._bbep.iCSPin) {
    pThis->cs_pin_->digital_write(value);
//    if (value) {
//      pThis->disable();  // SPI CS
//    } else {
//      pThis->enable();
//    }
  } else if (pin == pThis->_bbepaper._bbep.iRSTPin) {
    pThis->reset_pin_->digital_write(value);
  }
} /* set_gpio() */

uint8_t get_gpio(uint8_t pin) {
  // This is only used for the BUSY pin, so hard code it
  (void) pin;
  return (uint8_t) pThis->busy_pin_->digital_read();
} /* get_gpio() */

void bb_epaper::setup() {
  LOG_DISPLAY("setup:", "bb_epaper", this);
  this->setup_pins_();
  this->spi_setup();
  this->reset_();
  pThis = this;
  // Set up the function pointers to allow bb_epaper to access SPI+GPIO
  _bbepaper.setWritefn(spi_write);
  _bbepaper.setSetGPIOfn(set_gpio);
  _bbepaper.setGetGPIOfn(get_gpio);
  _bbepaper._bbep.iSpeed = 1; // Tell it not to use Bit Bang for SPI
  _bbepaper.setPanelType(EP154_200x200); // debug
  _bbepaper.allocBuffer();
  _bbepaper.fillScreen(BBEP_WHITE);
  _bbepaper.writePlane(PLANE_DUPLICATE);
//  _bbepaper.begin(EPD_WAVESHARE_154);
} /* setup() */

void bb_epaper::setup_pins_() {
  LOG_DISPLAY("setup_pins:", "bb_epaper", this);
  this->_bbepaper._bbep.iRSTPin = this->reset_pin_->get_pin();
  this->_bbepaper._bbep.iDCPin = this->dc_pin_->get_pin();
  this->_bbepaper._bbep.iCSPin = this->cs_pin_->get_pin();
  this->_bbepaper._bbep.iBUSYPin = this->busy_pin_->get_pin();
  ESP_LOGCONFIG(TAG, "Pins - CS:%d DC:%d RST:%d BUSY:%d", this->cs_pin_->get_pin(), this->dc_pin_->get_pin(), this->reset_pin_->get_pin(), this->busy_pin_->get_pin());
  pinMode(this->cs_pin_->get_pin(), OUTPUT);
  this->cs_pin_->digital_write(true);
  pinMode(this->dc_pin_->get_pin(), OUTPUT);
  this->dc_pin_->digital_write(false);
  if (this->reset_pin_ != nullptr) {
    pinMode(this->reset_pin_->get_pin(), OUTPUT);
    this->reset_pin_->digital_write(true);
  }
  if (this->busy_pin_ != nullptr) {
    pinMode(this->busy_pin_->get_pin(), INPUT);
  }
}
float bb_epaper::get_setup_priority() const { return setup_priority::PROCESSOR; }

void bb_epaper::dump_config() {
  LOG_DISPLAY("dump_config()", "bb_epaper", this);
  ESP_LOGCONFIG(TAG, "  Model: %d", this->_bbepaper.getPanelType());
//  ESP_LOGCONFIG(TAG, "  Full Update Every: %" PRIu32, this->full_update_every_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  LOG_PIN("  DC Pin: ", this->dc_pin_);
  LOG_PIN("  Busy Pin: ", this->busy_pin_);
  LOG_UPDATE_INTERVAL(this);
}

void bb_epaper::update() {
  LOG_DISPLAY("update()", "bb_epaper", this);
  _bbepaper.writePlane();
  _bbepaper.refresh(REFRESH_FULL);
}

void bb_epaper::fill(Color color) {
  const uint8_t fill_color = color.is_on() ? BBEP_BLACK : BBEP_WHITE;
ESP_LOGCONFIG(TAG, "fill with %d", fill_color);
  _bbepaper.fillScreen(fill_color);
} /* fill() */

void HOT bb_epaper::draw_absolute_pixel_internal(int x, int y, Color color) {
  ESP_LOGCONFIG(TAG, "draw pixel at %d,%d", x, y);
  _bbepaper.drawPixel(x, y, (color.is_on() ? BBEP_BLACK : BBEP_WHITE));
} /* draw_absolute_pixel_internal() */

void bb_epaper::on_safe_shutdown() { this->deep_sleep(); }

}  // namespace bb_epaper
}  // namespace esphome
