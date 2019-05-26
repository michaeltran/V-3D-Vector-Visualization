#include <stdio.h>
	// yes, I know stdio.h is not good C++, but I like the *printf()
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include <iostream>
#include "dat.h"
#include "temperature.h"
#include "streamlines.h"
#include "colors.h"

// You need to adjust the location of these header files according to your configuration

#include <windows.h>
//#include <freeglut.h>
#include "GL\include\GL.H"
#include "GL\include\glu.h"
#include "GL\include\glut.h"
#include "GL\include\glui.h"

#include "Skeleton.h"
#include <time.h>
//#include "glui.h"

//
//
//	This is a sample OpenGL / GLUT / GLUI program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with radio buttons
//
//	The left mouse button allows rotation
//	The middle mouse button allows scaling
//	The glui window allows:
//		1. The 3d object to be transformed
//		2. The projection to be changed
//		3. The color of the axes to be changed
//		4. The axes to be turned on and off
//		5. The transformations to be reset
//		6. The program to quit
//
//	Author: Joe Graphics
//


//
// constants:
//
// NOTE: There are a bunch of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch() statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch() statements.  Those are #defines.
//
//


// title of these windows:
const char *WINDOWTITLE = { "Final Project -- Michael Tran" };
const char *GLUITITLE   = { "User Interface Window" };


// what the glui package defines as true and false:
const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };


// the escape key:
#define ESCAPE		0x1b


// initial window size:
const int INIT_WINDOW_SIZE = { 700 };


// size of the box:
const float BOXSIZE = { 2.f };



// multiplication factors for input interaction:
//  (these are known from previous experience)
const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


// able to use the left mouse for either rotation or scaling,
// in case have only a 2-button mouse:
enum LeftButton
{
	ROTATE,
	SCALE
};


// minimum allowable scale factor:
const float MINSCALE = { 0.05f };


// active mouse buttons (or them together):
const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };


// which projection:
enum Projections
{
	ORTHO,
	PERSP
};


// which button:
enum ButtonVals
{
    RECALCULATE,
	RESET,
	QUIT
};


// window background color (rgba):
const float BACKCOLOR[] = { 0., 0., 0., 0. };


// line width for the axes:
const GLfloat AXES_WIDTH   = { 3. };


// the color numbers:
// this order must match the radio button order
enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};


// the object numbers:
enum MODELS
{
    STREAMLINES,
    DYNAMIC_TEMPERATURE,
    TEMPERATURE1,
    TEMPERATURE2,
    DIESEL_FIELD1,
    DISTANCE_FIELD1,
    DISTANCE_FIELD2,
    ICELAND_CURRENT_FIELD,
    TORUS_FIELD,
};

enum MODEL_TYPE
{
    DAT_TYPE,
    PLY_TYPE,
    DYNAMIC_TYPE,
    STREAMLINE_TYPE,
};

enum STREAMRIBBON_TYPE
{
    EQUI_DISTANCE_STREAMRIBBON,
    REGULAR_STREAMRIBBON,
};

// the color definitions:
// this order must match the radio button order
const GLfloat Colors[8][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};


// fog parameters:
const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };



//
// non-constant global variables:
//
int	    ActiveButton;	// current button that is down
GLuint	AxesList;		// list to hold the axes
int	    AxesOn;			// != 0 means to draw the axes
int     NonLinearOn;
int	    DebugOn;		// != 0 means to print debugging info
int	    DepthCueOn;		// != 0 means to use intensity depth cueing
GLUI *	Glui;			// instance of glui window
int	    GluiWindow;		// the glut id for the glui window
int	    LeftButton;		// either ROTATE or SCALE
GLuint	BoxList;		// object display list
int	    MainWindow;		// window id for main graphics window
GLfloat	RotMatrix[4][4];// set by glui rotation widget
float	Scale, Scale2;	// scaling factors
int	    WhichColor;		// index into Colors[]
int	    WhichProjection;// ORTHO or PERSP
int	    Xmouse, Ymouse;	// mouse values
float	Xrot, Yrot;		// rotation angles in degrees
float	TransXYZ[3];	// set by glui translation widgets


//
// function prototypes:
//
void	Animate( void );
void	Buttons( int );
void	Display( void );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( void );
void	InitGlui( void );
void	InitGraphics( void );
void	InitLists( void );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( void );
void	Resize( int, int );
void	Visibility( int );

void	Arrow( float [3], float [3], float );
void	Cross( float [3], float [3], float [3] );
float	Dot( float [3], float [3] );
float	Unit( float [3], float [3] );
void	Axes( float );

//void    Display_Model(void);
void    set_view(GLenum mode);
//void    set_scene(GLenum mode, Polyhedron *poly);
void    display_shape_st(GLenum mode, streamlines *data);
void    display_shape_dt(GLenum mode, temperature *temp);
void    display_shape_dat(GLenum mode, dat *data);
void    display_shape(GLenum mode, Polyhedron *this_poly);
void    Choose_Object();
void    Choose_Color_Scheme();
void    Choose_Color_Scheme_MidPoint();
void    Choose_IsoContour_ScalarValue();
void    Choose_IsoContour_NumberOfContours();
void    Choose_XY_Index();
void    Choose_XZ_Index();
void    Choose_YZ_Index();
void    Choose_Smin();
void    Choose_Smax();
void    Choose_Opacity();
void    Choose_FieldId();
void    Choose_NumericalIntegration();
void    Choose_PQValue();
void    Choose_NULL();
void    MarchingTriangles_FindIntersection(float s_star, Vertex &vert0, Vertex &vert1, int &intersections, std::vector<node> &intersection_locations);
//void    MarchingSquares_FindIntersection(float s_star, node vert0, node vert1, int &intersections, std::vector<node> &dat_vector);
void    MarchingSquares_DrawIntersection(node edge1_intersection, node edge2_intersection);
void    MarchingSquares_DrawIntersection2(intersection_temperature edge1_intersection, intersection_temperature edge2_intersection);
void    DetermineVisibility();

double radius_factor = 0.9;
int display_mode = 0; 

int ObjectId = STREAMLINES;
int ColorSchemeId = RAINBOW;
int FieldId = FIELD3;
int NumericalIntegrationTypeId = RK4;
float ColorSchemeMidPoint = 0.5;
float ScalarValue = 76.141;
int NumberOfContours = 1;
char object_name[128]= "streamlines";
int object_type = STREAMLINE_TYPE;
int streamribbon_type = EQUI_DISTANCE_STREAMRIBBON;


float S_min = 0;
float S_max = 1000;
float X_min = -1;
float X_max = 1;
float Y_min = -1;
float Y_max = 1;
float Z_min = -1;
float Z_max = 1;
float Opacity_max = 0.1;
float Previous_Opacity = 0;
int BilinearOn = 0;
int IsosurfaceOn = 1;
int VolumeRenderingOn = 1;
int DirectOn = 1;
float DirectOpacity = 0.5;
int StreamlineOn = 0;
int p_value = 4;
int q_value = 4;
int EntropyStreamlinesOn = 0;
int StreamlineProbeOn = 0;
int SpatialClustersOn = 0;
int ShapeClustersOn = 0;
int FinalStreamlinesOn = 0;
int FinalStreamTapesOn = 0;
int IncludeColorOn = 0;
int IncludeTorsionOn = 0;
float ProbeTransXYZ[3];
int StreamribbonProbeOn = 0;
float StreamribbonProbeXYZ[3];

float Gradient_min = 0;
float Gradient_max = 0;

int XY_on = 1;
int XZ_on = 1;
int YZ_on = 1;

Polyhedron *poly = NULL;
//extern PlyFile *in_ply;
//extern FILE *this_file;

dat *data = NULL;
temperature *temperature_data = NULL;
streamlines *streamline_data = NULL;
int NX = 10;
int NY = 10;
int NZ = 10;
int tNX = 64;
int tNY = 64;
int tNZ = 64;

int XY_index = NZ / 2;
int XZ_index = NY / 2;
int YZ_index = NZ / 2;

/* which way is a surface facing: */
const int MINUS = { 0 };
const int PLUS = { 1 };

/* what is the major direction: */
#define X_direction 0
#define Y_direction 1
#define Z_direction 2

int Major; /* X, Y, or Z */
int Xside, Yside, Zside; /* which side is visible, PLUS or MINUS */


//
// main program:
//
int main( int argc, char *argv[] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// Load the model and data here

    // DAT
    //object_type = DAT_TYPE;
    //data = new dat("../models/temperature1.dat");
    //data->Build_Edge_List();
    //data->Build_Face_List();
    //data->Build_Lines(ScalarValue, NumberOfContours);


    //data->Build_Temperatures();

    object_type = STREAMLINE_TYPE;
    streamline_data = new streamlines(NX, NY, NZ);
    streamline_data->get_entropy_data(FieldId);
    streamline_data->Calculate_Colors(ColorSchemeId, ColorSchemeMidPoint);
    streamline_data->CompositeXY(Opacity_max, S_min, S_max);
    streamline_data->CompositeXZ(Opacity_max, S_min, S_max);
    streamline_data->CompositeYZ(Opacity_max, S_min, S_max);
    streamline_data->GetEntropyStreamlines(FieldId, NumericalIntegrationTypeId, p_value, q_value);
    streamline_data->get_streamtapes();

    //NX = 64;
    //NY = 64;
    //NZ = 64;

    //// DYNAMIC TYPE
    //object_type = DYNAMIC_TYPE;
    //temperature_data = new temperature(NX, NY, NZ, ColorSchemeId, ColorSchemeMidPoint);
    //temperature_data->GetGradientMinMax(Gradient_min, Gradient_max);
    //temperature_data->Build_Lines(ScalarValue, NumberOfContours);
    //temperature_data->CompositeXY(Opacity_max, S_min, S_max);
    //temperature_data->CompositeXZ(Opacity_max, S_min, S_max);
    //temperature_data->CompositeYZ(Opacity_max, S_min, S_max);

    // PLY
	//FILE *this_file = fopen("../models/diesel_field1.ply", "r");
	//poly = new Polyhedron (this_file);
	//fclose(this_file);
	////mat_ident( rotmat );	

	//poly->initialize(); // initialize everything

	//poly->calc_bounding_sphere();
	//poly->calc_face_normals_and_area();
	//poly->average_normals();


	// setup all the graphics stuff:

	InitGraphics();


	// create the display structures that will not change:

	InitLists();


	// init all the global variables used by Display():
	// this will also post a redisplay
	// it is important to call this before InitGlui()
	// so that the variables that glui will control are correct
	// when each glui widget is created

	Reset();


	// setup all the user interface stuff:

	InitGlui();


	// draw the scene once and wait for some interaction:
	// (will never return)

	glutMainLoop();

	// finalize the object if loaded

	if (poly != NULL)
		poly->finalize();

	// this is here to make the compiler happy:

	return 0;
}



//
// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display() from here -- let glutMainLoop() do it
//
void Animate( void )
{
	// put animation stuff in here -- change some global variables
	// for Display() to find:



	// force a call to Display() next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay();
}




//
// glui buttons callback:
//
void Buttons( int id )
{
	switch( id )
	{
		case RESET:
			Reset();
			Glui->sync_live();
			glutSetWindow( MainWindow );
			glutPostRedisplay();
			break;

		case QUIT:
			// gracefully close the glui window:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:

			Glui->close();
			glutSetWindow( MainWindow );
			glFinish();
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

        case RECALCULATE:
            if (object_type == STREAMLINE_TYPE)
            {
                streamline_data->get_entropy_data(FieldId);
                streamline_data->Calculate_Colors(ColorSchemeId, ColorSchemeMidPoint);
                streamline_data->CompositeXY(Opacity_max, S_min, S_max);
                streamline_data->CompositeXZ(Opacity_max, S_min, S_max);
                streamline_data->CompositeYZ(Opacity_max, S_min, S_max);
                streamline_data->GetEntropyStreamlines(FieldId, NumericalIntegrationTypeId, p_value, q_value);
                streamline_data->get_streamtapes();

                Display();
            }
            break;

		default:
			fprintf( stderr, "Don't know what to do with Button ID %d\n", id );
	}

}



//
// draw the complete scene:
//
void Display( void )
{
	GLsizei vx, vy, v;		// viewport dimensions
	GLint xl, yb;		// lower-left corner of viewport
	GLfloat scale2;		// real glui scale factor

	if( DebugOn != 0 )
	{
		fprintf( stderr, "Display\n" );
	}


	// set which window we want to do the graphics into:

	glutSetWindow( MainWindow );


	// erase the background:

	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );


	// specify shading to be flat:

	glShadeModel( GL_FLAT );

    glEnable(GL_COLOR_MATERIAL);

	// set the viewport to a square centered in the window:

	vx = glutGet( GLUT_WINDOW_WIDTH );
	vy = glutGet( GLUT_WINDOW_HEIGHT );
	v = vx < vy ? vx : vy;			// minimum dimension
	xl = ( vx - v ) / 2;
	yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D() IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	if( WhichProjection == ORTHO )
		glOrtho( -3., 3.,     -3., 3.,     0.1, 1000. );
	else
		gluPerspective( 90., 1.,	0.1, 1000. );


	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();


	// set the eye position, look-at position, and up-vector:
	// IF DOING 2D, REMOVE THIS -- OTHERWISE ALL YOUR 2D WILL DISAPPEAR !

	gluLookAt( 0., 0., 3.,     0., 0., 0.,     0., 1., 0. );


	// translate the objects in the scene:
	// note the minus sign on the z value
	// this is to make the appearance of the glui z translate
	// widget more intuitively match the translate behavior
	// DO NOT TRANSLATE IN Z IF YOU ARE DOING 2D !

	glTranslatef( (GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2] );


	// rotate the scene:
	// DO NOT ROTATE (EXCEPT ABOUT Z) IF YOU ARE DOING 2D !

	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );
	glMultMatrixf( (const GLfloat *) RotMatrix );


	// uniformly scale the scene:

	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );
	scale2 = 1. + Scale2;		// because glui translation starts at 0.
	if( scale2 < MINSCALE )
		scale2 = MINSCALE;
	glScalef( (GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2 );


	// set the fog parameters:
	// DON'T NEED THIS IF DOING 2D !

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	// Let us disable lighting right now
	glDisable(GL_LIGHTING);


	// possibly draw the axes:

	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[WhichColor][0] );
		glCallList( AxesList );
	}


	// set the color of the object:

	glColor3fv( Colors[WhichColor] );


	// Render the loaded object
    if (object_type == PLY_TYPE)
    {
        set_view(GL_RENDER);

        glTranslatef(0.0, 0.0, -3.0);

        glTranslatef((GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2]);

        glTranslatef(poly->center.entry[0], poly->center.entry[1], poly->center.entry[2]);

        glScalef((GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2);

        glRotatef((GLfloat)Yrot, 0., 1., 0.);
        glRotatef((GLfloat)Xrot, 1., 0., 0.);
        glMultMatrixf((const GLfloat *)RotMatrix);


        glScalef(1.0 / poly->radius, 1.0 / poly->radius, 1.0 / poly->radius);
        glTranslatef(-poly->center.entry[0], -poly->center.entry[1], -poly->center.entry[2]);

        display_shape(GL_RENDER, poly);
    }
    else if (object_type == DAT_TYPE)
    {
        set_view(GL_RENDER);

        glTranslatef(0.0, 0.0, -3.0);
        glTranslatef((GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2]);

        glScalef((GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2);

        glRotatef((GLfloat)Yrot, 0., 1., 0.);
        glRotatef((GLfloat)Xrot, 1., 0., 0.);
        glMultMatrixf((const GLfloat *)RotMatrix);

        display_shape_dat(GL_RENDER, data);
    }
    else if (object_type == DYNAMIC_TYPE)
    {
        set_view(GL_RENDER);

        glTranslatef(0.0, 0.0, -4.0);
        glTranslatef((GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2]);

        glScalef((GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2);

        glRotatef((GLfloat)Yrot, 0., 1., 0.);
        glRotatef((GLfloat)Xrot, 1., 0., 0.);
        glMultMatrixf((const GLfloat *)RotMatrix);

        display_shape_dt(GL_RENDER, temperature_data);
    }
    else if (object_type == STREAMLINE_TYPE)
    {
        set_view(GL_RENDER);

        glTranslatef(0.0, 0.0, -4.0);
        glTranslatef((GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2]);

        glScalef((GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2);

        glRotatef((GLfloat)Yrot, 0., 1., 0.);
        glRotatef((GLfloat)Xrot, 1., 0., 0.);
        glMultMatrixf((const GLfloat *)RotMatrix);

        display_shape_st(GL_RENDER, streamline_data);
    }


	// swap the double-buffered framebuffers:

	glutSwapBuffers();


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush() here, not glFinish() !

	glFlush();
}



//
// use glut to display a string of characters using a raster font:
//
void DoRasterString( float x, float y, float z, char *s )
{
	char c;			// one character to print

	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}



//
// use glut to display a string of characters using a stroke font:
//
void DoStrokeString( float x, float y, float z, float ht, char *s )
{
	char c;			// one character to print
	float sf;		// the scale factor

	glPushMatrix();
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		sf = ht / ( 119.05 + 33.33 );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix();
}



//
// return the number of seconds since the start of the program:
//
float ElapsedSeconds( void )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.;
}



//
// initialize the glui window:
//
void InitGlui( void )
{
    GLUI_Panel *panel;
    GLUI_RadioGroup *group;
    GLUI_Rotation *rot;
    GLUI_Translation *trans, *scale;


    // setup the glui window:

    glutInitWindowPosition( INIT_WINDOW_SIZE + 9, 0 );
    Glui = GLUI_Master.create_glui( (char *) GLUITITLE );


    Glui->add_statictext( (char *) GLUITITLE );
    Glui->add_separator();

    Glui->add_checkbox( "Axes", &AxesOn );

    Glui->add_checkbox( "Perspective", &WhichProjection );

    Glui->add_checkbox( "Intensity Depth Cue", &DepthCueOn );

    // Add a rollout for the axes color
    GLUI_Rollout *rollout = Glui->add_rollout(" Axes Color ", 0);
    {
        //panel = Glui->add_panel(  "Axes Color" );
        //GLUI_Rollout *rollout = Glui->add_rollout_to_panel(panel,  "Axes Color", 1 );
        //group = Glui->add_radiogroup_to_panel( panel, &WhichColor );

        group = Glui->add_radiogroup_to_panel(rollout, &WhichColor);
        {
            Glui->add_radiobutton_to_group(group, "Red");
            Glui->add_radiobutton_to_group(group, "Yellow");
            Glui->add_radiobutton_to_group(group, "Green");
            Glui->add_radiobutton_to_group(group, "Cyan");
            Glui->add_radiobutton_to_group(group, "Blue");
            Glui->add_radiobutton_to_group(group, "Magenta");
            Glui->add_radiobutton_to_group(group, "White");
            Glui->add_radiobutton_to_group(group, "Black");
        }
    }

	// Add a list for the different models
	//rollout = Glui->add_rollout(" Models ", 0);
    panel = Glui->add_panel("Choose object to open ");
    {
        GLUI_Listbox *obj_list = Glui->add_listbox_to_panel(panel, "Objects", &ObjectId, -1, (GLUI_Update_CB)Choose_Object);
        obj_list->add_item(STREAMLINES, "streamlines");
        obj_list->add_item(DYNAMIC_TEMPERATURE, "dynamic_temperature");
        obj_list->add_item(TEMPERATURE1, "temperature1");
        obj_list->add_item(TEMPERATURE2, "temperature2");
        obj_list->add_item(DIESEL_FIELD1, "diesel_field1");
        obj_list->add_item(DISTANCE_FIELD1, "distance_field1");
        obj_list->add_item(DISTANCE_FIELD2, "distance_field2");
        obj_list->add_item(ICELAND_CURRENT_FIELD, "iceland_current_field");
        obj_list->add_item(TORUS_FIELD, "torus_field");
    }

    panel = Glui->add_panel("Color");
    {
        GLUI_Listbox *obj_list = Glui->add_listbox_to_panel(panel, "Schemes", &ColorSchemeId, -1, (GLUI_Update_CB)Choose_Color_Scheme);
        obj_list->add_item(RAINBOW, "Rainbow");
        obj_list->add_item(BLUE_WHITE_RED, "Blue White Red");
        obj_list->add_item(HEAT_MAP, "Heat Map");
        obj_list->add_item(DISCRETE, "Discrete");

        Glui->add_edittext_to_panel(panel, "MidPoint", GLUI_EDITTEXT_FLOAT, &ColorSchemeMidPoint, -1, (GLUI_Update_CB)Choose_Color_Scheme_MidPoint);
        Glui->add_checkbox_to_panel(panel, "Non-Linear", &NonLinearOn);
    }

    //GLUI_Rollout *rollout2 = Glui->add_rollout("Old Stuff", 0);
    //{
    //    panel = Glui->add_panel_to_panel(rollout2, "Iso-contours");
    //    {
    //        Glui->add_spinner_to_panel(panel, "Scalar Value", GLUI_SPINNER_FLOAT, &ScalarValue, -1, (GLUI_Update_CB)Choose_IsoContour_ScalarValue);
    //        Glui->add_spinner_to_panel(panel, "Number of Contours", GLUI_SPINNER_INT, &NumberOfContours, -1, (GLUI_Update_CB)Choose_IsoContour_NumberOfContours);
    //    }

    //    panel = Glui->add_panel_to_panel(rollout2, "Dynamic Temperature Data");
    //    {
    //        //Glui->add_slider
    //        GLUI_Spinner *S_min_spinner = Glui->add_spinner_to_panel(panel, "S min", GLUI_SPINNER_FLOAT, &S_min, -1, (GLUI_Update_CB)Choose_Smin);
    //        S_min_spinner->set_float_limits(0, 100);

    //        GLUI_Spinner *S_max_spinner = Glui->add_spinner_to_panel(panel, "S max", GLUI_SPINNER_FLOAT, &S_max, -1, (GLUI_Update_CB)Choose_Smax);
    //        S_max_spinner->set_float_limits(0, 100);

    //        Glui->add_spinner_to_panel(panel, "XY Index", GLUI_SPINNER_INT, &XY_index, -1, (GLUI_Update_CB)Choose_XY_Index);
    //        Glui->add_spinner_to_panel(panel, "XZ Index", GLUI_SPINNER_INT, &XZ_index, -1, (GLUI_Update_CB)Choose_XZ_Index);
    //        Glui->add_spinner_to_panel(panel, "YZ Index", GLUI_SPINNER_INT, &YZ_index, -1, (GLUI_Update_CB)Choose_YZ_Index);

    //        Glui->add_spinner_to_panel(panel, "Gradient Min", GLUI_SPINNER_FLOAT, &Gradient_min, -1, (GLUI_Update_CB)Choose_NULL);
    //        Glui->add_spinner_to_panel(panel, "Gradient Max", GLUI_SPINNER_FLOAT, &Gradient_max, -1, (GLUI_Update_CB)Choose_NULL);

    //        Glui->add_checkbox_to_panel(panel, "XY", &XY_on);
    //        Glui->add_checkbox_to_panel(panel, "XZ", &XZ_on);
    //        Glui->add_checkbox_to_panel(panel, "YZ", &YZ_on);
    //    }

    //    panel = Glui->add_panel_to_panel(rollout2, "Assignment 5");
    //    {
    //        GLUI_Spinner *spinner = Glui->add_spinner_to_panel(panel, "Maximum Opacity", GLUI_SPINNER_FLOAT, &Opacity_max, -1, (GLUI_Update_CB)Choose_Opacity);
    //        spinner->set_float_limits(0, 1);

    //        Glui->add_checkbox_to_panel(panel, "Bilinear", &BilinearOn);

    //        Glui->add_checkbox_to_panel(panel, "Enable Iso-Surfaces", &IsosurfaceOn);
    //        Glui->add_checkbox_to_panel(panel, "Enable Volume-Rendering", &VolumeRenderingOn);
    //    }
    //}

    //panel = Glui->add_panel("Iso-contours");
    //{
    //    Glui->add_spinner_to_panel(panel, "Scalar Value", GLUI_SPINNER_FLOAT, &ScalarValue, -1, (GLUI_Update_CB)Choose_IsoContour_ScalarValue);
    //    Glui->add_spinner_to_panel(panel, "Number of Contours", GLUI_SPINNER_INT, &NumberOfContours, -1, (GLUI_Update_CB)Choose_IsoContour_NumberOfContours);
    //}

    //panel = Glui->add_panel("Dynamic Temperature Data");
    //{
    //    //Glui->add_slider
    //    GLUI_Spinner *S_min_spinner = Glui->add_spinner_to_panel(panel, "S min", GLUI_SPINNER_FLOAT, &S_min, -1, (GLUI_Update_CB)Choose_Smin);
    //    S_min_spinner->set_float_limits(0, 100);

    //    GLUI_Spinner *S_max_spinner = Glui->add_spinner_to_panel(panel, "S max", GLUI_SPINNER_FLOAT, &S_max, -1, (GLUI_Update_CB)Choose_Smax);
    //    S_max_spinner->set_float_limits(0, 100);

    //    Glui->add_spinner_to_panel(panel, "XY Index", GLUI_SPINNER_INT, &XY_index, -1, (GLUI_Update_CB)Choose_XY_Index);
    //    Glui->add_spinner_to_panel(panel, "XZ Index", GLUI_SPINNER_INT, &XZ_index, -1, (GLUI_Update_CB)Choose_XZ_Index);
    //    Glui->add_spinner_to_panel(panel, "YZ Index", GLUI_SPINNER_INT, &YZ_index, -1, (GLUI_Update_CB)Choose_YZ_Index);

    //    Glui->add_spinner_to_panel(panel, "Gradient Min", GLUI_SPINNER_FLOAT, &Gradient_min, -1, (GLUI_Update_CB)Choose_NULL);
    //    Glui->add_spinner_to_panel(panel, "Gradient Max", GLUI_SPINNER_FLOAT, &Gradient_max, -1, (GLUI_Update_CB)Choose_NULL);

    //    Glui->add_checkbox_to_panel(panel, "XY", &XY_on);
    //    Glui->add_checkbox_to_panel(panel, "XZ", &XZ_on);
    //    Glui->add_checkbox_to_panel(panel, "YZ", &YZ_on);
    //}

    //panel = Glui->add_panel("Assignment 5");
    //{
    //    GLUI_Spinner *spinner = Glui->add_spinner_to_panel(panel, "Maximum Opacity", GLUI_SPINNER_FLOAT, &Opacity_max, -1, (GLUI_Update_CB)Choose_Opacity);
    //    spinner->set_float_limits(0, 1);

    //    Glui->add_checkbox_to_panel(panel, "Bilinear", &BilinearOn);

    //    Glui->add_checkbox_to_panel(panel, "Enable Iso-Surfaces", &IsosurfaceOn);
    //    Glui->add_checkbox_to_panel(panel, "Enable Volume-Rendering", &VolumeRenderingOn);
    //}

    //panel = Glui->add_panel("Assignment 7");
    //{
    //    GLUI_Listbox *obj_list = Glui->add_listbox_to_panel(panel, "Field Type", &FieldId, -1, (GLUI_Update_CB)Choose_FieldId);
    //    obj_list->add_item(FIELD1, "Field 1");
    //    obj_list->add_item(FIELD2, "Field 2");
    //    obj_list->add_item(FIELD3, "Field 3");

    //    GLUI_Listbox *obj_list2 = Glui->add_listbox_to_panel(panel, "Numerical Integration Type", &NumericalIntegrationTypeId, -1);
    //    obj_list2->add_item(EULER, "Euler");
    //    obj_list2->add_item(RK2, "RK2");

    //    Glui->add_checkbox_to_panel(panel, "Direct", &DirectOn);

    //    GLUI_Spinner *spinner = Glui->add_spinner_to_panel(panel, "Direct Opacity", GLUI_SPINNER_FLOAT, &DirectOpacity, -1);
    //    spinner->set_float_limits(0, 1);

    //    Glui->add_checkbox_to_panel(panel, "Streamlines", &StreamlineOn);
    //    Glui->add_checkbox_to_panel(panel, "Streamline Probe", &StreamlineProbeOn);
    //    Glui->add_checkbox_to_panel(panel, "Streamribbon Probe", &StreamribbonProbeOn);

    //    GLUI_Listbox *obj_list3 = Glui->add_listbox_to_panel(panel, "Streamribbon Type", &streamribbon_type, -1);
    //    obj_list3->add_item(EQUI_DISTANCE_STREAMRIBBON, "Equi-distance");
    //    obj_list3->add_item(REGULAR_STREAMRIBBON, "Regular");
    //}

    panel = Glui->add_panel("Final Project");
    {
        GLUI_Spinner *S_min_spinner = Glui->add_spinner_to_panel(panel, "S min", GLUI_SPINNER_FLOAT, &S_min, -1, (GLUI_Update_CB)Choose_Smin);
        S_min_spinner->set_float_limits(0, 1000);

        GLUI_Spinner *S_max_spinner = Glui->add_spinner_to_panel(panel, "S max", GLUI_SPINNER_FLOAT, &S_max, -1, (GLUI_Update_CB)Choose_Smax);
        S_max_spinner->set_float_limits(0, 1000);

        GLUI_Spinner *spinner2 = Glui->add_spinner_to_panel(panel, "Volume Render Opacity", GLUI_SPINNER_FLOAT, &Opacity_max, -1, (GLUI_Update_CB)Choose_Opacity);
        spinner2->set_float_limits(0, 1);

        GLUI_Listbox *obj_list = Glui->add_listbox_to_panel(panel, "Field Type", &FieldId, -1, (GLUI_Update_CB)Choose_FieldId);
        obj_list->add_item(FIELD3, "Field 3");
        obj_list->add_item(FIELD1, "Field 1");
        obj_list->add_item(FIELD2, "Field 2");

        GLUI_Listbox *obj_list2 = Glui->add_listbox_to_panel(panel, "Numerical Integration Type", &NumericalIntegrationTypeId, -1, (GLUI_Update_CB)Choose_NumericalIntegration);
        obj_list2->add_item(RK4, "RK4");
        obj_list2->add_item(RK2, "RK2");
        obj_list2->add_item(EULER, "Euler");

        Glui->add_checkbox_to_panel(panel, "Direct", &DirectOn);

        GLUI_Spinner *spinner = Glui->add_spinner_to_panel(panel, "Direct Arrow Opacity", GLUI_SPINNER_FLOAT, &DirectOpacity, -1);
        spinner->set_float_limits(0, 1);

        Glui->add_checkbox_to_panel(panel, "Streamlines", &StreamlineOn);
        //Glui->add_checkbox_to_panel(panel, "Streamline Probe", &StreamlineProbeOn);
        //Glui->add_checkbox_to_panel(panel, "Streamribbon Probe", &StreamribbonProbeOn);
        //GLUI_Listbox *obj_list3 = Glui->add_listbox_to_panel(panel, "Streamribbon Type", &streamribbon_type, -1);
        //obj_list3->add_item(EQUI_DISTANCE_STREAMRIBBON, "Equi-distance");
        //obj_list3->add_item(REGULAR_STREAMRIBBON, "Regular");

        GLUI_Spinner *P_value_spinner = Glui->add_spinner_to_panel(panel, "P Value", GLUI_SPINNER_INT, &p_value, -1, (GLUI_Update_CB)Choose_PQValue);
        P_value_spinner->set_float_limits(1, 10);

        GLUI_Spinner *Q_value_spinner = Glui->add_spinner_to_panel(panel, "Q Value", GLUI_SPINNER_INT, &q_value, -1, (GLUI_Update_CB)Choose_PQValue);
        Q_value_spinner->set_float_limits(1, 10);

        Glui->add_checkbox_to_panel(panel, "Entropy Streamlines", &EntropyStreamlinesOn);
        Glui->add_checkbox_to_panel(panel, "Spatial Streamline Clusters", &SpatialClustersOn);
        Glui->add_checkbox_to_panel(panel, "Shape Streamline Clusters", &ShapeClustersOn);

        Glui->add_checkbox_to_panel(panel, "Final Streamlines", &FinalStreamlinesOn);
        Glui->add_checkbox_to_panel(panel, "Final Streamtapes", &FinalStreamTapesOn);
        Glui->add_checkbox_to_panel(panel, "Final Streamtapes (Color)", &IncludeColorOn);
        Glui->add_checkbox_to_panel(panel, "Final Streamtapes (Torsion)", &IncludeTorsionOn);

        Glui->add_button_to_panel(panel, "Recalculate", RECALCULATE, (GLUI_Update_CB)Buttons);
    }

    //panel = Glui->add_panel("Streamline Probe Transformation");
    //{
    //    Glui->add_column_to_panel(panel, GLUIFALSE);
    //    trans = Glui->add_translation_to_panel(panel, "Trans XY", GLUI_TRANSLATION_XY, &ProbeTransXYZ[0]);
    //    trans->set_speed(0.05f);

    //    Glui->add_column_to_panel(panel, GLUIFALSE);
    //    trans = Glui->add_translation_to_panel(panel, "Trans Z", GLUI_TRANSLATION_Z, &ProbeTransXYZ[2]);
    //    trans->set_speed(0.05f);
    //}

    //panel = Glui->add_panel("Streamribbon Probe Transformation");
    //{
    //    Glui->add_column_to_panel(panel, GLUIFALSE);
    //    trans = Glui->add_translation_to_panel(panel, "Trans XY", GLUI_TRANSLATION_XY, &StreamribbonProbeXYZ[0]);
    //    trans->set_speed(0.05f);

    //    Glui->add_column_to_panel(panel, GLUIFALSE);
    //    trans = Glui->add_translation_to_panel(panel, "Trans Z", GLUI_TRANSLATION_Z, &StreamribbonProbeXYZ[2]);
    //    trans->set_speed(0.05f);
    //}

    panel = Glui->add_panel( "Object Transformation" );
    {
        rot = Glui->add_rotation_to_panel(panel, "Rotation", (float *)RotMatrix);

        // allow the object to be spun via the glui rotation widget:
        rot->set_spin(1.0);

        Glui->add_column_to_panel(panel, GLUIFALSE);
        scale = Glui->add_translation_to_panel(panel, "Scale", GLUI_TRANSLATION_Y, &Scale2);
        scale->set_speed(0.005f);

        Glui->add_column_to_panel(panel, GLUIFALSE);
        trans = Glui->add_translation_to_panel(panel, "Trans XY", GLUI_TRANSLATION_XY, &TransXYZ[0]);
        trans->set_speed(0.05f);

        Glui->add_column_to_panel(panel, GLUIFALSE);
        trans = Glui->add_translation_to_panel(panel, "Trans Z", GLUI_TRANSLATION_Z, &TransXYZ[2]);
        trans->set_speed(0.05f);
    }

    Glui->add_checkbox( "Debug", &DebugOn );


    panel = Glui->add_panel( "", GLUIFALSE );

    Glui->add_button_to_panel( panel, "Reset", RESET, (GLUI_Update_CB) Buttons );

    Glui->add_column_to_panel( panel, GLUIFALSE );

    Glui->add_button_to_panel( panel, "Quit", QUIT, (GLUI_Update_CB) Buttons );


    // tell glui what graphics window it needs to post a redisplay to:

    Glui->set_main_gfx_window( MainWindow );


    // set the graphics window's idle function:

    GLUI_Master.set_glutIdleFunc( NULL );
}


//
// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions
//
void InitGraphics( void )
{
	// setup the display mode:
	// ( *must* be done before call to glutCreateWindow() )
	// ask for color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );


	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );


	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );


	// setup the clear values:

	//glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );
    glClearColor(0.25, 0.25, 0.25, 1);


	// setup the callback routines:


	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	//glutDisplayFunc( Display_Model );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( 0, NULL, 0 );

	// DO NOT SET THE GLUT IDLE FUNCTION HERE !!
	// glutIdleFunc( NULL );
	// let glui take care of it in InitGlui()
}




//
// initialize the display lists that will not change:
//
void InitLists( void )
{
	float dx = BOXSIZE / 2.;
	float dy = BOXSIZE / 2.;
	float dz = BOXSIZE / 2.;

	// create the object:

	BoxList = glGenLists( 1 );
	glNewList( BoxList, GL_COMPILE );

		glBegin( GL_QUADS );

			glColor3f( 0., 0., 1. );
			glNormal3f( 0., 0.,  1. );
				glVertex3f( -dx, -dy,  dz );
				glVertex3f(  dx, -dy,  dz );
				glVertex3f(  dx,  dy,  dz );
				glVertex3f( -dx,  dy,  dz );

			glNormal3f( 0., 0., -1. );
				glTexCoord2f( 0., 0. );
				glVertex3f( -dx, -dy, -dz );
				glTexCoord2f( 0., 1. );
				glVertex3f( -dx,  dy, -dz );
				glTexCoord2f( 1., 1. );
				glVertex3f(  dx,  dy, -dz );
				glTexCoord2f( 1., 0. );
				glVertex3f(  dx, -dy, -dz );

			glColor3f( 1., 0., 0. );
			glNormal3f(  1., 0., 0. );
				glVertex3f(  dx, -dy,  dz );
				glVertex3f(  dx, -dy, -dz );
				glVertex3f(  dx,  dy, -dz );
				glVertex3f(  dx,  dy,  dz );

			glNormal3f( -1., 0., 0. );
				glVertex3f( -dx, -dy,  dz );
				glVertex3f( -dx,  dy,  dz );
				glVertex3f( -dx,  dy, -dz );
				glVertex3f( -dx, -dy, -dz );

			glColor3f( 0., 1., 0. );
			glNormal3f( 0.,  1., 0. );
				glVertex3f( -dx,  dy,  dz );
				glVertex3f(  dx,  dy,  dz );
				glVertex3f(  dx,  dy, -dz );
				glVertex3f( -dx,  dy, -dz );

			glNormal3f( 0., -1., 0. );
				glVertex3f( -dx, -dy,  dz );
				glVertex3f( -dx, -dy, -dz );
				glVertex3f(  dx, -dy, -dz );
				glVertex3f(  dx, -dy,  dz );

		glEnd();

	glEndList();


	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList();
}



//
// the keyboard callback:
//
void Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'o':
		case 'O':
			WhichProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			WhichProjection = PERSP;
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			Buttons( QUIT );	// will not return here
			break;			// happy compiler

		case 'r':
		case 'R':
			LeftButton = ROTATE;
			break;

		case 's':
		case 'S':
			LeftButton = SCALE;
			break;

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}


	// synchronize the GLUI display with the variables:

	Glui->sync_live();


	// force a call to Display():

	glutSetWindow( MainWindow );
	glutPostRedisplay();
}



//
// called when the mouse button transitions down or up:
//
void MouseButton( int button, int state, int x, int y )
{
	int b;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}


	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}
}



//
// called when the mouse moves while a button is down:
//
void MouseMotion( int x, int y )
{
	int dx, dy;		// change in mouse coordinates

	if( DebugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );


	dx = x - Xmouse;		// change in mouse coords
	dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		switch( LeftButton )
		{
			case ROTATE:
				Xrot += ( ANGFACT*dy );
				Yrot += ( ANGFACT*dx );
				break;

			case SCALE:
				Scale += SCLFACT * (float) ( dx - dy );
				if( Scale < MINSCALE )
					Scale = MINSCALE;
				break;
		}
	}


	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay();
}



//
// reset the transformations and the colors:
//
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene
//
void Reset( void )
{
	ActiveButton = 0;
	AxesOn = GLUITRUE;
    //NonLinearOn = GLUIFALSE;
	DebugOn = GLUIFALSE;
	DepthCueOn = GLUIFALSE;
	LeftButton = ROTATE;
	Scale  = 1.0;
	Scale2 = 0.0;		// because we add 1. to it in Display()
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	TransXYZ[0] = TransXYZ[1] = TransXYZ[2] = 0.;
    ProbeTransXYZ[0] = ProbeTransXYZ[1] = ProbeTransXYZ[2] = 0.;

	                  RotMatrix[0][1] = RotMatrix[0][2] = RotMatrix[0][3] = 0.;
	RotMatrix[1][0]                   = RotMatrix[1][2] = RotMatrix[1][3] = 0.;
	RotMatrix[2][0] = RotMatrix[2][1]                   = RotMatrix[2][3] = 0.;
	RotMatrix[3][0] = RotMatrix[3][1] = RotMatrix[3][3]                   = 0.;
	RotMatrix[0][0] = RotMatrix[1][1] = RotMatrix[2][2] = RotMatrix[3][3] = 1.;

    //ColorSchemeMidPoint = 0.5;
}



//
// called when user resizes the window:
//
void Resize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );

	// don't really need to do anything since window size is
	// checked each time in Display():

	glutSetWindow( MainWindow );
	glutPostRedisplay();
}


//
// handle a change to the window's visibility:
//
void Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}




//////////////////////////////////////////  EXTRA HANDY UTILITIES:  /////////////////////////////

// size of wings as fraction of length:

#define WINGS	0.10


// axes:

#define X	1
#define Y	2
#define Z	3


// x, y, z, axes:

static float axx[3] = { 1., 0., 0. };
static float ayy[3] = { 0., 1., 0. };
static float azz[3] = { 0., 0., 1. };


void Arrow( float tail[3], float head[3], float normalizedValue )
{
	float u[3], v[3], w[3];		// arrow coordinate system
	float d;			// wing distance
	float x, y, z;			// point to plot
	float mag;			// magnitude of major direction
	float f;			// fabs of magnitude
	int axis;			// which axis is the major


	// set w direction in u-v-w coordinate system:

	w[0] = head[0] - tail[0];
	w[1] = head[1] - tail[1];
	w[2] = head[2] - tail[2];


	// determine major direction:

	axis = X;
	mag = fabs( w[0] );
	if( (f=fabs(w[1]))  > mag )
	{
		axis = Y;
		mag = f;
	}
	if( (f=fabs(w[2]))  > mag )
	{
		axis = Z;
		mag = f;
	}


	// set size of wings and turn w into a Unit vector:

	d = WINGS * Unit( w, w );


	// draw the shaft of the arrow:

	glBegin( GL_LINE_STRIP );
		glVertex3fv( tail );
		glVertex3fv( head );
	glEnd();

	// draw two sets of wings in the non-major directions:

	if( axis != X )
	{
		Cross( w, axx, v );
		(void) Unit( v, v );
		Cross( v, w, u  );
		x = head[0] + d * ( u[0] - w[0] );
		y = head[1] + d * ( u[1] - w[1] );
		z = head[2] + d * ( u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
		x = head[0] + d * ( -u[0] - w[0] );
		y = head[1] + d * ( -u[1] - w[1] );
		z = head[2] + d * ( -u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
	}


	if( axis != Y )
	{
		Cross( w, ayy, v );
		(void) Unit( v, v );
		Cross( v, w, u  );
		x = head[0] + d * ( u[0] - w[0] );
		y = head[1] + d * ( u[1] - w[1] );
		z = head[2] + d * ( u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
		x = head[0] + d * ( -u[0] - w[0] );
		y = head[1] + d * ( -u[1] - w[1] );
		z = head[2] + d * ( -u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
	}



	if( axis != Z )
	{
		Cross( w, azz, v );
		(void) Unit( v, v );
		Cross( v, w, u  );
		x = head[0] + d * ( u[0] - w[0] );
		y = head[1] + d * ( u[1] - w[1] );
		z = head[2] + d * ( u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
		x = head[0] + d * ( -u[0] - w[0] );
		y = head[1] + d * ( -u[1] - w[1] );
		z = head[2] + d * ( -u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
			glVertex3fv( head );
			glVertex3f( x, y, z );
		glEnd();
	}
}



float Dot( float v1[3], float v2[3] )
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}



void Cross( float v1[3], float v2[3], float vout[3] )
{
	float tmp[3];

	tmp[0] = v1[1]*v2[2] - v2[1]*v1[2];
	tmp[1] = v2[0]*v1[2] - v1[0]*v2[2];
	tmp[2] = v1[0]*v2[1] - v2[0]*v1[1];

	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}



float Unit( float vin[3], float vout[3] )
{
	float dist, f ;

	dist = vin[0]*vin[0] + vin[1]*vin[1] + vin[2]*vin[2];

	if( dist > 0.0 )
	{
		dist = sqrt( dist );
		f = 1. / dist;
		vout[0] = f * vin[0];
		vout[1] = f * vin[1];
		vout[2] = f * vin[2];
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}

	return dist;
}



// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = {
		0.f, 1.f, 0.f, 1.f
	      };

static float xy[] = {
		-.5f, .5f, .5f, -.5f
	      };

static int xorder[] = {
		1, 2, -3, 4
		};


static float yx[] = {
		0.f, 0.f, -.5f, .5f
	      };

static float yy[] = {
		0.f, .6f, 1.f, 1.f
	      };

static int yorder[] = {
		1, 2, 3, -2, 4
		};


static float zx[] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
	      };

static float zy[] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
	      };

static int zorder[] = {
		1, 2, 3, 4, -5, 6
		};


// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;


// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;


//
//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)
//
void Axes( float length )
{
	int i, j;			// counters
	float fact;			// character scale factor
	float base;			// character start location


	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd();
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd();

	fact = LENFRAC * length;
	base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( i = 0; i < 4; i++ )
		{
			j = xorder[i];
			if( j < 0 )
			{
				
				glEnd();
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd();

	glBegin( GL_LINE_STRIP );
		for( i = 0; i < 5; i++ )
		{
			j = yorder[i];
			if( j < 0 )
			{
				
				glEnd();
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd();

	glBegin( GL_LINE_STRIP );
		for( i = 0; i < 6; i++ )
		{
			j = zorder[i];
			if( j < 0 )
			{
				
				glEnd();
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd();
}

// Set the view and the lighting properties
void set_view(GLenum mode)
{
	icVector3 up, ray, view;
	GLfloat light_ambient0[] = { 0.3, 0.3, 0.3, 1.0 };
	GLfloat light_diffuse0[] = { 0.7, 0.7, 0.7, 1.0 };
	GLfloat light_specular0[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_ambient1[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse1[] = { 0.5, 0.5, 0.5, 1.0 };
	GLfloat light_specular1[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_ambient2[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_diffuse2[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular2[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_position[] = { 0.0, 0.0, 0.0, 1.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular0);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular1);


    glMatrixMode(GL_PROJECTION);
	if (mode == GL_RENDER)
		glLoadIdentity();

	if (WhichProjection == ORTHO)
		glOrtho(-radius_factor, radius_factor, -radius_factor, radius_factor, 0.0, 40.0);
	else
		gluPerspective(45.0, 1.0, 0.1, 40.0);

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	light_position[0] = 5.5;
	light_position[1] = 0.0;
	light_position[2] = 0.0;
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	light_position[0] = -0.1;
	light_position[1] = 0.0;
	light_position[2] = 0.0;
	glLightfv(GL_LIGHT2, GL_POSITION, light_position);
}

// Set the scene for the object based on the center of the object
//void set_scene(GLenum mode, Polyhedron *poly)
//{
//	glTranslatef(0.0, 0.0, -3.0);
//
//	glScalef(1.0/poly->radius, 1.0/poly->radius, 1.0/poly->radius);
//	glTranslatef(-poly->center.entry[0], -poly->center.entry[1], -poly->center.entry[2]);
//}

void display_shape_st(GLenum mode, streamlines *data)
{
    unsigned int i, j, k;
    GLfloat mat_diffuse[4];

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1., 1.);

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    float x, y, z;
    float vx, vy, vz;

    // Pre-compute Min/Max
    float VMmin, VMmax;
    data->get_vector_field(FieldId, -1, -1, -1, vx, vy, vz);
    VMmin = sqrt(pow(vx, 2) + pow(vy, 2) + pow(vz, 2));
    VMmax = VMmin;

    for (i = 0; i < NX; i++)
    {
        for (j = 0; j < NY; j++)
        {
            for (k = 0; k < NZ; k++)
            {
                x = -1. + ((i * 2.) / NX);
                y = -1. + ((j * 2.) / NY);
                z = -1. + ((k * 2.) / NZ);

                data->get_vector_field(FieldId, x, y, z, vx, vy, vz);

                //if (NonLinearOn != 0)
                //{
                //    S = S * S * S;
                //}

                float VM = sqrt(pow(vx, 2) + pow(vy, 2) + pow(vz, 2));

                if (VM < VMmin)
                {
                    VMmin = VM;
                }
                else if (VM > VMmax)
                {
                    VMmax = VM;
                }
            }
        }
    }

    if (DirectOn == 1)
    {
        for (i = 0; i <= NX; i++)
        {
            for (j = 0; j <= NY; j++)
            {
                for (k = 0; k <= NZ; k++)
                {
                    x = -1. + ((i * 2.) / NX);
                    y = -1. + ((j * 2.) / NY);
                    z = -1. + ((k * 2.) / NZ);

                    data->get_vector_field(FieldId, x, y, z, vx, vy, vz);

                    float VM = sqrt(pow(vx, 2) + pow(vy, 2) + pow(vz, 2));
                    float normalized_value = (VM - VMmin) / (VMmax - VMmin);

                    float maxv = abs(vx);
                    if (abs(vy) > maxv)
                    {
                        maxv = abs(vy);
                    }
                    if (abs(vz) > maxv)
                    {
                        maxv = abs(vz);
                    }

                    maxv = maxv * NX * 2;

                    float tail[3];
                    tail[0] = x - (vx / maxv);
                    tail[1] = y - (vy / maxv);
                    tail[2] = z - (vz / maxv);

                    float head[3];
                    head[0] = x + (vx / maxv);
                    head[1] = y + (vy / maxv);
                    head[2] = z + (vz / maxv);

                    if (ColorSchemeId == BLUE_WHITE_RED)
                    {
                        float rgb[3];
                        BlueWhiteRed(rgb, normalized_value, ColorSchemeMidPoint);
                        glColor4f(rgb[0], rgb[1], rgb[2], DirectOpacity);
                    }
                    else if (ColorSchemeId == HEAT_MAP)
                    {
                        float rgb[3];
                        HeatMap(rgb, normalized_value);
                        glColor4f(rgb[0], rgb[1], rgb[2], DirectOpacity);
                    }
                    else if (ColorSchemeId == RAINBOW)
                    {
                        float rgb[3];
                        Rainbow(rgb, normalized_value);
                        glColor4f(rgb[0], rgb[1], rgb[2], DirectOpacity);
                    }
                    else if (ColorSchemeId == DISCRETE)
                    {
                        float rgb[3];
                        DiscreteColorScheme(rgb, normalized_value);
                        glColor4f(rgb[0], rgb[1], rgb[2], DirectOpacity);
                    }

                    Arrow(tail, head, normalized_value);
                }
            }
        }
    }

    if (StreamlineOn == 1)
    {
        srand(time(NULL));

        for (int it = 0; it < 13; it++)
        {
            //x = ((rand() % 200) / 100.) - 1;
            //y = ((rand() % 200) / 100.) - 1;
            //z = ((rand() % 200) / 100.) - 1;

            if (it == 0)
            {
                x = 0;
                y = 0;
                z = 0;
            }
            else if (it == 1)
            {
                x = 0.5;
                y = 0.5;
                z = 0;
            }
            else if (it == 2)
            {
                x = -0.5;
                y = 0.5;
                z = 0;
            }
            else if (it == 3)
            {
                x = 0.5;
                y = -0.5;
                z = 0;
            }
            else if (it == 4)
            {
                x = -0.5;
                y = -0.5;
                z = 0;
            }
            else if (it == 5)
            {
                x = 0.5;
                y = 0;
                z = 0.5;
            }
            else if (it == 6)
            {
                x = -0.5;
                y = 0;
                z = 0.5;
            }
            else if (it == 7)
            {
                x = 0.5;
                y = 0;
                z = -0.5;
            }
            else if (it == 8)
            {
                x = -0.5;
                y = 0;
                z = -0.5;
            }
            else if (it == 9)
            {
                x = 0;
                y = 0.5;
                z = 0.5;
            }
            else if (it == 10)
            {
                x = 0;
                y = -0.5;
                z = 0.5;
            }
            else if (it == 11)
            {
                x = 0;
                y = 0.5;
                z = -0.5;
            }
            else if (it == 12)
            {
                x = 0;
                y = -0.5;
                z = -0.5;
            }

            std::vector<streamline_coordinate> streamline;
            data->get_streamline(x, y, z, FieldId, NumericalIntegrationTypeId, streamline);

            float rgb[3];
            rgb[0] = 0;
            rgb[1] = 0;
            rgb[2] = 0;
            glColor3f(rgb[0], rgb[1], rgb[2]);

            glBegin(GL_LINE_STRIP);
            for (i = 0; i < streamline.size(); i++)
            {
                float coord[3];
                coord[0] = streamline[i].x;
                coord[1] = streamline[i].y;
                coord[2] = streamline[i].z;

                glVertex3fv(coord);
            }
            glEnd();
        }
    }

    if (StreamlineProbeOn == 1)
    {
        float x = ProbeTransXYZ[0];
        float y = ProbeTransXYZ[1];
        float z = ProbeTransXYZ[2];

        std::vector<streamline_coordinate> streamline;
        data->get_streamline(x, y, z, FieldId, NumericalIntegrationTypeId, streamline);

        float rgb[3];
        rgb[0] = 1;
        rgb[1] = 1;
        rgb[2] = 1;
        glColor3f(rgb[0], rgb[1], rgb[2]);

        glBegin(GL_LINE_STRIP);
        for (i = 0; i < streamline.size(); i++)
        {
            float coord[3];
            coord[0] = streamline[i].x;
            coord[1] = streamline[i].y;
            coord[2] = streamline[i].z;

            glVertex3fv(coord);
        }
        glEnd();
    }

    if (ShapeClustersOn == 1)
    {
        for (int it = 0; it < data->cluster_spatial.size(); it++)
        {
            for (int it3 = 0; it3 < data->cluster_spatial[it].cluster_shape.size(); it3++)
            {
                glColor3f(data->cluster_spatial[it].cluster_shape[it3].rgb[0], data->cluster_spatial[it].cluster_shape[it3].rgb[1], data->cluster_spatial[it].cluster_shape[it3].rgb[2]);

                for (int it2 = 0; it2 < data->cluster_spatial[it].cluster_shape[it3].streamlines.size(); it2++)
                {
                    std::vector<streamline_coordinate> streamline = data->cluster_spatial[it].cluster_shape[it3].streamlines[it2].streamline;

                    glBegin(GL_LINE_STRIP);
                    for (i = 0; i < streamline.size(); i++)
                    {
                        float coord[3];
                        coord[0] = streamline[i].x;
                        coord[1] = streamline[i].y;
                        coord[2] = streamline[i].z;

                        glVertex3fv(coord);
                    }
                    glEnd();
                }
            }
        }
    }
    else if (SpatialClustersOn == 1)
    {
        for (int it = 0; it < data->cluster_spatial.size(); it++)
        {
            glColor3f(data->cluster_spatial[it].rgb[0], data->cluster_spatial[it].rgb[1], data->cluster_spatial[it].rgb[2]);

            for (int it2 = 0; it2 < data->cluster_spatial[it].streamlines.size(); it2++)
            {
                std::vector<streamline_coordinate> streamline = data->cluster_spatial[it].streamlines[it2].streamline;

                glBegin(GL_LINE_STRIP);
                for (i = 0; i < streamline.size(); i++)
                {
                    float coord[3];
                    coord[0] = streamline[i].x;
                    coord[1] = streamline[i].y;
                    coord[2] = streamline[i].z;

                    glVertex3fv(coord);
                }
                glEnd();
            }
        }
    }
    else if (EntropyStreamlinesOn == 1)
    {
        for (int it = 0; it < data->entropy_streamlines.size(); it++)
        {
            std::vector<streamline_coordinate> streamline = data->entropy_streamlines[it].streamline;

            float rgb[3];
            rgb[0] = 1;
            rgb[1] = 1;
            rgb[2] = 1;
            glColor3f(rgb[0], rgb[1], rgb[2]);

            glBegin(GL_LINE_STRIP);
            for (i = 0; i < streamline.size(); i++)
            {
                float coord[3];
                coord[0] = streamline[i].x;
                coord[1] = streamline[i].y;
                coord[2] = streamline[i].z;

                glVertex3fv(coord);
            }
            glEnd();
        }
    }

    if (FinalStreamlinesOn == 1)
    {
        for (int it = 0; it < data->chosen_entropy_streamlines.size(); it++)
        {
            std::vector<streamline_coordinate> streamline = data->chosen_entropy_streamlines[it].streamline.streamline;

            glColor3f(data->chosen_entropy_streamlines[it].rgb[0], data->chosen_entropy_streamlines[it].rgb[1], data->chosen_entropy_streamlines[it].rgb[2]);

            glBegin(GL_LINE_STRIP);
            for (i = 0; i < streamline.size(); i++)
            {
                float coord[3];
                coord[0] = streamline[i].x;
                coord[1] = streamline[i].y;
                coord[2] = streamline[i].z;

                glVertex3fv(coord);
            }
            glEnd();
        }
    }

    if (FinalStreamTapesOn == 1)
    {
        for (int it = 0; it < data->torsion_streamlines.size(); it++)
        {
            std::vector<streamline_torsion_part> tor = data->torsion_streamlines[it].tor;

            float coord[3];

            glColor4f(1, 1, 1, 0.5);
            glBegin(GL_LINE_STRIP);
            for (i = 0; i < tor.size(); i++)
            {
                coord[0] = tor[i].x;
                coord[1] = tor[i].y;
                coord[2] = tor[i].z;

                glVertex3fv(coord);
            }
            glEnd();

            if (IncludeColorOn == 1)
            {
                glColor4f(data->torsion_streamlines[it].rgb[0], data->torsion_streamlines[it].rgb[1], data->torsion_streamlines[it].rgb[2], 0.5);
            }
            else
            {
                glColor4f(0, 0, 0, 0.5);
            }

            if (IncludeTorsionOn == 1)
            {
                glBegin(GL_QUAD_STRIP);
                for (i = 0; i < tor.size(); i++)
                {
                    coord[0] = tor[i].x + (tor[i].normalized_binormal_x * 0.05 * tor[i].normalized_width);
                    coord[1] = tor[i].y + (tor[i].normalized_binormal_y * 0.05 * tor[i].normalized_width);
                    coord[2] = tor[i].z + (tor[i].normalized_binormal_z * 0.05 * tor[i].normalized_width);
                    glVertex3fv(coord);

                    coord[0] = tor[i].x - (tor[i].normalized_binormal_x * 0.05 * tor[i].normalized_width);
                    coord[1] = tor[i].y - (tor[i].normalized_binormal_y * 0.05 * tor[i].normalized_width);
                    coord[2] = tor[i].z - (tor[i].normalized_binormal_z * 0.05 * tor[i].normalized_width);
                    glVertex3fv(coord);
                }
                glEnd();
            }
            else 
            {
                glBegin(GL_QUAD_STRIP);
                for (i = 0; i < tor.size(); i++)
                {
                    coord[0] = tor[i].x + (tor[i].normalized_binormal_x * 0.05);
                    coord[1] = tor[i].y + (tor[i].normalized_binormal_y * 0.05);
                    coord[2] = tor[i].z + (tor[i].normalized_binormal_z * 0.05);
                    glVertex3fv(coord);

                    coord[0] = tor[i].x - (tor[i].normalized_binormal_x * 0.05);
                    coord[1] = tor[i].y - (tor[i].normalized_binormal_y * 0.05);
                    coord[2] = tor[i].z - (tor[i].normalized_binormal_z * 0.05);
                    glVertex3fv(coord);
                }
                glEnd();
            }

            data->get_vector_field(FieldId, tor[tor.size() - 1].x, tor[tor.size() - 1].y, tor[tor.size() - 1].z, vx, vy, vz);
            float magnitude = sqrt(vx * vx + vy * vy + vz * vz);

            glBegin(GL_TRIANGLES);
            coord[0] = tor[tor.size() - 1].x + (tor[tor.size() - 1].normalized_binormal_x * 0.1);
            coord[1] = tor[tor.size() - 1].y + (tor[tor.size() - 1].normalized_binormal_y * 0.1);
            coord[2] = tor[tor.size() - 1].z + (tor[tor.size() - 1].normalized_binormal_z * 0.1);
            glVertex3fv(coord);

            coord[0] = tor[tor.size() - 1].x - (tor[tor.size() - 1].normalized_binormal_x * 0.1);
            coord[1] = tor[tor.size() - 1].y - (tor[tor.size() - 1].normalized_binormal_y * 0.1);
            coord[2] = tor[tor.size() - 1].z - (tor[tor.size() - 1].normalized_binormal_z * 0.1);
            glVertex3fv(coord);

            coord[0] = tor[tor.size() - 1].x + (vx/magnitude) * 0.1;
            coord[1] = tor[tor.size() - 1].y + (vy/magnitude) * 0.1;
            coord[2] = tor[tor.size() - 1].z + (vz/magnitude) * 0.1;
            glVertex3fv(coord);
            glEnd();
        }
    }

    if (false)
    {
        float x = StreamribbonProbeXYZ[0];
        float y = StreamribbonProbeXYZ[1];
        float z = StreamribbonProbeXYZ[2];

        float x1 = x - 0.5;
        float y1 = y - 0.5;
        float z1 = z - 0.5;

        std::vector<streamline_coordinate> streamline1;

        data->get_streamline(x1, y1, z1, FieldId, NumericalIntegrationTypeId, streamline1);

        if (streamline1.size() > 1)
        {

            std::vector<streamline_torsion_part> tor;

            data->get_streamtape(streamline1, tor);

            float rgb[3];
            rgb[0] = 0;
            rgb[1] = 1;
            rgb[2] = 1;
            glColor4f(rgb[0], rgb[1], rgb[2], 0.5);

            float coord[3];
            glBegin(GL_LINE_STRIP);
            for (i = 0; i < streamline1.size(); i++)
            {
                coord[0] = streamline1[i].x;
                coord[1] = streamline1[i].y;
                coord[2] = streamline1[i].z;
                glVertex3fv(coord);
            }
            glEnd();

            rgb[0] = 1;
            rgb[1] = 0;
            rgb[2] = 0;
            glColor4f(rgb[0], rgb[1], rgb[2], 0.5);

            //glBegin(GL_QUAD_STRIP);
            //for (i = 0; i < tor.size(); i++)
            //{
            //    coord[0] = tor[i].x + (tor[i].normalized_x * 0.1 * tor[i].normalized_width);
            //    coord[1] = tor[i].y + (tor[i].normalized_y * 0.1 * tor[i].normalized_width);
            //    coord[2] = tor[i].z + (tor[i].normalized_z * 0.1 * tor[i].normalized_width);
            //    glVertex3fv(coord);

            //    //coord[0] = tor[i].x - (tor[i].normalized_x * 0.1 * tor[i].normalized_width);
            //    //coord[1] = tor[i].y - (tor[i].normalized_y * 0.1 * tor[i].normalized_width);
            //    //coord[2] = tor[i].z - (tor[i].normalized_z * 0.1 * tor[i].normalized_width);

            //    coord[0] = tor[i].x;
            //    coord[1] = tor[i].y;
            //    coord[2] = tor[i].z;
            //    glVertex3fv(coord);
            //}
            //glEnd();

            glBegin(GL_QUAD_STRIP);
            for (i = 0; i < tor.size(); i++)
            {
                coord[0] = tor[i].x + (tor[i].normalized_binormal_x * 0.05);
                coord[1] = tor[i].y + (tor[i].normalized_binormal_y * 0.05);
                coord[2] = tor[i].z + (tor[i].normalized_binormal_z * 0.05);
                glVertex3fv(coord);

                coord[0] = tor[i].x - (tor[i].normalized_binormal_x * 0.05);
                coord[1] = tor[i].y - (tor[i].normalized_binormal_y * 0.05);
                coord[2] = tor[i].z - (tor[i].normalized_binormal_z * 0.05);
                glVertex3fv(coord);
            }
            glEnd();
        }
    }

    if (StreamribbonProbeOn == 1)
    {
        if (streamribbon_type == EQUI_DISTANCE_STREAMRIBBON)
        {
            float x = StreamribbonProbeXYZ[0];
            float y = StreamribbonProbeXYZ[1];
            float z = StreamribbonProbeXYZ[2];

            std::vector<streamline_coordinate> streamline;
            std::vector<streamline_coordinate> streamline_ribbon;

            data->get_streamline(x, y, z, FieldId, NumericalIntegrationTypeId, streamline);
            data->get_streamribbon(FieldId, NumericalIntegrationTypeId, streamline, streamline_ribbon);

            int minSize = streamline.size();
            if (minSize > streamline_ribbon.size())
            {
                minSize = streamline_ribbon.size();
            }

            float rgb[3];
            rgb[0] = 0;
            rgb[1] = 1;
            rgb[2] = 1;
            glColor4f(rgb[0], rgb[1], rgb[2], 0.5);

            float coord[3];
            glBegin(GL_QUAD_STRIP);
            for (i = 0; i < minSize; i++)
            {
                coord[0] = streamline[i].x;
                coord[1] = streamline[i].y;
                coord[2] = streamline[i].z;
                glVertex3fv(coord);

                coord[0] = streamline_ribbon[i].x;
                coord[1] = streamline_ribbon[i].y;
                coord[2] = streamline_ribbon[i].z;
                glVertex3fv(coord);
            }
            glEnd();
        }
        else if (streamribbon_type == REGULAR_STREAMRIBBON)
        {
            float x = StreamribbonProbeXYZ[0];
            float y = StreamribbonProbeXYZ[1];
            float z = StreamribbonProbeXYZ[2];

            float x1 = x - 0.05;
            float y1 = y - 0.05;
            float z1 = z - 0.05;

            //float x2 = x;
            //float y2 = y;
            //float z2 = z;

            float x3 = x + 0.05;
            float y3 = y + 0.05;
            float z3 = z + 0.05;

            std::vector<streamline_coordinate> streamline1;
            //std::vector<streamline_coordinate> streamline2;
            std::vector<streamline_coordinate> streamline3;

            data->get_streamline(x1, y1, z1, FieldId, NumericalIntegrationTypeId, streamline1);
            //data->get_streamline(x2, y2, z2, FieldId, NumericalIntegrationTypeId, streamline2);
            data->get_streamline(x3, y3, z3, FieldId, NumericalIntegrationTypeId, streamline3);

            int minSize = streamline1.size();
            //if (minSize > streamline2.size())
            //{
            //    minSize = streamline2.size();
            //}
            if (minSize > streamline3.size())
            {
                minSize = streamline3.size();
            }

            float rgb[3];
            rgb[0] = 0;
            rgb[1] = 1;
            rgb[2] = 1;
            glColor4f(rgb[0], rgb[1], rgb[2], 0.5);

            float coord[3];
            glBegin(GL_QUAD_STRIP);
            for (i = 0; i < minSize; i++)
            {
                coord[0] = streamline1[i].x;
                coord[1] = streamline1[i].y;
                coord[2] = streamline1[i].z;
                glVertex3fv(coord);

                coord[0] = streamline3[i].x;
                coord[1] = streamline3[i].y;
                coord[2] = streamline3[i].z;
                glVertex3fv(coord);
            }
            glEnd();
        }
    }

    //Assignment 5
    if (VolumeRenderingOn == 1)
    {
        DetermineVisibility();

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        // Choose Bilinear or Not
        int filter = GL_NEAREST;
        if (BilinearOn == 1)
            filter = GL_LINEAR;

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        if (Major == Z_direction)
        {
            float z0;
            float dz;
            int iterator;
            int step;

            if (Zside == PLUS)
            {
                z0 = -1.;
                dz = 2. / (float)(tNZ - 1);
                iterator = 0;
                step = 1;
            }
            else
            {
                z0 = 1.;
                dz = -2. / (float)(tNZ - 1);
                iterator = tNZ - 1;
                step = -1;
            }

            float zcoord = z0;
            for (int z = 0; z < tNZ; z++)
            {
                if (Opacity_max != 0)
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, 4, tNX, tNY, 0, GL_RGBA,
                        GL_UNSIGNED_BYTE, &data->TextureXY[iterator][0][0][0]);
                    glBegin(GL_QUADS);

                    glTexCoord2f(0., 0.);
                    glVertex3f(-1., -1., zcoord);
                    glTexCoord2f(1., 0.);
                    glVertex3f(1., -1., zcoord);
                    glTexCoord2f(1., 1.);
                    glVertex3f(1., 1., zcoord);
                    glTexCoord2f(0., 1.);
                    glVertex3f(-1., 1., zcoord);
                    glEnd();
                }
                iterator += step;
                zcoord += dz;
            }
        }
        else if (Major == Y_direction)
        {
            float y0;
            float dy;
            int iterator;
            int step;

            if (Yside == PLUS)
            {
                y0 = -1.;
                dy = 2. / (float)(tNY - 1);
                iterator = 0;
                step = 1;
            }
            else
            {
                y0 = 1.;
                dy = -2. / (float)(tNY - 1);
                iterator = tNY - 1;
                step = -1;
            }

            float ycoord = y0;
            for (int y = 0; y < tNY; y++)
            {
                if (Opacity_max != 0)
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, 4, tNX, tNZ, 0, GL_RGBA,
                        GL_UNSIGNED_BYTE, &data->TextureXZ[iterator][0][0][0]);
                    glBegin(GL_QUADS);

                    glTexCoord2f(0., 0.);
                    glVertex3f(-1., ycoord, -1.);
                    glTexCoord2f(1., 0.);
                    glVertex3f(1., ycoord, -1.);
                    glTexCoord2f(1., 1.);
                    glVertex3f(1., ycoord, 1.);
                    glTexCoord2f(0., 1.);
                    glVertex3f(-1., ycoord, 1.);
                    glEnd();
                }
                iterator += step;
                ycoord += dy;
            }
        }
        else if (Major == X_direction)
        {
            float x0;
            float dx;
            int iterator;
            int step;

            if (Xside == PLUS)
            {
                x0 = -1.;
                dx = 2. / (float)(tNX - 1);
                iterator = 0;
                step = 1;
            }
            else
            {
                x0 = 1.;
                dx = -2. / (float)(tNX - 1);
                iterator = tNX - 1;
                step = -1;
            }

            float xcoord = x0;
            for (int x = 0; x < tNX; x++)
            {
                if (Opacity_max != 0)
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, 4, tNX, tNY, 0, GL_RGBA,
                        GL_UNSIGNED_BYTE, &data->TextureYZ[iterator][0][0][0]);
                    glBegin(GL_QUADS);

                    glTexCoord2f(0., 0.);
                    glVertex3f(xcoord, -1., -1.);
                    glTexCoord2f(1., 0.);
                    glVertex3f(xcoord, 1., -1.);
                    glTexCoord2f(1., 1.);
                    glVertex3f(xcoord, 1., 1.);
                    glTexCoord2f(0., 1.);
                    glVertex3f(xcoord, -1., 1.);
                    glEnd();
                }
                iterator += step;
                xcoord += dx;
            }
        }


        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }
}




// OpenGL rendering of the dynamic temperature object
// 
void display_shape_dt(GLenum mode, temperature *temp)
{
    int i, j, k;
    GLfloat mat_diffuse[4];

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1., 1.);

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glShadeModel(GL_SMOOTH);
    //glShadeModel(GL_FLAT);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    // Pre-compute Min/Max
    float Smin, Smax;

    Smin = temp->grid_3d[0][0][0].T;
    Smax = Smin;

    for (i = 0; i < NX; i++)
    {
        for (j = 0; j < NY; j++)
        {
            for (k = 0; k < NZ; k++)
            {
                float S = temp->grid_3d[i][j][k].T;

                if (NonLinearOn != 0)
                {
                    S = S * S * S;
                }

                if (S < Smin)
                {
                    Smin = S;
                }
                else if (S > Smax)
                {
                    Smax = S;
                }
            }
        }
    }

    if (IsosurfaceOn == 1)
    {
        // Draw Wireframes
        for (int z = 0; z < temp->intersection_lines.size(); z++)
        {
            MarchingSquares_DrawIntersection2(temp->intersection_lines[z][0], temp->intersection_lines[z][1]);
        }
    }

    float x;
    float y;
    float z;
    float S;
    float gradient;

    if (XY_on == 1)
    {
        // XY Plane
        k = XY_index;
        if (k >= NZ)
        {
            k = NZ - 1;
        }
        else if (k < 0)
        {
            k = 0;
        }

        for (i = 0; i < NX - 1; i++)
        {
            for (j = 0; j < NY - 1; j++)
            {
                glBegin(GL_POLYGON);
                for (int it = 0; it < 4; it++)
                {
                    node_temperature vert;

                    if (it == 0)
                    {
                        vert = temp->grid_3d[i][j][k];
                    }
                    else if (it == 1)
                    {
                        vert = temp->grid_3d[i + 1][j][k];
                    }
                    else if (it == 2)
                    {
                        vert = temp->grid_3d[i + 1][j + 1][k];
                    }
                    else //if (it == 3)
                    {
                        vert = temp->grid_3d[i][j + 1][k];
                    }

                    S = vert.T;
                    x = vert.x;
                    y = vert.y;
                    z = vert.z;
                    gradient = vert.grad;

                    if (S < S_min || S > S_max)
                    {
                        continue;
                    }
                    if (gradient < Gradient_min || gradient > Gradient_max)
                    {
                        continue;
                    }

                    if (NonLinearOn != 0)
                    {
                        S = S * S * S;
                    }

                    float normalized_value = (S - Smin) / (Smax - Smin);

                    if (ColorSchemeId == BLUE_WHITE_RED)
                    {
                        float rgb[3];
                        BlueWhiteRed(rgb, normalized_value, ColorSchemeMidPoint);
                        glColor3f(rgb[0], rgb[1], rgb[2]);
                    }
                    else if (ColorSchemeId == HEAT_MAP)
                    {
                        float rgb[3];
                        HeatMap(rgb, normalized_value);
                        glColor3f(rgb[0], rgb[1], rgb[2]);
                    }
                    else if (ColorSchemeId == RAINBOW)
                    {
                        float rgb[3];
                        Rainbow(rgb, normalized_value);
                        glColor3f(rgb[0], rgb[1], rgb[2]);
                    }
                    else if (ColorSchemeId == DISCRETE)
                    {
                        float rgb[3];
                        DiscreteColorScheme(rgb, normalized_value);
                        glColor3f(rgb[0], rgb[1], rgb[2]);
                    }

                    glNormal3d(0, 0, 1);
                    glVertex3d(x, y, z);
                }
                glEnd();
            }
        }
    }

    if (XZ_on == 1)
    {
        // XZ Plane
        j = XZ_index;
        if (j >= NY)
        {
            j = NY - 1;
        }
        else if (j < 0)
        {
            j = 0;
        }

        for (i = 0; i < NX - 1; i++)
        {
            for (k = 0; k < NZ - 1; k++)
            {
                glBegin(GL_POLYGON);
                for (int it = 0; it < 4; it++)
                {
                    node_temperature vert;

                    if (it == 0)
                    {
                        vert = temp->grid_3d[i][j][k];
                    }
                    else if (it == 1)
                    {
                        vert = temp->grid_3d[i + 1][j][k];
                    }
                    else if (it == 2)
                    {
                        vert = temp->grid_3d[i + 1][j][k + 1];
                    }
                    else if (it == 3)
                    {
                        vert = temp->grid_3d[i][j][k + 1];
                    }

                    S = vert.T;
                    x = vert.x;
                    y = vert.y;
                    z = vert.z;
                    gradient = vert.grad;

                    if (S < S_min || S > S_max)
                    {
                        continue;
                    }
                    if (gradient < Gradient_min || gradient > Gradient_max)
                    {
                        continue;
                    }

                    if (NonLinearOn != 0)
                    {
                        S = S * S * S;
                    }

                    float normalized_value = (S - Smin) / (Smax - Smin);

                    if (ColorSchemeId == BLUE_WHITE_RED)
                    {
                        float rgb[3];
                        BlueWhiteRed(rgb, normalized_value, ColorSchemeMidPoint);
                        glColor3f(rgb[0], rgb[1], rgb[2]);
                    }
                    else if (ColorSchemeId == HEAT_MAP)
                    {
                        float rgb[3];
                        HeatMap(rgb, normalized_value);
                        glColor3f(rgb[0], rgb[1], rgb[2]);
                    }
                    else if (ColorSchemeId == RAINBOW)
                    {
                        float rgb[3];
                        Rainbow(rgb, normalized_value);
                        glColor3f(rgb[0], rgb[1], rgb[2]);
                    }
                    else if (ColorSchemeId == DISCRETE)
                    {
                        float rgb[3];
                        DiscreteColorScheme(rgb, normalized_value);
                        glColor3f(rgb[0], rgb[1], rgb[2]);
                    }

                    glNormal3d(0, 0, 1);
                    glVertex3d(x, y, z);
                }
                glEnd();
            }
        }
    }

    if (YZ_on == 1)
    {
        // YZ Plane
        i = YZ_index;
        if (i >= NX)
        {
            i = NX - 1;
        }
        else if (i < 0)
        {
            i = 0;
        }

        for (j = 0; j < NY - 1; j++)
        {
            for (k = 0; k < NZ - 1; k++)
            {
                glBegin(GL_POLYGON);
                for (int it = 0; it < 4; it++)
                {
                    node_temperature vert;

                    if (it == 0)
                    {
                        vert = temp->grid_3d[i][j][k];
                    }
                    else if (it == 1)
                    {
                        vert = temp->grid_3d[i][j + 1][k];
                    }
                    else if (it == 2)
                    {
                        vert = temp->grid_3d[i][j + 1][k + 1];
                    }
                    else if (it == 3)
                    {
                        vert = temp->grid_3d[i][j][k + 1];
                    }

                    S = vert.T;
                    x = vert.x;
                    y = vert.y;
                    z = vert.z;
                    gradient = vert.grad;

                    if (S < S_min || S > S_max)
                    {
                        continue;
                    }
                    if (gradient < Gradient_min || gradient > Gradient_max)
                    {
                        continue;
                    }

                    if (NonLinearOn != 0)
                    {
                        S = S * S * S;
                    }

                    float normalized_value = (S - Smin) / (Smax - Smin);

                    if (ColorSchemeId == BLUE_WHITE_RED)
                    {
                        float rgb[3];
                        BlueWhiteRed(rgb, normalized_value, ColorSchemeMidPoint);
                        glColor3f(rgb[0], rgb[1], rgb[2]);
                    }
                    else if (ColorSchemeId == HEAT_MAP)
                    {
                        float rgb[3];
                        HeatMap(rgb, normalized_value);
                        glColor3f(rgb[0], rgb[1], rgb[2]);
                    }
                    else if (ColorSchemeId == RAINBOW)
                    {
                        float rgb[3];
                        Rainbow(rgb, normalized_value);
                        glColor3f(rgb[0], rgb[1], rgb[2]);
                    }
                    else if (ColorSchemeId == DISCRETE)
                    {
                        float rgb[3];
                        DiscreteColorScheme(rgb, normalized_value);
                        glColor3f(rgb[0], rgb[1], rgb[2]);
                    }

                    glNormal3d(0, 0, 1);
                    glVertex3d(x, y, z);
                }
                glEnd();
            }
        }
    }

    //Assignment 5
    if (VolumeRenderingOn == 1)
    {
        DetermineVisibility();

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        // Choose Bilinear or Not
        int filter = GL_NEAREST;
        if (BilinearOn == 1)
            filter = GL_LINEAR;

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);


        if (Major == Z_direction)
        {
            float z0;
            float dz;
            int iterator;
            int step;

            if (Zside == PLUS)
            {
                z0 = -1.;
                dz = 2. / (float)(tNZ - 1);
                iterator = 0;
                step = 1;
            }
            else
            {
                z0 = 1.;
                dz = -2. / (float)(tNZ - 1);
                iterator = NZ - 1;
                step = -1;
            }

            float zcoord = z0;
            for (int z = 0; z < NZ; z++)
            {
                if (Opacity_max != 0)
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, 4, tNX, tNY, 0, GL_RGBA,
                        GL_UNSIGNED_BYTE, &temp->TextureXY[iterator][0][0][0]);
                    glBegin(GL_QUADS);

                    glTexCoord2f(0., 0.);
                    glVertex3f(-1., -1., zcoord);
                    glTexCoord2f(1., 0.);
                    glVertex3f(1., -1., zcoord);
                    glTexCoord2f(1., 1.);
                    glVertex3f(1., 1., zcoord);
                    glTexCoord2f(0., 1.);
                    glVertex3f(-1., 1., zcoord);
                    glEnd();
                }
                iterator += step;
                zcoord += dz;
            }
        }
        else if (Major == Y_direction)
        {
            float y0;
            float dy;
            int iterator;
            int step;

            if (Yside == PLUS)
            {
                y0 = -1.;
                dy = 2. / (float)(tNY - 1);
                iterator = 0;
                step = 1;
            }
            else
            {
                y0 = 1.;
                dy = -2. / (float)(tNY - 1);
                iterator = tNY - 1;
                step = -1;
            }

            float ycoord = y0;
            for (int y = 0; y < tNY; y++)
            {
                if (Opacity_max != 0)
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, 4, tNX, tNZ, 0, GL_RGBA,
                        GL_UNSIGNED_BYTE, &temp->TextureXZ[iterator][0][0][0]);
                    glBegin(GL_QUADS);

                    glTexCoord2f(0., 0.);
                    glVertex3f(-1., ycoord, -1.);
                    glTexCoord2f(1., 0.);
                    glVertex3f(1., ycoord, -1.);
                    glTexCoord2f(1., 1.);
                    glVertex3f(1., ycoord, 1.);
                    glTexCoord2f(0., 1.);
                    glVertex3f(-1., ycoord, 1.);
                    glEnd();
                }
                iterator += step;
                ycoord += dy;
            }
        }
        else if (Major == X_direction)
        {
            float x0;
            float dx;
            int iterator;
            int step;

            if (Xside == PLUS)
            {
                x0 = -1.;
                dx = 2. / (float)(tNX - 1);
                iterator = 0;
                step = 1;
            }
            else
            {
                x0 = 1.;
                dx = -2. / (float)(tNX - 1);
                iterator = tNX - 1;
                step = -1;
            }

            float xcoord = x0;
            for (int x = 0; x < tNX; x++)
            {
                if (Opacity_max != 0)
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, 4, tNX, tNY, 0, GL_RGBA,
                        GL_UNSIGNED_BYTE, &temp->TextureYZ[iterator][0][0][0]);
                    glBegin(GL_QUADS);

                    glTexCoord2f(0., 0.);
                    glVertex3f(xcoord, -1., -1.);
                    glTexCoord2f(1., 0.);
                    glVertex3f(xcoord, 1., -1.);
                    glTexCoord2f(1., 1.);
                    glVertex3f(xcoord, 1., 1.);
                    glTexCoord2f(0., 1.);
                    glVertex3f(xcoord, -1., 1.);
                    glEnd();
                }
                iterator += step;
                xcoord += dx;
            }
        }


        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }
}

void DetermineVisibility()
{
    float xr, yr;
    float cx, sx;
    float cy, sy;
    float nzx, nzy, nzz; /* z component of normal for x side, y side, and z side */
    xr = Xrot * (M_PI / 180.);
    yr = Yrot * (M_PI / 180.);
    cx = cos(xr);
    sx = sin(xr);
    cy = cos(yr);
    sy = sin(yr);
    nzx = -sy;
    nzy = sx * cy;
    nzz = cx * cy;
    /* which sides of the cube are showing: */
    /* the Xside being shown to the user is MINUS or PLUS */
    Xside = (nzx > 0. ? PLUS : MINUS);
    Yside = (nzy > 0. ? PLUS : MINUS);
    Zside = (nzz > 0. ? PLUS : MINUS);
    /* which direction needs to be composited: */
    if (fabs(nzx) > fabs(nzy) && fabs(nzx) > fabs(nzz))
        Major = X_direction;
    else if (fabs(nzy) > fabs(nzx) && fabs(nzy) > fabs(nzz))
        Major = Y_direction;
    else
        Major = Z_direction;
}

// OpenGL rendering of the .dat object
// QUAD MESH
void display_shape_dat(GLenum mode, dat *data)
{
    unsigned int i, j;
    GLfloat mat_diffuse[4];

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1., 1.);

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    // Pre-compute Min/Max
    float Smin, Smax;

    for (i = 0; i < data->vertexList.size(); i++)
    {
        float S = data->vertexList[i].s;

        if (NonLinearOn != 0)
        {
            S = S * S * S;
        }

        if (i == 0)
        {
            Smin = S;
            Smax = S;
        }
        else
        {
            if (S < Smin)
            {
                Smin = S;
            }
            else if (S > Smax)
            {
                Smax = S;
            }
        }
    }

    // Render the polygonal mesh
    for (int i = 0; i < data->quadList.size(); i++)
    {
        //MARCHING SQUARES ALGORITHM
        node vert0 = data->vertexList[data->quadList[i].verts[0]];
        node vert1 = data->vertexList[data->quadList[i].verts[1]];
        node vert2 = data->vertexList[data->quadList[i].verts[2]];
        node vert3 = data->vertexList[data->quadList[i].verts[3]];

        for (int z = 0; z < data->quadList[i].lines.size(); z++)
        {
            MarchingSquares_DrawIntersection(data->quadList[i].lines[z][0], data->quadList[i].lines[z][1]);
        }

        glBegin(GL_QUADS);
        for (j = 0; j < 4; j++)
        {
            node vert;
            if (j == 0)
            {
                vert = vert0;
            }
            else if (j == 1)
            {
                vert = vert1;
            }
            else if (j == 2)
            {
                vert = vert2;
            }
            else if (j == 3)
            {
                vert = vert3;
            }

            float S = vert.s;

            if (NonLinearOn != 0)
            {
                S = S * S * S;
            }

            float normalized_value = (S - Smin) / (Smax - Smin);

            if (ColorSchemeId == BLUE_WHITE_RED)
            {
                float rgb[3];
                BlueWhiteRed(rgb, normalized_value, ColorSchemeMidPoint);
                glColor3f(rgb[0], rgb[1], rgb[2]);
            }
            else if (ColorSchemeId == HEAT_MAP)
            {
                float rgb[3];
                HeatMap(rgb, normalized_value);
                glColor3f(rgb[0], rgb[1], rgb[2]);
            }
            else if (ColorSchemeId == RAINBOW)
            {
                float rgb[3];
                Rainbow(rgb, normalized_value);
                glColor3f(rgb[0], rgb[1], rgb[2]);
            }
            else if (ColorSchemeId == DISCRETE)
            {
                float rgb[3];
                DiscreteColorScheme(rgb, normalized_value);
                glColor3f(rgb[0], rgb[1], rgb[2]);
            }

            glNormal3d(vert.nx, vert.ny, vert.nz);
            glVertex3d(vert.x, vert.y, vert.z);
        }
        glEnd();
    }
}

// OpenGL rendering of the .ply object
// TRIANGLE MESH
void display_shape(GLenum mode, Polyhedron *this_poly)
{
    unsigned int i, j;
    GLfloat mat_diffuse[4];

    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1., 1.);

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Set the material property (esp. color) of the object
    //mat_diffuse[0] = 0.6;
    //mat_diffuse[1] = 0.8;
    //mat_diffuse[2] = 0.7;
    //mat_diffuse[3] = 1.0;
    //glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    // Pre-compute Min/Max
    float Smin, Smax;
    float Smin_Color, Smax_Color;

    for (i = 0; i < this_poly->ntris; i++)
    {
        Triangle *temp_t = this_poly->tlist[i];
        for (j = 0; j < 3; j++)
        {
            Vertex *temp_v = temp_t->verts[j];
            float S = temp_v->s;
            float S_Color = S;

            if (NonLinearOn != 0)
            {
                S_Color = S * S * S;
            }

            if (i == 0 && j == 0)
            {
                Smin = S;
                Smax = S;

                Smin_Color = S_Color;
                Smax_Color = S_Color;
            }
            else
            {
                if (S < Smin)
                {
                    Smin = S;
                }
                else if (S > Smax)
                {
                    Smax = S;
                }

                if (S_Color < Smin_Color)
                {
                    Smin_Color = S_Color;
                }
                else if (S_Color > Smax_Color)
                {
                    Smax_Color = S_Color;
                }
            }
        }
    }

    // Render the polygonal mesh
    for (i = 0; i < this_poly->ntris; i++) {
        if (mode == GL_SELECT)
            glLoadName(i + 1);

        Triangle *temp_t = this_poly->tlist[i];
        {
            for (int iter = 0; iter < NumberOfContours; iter++)
            {
                float s_star = 0;
                if (NumberOfContours == 1)
                {
                    s_star = ScalarValue;
                }
                else if (NumberOfContours > 1)
                {
                    s_star = ((Smax - Smin) / (NumberOfContours + 1)) * (iter + 1);
                }

                std::vector<node> intersection_locations;

                int intersections = 0;
                MarchingTriangles_FindIntersection(s_star, *temp_t->verts[0], *temp_t->verts[1], intersections, intersection_locations);
                MarchingTriangles_FindIntersection(s_star, *temp_t->verts[1], *temp_t->verts[2], intersections, intersection_locations);
                MarchingTriangles_FindIntersection(s_star, *temp_t->verts[2], *temp_t->verts[0], intersections, intersection_locations);

                if (intersections == 2)
                {
                    // Connect Edges
                    MarchingSquares_DrawIntersection(intersection_locations[0], intersection_locations[1]);
                }
                else if (intersections == 3)
                {
                    std::cout << "UNHANDLED\n";
                }
            }

            glBegin(GL_POLYGON);
            for (j = 0; j < 3; j++)
            {  
                Vertex *temp_v = temp_t->verts[j];

                float S = temp_v->s;

                if (NonLinearOn != 0)
                {
                    S = S * S * S;
                }

                float normalized_value = (S - Smin_Color) / (Smax_Color - Smin_Color);

                if (ColorSchemeId == BLUE_WHITE_RED)
                {
                    float rgb[3];
                    BlueWhiteRed(rgb, normalized_value, ColorSchemeMidPoint);
                    glColor3f(rgb[0], rgb[1], rgb[2]);
                }
                else if (ColorSchemeId == HEAT_MAP)
                {
                    float rgb[3];
                    HeatMap(rgb, normalized_value);
                    glColor3f(rgb[0], rgb[1], rgb[2]);
                }
                else if (ColorSchemeId == RAINBOW)
                {
                    float rgb[3];
                    Rainbow(rgb, normalized_value);
                    glColor3f(rgb[0], rgb[1], rgb[2]);
                }
                else if (ColorSchemeId == DISCRETE)
                {
                    float rgb[3];
                    DiscreteColorScheme(rgb, normalized_value);
                    glColor3f(rgb[0], rgb[1], rgb[2]);
                }

                glNormal3d(temp_v->normal.entry[0], temp_v->normal.entry[1], temp_v->normal.entry[2]);
                glVertex3d(temp_v->x, temp_v->y, temp_v->z);
            }
            glEnd();
        }
    }
}

void MarchingTriangles_FindIntersection(float s_star, Vertex &vert0, Vertex &vert1, int &intersections, std::vector<node> &intersection_locations)
{
    float t_star = (s_star - vert0.s) / (vert1.s - vert0.s);

    if (t_star >= 0 && t_star <= 1)
    {
        intersections++;

        float x_star = (1 - t_star) * vert0.x + t_star * vert1.x;
        float y_star = (1 - t_star) * vert0.y + t_star * vert1.y;
        float z_star = (1 - t_star) * vert0.z + t_star * vert1.z;

        node intersection_location;
        intersection_location.x = x_star;
        intersection_location.y = y_star;
        intersection_location.z = z_star;

        intersection_locations.push_back(intersection_location);
    }
}

void MarchingSquares_DrawIntersection(node edge1_intersection, node edge2_intersection)
{
    glColor3f(0, 0, 0);
    glBegin(GL_LINES);
    glNormal3d(0, 0, 1);
    glVertex3d(edge1_intersection.x, edge1_intersection.y, edge1_intersection.z);
    glVertex3d(edge2_intersection.x, edge2_intersection.y, edge2_intersection.z);
    glEnd();
}

void MarchingSquares_DrawIntersection2(intersection_temperature edge1_intersection, intersection_temperature edge2_intersection)
{
    glColor3f(0, 0, 0);
    glBegin(GL_LINES);
    glNormal3d(0, 0, 1);
    glVertex3d(edge1_intersection.x, edge1_intersection.y, edge1_intersection.z);
    glVertex3d(edge2_intersection.x, edge2_intersection.y, edge2_intersection.z);
    glEnd();
}

// NOT USED
//void Display_Model(void)
//{
//	GLint viewport[4];
//	int jitter;
//
//	glClearColor (1.0, 1.0, 1.0, 1.0);  // background for rendering color coding and lighting
//	glGetIntegerv (GL_VIEWPORT, viewport);
// 
//	set_view(GL_RENDER, poly);
//	set_scene(GL_RENDER, poly);
//	display_shape(GL_RENDER, poly);
//	glFlush();
//	glutSwapBuffers();
//	glFinish();
//}

void Choose_Object()
{
    int w, h;
    switch(ObjectId){
        case STREAMLINES:
            strcpy(object_name, "streamlines");
            object_type = STREAMLINE_TYPE;
            break;
        case DYNAMIC_TEMPERATURE:
            strcpy(object_name, "dynamic_temperature");
            object_type = DYNAMIC_TYPE;
            break;
        case TEMPERATURE1:
            strcpy(object_name, "temperature1");
            object_type = DAT_TYPE;
            break;
        case TEMPERATURE2:
            strcpy(object_name, "temperature2");
            object_type = DAT_TYPE;
            break;
        case DIESEL_FIELD1:
            strcpy(object_name, "diesel_field1");
            object_type = PLY_TYPE;
            break;
        case DISTANCE_FIELD1:
            strcpy(object_name, "distance_field1");
            object_type = PLY_TYPE;
            break;
        case DISTANCE_FIELD2:
            strcpy(object_name, "distance_field2");
            object_type = PLY_TYPE;
            break;
        case ICELAND_CURRENT_FIELD:
            strcpy(object_name, "iceland_current_field");
            object_type = PLY_TYPE;
            break;
        case TORUS_FIELD:
            strcpy(object_name, "torus_field");
            object_type = PLY_TYPE;
            break;
    }
    Reset();

    if (object_type == PLY_TYPE) // Initialize PLY
    {
        if (poly != NULL)
        {
            poly->finalize();
        }

        char tmp_str[512];

        sprintf(tmp_str, "../models/%s.ply", object_name);

        FILE *this_file = fopen(tmp_str, "r");
        poly = new Polyhedron(this_file);
        fclose(this_file);

        ////Following codes build the edge information
        clock_t start, finish; //Used to show the time assumed
        start = clock(); //Get the first time

        poly->initialize(); // initialize everything

        poly->calc_bounding_sphere();
        poly->calc_face_normals_and_area();
        poly->average_normals();

        finish = clock(); //Get the current time after finished
        double t = (double)(finish - start) / CLOCKS_PER_SEC;

        printf("\n");
        printf("The number of the edges of the object %s is %d \n", object_name, poly->nedges);
        printf("The Euler Characteristics of the object %s is %d \n", object_name, (poly->nverts - poly->nedges + poly->ntris));

        printf("Time to building the edge link is %f seconds\n", t);
    }
    else if (object_type == DAT_TYPE) // Initialize DAT
    {
        char tmp_str[512];
        sprintf(tmp_str, "../models/%s.dat", object_name);

        data = new dat(tmp_str);
        data->Build_Edge_List();
        data->Build_Face_List();
        data->Build_Lines(ScalarValue, NumberOfContours);
    }
    else if (object_type == DYNAMIC_TYPE) // Initialize Dynamic
    {
        //TEMP
        //temperature_data = new temperature(NX, NY, NZ, ColorSchemeId, ColorSchemeMidPoint);
        temperature_data = new temperature(64, 64, 64, ColorSchemeId, ColorSchemeMidPoint);
        temperature_data->GetGradientMinMax(Gradient_min, Gradient_max);
        temperature_data->Build_Lines(ScalarValue, NumberOfContours);
        temperature_data->CompositeXY(Opacity_max, S_min, S_max);
        temperature_data->CompositeXZ(Opacity_max, S_min, S_max);
        temperature_data->CompositeYZ(Opacity_max, S_min, S_max);
    }
    else if (object_type == STREAMLINE_TYPE)
    {
        streamline_data = new streamlines(NX, NY, NZ);
        streamline_data->get_entropy_data(FieldId);
        streamline_data->CompositeXY(Opacity_max, S_min, S_max);
        streamline_data->CompositeXZ(Opacity_max, S_min, S_max);
        streamline_data->CompositeYZ(Opacity_max, S_min, S_max);
        streamline_data->GetEntropyStreamlines(FieldId, NumericalIntegrationTypeId, p_value, q_value);
        streamline_data->get_streamtapes();
    }
				
    Glui->sync_live();
    glutSetWindow( MainWindow );
    glutPostRedisplay();
}

void Choose_Color_Scheme()
{
    if (object_type == DYNAMIC_TYPE)
    {
        temperature_data->Calculate_Colors(ColorSchemeId, ColorSchemeMidPoint);
        temperature_data->CompositeXY(Opacity_max, S_min, S_max);
        temperature_data->CompositeXZ(Opacity_max, S_min, S_max);
        temperature_data->CompositeYZ(Opacity_max, S_min, S_max);
    }
    else if (object_type == STREAMLINE_TYPE)
    {
        streamline_data->get_entropy_data(FieldId);
        streamline_data->Calculate_Colors(ColorSchemeId, ColorSchemeMidPoint);
        streamline_data->CompositeXY(Opacity_max, S_min, S_max);
        streamline_data->CompositeXZ(Opacity_max, S_min, S_max);
        streamline_data->CompositeYZ(Opacity_max, S_min, S_max);
        streamline_data->GetEntropyStreamlines(FieldId, NumericalIntegrationTypeId, p_value, q_value);
        streamline_data->get_streamtapes();
    }
}

void Choose_Color_Scheme_MidPoint()
{
    if (ColorSchemeMidPoint > 1)
    {
        ColorSchemeMidPoint = 1;
    }
    else if (ColorSchemeMidPoint < 0)
    {
        ColorSchemeMidPoint = 0;
    }
}

void Choose_IsoContour_ScalarValue()
{
    if (object_type == DAT_TYPE)
    {
        data->Build_Lines(ScalarValue, NumberOfContours);
    }
    else if (object_type == DYNAMIC_TYPE)
    {
        temperature_data->Build_Lines(ScalarValue, NumberOfContours);
    }
}

void Choose_IsoContour_NumberOfContours()
{
    if (NumberOfContours < 0)
    {
        NumberOfContours = 0;
    }

    if (object_type == DAT_TYPE)
    {
        data->Build_Lines(ScalarValue, NumberOfContours);
    }
    else if (object_type == DYNAMIC_TYPE)
    {
        temperature_data->Build_Lines(ScalarValue, NumberOfContours);
    }
}

void Choose_XY_Index()
{
    //if (XY_index < 0)
    //{
    //    XY_index = 0;
    //}
    //else if (XY_index >= NZ)
    //{
    //    XY_index = NZ - 1;
    //}
}

void Choose_XZ_Index()
{
    //if (XZ_index < 0)
    //{
    //    XZ_index = 0;
    //}
    //else if (XZ_index >= NY)
    //{
    //    XZ_index = NY - 1;
    //}
}

void Choose_YZ_Index()
{
    //if (YZ_index < 0)
    //{
    //    YZ_index = 0;
    //}
    //else if (YZ_index >= NX)
    //{
    //    YZ_index = NX - 1;
    //}
}

void Choose_Smin()
{
    if (object_type == DYNAMIC_TYPE)
    {
        // Choose Smin
        temperature_data->CompositeXY(Opacity_max, S_min, S_max);
        temperature_data->CompositeXZ(Opacity_max, S_min, S_max);
        temperature_data->CompositeYZ(Opacity_max, S_min, S_max);
    }
    else if (object_type == STREAMLINE_TYPE)
    {
        streamline_data->CompositeXY(Opacity_max, S_min, S_max);
        streamline_data->CompositeXZ(Opacity_max, S_min, S_max);
        streamline_data->CompositeYZ(Opacity_max, S_min, S_max);
    }
}

void Choose_Smax()
{
    if (object_type == DYNAMIC_TYPE)
    {
        // Choose Smax
        temperature_data->CompositeXY(Opacity_max, S_min, S_max);
        temperature_data->CompositeXZ(Opacity_max, S_min, S_max);
        temperature_data->CompositeYZ(Opacity_max, S_min, S_max);
    }
    else if (object_type == STREAMLINE_TYPE)
    {
        streamline_data->CompositeXY(Opacity_max, S_min, S_max);
        streamline_data->CompositeXZ(Opacity_max, S_min, S_max);
        streamline_data->CompositeYZ(Opacity_max, S_min, S_max);
    }
}

void Choose_Opacity()
{
    if (object_type == DYNAMIC_TYPE)
    {
        // Choose Opacity
        temperature_data->CompositeXY(Opacity_max, S_min, S_max);
        temperature_data->CompositeXZ(Opacity_max, S_min, S_max);
        temperature_data->CompositeYZ(Opacity_max, S_min, S_max);
    }
    else if (object_type == STREAMLINE_TYPE)
    {
        streamline_data->CompositeXY(Opacity_max, S_min, S_max);
        streamline_data->CompositeXZ(Opacity_max, S_min, S_max);
        streamline_data->CompositeYZ(Opacity_max, S_min, S_max);
    }
}

void Choose_FieldId()
{
    if (object_type == STREAMLINE_TYPE)
    {
        streamline_data->get_entropy_data(FieldId);
        streamline_data->Calculate_Colors(ColorSchemeId, ColorSchemeMidPoint);
        streamline_data->CompositeXY(Opacity_max, S_min, S_max);
        streamline_data->CompositeXZ(Opacity_max, S_min, S_max);
        streamline_data->CompositeYZ(Opacity_max, S_min, S_max);
        streamline_data->GetEntropyStreamlines(FieldId, NumericalIntegrationTypeId, p_value, q_value);
        streamline_data->get_streamtapes();
    }
}

void Choose_NumericalIntegration()
{
    if (object_type == STREAMLINE_TYPE)
    {
        streamline_data->get_entropy_data(FieldId);
        streamline_data->Calculate_Colors(ColorSchemeId, ColorSchemeMidPoint);
        streamline_data->CompositeXY(Opacity_max, S_min, S_max);
        streamline_data->CompositeXZ(Opacity_max, S_min, S_max);
        streamline_data->CompositeYZ(Opacity_max, S_min, S_max);
        streamline_data->GetEntropyStreamlines(FieldId, NumericalIntegrationTypeId, p_value, q_value);
        streamline_data->get_streamtapes();
    }
}

void Choose_PQValue()
{
    if (object_type == STREAMLINE_TYPE)
    {
        streamline_data->get_entropy_data(FieldId);
        streamline_data->Calculate_Colors(ColorSchemeId, ColorSchemeMidPoint);
        streamline_data->CompositeXY(Opacity_max, S_min, S_max);
        streamline_data->CompositeXZ(Opacity_max, S_min, S_max);
        streamline_data->CompositeYZ(Opacity_max, S_min, S_max);
        streamline_data->GetEntropyStreamlines(FieldId, NumericalIntegrationTypeId, p_value, q_value);
        streamline_data->get_streamtapes();
    }
}

void Choose_NULL()
{

}