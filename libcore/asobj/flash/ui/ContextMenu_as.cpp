// ContextMenu_as.cpp:  ActionScript "ContextMenu" class, for Gnash.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "ui/ContextMenu_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for getObjectInterface
#include "namedStrings.h"

namespace gnash {

// Forward declarations
namespace {
    as_value contextmenu_hideBuiltInItems(const fn_call& fn);
    as_value contextmenu_menuSelect(const fn_call& fn);
    as_value contextmenu_ctor(const fn_call& fn);
    void attachContextMenuInterface(as_object& o);
    void attachContextMenuStaticInterface(as_object& o);
    as_object* getContextMenuInterface();

}

class ContextMenu_as : public as_object
{

public:

    ContextMenu_as()
        :
        as_object(getExportedInterface())
    {}

    ContextMenu_as(const as_value& callback)
		:
		as_object(getExportedInterface())
	{
		setCallback(callback);
	}

    ContextMenu_as(as_function* callback)
		:
		as_object(getExportedInterface())
	{
		setCallback(callback);
	}

	static void registerConstructor(as_object& global);

	// override from as_object ?
	//std::string get_text_value() const { return "ContextMenu"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }

private:

	/// Get the callback to call when user invokes the context menu.
	//
	/// If NULL, no action will be taken on select.
	///
	as_function* getCallback() 
	{
		as_value tmp;
		if (get_member(NSV::PROP_ON_SELECT, &tmp))
			return tmp.to_as_function();
		else return NULL;
	}

	/// Set the callback to call when user invokes the context menu.
	//
	/// @param callback
	///	The function to call. If the value is not a function, no
	///	action will be taken on select.
	///
	void setCallback(const as_value& callback)
	{
		set_member(NSV::PROP_ON_SELECT, callback);
	}

	/// Attach the exported interface of this ActionScript class
	/// to the given object.
	static void attachExportedInterface(as_object& o);

	/// Get the ContextMenu.prototype ActionScript object
	static as_object* getExportedInterface();

	static as_value ctor_method(const fn_call& fn);

	static as_value hideBuiltInItems_method(const fn_call& fn);

	static as_value copy_method(const fn_call& fn);

};


/* static private */
void
ContextMenu_as::attachExportedInterface(as_object& o)
{
	o.init_member("copy", new builtin_function(ContextMenu_as::copy_method));
	o.init_member("hideBuiltInItems", new builtin_function(ContextMenu_as::hideBuiltInItems_method));
}

/* static private */
as_object*
ContextMenu_as::getExportedInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getObjectInterface());
		attachExportedInterface(*o);
	}
	return o.get();
}


/* static private */
as_value
ContextMenu_as::copy_method(const fn_call& fn)
{
	boost::intrusive_ptr<ContextMenu_as> ptr = ensureType<ContextMenu_as>(fn.this_ptr);
	UNUSED(ptr);
	log_unimpl (__FUNCTION__);
	return as_value();
}

/* static private */
as_value
ContextMenu_as::hideBuiltInItems_method(const fn_call& fn)
{
	boost::intrusive_ptr<ContextMenu_as> ptr = ensureType<ContextMenu_as>(fn.this_ptr);
	UNUSED(ptr);
	log_unimpl (__FUNCTION__);
	return as_value();
}

/* static private */
as_value
ContextMenu_as::ctor_method(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj;
	if ( fn.nargs > 0 )
       		obj = new ContextMenu_as(fn.arg(0));
	else
		obj = new ContextMenu_as();
	
	return as_value(obj.get()); // will keep alive
}

/* static public */
void
ContextMenu_as::registerConstructor(as_object& global)
{
	// This is going to be the global ContextMenu "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(ContextMenu_as::ctor_method, ContextMenu_as::getExportedInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		ContextMenu_as::attachExportedInterface(*cl);
		     
	}

	// Register _global.ContextMenu
	global.init_member("ContextMenu", cl.get());

}

// extern (used by Global.cpp)
void contextmenu_class_init(as_object& global)
{
	ContextMenu_as::registerConstructor(global);
}


namespace {

void
attachContextMenuInterface(as_object& o)
{
    o.init_member("hideBuiltInItems", new builtin_function(contextmenu_hideBuiltInItems));
    o.init_member("menuSelect", new builtin_function(contextmenu_menuSelect));
}

void
attachContextMenuStaticInterface(as_object& o)
{

}

as_object*
getContextMenuInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachContextMenuInterface(*o);
    }
    return o.get();
}

as_value
contextmenu_hideBuiltInItems(const fn_call& fn)
{
    boost::intrusive_ptr<ContextMenu_as> ptr =
        ensureType<ContextMenu_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
contextmenu_menuSelect(const fn_call& fn)
{
    boost::intrusive_ptr<ContextMenu_as> ptr =
        ensureType<ContextMenu_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
contextmenu_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new ContextMenu_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

