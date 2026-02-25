#include "bb_epaperbase.h"
#include <bitset>
#include <cinttypes>
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace epaper {

BB_EPaperBase *pThis;

static const char *const TAG = "bb_epaper";

void spi_write(const uint8_t *pData, int iLen) { pThis->write_array(pData, iLen); } /* spi_write() */

void set_gpio(uint8_t pin, uint8_t value) {
  // ESPHome uses a pointer to a GPIO structure, so fudge it for now
  if (pin == pThis->_bbepaper._bbep.iDCPin) {
    pThis->dc_pin_->digital_write(value);
  } else if (pin == pThis->_bbepaper._bbep.iCSPin) {
    if (value) {
      pThis->disable();  // SPI CS
    } else {
      pThis->enable();
    }
  } else if (pin == pThis->_bbepaper._bbep.iRSTPin) {
    pThis->reset_pin_->digital_write(value);
  }
} /* set_gpio() */

uint8_t get_gpio(uint8_t pin) {
  // This is only used for the BUSY pin, so hard code it
  (void) pin;
  return (uint8_t) pThis->busy_pin_->digital_read();
} /* get_gpio() */

void BB_EPaperBase::setup() {
  this->setup_pins_();
  this->spi_setup();
  this->reset_();
  pThis = this;
  // Set up the function pointers to allow bb_epaper to access SPI+GPIO
  _bbepaper.setWritefn(spi_write);
  _bbepaper.setSetGPIOfn(set_gpio);
  _bbepaper.setGetGPIOfn(get_gpio);
  _bbepaper.begin(EPD_WAVESHARE_154);
  _bbepaper.allocBuffer();
} /* setup() */

void BB_EPaperBase::setup_pins_() {
  this->dc_pin_->setup();  // OUTPUT
  this->dc_pin_->digital_write(false);
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();  // OUTPUT
    this->reset_pin_->digital_write(true);
  }
  if (this->busy_pin_ != nullptr) {
    this->busy_pin_->setup();  // INPUT
  }
}
float BB_EPaperBase::get_setup_priority() const { return setup_priority::PROCESSOR; }

void BB_EPaperBase::update() {
  _bbepaper.writePlane();
  _bbepaper.refresh(REFRESH_FULL);
}
void BB_EPaperBase::fill(Color color) {
  // flip logic
  const uint8_t fill = color.is_on() ? BBEP_BLACK : BBEP_WHITE;
  _bbepaper.fillScreen(fill);
} /* fill() */

void BB_EPaperBase::draw_absolute_pixel_internal(int x, int y, Color color) {
  _bbepaper.drawPixel(x, y, (color.is_on() ? BBEP_BLACK : BBEP_WHITE));
} /* draw_absolute_pixel_internal() */
void BB_EPaperBase::on_safe_shutdown() { this->deep_sleep(); }

}  // namespace bb_epaperbase
}  // namespace esphome
