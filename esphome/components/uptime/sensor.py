import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, UNIT_SECOND, ICON_TIMER

uptime_ns = cg.esphome_ns.namespace('uptime')
UptimeSensor = uptime_ns.class_('UptimeSensor', sensor.PollingSensorComponent)

CONFIG_SCHEMA = cv.nameable(sensor.sensor_schema(UNIT_SECOND, ICON_TIMER, 0).extend({
    cv.GenerateID(): cv.declare_variable_id(UptimeSensor),
}).extend(cv.polling_component_schema('60s')))


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield sensor.register_sensor(var, config)
