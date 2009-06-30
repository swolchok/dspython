cdef extern from "nds/touch.h":
	ctypedef struct touchPosition:
		short int rawx
		short int rawy
		short int px
		short int py
		short int z1
		short int z2

cdef extern from "nds/arm7/touch.h":
	void touchReadXY(touchPosition*)

def wtouchReadXY():
	cdef touchPosition pos
	touchReadXY(&pos)
	return (pos.rawx, pos.rawy, pos.px, pos.py, pos.z1, pos.z2)
