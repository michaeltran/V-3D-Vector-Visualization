/****************************************************************************

  GLUI User Interface Toolkit
  ---------------------------

     glui_commandline.cpp - GLUI_CommandLine control class


      --------------------------------------------------

  Copyright (c) 1998 Paul Rademacher, 2005 William Baxter

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
  3. This notice may not be removed or altered from any source distribution.

  This program is -not- in the public domain.

*****************************************************************************/
#include <Windows.h>
#include "include\GL\glui.h"
#include "glui_internal.h"

/****************************** GLUI_CommandLine::GLUI_CommandLine() **********/
GLUI_CommandLine::GLUI_CommandLine( GLUI_Node *parent, const GLUI_String &name,
                                    void *data, int id, GLUI_CB cb )
{
  common_init();
  set_name( name );

  data_type   = GLUI_EDITTEXT_TEXT;
  ptr_val     = data;
  user_id     = id;
  callback    = cb;

  live_type = GLUI_LIVE_TEXT;

  parent->add_control( this );

  init_live();
}

/****************************** GLUI_CommandLine::key_handler() **********/

int    GLUI_CommandLine::key_handler( unsigned char key,int modifiers )
{
  int ret;

  if ( NOT glui )
    return false;

  if ( debug )
    dump( stdout, "-> CMD_TEXT KEY HANDLER" );

  if ( key == 13 ) {           /* RETURN */
    commit_flag = true;
  }

  ret = Super::key_handler( key, modifiers );

  if ( debug )
    dump( stdout, "<- CMD_TEXT KEY HANDLER" );

  return ret;
}


/****************************** GLUI_CommandLine::deactivate() **********/

void    GLUI_CommandLine::deactivate()
{
  // if the commit_flag is set, add the current command to
  // history and call deactivate as normal

  // Trick deactivate into calling callback if and only if commit_flag set.
  // A bit subtle, but deactivate checks that orig_text and text
  // are the same to decide whether or not to call the callback.
  // Force them to be different for commit, and the same for no commit.
  if (commit_flag) {
    add_to_history(text.c_str());
    orig_text = "";
    Super::deactivate( );
    set_text( "" );
    commit_flag = false;
  }
  else {
    orig_text = text;
  }
}

/**************************** GLUI_CommandLine::special_handler() **********/

int    GLUI_CommandLine::special_handler( int key,int modifiers )
{
  if ( NOT glui )
    return false;

  if ( debug )
    printf( "CMD_TEXT SPECIAL:%d - mod:%d   subs:%d/%d  ins:%d  sel:%d/%d\n",
	    key, modifiers, substring_start, substring_end,insertion_pt,
	    sel_start, sel_end );

  if ( key == GLUT_KEY_UP )  // PREVIOUS HISTORY
  {
    scroll_history(-1);
  }
  else if ( key == GLUT_KEY_DOWN )  // NEXT HISTORY
  {
    scroll_history(+1);
  }
  else {
    return Super::special_handler( key, modifiers );
  }
  return false;
}



/**************************** GLUI_CommandLine::scroll_history() ********/

void    GLUI_CommandLine::scroll_history( int direction )
{
  recall_history(curr_hist + direction);
}

/**************************** GLUI_CommandLine::recall_history() ********/

void    GLUI_CommandLine::recall_history( int hist_num )
{
  if (hist_num < oldest_hist OR
      hist_num > newest_hist OR
      hist_num == curr_hist)
    return;

  // Commit the current text first before we blow it away!
  if (curr_hist == newest_hist) {
    get_history_str(newest_hist) = text;
  }

  curr_hist = hist_num;
  set_text(get_history_str(curr_hist));
  sel_end = sel_start = insertion_pt = (int)text.length();
  update_and_draw_text();
}

/**************************** GLUI_CommandLine::add_to_history() ********/

void    GLUI_CommandLine::add_to_history( const GLUI_String &cmd )
{
  if (cmd.empty()) return; // don't add if it's empty

  curr_hist = newest_hist;
  get_history_str(newest_hist) = text;

  newest_hist = ++curr_hist;
  if ( newest_hist >= HIST_SIZE )
  {
    // bump oldest off the list
    hist_list.erase(hist_list.begin());
    hist_list.push_back("");

    oldest_hist++;
  }
}

/**************************** GLUI_CommandLine::reset_history() ********/

void    GLUI_CommandLine::reset_history()
{
  oldest_hist = newest_hist = curr_hist = 0;
}



/*************************************** GLUI_CommandLine::dump() **************/

void   GLUI_CommandLine::dump( FILE *out, const GLUI_String &name )
{
  fprintf( out,
	   "%s (commandline@%p):  ins_pt:%d  subs:%d/%d  sel:%d/%d   len:%d\n",
	   name.c_str(), this,
	   insertion_pt, substring_start, substring_end, sel_start, sel_end,
	   (int)text.length());
}


