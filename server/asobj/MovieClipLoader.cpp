// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
//
//

// Implementation of ActionScript MovieClipLoader class.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"
#include "as_function.h"
#include "MovieClipLoader.h"
#include "movie_definition.h"
#include "tu_file.h"
#include "image.h"
//#include "render.h"
//#include "impl.h"
#include "URL.h"
#include "GnashException.h"
#include "sprite_instance.h"
#include "character.h"
#include "fn_call.h"


#ifdef HAVE_LIBXML
// TODO: http and sockets and such ought to be factored out into an
// abstract driver, like we do for file access.
#include <libxml/nanohttp.h>
#ifdef HAVE_WINSOCK
# include <windows.h>
# include <sys/stat.h>
# include <io.h>
#else
# include <unistd.h>
# include <fcntl.h>
#endif
#endif

#include "log.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <typeinfo> 
#include <string>

namespace gnash {

  
  MovieClipLoader::MovieClipLoader()
      // :     character(0, 0)
{
  log_msg("%s: \n", __FUNCTION__);
  _mcl.bytes_loaded = 0;
  _mcl.bytes_total = 0;  
}

MovieClipLoader::~MovieClipLoader()
{
  log_msg("%s: \n", __FUNCTION__);
}

void
MovieClipLoader::load(const tu_string& /*filespec*/)
{
  log_msg("%s: \n", __FUNCTION__);
}

// progress of the downloaded file(s).
struct mcl *
MovieClipLoader::getProgress(as_object* /*ao*/)
{
  //log_msg("%s: \n", __FUNCTION__);

  return &_mcl;
}


bool
MovieClipLoader::loadClip(const tu_string&, void *)
{
  log_msg("%s: \n", __FUNCTION__);

  return false;
}

void
MovieClipLoader::unloadClip(void *)
{
  log_msg("%s: \n", __FUNCTION__);
}


void
MovieClipLoader::addListener(void *)
{
  log_msg("%s: \n", __FUNCTION__);
}


void
MovieClipLoader::removeListener(void *)
{
  log_msg("%s: \n", __FUNCTION__);
}

  
// Callbacks
void
MovieClipLoader::onLoadStart(void *)
{
  log_msg("%s: \n", __FUNCTION__);
}

void
MovieClipLoader::onLoadProgress(void *)
{
  log_msg("%s: \n", __FUNCTION__);
}

void
MovieClipLoader::onLoadInit(void *)
{
  log_msg("%s: \n", __FUNCTION__);
}

void
MovieClipLoader::onLoadComplete(void *)
{
  log_msg("%s: \n", __FUNCTION__);
}

void
MovieClipLoader::onLoadError(void *)
{
  log_msg("%s: \n", __FUNCTION__);
}

void
MovieClipLoader::on_button_event(event_id event)
{
  log_msg("%s: \n", __FUNCTION__);
  
  // Set our mouse state (so we know how to render).
  switch (event.m_id)
    {
    case event_id::ROLL_OUT:
    case event_id::RELEASE_OUTSIDE:
      _mouse_state = MOUSE_UP;
      break;
      
    case event_id::RELEASE:
    case event_id::ROLL_OVER:
    case event_id::DRAG_OUT:
      _mouse_state = MOUSE_OVER;
      break;
      
    case event_id::PRESS:
    case event_id::DRAG_OVER:
      _mouse_state = MOUSE_DOWN;
      break;
      
    default:
      assert(0);	// missed a case?
      break;
    };
  
  // @@ eh, should just be a lookup table.
#if 0
  // Add appropriate actions to the movie's execute list...
  for (int i = 0; i < m_def->m_button_actions.size(); i++) {
    if (m_def->m_button_actions[i].m_conditions & c) {
      // Matching action.
      for (int j = 0; j < m_def->m_button_actions[i].m_actions.size(); j++) {
        get_parent()->add_action_buffer(m_def->m_button_actions[i].m_actions[j]);
      }
    }
  }
#endif
  // Call conventional attached method.
  // @@ TODO
}

void moviecliploader_loadclip(const fn_call& fn)
{
	as_value	val, method;

	log_msg("%s: nargs = %d\n", __FUNCTION__, fn.nargs);

	moviecliploader_as_object* ptr = \
		dynamic_cast<moviecliploader_as_object*>(fn.this_ptr);

	assert(ptr);
  
	as_value& url_arg = fn.arg(0);
	if ( url_arg.get_type() != as_value::STRING )
	{
		log_error("Malformed SWF, MovieClipLoader.loadClip() first argument is not a string (%s)", url_arg.to_string());
		fn.result->set_bool(false);
		return;
	}

	std::string str_url = fn.arg(0).to_string(); 
	character* target = fn.env->find_target(fn.arg(1));
	if ( ! target )
	{
		log_error("Could not find target %s", fn.arg(1).to_string());
		fn.result->set_bool(false);
		return;
	}

	log_msg("load clip: %s, target is: %p\n",
		str_url.c_str(), (void*)target);

	// Get a pointer to target's sprite parent 
	character* parent = target->get_parent();
	assert(parent);

#if 0 // urls are resolved relative to base url !
	//
	// Extract root movie URL 
	// @@ could be cached somewhere...
	//
	as_value parent_url;
	if ( ! parent->get_member("_url", &parent_url) )
	{
		log_msg("FIXME: no _url member in target parent!");
	}

	log_msg(" target's parent url: %s\n", parent_url.to_string());

	//
	// Resolve relative urls
	// @@ todo

	// We have a problem with exceptions here...
	// unless we heap-allocate the URL or define
	// a default ctor + assignment op we can't
	// wrap in a try/catch block w/out hiding
	// the variable inside the block.
	//
	URL url(str_url.c_str(), URL(parent_url.to_string()));
#else
	URL url(str_url.c_str(), get_base_url());
#endif
	
	log_msg(" resolved url: %s\n", url.str().c_str());
			 
	// Call the callback since we've started loading the file
	if (fn.this_ptr->get_member("onLoadStart", &method))
	{
	//log_msg("FIXME: Found onLoadStart!\n");
		as_c_function_ptr	func = method.to_c_function();
		fn.env->set_variable("success", true);
		if (func)
		{
			// It's a C function.  Call it.
			//log_msg("Calling C function for onLoadStart\n");
			(*func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
		}
		else if (as_function* as_func = method.to_as_function())
		{
		// It's an ActionScript function.  Call it.
			//log_msg("Calling ActionScript function for onLoadStart\n");
			(*as_func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
		}
		else
		{
			log_error("error in call_method(): method is not a function\n");
		}    
	}

	// Call the callback since we've started loading the file
	if (fn.this_ptr->get_member("onLoadStart", &method))
	{
	//log_msg("FIXME: Found onLoadStart!\n");
		as_c_function_ptr	func = method.to_c_function();
		fn.env->set_variable("success", true);
		if (func)
		{
			// It's a C function.  Call it.
			//log_msg("Calling C function for onLoadStart\n");
			(*func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
		}
		else if (as_function* as_func = method.to_as_function())
		{
		// It's an ActionScript function.  Call it.
			//log_msg("Calling ActionScript function for onLoadStart\n");
			(*as_func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
		}
		else
		{
			log_error("error in call_method(): method is not a function\n");
		}    
	}

	std::string path = url.path();
	std::string suffix = path.substr(path.size() - 4);
	log_msg("File suffix to load is: %s\n", suffix.c_str());

	movie_definition* md = create_library_movie(url);
	if (md == NULL) {
		log_error("can't create movie_definition for %s\n",
			url.str().c_str());
		fn.result->set_bool(false);
		return;
	}

	log_msg("movie definition created\n");

	gnash::movie_interface* extern_movie;
	extern_movie = md->create_instance();
	if (extern_movie == NULL) {
		log_error("can't create extern movie_interface "
			"for %s\n", url.str().c_str());
		fn.result->set_bool(false);
		return;
	}

	log_msg("movie instance created\n");

	save_extern_movie(extern_movie);

	character* tar = target;
	const char* name = tar->get_name().c_str();
	uint16_t depth = tar->get_depth();
	bool use_cxform = false;
	cxform color_transform =  tar->get_cxform();
	bool use_matrix = false;
	matrix mat = tar->get_matrix();
	float ratio = tar->get_ratio();
	uint16_t clip_depth = tar->get_clip_depth();

	character* new_movie = extern_movie->get_root_movie();

	new_movie->set_parent(parent);

	parent->replace_display_object(
			   new_movie,
			   name,
			   depth,
			   use_cxform,
			   color_transform,
			   use_matrix,
			   mat,
			   ratio,
			   clip_depth);
  
	struct mcl *mcl_data = ptr->mov_obj.getProgress(target);

	// the callback since we're done loading the file
	// FIXME: these both probably shouldn't be set to the same value
	//mcl_data->bytes_loaded = stats.st_size;
	//mcl_data->bytes_total = stats.st_size;
	mcl_data->bytes_loaded = 666; // fake values for now
	mcl_data->bytes_total = 666;

	fn.env->set_member("target_mc", target);
	moviecliploader_onload_complete(fn);
	//env->pop();
  
	fn.result->set_bool(true);

}

void
moviecliploader_unloadclip(const fn_call& fn)
{
  const std::string filespec = fn.arg(0).to_string();
  log_msg("%s: FIXME: Load Movie Clip: %s\n", __FUNCTION__, filespec.c_str());
  
}

void
moviecliploader_new(const fn_call& fn)
{

  log_msg("%s: args=%d\n", __FUNCTION__, fn.nargs);
  
  //const tu_string filespec = fn.arg(0).to_string();
  
  as_object*	mov_obj = new moviecliploader_as_object;
  //log_msg("\tCreated New MovieClipLoader object at %p\n", mov_obj);

  mov_obj->set_member("loadClip",
                      &moviecliploader_loadclip);
  mov_obj->set_member("unloadClip",
                      &moviecliploader_unloadclip);
  mov_obj->set_member("getProgress",
                      &moviecliploader_getprogress);

#if 0
  // Load the default event handlers. These should really never
  // be called directly, as to be useful they are redefined
  // within the SWF script. These get called if there is a problem
  // Setup the event handlers
  mov_obj->set_event_handler(event_id::LOAD_INIT,
                             (as_c_function_ptr)&event_test);
  mov_obj->set_event_handler(event_id::LOAD_START,
                             (as_c_function_ptr)&event_test);
  mov_obj->set_event_handler(event_id::LOAD_PROGRESS,
                             (as_c_function_ptr)&event_test);
  mov_obj->set_event_handler(event_id::LOAD_ERROR,
                             (as_c_function_ptr)&event_test);
#endif
  
  fn.result->set_as_object(mov_obj);
}

void
moviecliploader_onload_init(const fn_call& /*fn*/)
{
  log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
}

// Invoked when a call to MovieClipLoader.loadClip() has successfully
// begun to download a file.
void
moviecliploader_onload_start(const fn_call& /*fn*/)
{
  log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
}

// Invoked every time the loading content is written to disk during
// the loading process.
void
moviecliploader_getprogress(const fn_call& fn)
{
  //log_msg("%s: nargs = %d\n", __FUNCTION__, nargs);
  
  moviecliploader_as_object*	ptr = (moviecliploader_as_object*) (as_object*) fn.this_ptr;
  assert(ptr);
  
  as_object *target = (as_object*) fn.arg(0).to_object();
  
  struct mcl *mcl_data = ptr->mov_obj.getProgress(target);

  mcl_as_object *mcl_obj = (mcl_as_object *)new mcl_as_object;

  mcl_obj->set_member("bytesLoaded", mcl_data->bytes_loaded);
  mcl_obj->set_member("bytesTotal",  mcl_data->bytes_total);
  
  fn.result->set_as_object(mcl_obj);
}

// Invoked when a file loaded with MovieClipLoader.loadClip() has
// completely downloaded.
void
moviecliploader_onload_complete(const fn_call& fn)
{
  as_value	val, method;
  //log_msg("%s: FIXME: nargs = %d\n", __FUNCTION__, nargs);
  //moviecliploader_as_object*	ptr = (moviecliploader_as_object*) (as_object*) this_ptr;
  
  std::string url = fn.arg(0).to_string();  
  //as_object *target = (as_object *)env->bottom(first_arg-1).to_object();
  //log_msg("load clip: %s, target is: %p\n", url.c_str(), target);

  //log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
  if (fn.this_ptr->get_member("onLoadComplete", &method)) {
    //log_msg("FIXME: Found onLoadComplete!\n");
    as_c_function_ptr	func = method.to_c_function();
    fn.env->set_variable("success", true);
    if (func)
      {
        // It's a C function.  Call it.
        //log_msg("Calling C function for onLoadComplete\n");
        (*func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
      }
    else if (as_function* as_func = method.to_as_function())
      {
        // It's an ActionScript function.  Call it.
        //log_msg("Calling ActionScript function for onLoadComplete\n");
        (*as_func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
      }
    else
      {
        log_error("error in call_method(): method is not a function\n");
      }    
  } else {
    log_error("Couldn't find onLoadComplete!\n");
  }
}

// Invoked when a file loaded with MovieClipLoader.loadClip() has failed to load.
void
moviecliploader_onload_error(const fn_call& fn)
{
  //log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
  as_value	val, method;
  log_msg("%s: FIXME: nargs = %d\n", __FUNCTION__, fn.nargs);
  //moviecliploader_as_object*	ptr = (moviecliploader_as_object*) (as_object*) this_ptr;
  
  std::string url = fn.arg(0).to_string();  
  as_object *target = (as_object*) fn.arg(1).to_object();
  log_msg("load clip: %s, target is: %p\n", url.c_str(), (void *)target);

  //log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
  if (fn.this_ptr->get_member("onLoadError", &method)) {
    //log_msg("FIXME: Found onLoadError!\n");
    as_c_function_ptr	func = method.to_c_function();
    fn.env->set_variable("success", true);
    if (func)
      {
        // It's a C function.  Call it.
        log_msg("Calling C function for onLoadError\n");
        (*func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
      }
    else if (as_function* as_func = method.to_as_function())
      {
        // It's an ActionScript function.  Call it.
        log_msg("Calling ActionScript function for onLoadError\n");
        (*as_func)(fn_call(&val, fn.this_ptr, fn.env, 0, 0));
      }
    else
      {
        log_error("error in call_method(): method is not a function\n");
      }    
  } else {
    log_error("Couldn't find onLoadError!\n");
  }
}

// This is the default event handler. To wind up here is an error.
void
moviecliploader_default(const fn_call& /*fn*/)
{
  log_msg("%s: FIXME: Default event handler, you shouldn't be here!\n", __FUNCTION__);
}

} // end of gnash namespace
