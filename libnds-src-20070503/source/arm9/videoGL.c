/*---------------------------------------------------------------------------------
	$Id: videoGL.c,v 1.35 2007/04/02 07:44:32 gabebear Exp $

	Video API vaguely similar to OpenGL

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

	$Log: videoGL.c,v $
	Revision 1.35  2007/04/02 07:44:32  gabebear
	- MATRIX_READ_MODELVIEW is MATRIX_READ_CLIP
	- add defines for fog
	
	Revision 1.34  2007/03/18 17:12:32  gabebear
	updated glInit so that it calls glFlush once to prime the vertex/polygon buffers
	
	Revision 1.33  2007/02/22 06:24:25  gabebear
	-  changed glClearColor() so that it handles alpha as a fourth argument; the real OpenGL function takes alpha as a fourth argument.
	-  got rid of glClearAlpha() because glClearColor now handles this, glClearAlpha() was only recently added.
	- changed all array/pointer parameters that are constant in their functions to const so that you can pass arrays of constant data to those functions.
	- fixed glResetMatrixStack(); if a push or pop had just been executed it didn't work properly.
	- fixed some comments
	
	Revision 1.32  2007/02/11 13:18:26  wntrmute
	correct doxygen errors
	use GL_TEXTURE_TYPE_ENUM as type, not parameter
	
	Revision 1.31  2007/02/10 16:30:57  gabebear
	masks were glClearColor(), glClearAlpha(), and glClearPolyID() were wrong
	
	Revision 1.30  2007/02/10 16:01:10  gabebear
	- fixed some comments
	- removed CVS revision comments from videoGL.h and videoGL.c that were dated 2005 and older
	
	Revision 1.29  2007/02/10 05:13:46  gabebear
	- emptied videoGL.inl into videoGL.h so that doxygen works and because having the ability to remove inline functions isn't important anymore(I think).
	- emptied inlineGL.c; inlineGL.c and videoGL.inl should be deleted
	- inlined most functions. GCC "should" be able to work out if something that is inlined in the code really shouldn't be and create object code for the function. I believe I made pretty sane choices so I doubt this will be an issue.
	- fixed TEXTURE_PACK so that u and v are correct.
	- removed glTexCoord1i because it's not terribly useful and the fix to TEXTURE_PACK will make programs that use this and another glTexCoord fail, this was the only function that had S and T mapped correctly. The function does map to the OpenGL call pretty well, but it wasn't used properly in the examples. In the examples it was used to set two texcoords, which was more confusing than just using GFX_TEX_COORD directly. The OpenGL version only sets the S texcoord and sets T to 0.
	- GL_LIGHTING was defined, I deleted it. GL_LIGHTING is used with glEnable to swap between coloring vertices with normals/materials and coloring with the current color. The DS doesn't handle vertex colors like this. The color of a vertex on the DS is determined by the last command that computes a vertex color. The two commands that set vertex colors are glNormal() and glColor(), whichever was last executed determines the color of the vertex drawn. Implementing the OpenGL way of doing this on the DS would be a rather big waste of time and the current define didn't make any sense anyway.
	- removed glTranslate3f32(delta) because it doesn't map to an OpenGL call, or even make much sense. I doubt anyone was using it.
	- removed all depreciated functions
	- linking issues kept me from typing the sizeX/sizeY params on glTexImage2d() and glTexParameter() to GL_TEXTURE_SIZE_ENUM. I'm not sure how to fix this without inlining glTexImage2d() and glTexParameter().
	
	Revision 1.28  2007/01/31 22:57:28  gabebear
	- corrected typo where glClearAlpha() was glClearAplpha()
	- added doxygen for glClear*** functions
	
	Revision 1.27  2007/01/30 00:15:48  gabebear
	 - got rid of extra flags in glEnable and glDisable
	 - added glInit() which does pretty much the same thing that glReset did. It just initializes the GL-state at the start of the program and then never needs called again. Initializing the state explicitly should make code more stable when using different boot methods that may have fiddled with default states.
	 - depreciated glReset, because glInit does the same job better, setting up everythign per frame was a waste
	 - glInit sets up the read-plane(a.ka. clear-color) as blank instead of a bmp, and sets it's so that it is totally opaque with a poly-ID of zero. This lets antialiasing, and outlining work with simple glEnables!!!
	 - Changed glClearColor so that it sets the rear-plane ID instead of palette[0]
	 - added glClearAlpha() that sets the alpha of the rear-plane
	 - added glClearPolyID() that sets how things get outlined and antialiased
	
	I haven't tested this against the examples, another patch is on the way to fix the examples
	
	Revision 1.26  2007/01/20 00:30:48  dovoto
	Updated the glTexCoord2f() to pull texture size from state...texture sizes other than 128x128 should now work with the floating point version.
	
	Revision 1.25  2007/01/14 11:31:22  wntrmute
	bogus fixed types removed from libnds
	
	Revision 1.24  2007/01/11 05:35:41  dovoto
	Applied gabebear patch # 1632896
	fix gluPickMatrix()
	- no float / f32 version because all the parameters will always be regular ints
	- it actually works now
	
	fix gluFrustrumf32() and gluFrustum()
	- rename to glFrustrum because this is a GL function, not GLU (I'm breaking stuff...)
	- no longer changes matrix mode to projection (it's useful for more than projection)
	- multiplies instead of loads
	
	fix glOrthof32()
	- no longer changes matrix mode to projection (it's useful for more than projection)
	- multiplies instead of loads
	
	fix glGetFixed()
	- correctly waits for graphics engine to stop before getting any matrix
	- added ability to get projection and modelview matrices
	- fixed projection matrix get (it was grabbing modelview)
	- getting projection or position matrices now uses the matrix stack to preserve
	the other matrix. Not many people use the matrix stack and you would normally
	only do this at the beginning of a program so I doubt it will be a problem.
	
	Revision 1.23  2006/08/03 04:59:08  dovoto
	Added gluPickMatrix... (untested)
	
	Revision 1.22  2006/05/08 03:23:32  dovoto
	*** empty log message ***
	
	Revision 1.20  2006/05/08 03:19:51  dovoto
	Added glGetTexturePointer which allows the user to retreive a pointer to texture memory for the named texture.
	
	Small fix to the texture reset code.  The texture name is now reset as well.
	
	Revision 1.19  2006/01/05 08:13:26  dovoto
	Fixed gluLookAt (again)
	Major update to palette handling (likely a breaking change if you were using the gl texture palettes from before)
	

---------------------------------------------------------------------------------*/

#include <nds/jtypes.h>
#include <nds/memory.h>
#include <nds/bios.h>
#include <nds/arm9/math.h>
#include <nds/arm9/video.h>
#include <nds/arm9/videoGL.h>
#include <nds/arm9/trig_lut.h>

// holds the current state of the clear color register, initialized in glInit()
static uint32 clear_bits = 0;


//---------------------------------------------------------------------------------
// kinda big to be inlined, so it isn't
void glRotatef32i(int angle, int32 x, int32 y, int32 z) {
//---------------------------------------------------------------------------------

	int32 axis[3];
	int32 sine = SIN[angle &  LUT_MASK];
	int32 cosine = COS[angle & LUT_MASK];
	int32 one_minus_cosine = inttof32(1) - cosine;

	axis[0]=x;
	axis[1]=y;
	axis[2]=z;

	normalizef32(axis);   // should require passed in normalized?

	MATRIX_MULT3x3 = cosine + mulf32(one_minus_cosine, mulf32(axis[0], axis[0]));
	MATRIX_MULT3x3 = mulf32(one_minus_cosine, mulf32(axis[0], axis[1])) - mulf32(axis[2], sine);
	MATRIX_MULT3x3 = mulf32(mulf32(one_minus_cosine, axis[0]), axis[2]) + mulf32(axis[1], sine);

	MATRIX_MULT3x3 = mulf32(mulf32(one_minus_cosine, axis[0]),  axis[1]) + mulf32(axis[2], sine);
	MATRIX_MULT3x3 = cosine + mulf32(mulf32(one_minus_cosine, axis[1]), axis[1]);
	MATRIX_MULT3x3 = mulf32(mulf32(one_minus_cosine, axis[1]), axis[2]) - mulf32(axis[0], sine);

	MATRIX_MULT3x3 = mulf32(mulf32(one_minus_cosine, axis[0]), axis[2]) - mulf32(axis[1], sine);
	MATRIX_MULT3x3 = mulf32(mulf32(one_minus_cosine, axis[1]), axis[2]) + mulf32(axis[0], sine);
	MATRIX_MULT3x3 = cosine + mulf32(mulf32(one_minus_cosine, axis[2]), axis[2]);

}




//---------------------------------------------------------------------------------
void glMaterialf(GL_MATERIALS_ENUM mode, rgb color) {
//---------------------------------------------------------------------------------
	static uint32 diffuse_ambient = 0;
	static uint32 specular_emission = 0;

	switch(mode) {

		case GL_AMBIENT:
			diffuse_ambient = (color << 16) | (diffuse_ambient & 0xFFFF);
			break;

		case GL_DIFFUSE:
			diffuse_ambient = color | (diffuse_ambient & 0xFFFF0000);
			break;

		case GL_AMBIENT_AND_DIFFUSE:
			diffuse_ambient= color + (color << 16);
			break;

		case GL_SPECULAR:
			specular_emission = color | (specular_emission & 0xFFFF0000);
			break;

		case GL_SHININESS:
			break;

		case GL_EMISSION:
			specular_emission = (color << 16) | (specular_emission & 0xFFFF);
			break;
	}

	GFX_DIFFUSE_AMBIENT = diffuse_ambient;
	GFX_SPECULAR_EMISSION = specular_emission;
}

//---------------------------------------------------------------------------------
void glInit(void) {
//---------------------------------------------------------------------------------
	while (GFX_STATUS & (1<<27)); // wait till gfx engine is not busy
  
	// Clear the FIFO
	GFX_STATUS |= (1<<29);

	// Clear overflows from list memory
	glResetMatrixStack();
	
	// prime the vertex/polygon buffers
	glFlush(0);

	// reset the control bits
	GFX_CONTROL = 0;

	// reset the rear-plane(a.k.a. clear color) to black, ID=0, and opaque
	glClearColor(0,0,0,31);
	glClearPolyID(0);

	// reset stored texture locations
	glResetTextures();
	
	// reset the depth to it's max
	glClearDepth(GL_MAX_DEPTH);

	GFX_TEX_FORMAT = 0;
	GFX_POLY_FORMAT = 0;
  
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
}

//---------------------------------------------------------------------------------
// Texture
//---------------------------------------------------------------------------------
// Texture globals
//---------------------------------------------------------------------------------

uint32 textures[MAX_TEXTURES];

uint32 activeTexture = 0;

uint32* nextBlock = (uint32*)0x06800000;
uint32  nextPBlock = 0;
int nameCount = 1;
//---------------------------------------------------------------------------------
void glResetTextures(void) {
//---------------------------------------------------------------------------------
	activeTexture = 0;
	nextBlock = (uint32*)0x06800000;
	nextPBlock = 0;
	nameCount = 1;
}

//---------------------------------------------------------------------------------
//	glGenTextures creates integer names for your table
//	takes n as the number of textures to generate and 
//	a pointer to the names array that it needs to fill.
//  Returns 1 if succesful and 0 if out of texture names
//---------------------------------------------------------------------------------

int glGenTextures(int n, int *names) {
//---------------------------------------------------------------------------------
	

	int index = 0;

	for(index = 0; index < n; index++) {
		if(nameCount >= MAX_TEXTURES)
			return 0;
		else
			names[index] = nameCount++;
	}

	return 1;
}

//---------------------------------------------------------------------------------
// glBindTexure sets the current named
//	texture to the active texture.  Target 
//	is ignored as all DS textures are 2D
//---------------------------------------------------------------------------------
void glBindTexture(int target, int name) {
//---------------------------------------------------------------------------------
	if (name == 0) 
		GFX_TEX_FORMAT = 0; 
	else 
		GFX_TEX_FORMAT = textures[name]; 

	
	activeTexture = name;
}
//---------------------------------------------------------------------------------
// glColorTable establishes the location of the current palette.
//	Roughly follows glColorTableEXT. Association of palettes with 
//	named textures is left to the application. 
//---------------------------------------------------------------------------------
void glColorTable( uint8 format, uint32 addr ) {
//---------------------------------------------------------------------------------
	GFX_PAL_FORMAT = addr>>(4-(format==GL_RGB4));
}
                     
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
void glTexCoord2f32(int32 u, int32 v) { 
//---------------------------------------------------------------------------------
  int x, y; 
   
  x = ((textures[activeTexture]) >> 20) & 7; 
  y = ((textures[activeTexture]) >> 23) & 7; 

  glTexCoord2t16(f32tot16 (mulf32(u,inttof32(8<<x))), f32tot16 (mulf32(v,inttof32(8<<y)))); 
}

//---------------------------------------------------------------------------------
void glTexCoord2f(float s, float t) {
//---------------------------------------------------------------------------------
	int x = ((textures[activeTexture]) >> 20) & 7; 
    int y = ((textures[activeTexture]) >> 23) & 7; 
    
    glTexCoord2t16(floattot16(s*(8 << x)), floattot16(t*(8<<y)));
}

//---------------------------------------------------------------------------------
// glTexParameter although named the same 
//	as its gl counterpart it is not compatible
//	Effort may be made in the future to make it so.
//---------------------------------------------------------------------------------
void glTexParameter(	uint8 sizeX, uint8 sizeY,
						const uint32* addr,
						GL_TEXTURE_TYPE_ENUM mode,
						uint32 param) {
//---------------------------------------------------------------------------------
	textures[activeTexture] = param | (sizeX << 20) | (sizeY << 23) | (((uint32)addr >> 3) & 0xFFFF) | (mode << 26);
}
//---------------------------------------------------------------------------------
//glGetTexturePointer gets a pointer to vram which contains the texture
//
//---------------------------------------------------------------------------------
void* glGetTexturePointer(	int name) {
//---------------------------------------------------------------------------------
	return (void*) ((textures[name] & 0xFFFF) << 3);
}

//---------------------------------------------------------------------------------
u32 glGetTexParameter(){
//---------------------------------------------------------------------------------
  return textures[activeTexture];
}


//---------------------------------------------------------------------------------
inline uint32 alignVal( uint32 val, uint32 to )
{
  return (val & (to-1))? (val & ~(to-1)) + to : val;
}

//---------------------------------------------------------------------------------
int getNextPaletteSlot(u16 count, uint8 format){
//---------------------------------------------------------------------------------
  // ensure the result aligns on a palette block for this format
  uint32 result = alignVal(nextPBlock, 1<<(4-(format==GL_RGB4)));
  
  // convert count to bytes and align to next (smallest format) palette block
  count = alignVal( count<<1, 1<<3 ); 

  // ensure that end is within palette video mem
  if( result+count > 0x10000 )   // VRAM_F - VRAM_E
    return -1;

  nextPBlock = result+count;
  return (int)result;
} 

//---------------------------------------------------------------------------------
uint16* vramGetBank(uint16 *addr) {
//---------------------------------------------------------------------------------
	if(addr >= VRAM_A && addr < VRAM_B)
		return VRAM_A;
	else if(addr >= VRAM_B && addr < VRAM_C)
		return VRAM_B;
	else if(addr >= VRAM_C && addr < VRAM_D)
		return VRAM_C;
	else if(addr >= VRAM_D && addr < VRAM_E)
		return VRAM_D;
	else if(addr >= VRAM_E && addr < VRAM_F)
		return VRAM_E;
	else if(addr >= VRAM_F && addr < VRAM_G)
		return VRAM_F;
	else if(addr >= VRAM_G && addr < VRAM_H)
		return VRAM_H;
	else if(addr >= VRAM_H && addr < VRAM_I)
		return VRAM_H;
	else return VRAM_I;
}


//---------------------------------------------------------------------------------
int vramIsTextureBank(uint16 *addr) {
//---------------------------------------------------------------------------------
   uint16* vram = vramGetBank(addr);

   if(vram == VRAM_A)
   {
      if((VRAM_A_CR & 3) == ((VRAM_A_TEXTURE) & 3))
         return 1;
      else return 0;
   }
   else if(vram == VRAM_B)
   {
      if((VRAM_B_CR & 3) == ((VRAM_B_TEXTURE) & 3))
         return 1;
      else return 0;
   }
   else if(vram == VRAM_C)
   {
      if((VRAM_C_CR & 3) == ((VRAM_C_TEXTURE) & 3))
         return 1;
      else return 0;
   }
   else if(vram == VRAM_D)
   {
      if((VRAM_D_CR & 3) == ((VRAM_D_TEXTURE) & 3))
         return 1;
      else return 0;
   }
   else
      return 0;
   
} 
//---------------------------------------------------------------------------------
uint32* getNextTextureSlot(int size) {
//---------------------------------------------------------------------------------
   uint32* result = nextBlock;
   nextBlock += size >> 2;

   //uh-oh...out of texture memory in this bank...find next one assigned to textures
   while(!vramIsTextureBank((uint16*)nextBlock - 1) && nextBlock <= (uint32*)VRAM_E)
   {
      nextBlock = (uint32*)vramGetBank((uint16*)result) + (0x20000 >> 2); //next bank
      result = nextBlock;        
      nextBlock += size >> 2;
   }

   if(nextBlock > (uint32*)VRAM_E)
      return 0;

   else return result;   

} 

//---------------------------------------------------------------------------------
// Similer to glTextImage2D from gl it takes a pointer to data
//  Empty fields and target are unused but provided for code compatibility.
//  type is simply the texture type (GL_RGB, GL_RGB8 ect...)
//---------------------------------------------------------------------------------
int glTexImage2D(int target, int empty1, GL_TEXTURE_TYPE_ENUM type, int sizeX, int sizeY, int empty2, int param, const uint8* texture) {
//---------------------------------------------------------------------------------
  uint32 size = 0;
  uint32* addr;
  uint32 vramTemp;

  size = 1 << (sizeX + sizeY + 6);
  

  switch (type) {
    case GL_RGB:
    case GL_RGBA:
      size = size << 1;
      break;
    case GL_RGB4:
      size = size >> 2;
      break;
    case GL_RGB16:
      size = size >> 1;
      break;

    default:
      break;
  }
  
  addr = getNextTextureSlot(size);
  
  if(!addr)
    return 0;

  // unlock texture memory
  vramTemp = vramSetMainBanks(VRAM_A_LCD,VRAM_B_LCD,VRAM_C_LCD,VRAM_D_LCD);

  if (type == GL_RGB) {
    // We do GL_RGB as GL_RGBA, but we set each alpha bit to 1 during the copy
    u16 * src = (u16*)texture;
    u16 * dest = (u16*)addr;
    
    glTexParameter(sizeX, sizeY, addr, GL_RGBA, param);
    
    while (size--) {
      *dest++ = *src | (1 << 15);
      src++;
    }
  } else {
    // For everything else, we do a straight copy
    glTexParameter(sizeX, sizeY, addr, type, param);
    swiCopy((uint32*)texture, addr , size / 4 | COPY_MODE_WORD);
  }

  vramRestoreMainBanks(vramTemp);

  return 1;
}

 //---------------------------------------------------------------------------------
void glTexLoadPal(const u16* pal, u16 count, u32 addr) {
 //---------------------------------------------------------------------------------
 	vramSetBankE(VRAM_E_LCD);
 		
	swiCopy( pal, &VRAM_E[addr>>1] , count / 2 | COPY_MODE_WORD);
 
 	vramSetBankE(VRAM_E_TEX_PALETTE);
}
 
//---------------------------------------------------------------------------------
int gluTexLoadPal(const u16* pal, u16 count, uint8 format) {
//---------------------------------------------------------------------------------
  int addr = getNextPaletteSlot(count, format);
  
  if( addr>=0 )
    glTexLoadPal(pal, count, (u32) addr);
  
  return addr;
 }

//---------------------------------------------------------------------------------
void glGetInt(GL_GET_ENUM param, int* i) {
//---------------------------------------------------------------------------------
  switch (param) {
    case GL_GET_POLYGON_RAM_COUNT:
      *i = GFX_POLYGON_RAM_USAGE;
      break;
    case GL_GET_VERTEX_RAM_COUNT:
      *i = GFX_VERTEX_RAM_USAGE;
      break;
    case GL_GET_TEXTURE_WIDTH:
      *i = 8 << (((textures[activeTexture]) >> 20) & 7);
      break;
    case GL_GET_TEXTURE_HEIGHT:
      *i = 8 << (((textures[activeTexture]) >> 23) & 7);
      break;
    default:
      break;
  }
}


//---------------------------------------------------------------------------------
void glClearColor(uint8 red, uint8 green, uint8 blue, uint8 alpha) {
//---------------------------------------------------------------------------------
	GFX_CLEAR_COLOR = clear_bits = (clear_bits & 0xFFE08000) | (0x7FFF & RGB15(red, green, blue)) | ((alpha & 0x1F) << 16);
}

//---------------------------------------------------------------------------------
void glClearPolyID(uint8 ID) {
//---------------------------------------------------------------------------------
	GFX_CLEAR_COLOR = clear_bits = (clear_bits & 0xC0FFFFFF) | (( ID & 0x3F ) << 24 );
}


