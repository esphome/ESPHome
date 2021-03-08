# These are a helper decorators to help get schema from some
# components which uses volutuous in a way where validation
# is hidden in local functions

# These decorators should not modify at all what the functions
# originally do.
#
# However there is a property to further disable decorator
# impat.
#
# This is set to true by script/build_jsonschema.py
# only, so data is collected (again functionality is not modified)
EnableJsonSchemaCollect = False

extended_schemas = {}
list_schemas = {}
registry_schemas = {}
hidden_schemas = {}
typed_schemas = {}


def jschema_extractor(validator_name):
    if EnableJsonSchemaCollect:

        def decorator(func):
            hidden_schemas[str(func)] = validator_name
            return func

        return decorator

    def dummy(f):
        return f

    return dummy


def jschema_extended(func):
    if EnableJsonSchemaCollect:

        def decorate(*args, **kwargs):
            ret = func(*args, **kwargs)
            assert len(args) == 2
            extended_schemas[str(ret)] = args
            return ret

        return decorate

    return func


def jschema_composite(func):
    if EnableJsonSchemaCollect:

        def decorate(*args, **kwargs):
            ret = func(*args, **kwargs)
            # args length might be 2, but 2nd is always validator
            list_schemas[str(ret)] = args
            return ret

        return decorate

    return func


def jschema_registry(registry):
    if EnableJsonSchemaCollect:

        def decorator(func):
            registry_schemas[str(func)] = registry
            return func

        return decorator

    def dummy(f):
        return f

    return dummy


def jschema_typed(func):
    if EnableJsonSchemaCollect:

        def decorate(*args, **kwargs):
            ret = func(*args, **kwargs)
            typed_schemas[str(ret)] = (args, kwargs)
            return ret

        return decorate

    return func
