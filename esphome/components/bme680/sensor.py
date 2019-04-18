import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core
from esphome.components import i2c, sensor
from esphome.const import CONF_DURATION, CONF_GAS_RESISTANCE, CONF_HEATER, \
    CONF_HUMIDITY, CONF_ID, CONF_IIR_FILTER, CONF_OVERSAMPLING, CONF_PRESSURE, \
    CONF_TEMPERATURE, UNIT_OHM, ICON_GAS_CYLINDER, UNIT_CELSIUS, \
    ICON_THERMOMETER, UNIT_HECTOPASCAL, ICON_GAUGE, ICON_WATER_PERCENT, UNIT_PERCENT

DEPENDENCIES = ['i2c']

bme680_ns = cg.esphome_ns.namespace('bme680')
BME680Oversampling = bme680_ns.enum('BME680Oversampling')
OVERSAMPLING_OPTIONS = {
    'NONE': BME680Oversampling.BME680_OVERSAMPLING_NONE,
    '1X': BME680Oversampling.BME680_OVERSAMPLING_1X,
    '2X': BME680Oversampling.BME680_OVERSAMPLING_2X,
    '4X': BME680Oversampling.BME680_OVERSAMPLING_4X,
    '8X': BME680Oversampling.BME680_OVERSAMPLING_8X,
    '16X': BME680Oversampling.BME680_OVERSAMPLING_16X,
}

BME680IIRFilter = bme680_ns.enum('BME680IIRFilter')
IIR_FILTER_OPTIONS = {
    'OFF': BME680IIRFilter.BME680_IIR_FILTER_OFF,
    '1X': BME680IIRFilter.BME680_IIR_FILTER_1X,
    '3X': BME680IIRFilter.BME680_IIR_FILTER_3X,
    '7X': BME680IIRFilter.BME680_IIR_FILTER_7X,
    '15X': BME680IIRFilter.BME680_IIR_FILTER_15X,
    '31X': BME680IIRFilter.BME680_IIR_FILTER_31X,
    '63X': BME680IIRFilter.BME680_IIR_FILTER_63X,
    '127X': BME680IIRFilter.BME680_IIR_FILTER_127X,
}

BME680Component = bme680_ns.class_('BME680Component', cg.PollingComponent, i2c.I2CDevice)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_variable_id(BME680Component),
    cv.Optional(CONF_TEMPERATURE):
        sensor.sensor_schema(UNIT_CELSIUS, ICON_THERMOMETER, 1).extend({
            cv.Optional(CONF_OVERSAMPLING, default='16X'):
                cv.one_of(*OVERSAMPLING_OPTIONS, upper=True),
        }),
    cv.Optional(CONF_PRESSURE):
        sensor.sensor_schema(UNIT_HECTOPASCAL, ICON_GAUGE, 1).extend({
            cv.Optional(CONF_OVERSAMPLING, default='16X'):
                cv.one_of(*OVERSAMPLING_OPTIONS, upper=True),
        }),
    cv.Optional(CONF_HUMIDITY):
        sensor.sensor_schema(UNIT_PERCENT, ICON_WATER_PERCENT, 1).extend({
            cv.Optional(CONF_OVERSAMPLING, default='16X'):
                cv.one_of(*OVERSAMPLING_OPTIONS, upper=True),
        }),
    cv.Optional(CONF_GAS_RESISTANCE):
        sensor.sensor_schema(UNIT_OHM, ICON_GAS_CYLINDER, 1),
    cv.Optional(CONF_IIR_FILTER, default='OFF'): cv.one_of(*IIR_FILTER_OPTIONS, upper=True),
    cv.Optional(CONF_HEATER): cv.Any(None, cv.All(cv.Schema({
        cv.Optional(CONF_TEMPERATURE, default=320): cv.All(cv.Coerce(int), cv.Range(200, 400)),
        cv.Optional(CONF_DURATION, default='150ms'): cv.All(
            cv.positive_time_period_milliseconds, cv.Range(max=core.TimePeriod(milliseconds=4032)))
    }, cv.has_at_least_one_key(CONF_TEMPERATURE, CONF_DURATION)))),
}).extend(cv.polling_component_schema('60s')).extend(i2c.i2c_device_schema(0x76))


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield i2c.register_i2c_device(var, config)

    if CONF_TEMPERATURE in config:
        conf = config[CONF_TEMPERATURE]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_temperature_sensor(sens))
        cg.add(var.set_temperature_oversampling(OVERSAMPLING_OPTIONS[conf[CONF_OVERSAMPLING]]))

    if CONF_PRESSURE in config:
        conf = config[CONF_PRESSURE]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_pressure_sensor(sens))
        cg.add(var.set_pressure_oversampling(OVERSAMPLING_OPTIONS[conf[CONF_OVERSAMPLING]]))

    if CONF_HUMIDITY in config:
        conf = config[CONF_HUMIDITY]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_humidity_sensor(sens))
        cg.add(var.set_humidity_oversampling(OVERSAMPLING_OPTIONS[conf[CONF_OVERSAMPLING]]))

    if CONF_GAS_RESISTANCE in config:
        conf = config[CONF_GAS_RESISTANCE]
        sens = yield sensor.new_sensor(conf)
        cg.add(var.set_gas_resistance_sensor(sens))

    cg.add(var.set_iir_filter(IIR_FILTER_OPTIONS[config[CONF_IIR_FILTER]]))

    if CONF_HEATER in config:
        conf = config[CONF_HEATER]
        if not conf:
            cg.add(var.set_heater(0, 0))
        else:
            cg.add(var.set_heater(conf[CONF_TEMPERATURE], conf[CONF_DURATION]))
