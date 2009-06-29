/*
 * this file was generated from the Python C sources using the script
 * Stackless/core/extract_slp_methods.py .
 * please don't edit this output, but work on the script.
 */

typedef struct {
    void *type;
    size_t offset;
} _stackless_method;

#define MFLAG_IND 0x8000
#define MFLAG_OFS(meth) offsetof(PyTypeObject, slpflags.meth)
#define MFLAG_OFS_IND(meth) MFLAG_OFS(meth) + MFLAG_IND

static _stackless_method _stackless_methtable[] = {
	/* from classobject.c */
	{&PyInstance_Type,		MFLAG_OFS(tp_call)},
	{&PyMethod_Type,		MFLAG_OFS(tp_call)},
	/* from descrobject.c */
	{&PyMethodDescr_Type,		MFLAG_OFS(tp_call)},
	{&PyClassMethodDescr_Type,	MFLAG_OFS(tp_call)},
	{&PyMethodWrapper_Type,		MFLAG_OFS(tp_call)},
	/* from funcobject.c */
	{&PyFunction_Type,		MFLAG_OFS(tp_call)},
	/* from genobject.c */
	{&PyGen_Type,			MFLAG_OFS(tp_iternext)},
	/* from methodobject.c */
	{&PyCFunction_Type,		MFLAG_OFS(tp_call)},
	/* from channelobject.c */
	{&PyChannel_TypePtr,		MFLAG_OFS_IND(tp_iternext)},
	{0, 0} /* sentinel */
};
