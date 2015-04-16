/* ====================================================================
 * Copyright (c) 2004-2007 Open Source Applications Foundation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions: 
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software. 
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * ====================================================================
 */

#include <Python.h>
#include "structmember.h"

#include "common.h"
#include "errors.h"
#include "bases.h"
#include "locale.h"
#include "iterators.h"
#include "format.h"
#include "dateformat.h"
#include "numberformat.h"
#include "calendar.h"
#include "collator.h"
#include "charset.h"
#include "tzinfo.h"


/* const variable descriptor */

class t_descriptor {
public:
    PyObject_HEAD
    int flags;
    union {
        PyObject *value;
        PyObject *(*get)(PyObject *);
    } access;
};
    
#define DESCRIPTOR_STATIC 0x1

static void t_descriptor_dealloc(t_descriptor *self);
static PyObject *t_descriptor___get__(t_descriptor *self,
                                      PyObject *obj, PyObject *type);

static PyMemberDef t_descriptor_members[] = {
    { NULL, 0, 0, 0, NULL }
};

static PyMethodDef t_descriptor_methods[] = {
    { NULL, NULL, 0, NULL }
};


PyTypeObject ConstVariableDescriptorType = {
    PyObject_HEAD_INIT(NULL)
    0,                                   /* ob_size */
    "PyICU.ConstVariableDescriptor",     /* tp_name */
    sizeof(t_descriptor),                /* tp_basicsize */
    0,                                   /* tp_itemsize */
    (destructor)t_descriptor_dealloc,    /* tp_dealloc */
    0,                                   /* tp_print */
    0,                                   /* tp_getattr */
    0,                                   /* tp_setattr */
    0,                                   /* tp_compare */
    0,                                   /* tp_repr */
    0,                                   /* tp_as_number */
    0,                                   /* tp_as_sequence */
    0,                                   /* tp_as_mapping */
    0,                                   /* tp_hash  */
    0,                                   /* tp_call */
    0,                                   /* tp_str */
    0,                                   /* tp_getattro */
    0,                                   /* tp_setattro */
    0,                                   /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                  /* tp_flags */
    "const variable descriptor",         /* tp_doc */
    0,                                   /* tp_traverse */
    0,                                   /* tp_clear */
    0,                                   /* tp_richcompare */
    0,                                   /* tp_weaklistoffset */
    0,                                   /* tp_iter */
    0,                                   /* tp_iternext */
    t_descriptor_methods,                /* tp_methods */
    t_descriptor_members,                /* tp_members */
    0,                                   /* tp_getset */
    0,                                   /* tp_base */
    0,                                   /* tp_dict */
    (descrgetfunc)t_descriptor___get__,  /* tp_descr_get */
    0,                                   /* tp_descr_set */
    0,                                   /* tp_dictoffset */
    0,                                   /* tp_init */
    0,                                   /* tp_alloc */
    0,                                   /* tp_new */
};

static void t_descriptor_dealloc(t_descriptor *self)
{
    if (self->flags & DESCRIPTOR_STATIC)
    {
        Py_DECREF(self->access.value);
    }
    self->ob_type->tp_free((PyObject *) self);
}

PyObject *make_descriptor(PyObject *value)
{
    t_descriptor *self = (t_descriptor *)
        ConstVariableDescriptorType.tp_alloc(&ConstVariableDescriptorType, 0);

    if (self)
    {
        self->access.value = value;
        self->flags = DESCRIPTOR_STATIC;
    }
    else
        Py_DECREF(value);

    return (PyObject *) self;
}

PyObject *make_descriptor(PyTypeObject *value)
{
    t_descriptor *self = (t_descriptor *)
        ConstVariableDescriptorType.tp_alloc(&ConstVariableDescriptorType, 0);

    if (self)
    {
        Py_INCREF(value);
        self->access.value = (PyObject *) value;
        self->flags = DESCRIPTOR_STATIC;
    }

    return (PyObject *) self;
}

PyObject *make_descriptor(PyObject *(*get)(PyObject *))
{
    t_descriptor *self = (t_descriptor *)
        ConstVariableDescriptorType.tp_alloc(&ConstVariableDescriptorType, 0);

    if (self)
    {
        self->access.get = get;
        self->flags = 0;
    }

    return (PyObject *) self;
}

static PyObject *t_descriptor___get__(t_descriptor *self,
                                      PyObject *obj, PyObject *type)
{
    if (self->flags & DESCRIPTOR_STATIC)
    {
        Py_INCREF(self->access.value);
        return self->access.value;
    }
    else if (obj == NULL || obj == Py_None)
    {
        Py_INCREF(self);
        return (PyObject *) self;
    }

    return self->access.get(obj);
}


static PyMethodDef pyicu_funcs[] = {
    { NULL, NULL, 0, NULL }
};


extern "C" {

    void init_PyICU(void)
    {
        PyObject *m = Py_InitModule3("_PyICU", pyicu_funcs, "_PyICU");
        PyObject *ver;

        PyType_Ready(&ConstVariableDescriptorType);
        Py_INCREF(&ConstVariableDescriptorType);

#ifdef _MSC_VER
#define verstring(n) #n
        ver = PyString_FromString(verstring(PYICU_VER));
#else
        ver = PyString_FromString(PYICU_VER);
#endif
        PyObject_SetAttrString(m, "VERSION", ver); Py_DECREF(ver);

        ver = PyString_FromString(U_ICU_VERSION);
        PyObject_SetAttrString(m, "ICU_VERSION", ver); Py_DECREF(ver);

        ver = PyString_FromString(U_UNICODE_VERSION);
        PyObject_SetAttrString(m, "UNICODE_VERSION", ver); Py_DECREF(ver);

        PyObject *module = PyImport_ImportModule("PyICU");

        if (!module)
        {
            if (!PyErr_Occurred())
                PyErr_SetString(PyExc_ImportError, "PyICU");
            return;
        }

        PyExc_ICUError = PyObject_GetAttrString(module, "ICUError");
        PyExc_InvalidArgsError = PyObject_GetAttrString(module, "InvalidArgsError");
        Py_DECREF(module);

        _init_common(m);
        _init_errors(m);
        _init_bases(m);
        _init_locale(m);
        _init_iterators(m);
        _init_format(m);
        _init_dateformat(m);
        _init_numberformat(m);
        _init_calendar(m);
        _init_collator(m);
        _init_charset(m);
        _init_tzinfo(m);
    }
}