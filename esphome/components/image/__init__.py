import logging

from esphome import core
from esphome.components import display, font
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_FILE, CONF_ID, CONF_RESIZE
from esphome.core import CORE, HexInt
_LOGGER = logging.getLogger(__name__)

DEPENDENCIES = ['display']
MULTI_CONF = True

CONF_COLOR_DEPTH = 'color_depth'

ColorDepth = display.display_ns.enum('ColorDepth')
COLOR_DEPTH = {
    'BINARY': ColorDepth.COLOR_DEPTH_BINARY,
    'GRAYSCALE': ColorDepth.COLOR_DEPTH_GRAYSCALE,
    'RGB': ColorDepth.COLOR_DEPTH_RGB,
}

Image_ = display.display_ns.class_('Image')

CONF_RAW_DATA_ID = 'raw_data_id'

IMAGE_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(Image_),
    cv.Required(CONF_FILE): cv.file_,
    cv.Optional(CONF_RESIZE): cv.dimensions,
    cv.Optional(CONF_COLOR_DEPTH, default='BINARY'): cv.enum(COLOR_DEPTH, upper=True),
    cv.GenerateID(CONF_RAW_DATA_ID): cv.declare_id(cg.uint8),
})

CONFIG_SCHEMA = cv.All(font.validate_pillow_installed, IMAGE_SCHEMA)


def to_code(config):
    from PIL import Image

    path = CORE.relative_config_path(config[CONF_FILE])
    try:
        image = Image.open(path)
    except Exception as e:
        raise core.EsphomeError(f"Could not load image file {path}: {e}")

    width, height = image.size

    if CONF_RESIZE in config:
        image.thumbnail(config[CONF_RESIZE])
        width, height = image.size
    else:
        if width > 500 or height > 500:
            _LOGGER.warning("The image you requested is very big. Please consider using"
                            " the resize parameter.")

    if config[CONF_COLOR_DEPTH] == 'GRAYSCALE':
        image = image.convert('L', dither=Image.NONE)
        pixels = list(image.getdata())
        data = [0 for _ in range(height * width)]
        pos = 0
        for pix in pixels:
            data[pos] = pix
            pos += 1

    elif config[CONF_COLOR_DEPTH] == 'RGB':
        image = image.convert('RGB')
        pixels = list(image.getdata())
        data = [0 for _ in range(height * width * 3)]
        pos = 0
        for pix in pixels:
            data[pos] = pix[0]
            pos += 1
            data[pos] = pix[1]
            pos += 1
            data[pos] = pix[2]
            pos += 1

    elif config[CONF_COLOR_DEPTH] == 'BINARY':
        image = image.convert('1', dither=Image.NONE)
        width8 = ((width + 7) // 8) * 8
        data = [0 for _ in range(height * width8 // 8)]
        for y in range(height):
            for x in range(width):
                if image.getpixel((x, y)):
                    continue
                pos = x + y * width8
                data[pos // 8] |= 0x80 >> (pos % 8)

    rhs = [HexInt(x) for x in data]
    prog_arr = cg.progmem_array(config[CONF_RAW_DATA_ID], rhs)
    cg.new_Pvariable(config[CONF_ID], prog_arr, width, height, COLOR_DEPTH[config[CONF_COLOR_DEPTH]])
