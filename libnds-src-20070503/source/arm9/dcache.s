/*---------------------------------------------------------------------------------
  $Id: dcache.s,v 1.1 2006/07/04 01:35:32 wntrmute Exp $

  Copyright (C) 2005
  	Michael Noland (joat)
  	Jason Rogers (dovoto)
  	Dave Murphy (WinterMute)

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you
     must not claim that you wrote the original software. If you use
     this software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and
     must not be misrepresented as being the original software.
  3. This notice may not be removed or altered from any source
     distribution.

  $Log: dcache.s,v $
  Revision 1.1  2006/07/04 01:35:32  wntrmute
  move cache.s to dcache.s

  Revision 1.5  2006/06/18 21:18:32  wntrmute
  *** empty log message ***

  Revision 1.4  2005/08/23 17:06:10  wntrmute
  converted all endings to unix

  Revision 1.3  2005/08/03 05:12:29  wntrmute
  adjusted header for logging

---------------------------------------------------------------------------------*/
#define ICACHE_SIZE	0x2000
#define DCACHE_SIZE	0x1000
#define CACHE_LINE_SIZE	32
//---------------------------------------------------------------------------------
	.arm
//---------------------------------------------------------------------------------
	.global DC_FlushAll
//---------------------------------------------------------------------------------
DC_FlushAll:
/*---------------------------------------------------------------------------------
	Clean and invalidate entire data cache
---------------------------------------------------------------------------------*/
	mov	r1, #0
outer_loop:
	mov	r0, #0
inner_loop:
	orr	r2, r1, r0			@ generate segment and line address
	mcr	p15, 0, r2, c7, c14, 2		@ clean and flush the line
	add	r0, r0, #CACHE_LINE_SIZE
	cmp	r0, #DCACHE_SIZE/4
	bne	inner_loop
	add	r1, r1, #0x40000000
	cmp	r1, #0
	bne	outer_loop
	bx	lr

//---------------------------------------------------------------------------------
	.global DC_FlushRange
//---------------------------------------------------------------------------------
DC_FlushRange:
/*---------------------------------------------------------------------------------
	Clean and invalidate a range
---------------------------------------------------------------------------------*/
	add	r1, r1, r0
	bic	r0, r0, #(CACHE_LINE_SIZE - 1)
.flush:
	mcr	p15, 0, r0, c7, c14, 1		@ clean and flush address
	add	r0, r0, #CACHE_LINE_SIZE
	cmp	r0, r1
	blt	.flush
	bx	lr

//---------------------------------------------------------------------------------
	.global DC_InvalidateAll
//---------------------------------------------------------------------------------
DC_InvalidateAll:
/*---------------------------------------------------------------------------------
	Clean and invalidate entire data cache
---------------------------------------------------------------------------------*/
	mov	r0, #0
	mcr	p15, 0, r0, c7, c6, 0
	bx	lr

//---------------------------------------------------------------------------------
	.global DC_InvalidateRange
//---------------------------------------------------------------------------------
DC_InvalidateRange:
/*---------------------------------------------------------------------------------
	Invalidate a range
---------------------------------------------------------------------------------*/
	add	r1, r1, r0
	bic	r0, r0, #CACHE_LINE_SIZE - 1
.invalidate:
	mcr	p15, 0, r0, c7, c6, 1
	add	r0, r0, #CACHE_LINE_SIZE
	cmp	r0, r1
	blt	.invalidate
	bx	lr
