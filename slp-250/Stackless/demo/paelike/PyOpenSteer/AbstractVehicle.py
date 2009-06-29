"""
----------------------------------------------------------------------------

OpenSteer -- Steering Behaviors for Autonomous Characters

Copyright (c) 2002-2003, Sony Computer Entertainment America
Original author: Craig Reynolds <craig_reynolds@playstation.sony.com>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.


----------------------------------------------------------------------------

PyOpenSteer -- Port of OpenSteer to Python

Copyright (c) 2004 Lutz Paelike <lutz@fxcenter.de>

The license follows the original Opensteer license but must include 
this additional copyright notice.
----------------------------------------------------------------------------
"""

from LocalSpace import AbstractLocalSpace

class AbstractVehicle(AbstractLocalSpace):
	group = []
	def mass(self):
		""" """
		pass

	def setMass(self):
		""" """
		pass

	def radius(self):
		""" """
		pass

	def setRadius(self):
		""" """
		pass

	def velocity(self):
		""" """
		pass

	def speed(self):
		""" """
		pass

	def setSpeed(self):
		""" """
		pass
#
	def predictFuturePosition(self):
		""" """
		pass
		
	def maxForce(self):
		""" """
		pass

	def setMaxForce(self):
		""" """
		pass

	def velocity(self):
		""" """
		pass

	def maxSpeed(self):
		""" """
		pass

	def setMaxSpeed(self):
		""" """
		pass
