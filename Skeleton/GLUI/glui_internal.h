/*
  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
  claim that you wrote the original software. If you use this software
  in a product, an acknowledgment in the product documentation would be
  appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
  misrepresented as being the original software.
  3. This notice may not be removed or altered from any source
*/
#ifndef GLUI_INTERNAL_H
#define GLUI_INTERNAL_H

#include <cstdio>
#include <cmath>
#include <cassert>

#if defined(GLUI_FREEGLUT)

  // FreeGLUT does not yet work perfectly with GLUI
  //  - use at your own risk.
  
  #include <GL/freeglut.h>

#elif defined(GLUI_OPENGLUT)

  // OpenGLUT does not yet work properly with GLUI
  //  - use at your own risk.
  
  #include <GL/openglut.h>

#else 

  #ifdef __APPLE__
  #include <GLUT/glut.h>
  #else
  #include "include\GL\glut.h"
  #endif

#endif

#ifndef AND
#define AND &&
#define OR  ||
#define NOT !
#endif

#ifndef ABS
#define ABS(a) ((a)>=0 ? (a) : (-(a)))
#endif

/********************  bit comparisons and operations ***************/
#ifndef TEST_BIT
#define TEST_BIT( x, b )   (((x) & (1<<(b))) != 0 )
#define SET_BIT( x, b )    ((x) |= (1 << (b)))
#define CLEAR_BIT( x, b )  ((x) &= ~(1 << (b)))
#define TOGGLE_BIT( x, b ) ((TEST_BIT(x,b)) ?(CLEAR_BIT(x,b)):(SET_BIT(x,b)))
#endif

#ifndef TEST_AND
#define TEST_AND( a, b ) ((a&b)==b)
#endif


#ifndef M_PI
#define M_PI 3.141592654
#endif

/*********** flush the stdout and stderr output streams *************/
#ifndef flushout
#define flushout fflush(stdout)
#define flusherr fflush(stderr)
#endif

/********** Debugging functions *************************************/
#ifndef error_return
#define error_return( c ); {fprintf(stderr,c);return;}
#endif

/************************* floating-point random ********************/
#ifndef randf
#define randf()  ((float) rand() / (float)RAND_MAX )
#endif

#ifndef SIGN
#define SIGN(x) ((x)>=0 ?  1  :  -1)
#endif

/****************** conversion between degrees and radians **********/
#ifndef DEG2RAD
#define DEG2RAD(x) ((x)/180.0*M_PI)
#define RAD2DEG(x) ((x)/M_PI*180.0)
#endif

/***************** clamp a value to some fixed interval *************/
#ifndef CLAMP
#define CLAMP(x,lo,hi)  {if ((x) < (lo)) {(x)=(lo);} else if((x) > (hi)) {(x)=(hi);}}
#endif

/************ check if a value lies within a closed interval *********/
#ifndef IN_BOUNDS
#define IN_BOUNDS( x, lo, hi ) ( (x) >= (lo) AND (x) <= (hi) )
#endif

/************ check if a 2D point lies within a 2D box ***************/
#ifndef PT_IN_BOX
#define PT_IN_BOX( x, y, lo_x, hi_x, lo_y, hi_y ) \
( IN_BOUNDS(x,lo_x,hi_x) AND IN_BOUNDS(y,lo_y,hi_y) )
#endif

/****** check if value lies on proper side of another value     *****/
/*** if side is positive => proper side is positive, else negative **/
#ifndef CHECK_PROPER_SIDE
#define CHECK_PROPER_SIDE(x,val,side) ((side) > 0 ? (x) > (val) : (x) < (val))
#endif


/***** Small value when we want to do a comparison to 'close to zero' *****/
#ifndef FUDGE
#define FUDGE .00001
#endif


/******************* swap two values, using a temp variable *********/
#ifndef SWAP2
#define SWAP2(a,b,t) {t=a;a=b;b=t;}
#endif

#define VEC3_TO_ARRAY(v,a)  a[0]=v[0], a[1]=v[1], a[2]=v[2]

/**** Return the ASCII control code given the non-control ASCII character */
#define CTRL(c) ( (c>=('a'-1)) ? (c-'a'+1) : (c-'A'+1) )

#endif /* GLUI_INTERNAL_H */
