import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_HUMIDITY, CONF_ID, CONF_MODEL, CONF_PIN, CONF_TEMPERATURE, \
    CONF_UPDATE_INTERVAL, ICON_THERMOMETER, UNIT_CELSIUS, ICON_WATER_PERCENT, UNIT_PERCENT
from esphome.cpp_helpers import gpio_pin_expression
from esphome.pins import gpio_input_pullup_pin_schema

dht_ns = cg.esphome_ns.namespace('dht')
DHTModel = dht_ns.enum('DHTModel')
DHT_MODELS = {
    'AUTO_DETECT': DHTModel.DHT_MODEL_AUTO_DETECT,
    'DHT11': DHTModel.DHT_MODEL_DHT11,
    'DHT22': DHTModel.DHT_MODEL_DHT22,
    'AM2302': DHTModel.DHT_MODEL_AM2302,
    'RHT03': DHTModel.DHT_MODEL_RHT03,
    'SI7021': DHTModel.DHT_MODEL_SI7021,
}
DHT = dht_ns.class_('DHT', cg.PollingComponent)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_variable_id(DHT),
    cv.Required(CONF_PIN): gpio_input_pullup_pin_schema,
    cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1),
    cv.Optional(CONF_HUMIDITY): sensor.sensor_schema(UNIT_PERCENT, ICON_WATER_PERCENT, 0),
    cv.Optional(CONF_MODEL, default='auto detect'): cv.one_of(*DHT_MODELS, upper=True, space='_'),
    cv.Optional(CONF_UPDATE_INTERVAL, default='60s'): cv.update_interval,
}).extend(cv.polling_component_schema('60s'))


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)

    pin = yield gpio_pin_expression(config[CONF_PIN])
    cg.add(var.set_pin(pin))

    if CONF_TEMPERATURE in config:
        sens = yield sensor.new_sensor(config[CONF_TEMPERATURE])
        cg.add(var.set_temperature_sensor(sens))
    if CONF_HUMIDITY in config:
        sens = yield sensor.new_sensor(config[CONF_HUMIDITY])
        cg.add(var.set_humidity_sensor(sens))

    cg.add(var.set_dht_model(DHT_MODELS[config[CONF_MODEL]]))
