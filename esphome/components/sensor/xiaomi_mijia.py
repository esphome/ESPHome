import voluptuous as vol

from esphome.components import esp32_ble_tracker, sensor
from esphome.components.esp32_ble_tracker import CONF_ESP32_BLE_ID, ESP32BLETracker, \
    make_address_array
import esphome.config_validation as cv
from esphome.const import CONF_BATTERY_LEVEL, CONF_HUMIDITY, CONF_MAC_ADDRESS, CONF_MAKE_ID, \
    CONF_NAME, CONF_TEMPERATURE
from esphome.cpp_generator import Pvariable, get_variable

DEPENDENCIES = ['esp32_ble_tracker']

PLATFORM_SCHEMA = sensor.PLATFORM_SCHEMA.extend({
    cv.GenerateID(CONF_MAKE_ID): cv.declare_variable_id(esp32_ble_tracker.XiaomiDevice),
    cv.GenerateID(CONF_ESP32_BLE_ID): cv.use_variable_id(ESP32BLETracker),
    vol.Required(CONF_MAC_ADDRESS): cv.mac_address,
    vol.Optional(CONF_TEMPERATURE): cv.nameable(esp32_ble_tracker.XIAOMI_SENSOR_SCHEMA),
    vol.Optional(CONF_HUMIDITY): cv.nameable(esp32_ble_tracker.XIAOMI_SENSOR_SCHEMA),
    vol.Optional(CONF_BATTERY_LEVEL): cv.nameable(esp32_ble_tracker.XIAOMI_SENSOR_SCHEMA),
})


def to_code(config):
    for hub in get_variable(config[CONF_ESP32_BLE_ID]):
        yield
    rhs = hub.make_xiaomi_device(make_address_array(config[CONF_MAC_ADDRESS]))
    dev = Pvariable(config[CONF_MAKE_ID], rhs)
    if CONF_TEMPERATURE in config:
        conf = config[CONF_TEMPERATURE]
        sensor.register_sensor(dev.Pmake_temperature_sensor(conf[CONF_NAME]), conf)
    if CONF_HUMIDITY in config:
        conf = config[CONF_HUMIDITY]
        sensor.register_sensor(dev.Pmake_humidity_sensor(conf[CONF_NAME]), conf)
    if CONF_BATTERY_LEVEL in config:
        conf = config[CONF_BATTERY_LEVEL]
        sensor.register_sensor(dev.Pmake_battery_level_sensor(conf[CONF_NAME]), conf)
