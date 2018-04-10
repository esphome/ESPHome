from __future__ import print_function

import codecs
import errno
import os

from esphomeyaml.config import get_component
from esphomeyaml.const import CONF_BOARD, CONF_ESPHOMEYAML, CONF_LIBRARY_URI, CONF_LOGGER, \
    CONF_NAME, CONF_PLATFORM, ESP_PLATFORM_ESP32, ESP_PLATFORM_ESP8266
from esphomeyaml.core import ESPHomeYAMLError

CPP_AUTO_GENERATE_BEGIN = u'// ========== AUTO GENERATED CODE BEGIN ==========='
CPP_AUTO_GENERATE_END = u'// =========== AUTO GENERATED CODE END ============'
INI_AUTO_GENERATE_BEGIN = u'; ========== AUTO GENERATED CODE BEGIN ==========='
INI_AUTO_GENERATE_END = u'; =========== AUTO GENERATED CODE END ============'

CPP_BASE_FORMAT = (u"""// Auto generated code by esphomeyaml
#include "esphomelib/application.h"

using namespace esphomelib;

void setup() {
  // ===== DO NOT EDIT ANYTHING BELOW THIS LINE =====
  """, u"""
  // ========= YOU CAN EDIT AFTER THIS LINE =========
  App.setup();
}

void loop() {
  App.loop();
  delay(1);
}
""")

INI_BASE_FORMAT = (u"""; Auto generated code by esphomeyaml

[common]
lib_deps = 
build_flags = 
upload_flags = 

; ===== DO NOT EDIT ANYTHING BELOW THIS LINE =====
""", u"""
; ========= YOU CAN EDIT AFTER THIS LINE =========

""")

INI_CONTENT_FORMAT = u"""[env:{env}]
platform = {platform}
board = {board}
framework = arduino
lib_deps =
    {esphomeyaml_uri}
    ${{common.lib_deps}}
build_flags ={build_flags}
    ${{common.build_flags}}
"""

PLATFORM_TO_PLATFORMIO = {
    ESP_PLATFORM_ESP32: 'espressif32',
    ESP_PLATFORM_ESP8266: 'espressif8266'
}


def get_ini_content(config):
    d = {
        u'env': config[CONF_ESPHOMEYAML][CONF_NAME],
        u'platform': PLATFORM_TO_PLATFORMIO[config[CONF_ESPHOMEYAML][CONF_PLATFORM]],
        u'board': config[CONF_ESPHOMEYAML][CONF_BOARD],
        u'esphomeyaml_uri': config[CONF_ESPHOMEYAML][CONF_LIBRARY_URI],
        u'build_flags': u'',
    }
    if CONF_LOGGER in config:
        build_flags = get_component(CONF_LOGGER).get_build_flags(config[CONF_LOGGER])
        if build_flags:
            d[u'build_flags'] = u'\n    ' + build_flags
    return INI_CONTENT_FORMAT.format(**d)


def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc:  # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise


def find_begin_end(text, begin_s, end_s):
    begin_index = text.find(begin_s)
    if begin_index == -1:
        raise ESPHomeYAMLError(u"Could not find auto generated code begin in file, either"
                               u"delete the main sketch file or insert the comment again.")
    if text.find(begin_s, begin_index + 1) != -1:
        raise ESPHomeYAMLError(u"Found multiple auto generate code begins, don't know"
                               u"which to chose, please remove one of them.")
    end_index = text.find(end_s)
    if end_index == -1:
        raise ESPHomeYAMLError(u"Could not find auto generated code end in file, either"
                               u"delete the main sketch file or insert the comment again.")
    if text.find(end_s, end_index + 1) != -1:
        raise ESPHomeYAMLError(u"Found multiple auto generate code endings, don't know"
                               u"which to chose, please remove one of them.")

    return text[:begin_index], text[(end_index + len(end_s)):]


def write_platformio_ini(content, path):
    if os.path.isfile(path):
        try:
            with codecs.open(path, 'r', encoding='utf-8') as f:
                text = f.read()
        except OSError:
            raise ESPHomeYAMLError(u"Could not read ini file at {}".format(path))
        prev_file = text
        content_format = find_begin_end(text, INI_AUTO_GENERATE_BEGIN, INI_AUTO_GENERATE_END)
    else:
        prev_file = None
        mkdir_p(os.path.dirname(path))
        content_format = INI_BASE_FORMAT
    full_file = content_format[0] + INI_AUTO_GENERATE_BEGIN + '\n' + \
        content + INI_AUTO_GENERATE_END + content_format[1]
    if prev_file == full_file:
        return
    with codecs.open(path, mode='w+', encoding='utf-8') as f:
        f.write(full_file)


def write_cpp(code_s, path):
    if os.path.isfile(path):
        try:
            with codecs.open(path, 'r', encoding='utf-8') as f:
                text = f.read()
        except OSError:
            raise ESPHomeYAMLError(u"Could not read C++ file at {}".format(path))
        prev_file = text
        code_format = find_begin_end(text, CPP_AUTO_GENERATE_BEGIN, CPP_AUTO_GENERATE_END)
    else:
        prev_file = None
        mkdir_p(os.path.dirname(path))
        code_format = CPP_BASE_FORMAT

    full_file = code_format[0] + CPP_AUTO_GENERATE_BEGIN + '\n' + \
        code_s + CPP_AUTO_GENERATE_END + code_format[1]
    if prev_file == full_file:
        return
    with codecs.open(path, 'w+', encoding='utf-8') as f:
        f.write(full_file)
