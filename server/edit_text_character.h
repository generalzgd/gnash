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


#ifndef _GNASH_EDIT_TEXT_CHARACTER_H_
#define _GNASH_EDIT_TEXT_CHARACTER_H_

#include "character.h" // for inheritance
#include "edit_text_character_def.h" // for inlines
#include "styles.h" // for fill_style and line_style
#include "text.h" // for text_glyph_record


// Forward declarations
namespace gnash {
	class text_character_def; 
	class text_glyph_record; 
	//class sprite_instance;
}

namespace gnash {

/// An instance of an edit_text_character_def 
class edit_text_character : public character
{

public:

	edit_text_character(
			character* parent,
			edit_text_character_def* def,
			int id);

	~edit_text_character();

	virtual bool can_handle_mouse_event()	{	return true; }
	virtual movie*	get_topmost_mouse_entity(float x, float y);	
	virtual bool on_event(event_id id);	
	virtual movie_root *get_root();

	virtual const char* get_variable_name() const
	{
		return m_def->get_variable_name().c_str();
	}

	/// Set our text to the given string.
	virtual void	set_text_value(const char* new_text);

	virtual const char*	get_text_value() const
	{
		return m_text.c_str();
	}

	/// We have a "text" member.
	void set_member(const tu_stringi& name, const as_value& val);

	bool get_member(const tu_stringi& name, as_value* val);

	/// Draw the dynamic string.
	void	display();

	void get_invalidated_bounds(rect*, bool);

	virtual float	get_height() const;
	virtual float	get_width() const;

private:

	/// The actual text
	tu_string	m_text;

	/// immutable definition of this object, as read
	/// from the SWF stream. Assured to be not-NULL
	/// by constructor. This might change in the future
	edit_text_character_def*	m_def;

	/// bounds of dynamic text, as laid out
	rect	m_text_bounding_box;

	/// Reset our text bounding box to the given point.
	void	reset_bounding_box(float x, float y)
	{
		m_text_bounding_box.enclose_point(x,y);
	}

	std::vector<text_glyph_record>	m_text_glyph_records;

	/// used to pass a color on to shape_character::display()
	std::vector<fill_style>	m_dummy_style;

	std::vector<line_style>	m_dummy_line_style;

	/// Convert the characters in m_text into a series of
	/// text_glyph_records to be rendered.
	void	format_text();

	/// Does LEFT/CENTER/RIGHT alignment on the records in
	/// m_text_glyph_records[], starting with
	/// last_line_start_record and going through the end of
	/// m_text_glyph_records.
	float align_line(edit_text_character_def::alignment align,
			int last_line_start_record, float x);

	/// Set our font, return previously set one.
	/// This is private for now, but might eventally
	/// be public, for setting fonts from ActionScript.
	const font* set_font(const font* newfont);

	const font* _font;

	bool m_has_focus;
	int m_cursor;
	void show_cursor();
	float m_xcursor;
	float m_ycursor;

	/// Associate a variable to the text of this character
	//
	/// Setting the associated variable actually changes the
	/// displayed text. Getting the variable would return the
	/// displayed text.
	///
	/// If the given variable already exist use it to set
	/// current text before overriding it.
	///
	/// @param varname
	///	Name of the variable. Can contain path information,
	///	in that case the path is resolved relative to this
	///	character's parent.
	///
	void registerTextVariable(const std::string& varname);

};

} // namespace gnash

#endif // _GNASH_EDIT_TEXT_CHARACTER_H_
