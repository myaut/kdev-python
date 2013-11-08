#!/usr/bin/env python
# -*- Coding:utf-8 -*-

# Copyright 2013 by Sven Brauch <svenbrauch@googlemail.com>
# License: GNU GPL v3 or later
# The script output is not copyrighted, use it for whatever you want.

# WARNING: This script does things which can cause bad stuff to happen
# to your system in case you execute it on a module which has been
# engineered to be mailicious.
# I thus recommend to run this script as a user with minimal privileges
# (i.e. not as yourself), or even in a chroot.
# In any case, I'm not responsible for any damage caused by this script.

# This script will dump a SINGLE MODULE to a python "header" file.
# It will read one module, and give you one output file.
# Any submodules of the imported object (anything with type "module")
# will be ignored by this script, if you want to dump those,
# you will have to manually (or with a script) generate a directory
import re
# structure and re-run this script.
import traceback

import os
import sys
import types
import inspect
import importlib
import __builtin__

def debugmsg(message):
    sys.stderr.write(message + "\n")
    sys.stderr.flush()

def structseq_to_py(seq, name="INSERT_NAME"):
    """Turns a "structseq" object to a python pseudoclass."""
    sseq = str(seq)
    sseq = '('.join(sseq.split('(')[1:])
    sseq = ')'.join(sseq.split(')')[:1])
    print("class {0}:".format(name))
    for item in sseq.split(','):
        item = item.strip()
        key, value = item.split('=')
        print(indent("{0} = {1}".format(key, value)))

def indent(code, depth=4):
    code = code.split('\n')
    code = [" "*depth + line for line in code]
    return '\n'.join(code)

def clearIndent(code):
    assert isinstance(code, str)
    code = code.split('\n')
    code = [line.strip() for line in code]
    return '\n'.join(code)

def syntaxValid(code):
    try:
        compile(code, "<no file>", 'exec')
    except SyntaxError:
        return False
    return True

def sanitize(expr):
    assert isinstance(expr, str)
    replace = {
        '*':'', '[':'', ']':'',
        'from':'_from', 'class':'_class', '-':'_', 'lambda':'_lambda', "raise":"_raise",
        '\\':'ESC', ' ':'', "<":'"', ">":'"', "self,":"", "self":"",
        ",,":",", '...':'more_args', '+':"plus"
    }
    result = expr
    for before, after in replace.iteritems():
        result = result.replace(before, after)
    result = re.sub(r"\.\d", "_", result)
    result = result.replace("=,", "=[],").replace("=)", "=[])")
    return result

def strict_sanitize(expr):
    assert isinstance(expr, str)
    expr = sanitize(expr)
    forbidden = ["=()", '(', ')', '"', "'", " ", ",", "|", "%", '#']
    for char in forbidden:
        expr = expr.replace(char, "")
    if expr == ".":
        return "None"
    if len(expr) > 0 and expr[0].isdigit():
        expr = "_" + expr
    if len(expr) == 0:
        expr = "_"
    return expr

def isSpace(char):
    return char == ' ' or char == '\t'

def removeAtCorner(string, char, direction):
    i = 0
    assert direction in ['<', '>']
    if direction == '>':
        iterator = range(0, len(string))
        def r(s, a): return s[a-1:]
    if direction == '<':
        iterator = [len(string)-x-1 for x in range(0, len(string))]
        def r(s, a): return s[:a+1]

    atBeginning = True
    for i in iterator:
        if isSpace(string[i]) and atBeginning:
            continue
        elif string[i] == char:
            atBeginning = False
        else:
            return r(string, i)
    return str()

likely_substitutions = {
    "integer": "int",
    "string": "str",
    "long": "int",
    "dictionary": "dict",
    "double": "float",
}

def guess_return_type_from_synopsis(synopsis, root):
    container = ""
    for item in re.finditer("return", synopsis, re.I):
        scan = synopsis[item.start():item.end()+60]
        def apply_container(value):
            if len(container) > 0:
                return "{0}([{1}])".format(container, value)
            else:
                return value
        for word in scan.split():
            if word.find('.') != -1 and word != '...':
                break # end of sentence -- stop
            word = word.replace(',', '')
            if word in ["True", "False", "true", "false", "bool", "boolean"]:
                return apply_container("bool()")
            if word in ["dict", "dictionary"]:
                return "dict()"
            if word in ["string", "str", "represenation"]:
                return "str()"
            if word in ["list", "iterable"]:
                container = "list"
                continue
            if word in ["set"]:
                container = "set"
                continue
            if word in ["number", "int", "integer"]:
                return apply_container("int()")
            if word in ["float", "ratio", "fraction"]:
                return apply_container("float()")
            if hasattr(root.module, word) and type(getattr(root.module, word) == type(object)):
                return apply_container(word + "()")
            if word[-1] == "s" and hasattr(root.module, word[:-1]) and type(getattr(root.module, word[:-1]) == type(object)):
                # plural form, "list of ints"
                return apply_container(word[:-1] + "()")
            if hasattr(__builtin__, word) and type(getattr(__builtin__, word)) == type(object):
                return apply_container(word + "()")
    if len(container) > 0:
        return container + "()"
    return "None"

def parse_synopsis(funcdef, original, root, needSelfArg=False):
    """Parse a function description in the following format:
    module.func(param1, param2, [optional_param1 = default1, [optional_param2 = default2]]) -> return_type
    This tries to be as error-prone as possible in order to convert everything into a valid parameter list."""
    # first, take the parts before and after the arrow:
    assert isinstance(funcdef, str)
    funcdef = funcdef.replace("<==>", " -> ")
    s = funcdef.split(' -> ')
    definition = s[0]
    returnType = s[1] if len(s) > 1 else "None"
    # Sometimes, people do fancy stuff in the return type, like "... -> ndarray or None if arg is False"
    # Thus, we only use the first word... well.
    returnType = strict_sanitize(returnType.split(' ')[0])
    if returnType in likely_substitutions:
        returnType = likely_substitutions[returnType]
    if returnType != 'None':
        returnType += "()"
    if returnType == 'None' or returnType == '_()':
        returnType = guess_return_type_from_synopsis(original, root)
    # Okay, now the fun part: parse the parameter list
    inParamList = False
    brackets = 0
    paramList = ""
    for char in definition:
        if char == '(' and not inParamList:
            inParamList = True
        elif char == '(':
            brackets += 1
        if char == ')' and brackets > 0:
            brackets -= 1
        elif char == ')' and inParamList:
            break
        if inParamList and char not in '()':
            paramList += char
    paramList = paramList.split(',')
    resultingParamList = []
    atDefault = False
    for param in paramList:
        defaultValue = None
        # extract the name of the param
        param = param.replace(' ', '').replace('\t', '')
        # check for default values
        if removeAtCorner(param, '[', '>') != removeAtCorner(param, ' ', '>') or param.find('=') != -1:
            # default parameter list starts  or continues with this parameter
            atDefault = True
        if atDefault:
            defaultValue = "None"
            if param.find('=') != -1:
                # default value was provided; clean trailing "[" and "]" chars
                defaultValue = removeAtCorner(removeAtCorner(param.split('=')[1], ']', '<'), '[', '<')
                param = param.split('=')[0]
            if len(str(defaultValue)) == 0 or str(defaultValue).isspace():
                # just write anything, otherwise it's syntactically invalid
                defaultValue = "None"
        if removeAtCorner(param, '[', '<') != removeAtCorner(param, ' ', '<'):
            # default parameter list starts or continues after this parameter
            atDefault = True
        param = strict_sanitize(param)
        if param == '':
            continue
        if defaultValue:
            resultingParamList.append("{0}={1}".format(param, sanitize(defaultValue)))
        else:
            resultingParamList.append(param)
    if needSelfArg:
        # we're in a class, make sure there's a "self"
        if len(resultingParamList) == 0 or ( [resultingParamList[0].find(x) for x in ["self", "cls"]] == [-1, -1] ):
            resultingParamList.insert(0, "self")
    return ', '.join(resultingParamList), returnType


class ModuleDumper:
    def __init__(self, module):
        self.module = module
        self.code = str()
        self.indentDepth = 0

    def increaseIndent(self):
        self.indentDepth += 4

    def decreaseIndent(self):
        self.indentDepth -= 4
        if self.indentDepth < 0:
            self.indentDepth = 0

    def emit(self, code):
        print(indent(code, self.indentDepth))

    def dump(self):
        debugmsg("Processing module {0}".format(self.module.__name__))
        for member, value in inspect.getmembers(self.module):
            dumper = dumperForObject(value, member, self)
            dumper.dump()

class ScalarDumper:
    def __init__(self, name, value, root):
        self.name = name
        self.value = value
        self.root = root

    def dump(self):
        value = type(self.value).__name__ + "()" if self.value is not None else "None"
        self.root.emit("{0} = {1}".format(self.name, value))

class FunctionDumper:
    def __init__(self, function, root):
        self.function = function
        self.root = root
        assert isinstance(self.root, ModuleDumper)

    def dump(self):
        try:
            arguments = inspect.getargspec(self.function)
            arglist = list()
            for index, argument in enumerate(arguments.args):
                if len(arguments.args) - index - 1 > len(arguments.defaults):
                    # no default value -> normal argument
                    arglist.append(argument)
                else:
                    # there's a default value
                    defaultIndex = index - (len(arguments.args) - len(arguments.defaults))
                    rawDefaultValue = arguments.defaults[defaultIndex]
                    if type(rawDefaultValue) == type(object):
                        defaultValue = strict_sanitize(str(rawDefaultValue)) + "()"
                    else:
                        defaultValue = '"{0}"'.format(str(rawDefaultValue).replace("\n", " "))
                    if len(defaultValue) == 0 or defaultValue.isspace():
                        defaultValue = "None"
                    arglist.append("{0}={1}".format(argument, defaultValue))
            if self.root.indentDepth > 0:
                # we're in a class, make sure there's a "self"
                if len(arglist) == 0 or ( arglist[0].find("self") == -1 and arglist[0].find("cls") == -1 ):
                    arglist.insert(0, "self")
            arglist = ', '.join(arglist)
        except TypeError:
            # not a python function, can't inspect it. try guessing argspec from docstring
            arglist = None
        try:
            docstring = self.function.__doc__.split('\n')[0] if self.function.__doc__ else str()
            synArglist, returnValue = parse_synopsis(docstring, str(self.function.__doc__), self.root, self.root.indentDepth > 0)
        except Exception as e:
            debugmsg("  Warning: Function argument extraction failed: {0}".format(e))
            debugmsg("   * Traceback follows, but the error was ignored since it is not fatal.")
            traceback.print_exc(file=sys.stderr)
            synArglist = ""
            returnValue = "None"
        if arglist is None:
            arglist = synArglist
        funcname = self.function.__name__
        if funcname[0].isdigit():
            funcname = '_' + funcname
        self.root.emit("def {0}({1}):".format(strict_sanitize(funcname), arglist))
        self.root.increaseIndent()
        self.root.emit('"""{0}"""'.format(str(self.function.__doc__).replace('"""', '___')))
        self.root.emit("return {0}".format(returnValue))
        self.root.decreaseIndent()

class ClassDumper:
    def __init__(self, klass, root):
        self.klass = klass
        self.root = root
        assert isinstance(self.root, ModuleDumper)

    def dump(self):
        debugmsg("Generating documentation for class {0}".format(self.klass.__name__))
        self.root.emit("class {0}:".format(self.klass.__name__))
        self.root.increaseIndent()
        for member, value in inspect.getmembers(self.klass):
            if type(value) == type:
                continue
            dumper = dumperForObject(value, member, self.root)
            dumper.dump()
        self.root.decreaseIndent()

dumpers = {
    types.FunctionType: FunctionDumper,
    types.BuiltinFunctionType: FunctionDumper,
    types.BuiltinMethodType: FunctionDumper,
    type: ClassDumper
}
try:
    dumpers[types.ClassType] = ClassDumper # python 2
except:
    pass

def dumperForObject(object, memberName, root):
    try:
        return dumpers[type(object)](object, root)
    except:
        if hasattr(object, "__call__"):
            return FunctionDumper(object, root);
        return ScalarDumper(memberName, object, root)

if __name__ == '__main__':
    try:
        dumper = ModuleDumper(importlib.import_module(sys.argv[1]))
    except IndexError:
        debugmsg("Usage: introspect.py <python_module_name>")
        exit(1)
    dumper.dump()
    debugmsg("All done -- looks good so far.")