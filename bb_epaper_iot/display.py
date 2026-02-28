from esphome import core
import esphome.codegen as cg
from esphome.components import display
import esphome.config_validation as cv
from esphome.const import (
    CONF_FULL_UPDATE_EVERY,
    CONF_ID,
    CONF_LAMBDA,
    CONF_MODEL,
    CONF_PAGES,
)

epaper_ns = cg.esphome_ns.namespace("bb_epaper_iot")

bb_epaper_iot = epaper_ns.class_(
    "bb_epaper_iot", cg.PollingComponent, display.DisplayBuffer
)

CONFIG_SCHEMA = cv.Schema(
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(bb_epaper_iot),
            cv.Required(CONF_MODEL): cv.string,
            cv.Optional(CONF_FULL_UPDATE_EVERY): cv.int_range(min=1, max=4294967295),
        }
    )
    .extend(cv.polling_component_schema("30s")),
    cv.has_at_most_one_key(CONF_PAGES, CONF_LAMBDA),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await display.register_display(var, config)

    cg.add(var.set_model(config[CONF_MODEL]))
    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(display.DisplayRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))
    if CONF_FULL_UPDATE_EVERY in config:
        cg.add(var.set_full_update_every(config[CONF_FULL_UPDATE_EVERY]))
    cg.add_library(
        name="bb_epaper",
        repository="https://github.com/bitbank2/bb_epaper.git",
        version=None,
    )
