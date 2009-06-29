/*---------------------------------------------------------------------------------
	$Id: touch.c,v 1.5 2007/05/01 22:53:09 wntrmute Exp $

	touch screen input code

 	Copyright (C) 2005
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source
		distribution.

	$Log: touch.c,v $
	Revision 1.5  2007/05/01 22:53:09  wntrmute
	remove stupid mailBusy check
	
	Revision 1.4  2005/10/09 20:27:23  wntrmute
	add check for arm7 updating IPC
	
	Revision 1.3  2005/08/30 17:53:11  wntrmute
	only include required headers
	
	Revision 1.2  2005/08/23 17:06:10  wntrmute
	converted all endings to unix

	Revision 1.1  2005/08/04 17:58:30  wntrmute
	added touch co-ordinate wrapper



---------------------------------------------------------------------------------*/

//#include <nds.h>

#include <nds/ipc.h>
#include <nds/arm9/input.h>

//---------------------------------------------------------------------------------
touchPosition touchReadXY() {
//---------------------------------------------------------------------------------
	
	touchPosition touchPos;

	touchPos.x = IPC->touchX;
	touchPos.y = IPC->touchY;

	touchPos.px = IPC->touchXpx;
	touchPos.py = IPC->touchYpx;

	return touchPos;

}

