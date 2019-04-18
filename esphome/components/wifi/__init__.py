from esphome.automation import CONDITION_REGISTRY, Condition
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_AP, CONF_BSSID, CONF_CHANNEL, CONF_DNS1, CONF_DNS2, CONF_DOMAIN, \
    CONF_FAST_CONNECT, CONF_GATEWAY, CONF_HIDDEN, CONF_ID, CONF_MANUAL_IP, CONF_NETWORKS, \
    CONF_PASSWORD, CONF_POWER_SAVE_MODE, CONF_REBOOT_TIMEOUT, CONF_SSID, CONF_STATIC_IP, \
    CONF_SUBNET, CONF_USE_ADDRESS
from esphome.core import CORE, HexInt, coroutine_with_priority

AUTO_LOAD = ['network']

wifi_ns = cg.esphome_ns.namespace('wifi')
IPAddress = cg.global_ns.class_('IPAddress')
ManualIP = wifi_ns.struct('ManualIP')
WiFiComponent = wifi_ns.class_('WiFiComponent', cg.Component)
WiFiAP = wifi_ns.struct('WiFiAP')

WiFiPowerSaveMode = wifi_ns.enum('WiFiPowerSaveMode')
WIFI_POWER_SAVE_MODES = {
    'NONE': WiFiPowerSaveMode.WIFI_POWER_SAVE_NONE,
    'LIGHT': WiFiPowerSaveMode.WIFI_POWER_SAVE_LIGHT,
    'HIGH': WiFiPowerSaveMode.WIFI_POWER_SAVE_HIGH,
}
WiFiConnectedCondition = wifi_ns.class_('WiFiConnectedCondition', Condition)


def validate_password(value):
    value = cv.string_strict(value)
    if not value:
        return value
    if len(value) < 8:
        raise cv.Invalid(u"WPA password must be at least 8 characters long")
    if len(value) > 64:
        raise cv.Invalid(u"WPA password must be at most 64 characters long")
    return value


def validate_channel(value):
    value = cv.positive_int(value)
    if value < 1:
        raise cv.Invalid("Minimum WiFi channel is 1")
    if value > 14:
        raise cv.Invalid("Maximum WiFi channel is 14")
    return value


AP_MANUAL_IP_SCHEMA = cv.Schema({
    cv.Required(CONF_STATIC_IP): cv.ipv4,
    cv.Required(CONF_GATEWAY): cv.ipv4,
    cv.Required(CONF_SUBNET): cv.ipv4,
})

STA_MANUAL_IP_SCHEMA = AP_MANUAL_IP_SCHEMA.extend({
    cv.Optional(CONF_DNS1, default="0.0.0.0"): cv.ipv4,
    cv.Optional(CONF_DNS2, default="0.0.0.0"): cv.ipv4,
})

WIFI_NETWORK_BASE = cv.Schema({
    cv.GenerateID(): cv.declare_variable_id(WiFiAP),
    cv.Optional(CONF_SSID): cv.ssid,
    cv.Optional(CONF_PASSWORD): validate_password,
    cv.Optional(CONF_CHANNEL): validate_channel,
    cv.Optional(CONF_MANUAL_IP): STA_MANUAL_IP_SCHEMA,
})

WIFI_NETWORK_AP = WIFI_NETWORK_BASE.extend({

})

WIFI_NETWORK_STA = WIFI_NETWORK_BASE.extend({
    cv.Optional(CONF_BSSID): cv.mac_address,
    cv.Optional(CONF_HIDDEN): cv.boolean,
})


def validate(config):
    if CONF_PASSWORD in config and CONF_SSID not in config:
        raise cv.Invalid("Cannot have WiFi password without SSID!")

    if CONF_SSID in config:
        network = {CONF_SSID: config.pop(CONF_SSID)}
        if CONF_PASSWORD in config:
            network[CONF_PASSWORD] = config.pop(CONF_PASSWORD)
        if CONF_NETWORKS in config:
            raise cv.Invalid("You cannot use the 'ssid:' option together with 'networks:'. Please "
                             "copy your network into the 'networks:' key")
        config[CONF_NETWORKS] = cv.ensure_list(WIFI_NETWORK_STA)(network)

    if (CONF_NETWORKS not in config) and (CONF_AP not in config):
        raise cv.Invalid("Please specify at least an SSID or an Access Point "
                         "to create.")

    if config.get(CONF_FAST_CONNECT, False):
        networks = config.get(CONF_NETWORKS, [])
        if not networks:
            raise cv.Invalid("At least one network required for fast_connect!")
        if len(networks) != 1:
            raise cv.Invalid("Fast connect can only be used with one network!")

    if CONF_USE_ADDRESS not in config:
        if CONF_MANUAL_IP in config:
            use_address = str(config[CONF_MANUAL_IP][CONF_STATIC_IP])
        else:
            use_address = CORE.name + config[CONF_DOMAIN]
        config[CONF_USE_ADDRESS] = use_address

    return config


CONFIG_SCHEMA = cv.All(cv.Schema({
    cv.GenerateID(): cv.declare_variable_id(WiFiComponent),
    cv.Optional(CONF_NETWORKS): cv.ensure_list(WIFI_NETWORK_STA),

    cv.Optional(CONF_SSID): cv.ssid,
    cv.Optional(CONF_PASSWORD): validate_password,
    cv.Optional(CONF_MANUAL_IP): STA_MANUAL_IP_SCHEMA,

    cv.Optional(CONF_AP): WIFI_NETWORK_AP,
    cv.Optional(CONF_DOMAIN, default='.local'): cv.domain_name,
    cv.Optional(CONF_REBOOT_TIMEOUT, default='5min'): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_POWER_SAVE_MODE, default='NONE'):
        cv.one_of(*WIFI_POWER_SAVE_MODES, upper=True),
    cv.Optional(CONF_FAST_CONNECT, default=False): cv.boolean,
    cv.Optional(CONF_USE_ADDRESS): cv.string_strict,

    cv.Optional('hostname'): cv.invalid("The hostname option has been removed in 1.11.0"),
}), validate)


def safe_ip(ip):
    if ip is None:
        return IPAddress(0, 0, 0, 0)
    return IPAddress(*ip.args)


def manual_ip(config):
    if config is None:
        return None
    return cg.StructInitializer(
        ManualIP,
        ('static_ip', safe_ip(config[CONF_STATIC_IP])),
        ('gateway', safe_ip(config[CONF_GATEWAY])),
        ('subnet', safe_ip(config[CONF_SUBNET])),
        ('dns1', safe_ip(config.get(CONF_DNS1))),
        ('dns2', safe_ip(config.get(CONF_DNS2))),
    )


def wifi_network(config, static_ip):
    ap = cg.variable(config[CONF_ID], WiFiAP())
    if CONF_SSID in config:
        cg.add(ap.set_ssid(config[CONF_SSID]))
    if CONF_PASSWORD in config:
        cg.add(ap.set_password(config[CONF_PASSWORD]))
    if CONF_BSSID in config:
        cg.add(ap.set_bssid([HexInt(i) for i in config[CONF_BSSID].parts]))
    if CONF_HIDDEN in config:
        cg.add(ap.set_hidden(config[CONF_HIDDEN]))
    if CONF_CHANNEL in config:
        cg.add(ap.set_channel(config[CONF_CHANNEL]))
    if static_ip is not None:
        cg.add(ap.set_manual_ip(manual_ip(static_ip)))

    return ap


@coroutine_with_priority(60.0)
def to_code(config):
    rhs = WiFiComponent.new()
    wifi = cg.Pvariable(config[CONF_ID], rhs)
    cg.add(wifi.set_use_address(config[CONF_USE_ADDRESS]))

    for network in config.get(CONF_NETWORKS, []):
        cg.add(wifi.add_sta(wifi_network(network, config.get(CONF_MANUAL_IP))))

    if CONF_AP in config:
        cg.add(wifi.set_ap(wifi_network(config[CONF_AP], config.get(CONF_MANUAL_IP))))

    cg.add(wifi.set_reboot_timeout(config[CONF_REBOOT_TIMEOUT]))
    cg.add(wifi.set_power_save_mode(WIFI_POWER_SAVE_MODES[config[CONF_POWER_SAVE_MODE]]))
    cg.add(wifi.set_fast_connect(config[CONF_FAST_CONNECT]))

    if CORE.is_esp8266:
        cg.add_library('ESP8266WiFi', None)

    cg.add_define('USE_WIFI')

    # Register at end for OTA safe mode
    yield cg.register_component(wifi, config)


@CONDITION_REGISTRY.register('wifi.connected', cv.Schema({}))
def wifi_connected_to_code(config, condition_id, template_arg, args):
    rhs = WiFiConnectedCondition.new(template_arg)
    type = WiFiConnectedCondition.template(template_arg)
    yield cg.Pvariable(condition_id, rhs, type=type)
