import os
import sys
import types
import shutil


# Path/filename of the vim dictionary file to write to:
PYDICTION_DICT = r'complete-dict'
# Path/filename of the vim dictionary backup file:
PYDICTION_DICT_BACKUP = r'complete-dict.last'

# Sentintal to test if we should only output to stdout:
STDOUT_ONLY = False


def get_submodules(module_name, submodules):
    """Build a list of all the submodules of modules."""

    # Try to import a given module, so we can dir() it:
    try:
        imported_module = my_import(module_name)
    except ImportError, err:
        return submodules

    mod_attrs = dir(imported_module)

    for mod_attr in mod_attrs:
        if type(getattr(imported_module, mod_attr)) is types.ModuleType:
            submodules.append(module_name + '.' + mod_attr)

    return submodules


def write_dictionary(module_name):
    """Write to module attributes to the vim dictionary file."""
    prefix_on = '%s.%s'
    prefix_on_callable = '%s.%s('
    prefix_off = '%s'
    prefix_off_callable = '%s('

    try:
        imported_module = my_import(module_name)
    except ImportError, err:
        return

    mod_attrs = dir(imported_module)

    # Generate fully-qualified module names: 
    write_to.write('\n--- import %s ---\n' % module_name)
    for mod_attr in mod_attrs:
        if callable(getattr(imported_module, mod_attr)):
            # If an attribute is callable, show an opening parentheses:
            format = prefix_on_callable
        else:
            format = prefix_on
        write_to.write(format % (module_name, mod_attr) + '\n')

    # Generate submodule names by themselves, for when someone does
    # "from foo import bar" and wants to complete bar.baz.
    # This works the same no matter how many .'s are in the module.
    if module_name.count('.'):
        # Get the "from" part of the module. E.g., 'xml.parsers'
        # if the module name was 'xml.parsers.expat':
        first_part = module_name[:module_name.rfind('.')]
        # Get the "import" part of the module. E.g., 'expat'
        # if the module name was 'xml.parsers.expat'
        second_part = module_name[module_name.rfind('.') + 1:]
        write_to.write('\n--- from %s import %s ---\n' % 
                       (first_part, second_part))
        for mod_attr in mod_attrs:
            if callable(getattr(imported_module, mod_attr)):
                format = prefix_on_callable
            else:
                format = prefix_on
            write_to.write(format % (second_part, mod_attr) + '\n')

    # Generate non-fully-qualified module names: 
    write_to.write('\n--- from %s import * ---\n' % module_name)
    for mod_attr in mod_attrs:
        if callable(getattr(imported_module, mod_attr)):
            format = prefix_off_callable
        else:
            format = prefix_off
        write_to.write(format % mod_attr + '\n')


def my_import(name):
    """Make __import__ import "package.module" formatted names."""
    mod = __import__(name)
    components = name.split('.')
    for comp in components[1:]:
        mod = getattr(mod, comp)
    return mod


def remove_duplicates(seq, keep=()):
    """

    Remove duplicates from a sequence while perserving order.

    The optional tuple argument "keep" can be given to specificy 
    each string you don't want to be removed as a duplicate.
    """
    seq2 = []
    seen = set();
    for i in seq:
        if i in (keep):
            seq2.append(i)
            continue
        elif i not in seen:
            seq2.append(i)
        seen.add(i)
    return seq2
