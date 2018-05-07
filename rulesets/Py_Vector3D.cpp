// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2000-2011 Alistair Riddoch
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA


#include "Py_Vector3D.h"

#include "Py_Quaternion.h"
#include "Py_Message.h"

#include <Atlas/Message/Element.h>

static PyObject * Vector3D_dot(PyVector3D * self, PyVector3D * other)
{
    if (!PyVector3D_Check(other)) {
        PyErr_SetString(PyExc_TypeError, "Can only dot with Vector3D");
        return nullptr;
    }
    return PyFloat_FromDouble(Dot(self->coords, other->coords));
}

static PyObject * Vector3D_cross(PyVector3D * self, PyVector3D * other)
{
    if (!PyVector3D_Check(other)) {
        PyErr_SetString(PyExc_TypeError, "Can only cross with Vector3D");
        return nullptr;
    }
    PyVector3D * ret = newPyVector3D();
    if (ret != nullptr) {
        ret->coords = Cross(self->coords, other->coords);
    }
    return (PyObject *)ret;
}

static PyObject * Vector3D_rotatex(PyVector3D * self, PyObject * arg)
{
    if (!PyFloat_CheckExact(arg)) {
        PyErr_SetString(PyExc_TypeError, "Can only rotatex with a float");
        return nullptr;
    }
    double angle = PyFloat_AsDouble(arg);
    self->coords.rotateX(angle);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * Vector3D_rotatey(PyVector3D * self, PyObject * arg)
{
    if (!PyFloat_CheckExact(arg)) {
        PyErr_SetString(PyExc_TypeError, "Can only rotatey with a float");
        return nullptr;
    }
    double angle = PyFloat_AsDouble(arg);
    self->coords.rotateY(angle);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * Vector3D_rotatez(PyVector3D * self, PyObject * arg)
{
    if (!PyFloat_CheckExact(arg)) {
        PyErr_SetString(PyExc_TypeError, "Can only rotatez with a float");
        return nullptr;
    }
    double angle = PyFloat_AsDouble(arg);
    self->coords.rotateZ(angle);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * Vector3D_rotate(PyVector3D * self, PyQuaternion * arg)
{
    if (!PyQuaternion_Check(arg)) {
        PyErr_SetString(PyExc_TypeError, "Can only rotate with a quaternion");
        return nullptr;
    }
    self->coords.rotate(arg->rotation);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * Vector3D_angle(PyVector3D * self, PyVector3D * other)
{
    if (!PyVector3D_Check(other)) {
        PyErr_SetString(PyExc_TypeError, "Can get angle to Vector3D");
        return nullptr;
    }
    return PyFloat_FromDouble(Angle(self->coords, other->coords));
}

static PyObject * Vector3D_sqr_mag(PyVector3D * self)
{
    return PyFloat_FromDouble(self->coords.sqrMag());
}

static PyObject * Vector3D_mag(PyVector3D * self)
{
    return PyFloat_FromDouble(self->coords.mag());
}

static PyObject * Vector3D_is_valid(PyVector3D * self)
{
    PyObject * ret = self->coords.isValid() ? Py_True : Py_False;
    Py_INCREF(ret);
    return ret;
}

static PyObject * Vector3D_unit_vector(PyVector3D * self)
{
    PyVector3D * ret = newPyVector3D();
    if (ret == nullptr) {
        return nullptr;
    }
    ret->coords = self->coords;
    WFMath::CoordType the_mag = ret->coords.mag();
    if (!(the_mag > 0)) {
        PyErr_SetString(PyExc_ZeroDivisionError, "Attempt to normalize a vector with zero magnitude");
        return nullptr;
    }
    ret->coords /= the_mag;
    return (PyObject *)ret;
}

static PyObject *Vector3D_unit_vector_to(PyVector3D * self, PyVector3D * other)
{
    if (!PyVector3D_Check(other)) {
        PyErr_SetString(PyExc_TypeError, "Argument must be a Vector3D");
        return nullptr;
    }
    PyVector3D * ret = newPyVector3D();
    if (ret == nullptr) {
        return nullptr;
    }
    ret->coords = (other->coords - self->coords);
    WFMath::CoordType the_mag = ret->coords.mag();
    if (!(the_mag > 0)) {
        PyErr_SetString(PyExc_ZeroDivisionError, "Attempt to normalize a vector with zero magnitude");
        return nullptr;
    }
    ret->coords /= the_mag;
    return (PyObject *)ret;
}

static PyMethodDef Vector3D_methods[] = {
    {"dot",             (PyCFunction)Vector3D_dot,      METH_O},
    {"cross",           (PyCFunction)Vector3D_cross,    METH_O},
    {"rotatex",         (PyCFunction)Vector3D_rotatex,  METH_O},
    {"rotatey",         (PyCFunction)Vector3D_rotatey,  METH_O},
    {"rotatez",         (PyCFunction)Vector3D_rotatez,  METH_O},
    {"rotate",          (PyCFunction)Vector3D_rotate,   METH_O},
    {"angle",           (PyCFunction)Vector3D_angle,    METH_O},
    {"square_mag",      (PyCFunction)Vector3D_sqr_mag,  METH_NOARGS},
    {"mag",             (PyCFunction)Vector3D_mag,      METH_NOARGS},
    {"is_valid",        (PyCFunction)Vector3D_is_valid, METH_NOARGS},
    {"unit_vector",     (PyCFunction)Vector3D_unit_vector,      METH_NOARGS},
    {"unit_vector_to",  (PyCFunction)Vector3D_unit_vector_to,   METH_O},
    {nullptr,              nullptr}           /* sentinel */
};

static void Vector3D_dealloc(PyVector3D *self)
{
    self->coords.~Vector3D();
    Py_TYPE(self)->tp_free(self);
}

static PyObject* Vector3D_repr(PyVector3D * self)
{
    char buf[64];
    ::snprintf(buf, 64, "(%f, %f, %f)", self->coords.x(), self->coords.y(), self->coords.z());
    return PyUnicode_FromString(buf);
}

static PyObject * Vector3D_getattro(PyVector3D *self, PyObject *oname)
{
    char * name = PyUnicode_AsUTF8(oname);
    if (strcmp(name, "x") == 0) { return PyFloat_FromDouble(self->coords.x()); }
    if (strcmp(name, "y") == 0) { return PyFloat_FromDouble(self->coords.y()); }
    if (strcmp(name, "z") == 0) { return PyFloat_FromDouble(self->coords.z()); }

    return PyObject_GenericGetAttr((PyObject *)self, oname);
}

static int Vector3D_setattro(PyVector3D *self, PyObject *oname, PyObject *v)
{
    char * name = PyUnicode_AsUTF8(oname);
    float val;
    if (PyLong_Check(v)) {
        val = PyLong_AsLong(v);
    } else if (PyFloat_Check(v)) {
        val = PyFloat_AsDouble(v);
    } else {
        PyErr_SetString(PyExc_TypeError, "Vector3D attributes must be numeric");
        return -1;
    }
    if (strcmp(name, "x") == 0) {
        self->coords.x() = val;
    } else if (strcmp(name, "y") == 0) {
        self->coords.y() = val;
    } else if (strcmp(name, "z") == 0) {
        self->coords.z() = val;
    } else {
        PyErr_SetString(PyExc_AttributeError, "Vector3D attribute does not exist");
        return -1;
    }
    return 0;
}

static PyObject* Vector3D_compare(PyObject *a, PyObject *b, int op)
{
    PyObject *result = Py_NotImplemented;

    auto self = (PyVector3D*)a;
    if (PyVector3D_Check(b)) {
        auto other = (PyVector3D*)b;
        if (op == Py_EQ) {
            result = self->coords == other->coords ? Py_True : Py_False;
        } else if (op == Py_NE) {
            result = self->coords != other->coords ? Py_True : Py_False;
        }
    }

    Py_INCREF(result);
    return result;
}

/*
 * Vector3D sequence methods.
 */

#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#endif

static Py_ssize_t Vector3D_seq_length(PyVector3D * self)
{
    return 3;
}

static PyObject * Vector3D_seq_item(PyVector3D * self, Py_ssize_t item)
{
    if (item < 0 || item >= 3) {
        PyErr_SetString(PyExc_TypeError,"Vector3D.[]: Index out of range.");
        return 0;
    }
    return PyFloat_FromDouble(self->coords[item]);
}

static int Vector3D_seq_ass_item(PyVector3D * self,
                                 Py_ssize_t item,
                                 PyObject * val)
{
    if (item < 0 || item >= 3) {
        PyErr_SetString(PyExc_TypeError,"Vector3D.[]: Index out of range.");
        return -1;
    }
    if (!PyFloat_Check(val)) {
        PyErr_SetString(PyExc_TypeError,"Vector3D.[]: Value must be float.");
        return -1;
    }
    self->coords[item] = PyFloat_AsDouble(val);
    return 0;
}

static PyVector3D*Vector3D_num_add(PyVector3D*self,PyVector3D*other)
{
    if (!PyVector3D_Check(other)) {
        PyErr_SetString(PyExc_TypeError, "Can only add Vector3D to Vector3D");
        return nullptr;
    }
    PyVector3D * ret = newPyVector3D();
    if (ret != nullptr) {
        ret->coords = (self->coords + other->coords);
    }
    return ret;
}

static PyVector3D*Vector3D_num_sub(PyVector3D*self,PyVector3D*other)
{
    if (!PyVector3D_Check(other)) {
        PyErr_SetString(PyExc_TypeError, "Can only sub Vector3D from Vector3D");
        return nullptr;
    }
    PyVector3D * ret = newPyVector3D();
    if (ret != nullptr) {
        ret->coords = (self->coords - other->coords);
    }
    return ret;
}

static PyVector3D * Vector3D_num_mul(PyVector3D * self, PyObject * _other)
{
    double other;
    if (PyLong_Check(_other)) {
        other = PyLong_AsLong(_other);
    } else if (PyFloat_Check(_other)) {
        other = PyFloat_AsDouble(_other);
    } else {
        PyErr_SetString(PyExc_TypeError, "Vector3D can only be multiplied by numeric value");
        return nullptr;
    }
    PyVector3D * ret = newPyVector3D();
    if (ret != nullptr) {
        ret->coords = (self->coords * other);
    }
    return ret;
}

static PyVector3D * Vector3D_num_div(PyVector3D * self, PyObject * _other)
{
    double other;
    if (PyLong_Check(_other)) {
        other = PyLong_AsLong(_other);
    } else if (PyFloat_Check(_other)) {
        other = PyFloat_AsDouble(_other);
    } else {
        PyErr_SetString(PyExc_TypeError, "Vector3D can only be divided by numeric value");
        return nullptr;
    }
    PyVector3D * ret = newPyVector3D();
    if (ret != nullptr) {
        ret->coords = (self->coords / other);
    }
    return ret;
}

static PyVector3D * Vector3D_negative(PyVector3D * self)
{
    PyVector3D * ret = newPyVector3D();
    if (ret != nullptr) {
        ret->coords = -self->coords;
    }
    return ret;
}

static int Vector3D_init(PyVector3D * self, PyObject * args, PyObject * kwds)
{
    PyObject * clist;
    switch (PyTuple_Size(args)) {
        case 0:
            break;
        case 1:
            clist = PyTuple_GetItem(args, 0);
            if (!PyList_Check(clist)) {
                PyErr_SetString(PyExc_TypeError, "Vector3D() from single value must be a list");
                return -1;
            }
            if (PyList_Size(clist) != 3) {
                PyErr_SetString(PyExc_ValueError, "Vector3D() from a list must be 3 long");
                return -1;
            }
            for(int i = 0; i < 3; i++) {
                PyObject * item = PyList_GetItem(clist, i);
                if (PyLong_Check(item)) {
                    self->coords[i] = (float)PyLong_AsLong(item);
                } else if (PyFloat_Check(item)) {
                    self->coords[i] = PyFloat_AsDouble(item);
                } else if (PyMessage_Check(item)) {
                    PyMessage * mitem = (PyMessage*)item;
                    if (!mitem->m_obj->isNum()) {
                        PyErr_SetString(PyExc_TypeError, "Vector3D() must take list of floats, or ints");
                        return -1;
                    }
                    self->coords[i] = mitem->m_obj->asNum();
                } else {
                    PyErr_SetString(PyExc_TypeError, "Vector3D() must take list of floats, or ints");
                    return -1;
                }
            }
            self->coords.setValid();
            break;
        case 3:
            for(int i = 0; i < 3; i++) {
                PyObject * item = PyTuple_GetItem(args, i);
                if (PyLong_Check(item)) {
                    self->coords[i] = (float)PyLong_AsLong(item);
                } else if (PyFloat_Check(item)) {
                    self->coords[i] = PyFloat_AsDouble(item);
                } else {
                    PyErr_SetString(PyExc_TypeError, "Vector3D() must take list of floats, or ints");
                    return -1;
                }
            }
            self->coords.setValid();
            break;
        default:
            PyErr_SetString(PyExc_TypeError, "Vector3D must take list of floats, or ints, 3 ints or 3 floats");
            return -1;
            break;
    }
        
    return 0;
}

static PyObject * Vector3D_new(PyTypeObject * type, PyObject *, PyObject *)
{
    // This looks allot like the default implementation, except we call the
    // in-place constructor.
    PyVector3D * self = (PyVector3D *)type->tp_alloc(type, 0);
    if (self != nullptr) {
        new (&(self->coords)) Vector3D();
    }
    return (PyObject *)self;
}

static PySequenceMethods Vector3D_seq = {
    (lenfunc)Vector3D_seq_length,              /* sq_length */
    nullptr,                                      /* sq_concat */
    nullptr,                                      /* sq_repeat */
    (ssizeargfunc)Vector3D_seq_item,           /* sq_item */
    nullptr,                                      /* sq_slice */
    (ssizeobjargproc)Vector3D_seq_ass_item,    /* sq_ass_item */
    nullptr                                       /* sq_ass_slice */
};

static PyNumberMethods Vector3D_num = {
    (binaryfunc)Vector3D_num_add,   /* nb_add */
    (binaryfunc)Vector3D_num_sub,   /* nb_subtract */
    (binaryfunc)Vector3D_num_mul,   /* nb_multiply */
    0,                              /* nb_remainder */
    0,                              /* nb_divmod */
    0,                              /* nb_power */
    (unaryfunc)Vector3D_negative,   /* nb_negative */
    0,                              /* nb_positive */
    0,                              /* nb_absolute */
    0,                              /* nb_nonzero */
    0,                              /* nb_invert */
    0,                              /* nb_lshift */
    0,                              /* nb_rshift */
    0,                              /* nb_and */
    0,                              /* nb_xor */
    0,                              /* nb_or */
    0,                              /* nb_int */
    0,                              /* nb_reserved */
    0,                              /* nb_float */
    0,                              /* nb_inplace_add */
    0,                              /* nb_inplace_subtract */
    0,                              /* nb_inplace_multiply */
    0,                              /* nb_inplace_remainder */
    0,                              /* nb_inplace_power */
    0,                              /* nb_inplace_lshift */
    0,                              /* nb_inplace_rshift */
    0,                              /* nb_inplace_and */
    0,                              /* nb_inplace_xor */
    0,                              /* nb_inplace_or */
    (binaryfunc)Vector3D_num_div,   /* nb_floor_divide */
    (binaryfunc)Vector3D_num_div,   /* nb_true_divide */
    0,                              /* nb_inplace_floor_divide */
    0,                              /* nb_inplace_true_divide */
};

PyTypeObject PyVector3D_Type = {
        PyVarObject_HEAD_INIT(0, 0)
        "physics.Vector3D",             // tp_name
        sizeof(PyVector3D),             // tp_basicsize
        0,                              // tp_itemsize
        // methods 
        (destructor)Vector3D_dealloc,   // tp_dealloc
        0,                              // tp_print
        0,                              // tp_getattr
        0,                              // tp_setattr
        0,                              // tp_compare
        (reprfunc)Vector3D_repr,        // tp_repr
        &Vector3D_num,                  // tp_as_number
        &Vector3D_seq,                  // tp_as_sequence
        0,                              // tp_as_mapping
        0,                              // tp_hash
        0,                              // tp_call
        0,                              // tp_str
        (getattrofunc)Vector3D_getattro,// tp_getattro
        (setattrofunc)Vector3D_setattro,// tp_setattro
        0,                              // tp_as_buffer
        Py_TPFLAGS_DEFAULT,             // tp_flags
        "Vector3D objects",             // tp_doc
        0,                              // tp_travers
        0,                              // tp_clear
        (richcmpfunc)Vector3D_compare,  // tp_richcompare
        0,                              // tp_weaklistoffset
        0,                              // tp_iter
        0,                              // tp_iternext
        Vector3D_methods,               // tp_methods
        0,                              // tp_members
        0,                              // tp_getset
        0,                              // tp_base
        0,                              // tp_dict
        0,                              // tp_descr_get
        0,                              // tp_descr_set
        0,                              // tp_dictoffset
        (initproc)Vector3D_init,        // tp_init
        0,                              // tp_alloc
        Vector3D_new,                   // tp_new
};

PyVector3D * newPyVector3D()
{
    return (PyVector3D *)PyVector3D_Type.tp_new(&PyVector3D_Type, 0, 0);
}
