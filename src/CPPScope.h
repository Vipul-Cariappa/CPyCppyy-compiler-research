#ifndef CPYCPPYY_CPPSCOPE_H
#define CPYCPPYY_CPPSCOPE_H

#if PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION == 2

// In p2.2, PyHeapTypeObject is not yet part of the interface
#include "structmember.h"

typedef struct {
    PyTypeObject type;
    PyNumberMethods as_number;
    PySequenceMethods as_sequence;
    PyMappingMethods as_mapping;
    PyBufferProcs as_buffer;
    PyObject *name, *slots;
    PyMemberDef members[1];
} PyHeapTypeObject;

#endif

// Standard
#include <map>


namespace CPyCppyy {

/** Type object to hold class reference (this is only semantically a presentation
    of CPPScope instances, not in a C++ sense)
      @author  WLAV
      @date    07/06/2017
      @version 2.0
 */

typedef std::map<Cppyy::TCppObject_t, PyObject*> CppToPyMap_t;
namespace Utility { struct PyOperators; }

class CPPScope {
public:
    enum EFlags {
        kNone            = 0x0,
        kIsMeta          = 0x0001,
        kIsNamespace     = 0x0002,
        kIsException     = 0x0004,
        kIsSmart         = 0x0008,
        kIsPython        = 0x0010,
        kIsMultiCross    = 0x0020,
        kIsInComplete    = 0x0040,
        kNoImplicit      = 0x0080,
        kNoOSInsertion   = 0x0100,
        kGblOSInsertion  = 0x0200,
        kNoPrettyPrint   = 0x0400 };

public:
    PyHeapTypeObject   fType;
    Cppyy::TCppScope_t fCppType;
    uint32_t           fFlags;
    union {
        CppToPyMap_t*           fCppObjects;     // classes only
        std::vector<PyObject*>* fUsing;          // namespaces only
    } fImp;
    Utility::PyOperators*       fOperators;
    char*             fModuleName;

private:
    CPPScope() = delete;
};

typedef CPPScope CPPClass;

class CPPSmartClass : public CPPClass {
public:
    Cppyy::TCppScope_t  fUnderlyingType;
    Cppyy::TCppMethod_t fDereferencer;
};


//- metatype type and type verification --------------------------------------
extern PyTypeObject CPPScope_Type;

template<typename T>
inline bool CPPScope_Check(T* object)
{
// Short-circuit the type check by checking tp_new which all generated subclasses
// of CPPScope inherit.
    return object && \
        (Py_TYPE(object)->tp_new == CPPScope_Type.tp_new || \
         PyObject_TypeCheck(object, &CPPScope_Type));
}

template<typename T>
inline bool CPPScope_CheckExact(T* object)
{
    return object && Py_TYPE(object) == &CPPScope_Type;
}

//- creation -----------------------------------------------------------------
inline CPPScope* CPPScopeMeta_New(Cppyy::TCppScope_t klass, PyObject* args)
{
// Create and initialize a new scope meta class
    CPPScope* pymeta = (CPPScope*)PyType_Type.tp_new(&CPPScope_Type, args, nullptr);
    if (!pymeta) return pymeta;

// set the klass id, for instances and Python-side derived classes to pick up
    pymeta->fCppType         = klass;
    pymeta->fFlags           = CPPScope::kIsMeta;
    pymeta->fImp.fCppObjects = nullptr;
    pymeta->fOperators       = nullptr;
    pymeta->fModuleName      = nullptr;

    return pymeta;
}

} // namespace CPyCppyy

#endif // !CPYCPPYY_CPPSCOPE_H
