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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 

/* $Id: noseek_fd_adapter.cpp,v 1.3 2006/09/30 00:10:36 nihilus Exp $ */

#if defined(_WIN32) || defined(WIN32)
#define snprintf _snprintf
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDIN_CURL_H
# define HAVE_LIBCURL 1
#endif

#include "noseek_fd_adapter.h"
#include "tu_file.h"
#include "utility.h"
#include "GnashException.h"
#include <unistd.h>

//#define GNASH_NOSEEK_FD_VERBOSE 1

// define this if you want seeks back to be reported (on stderr)
//#define GNASH_NOSEEK_FD_WARN_SEEKSBACK 1


#include <stdexcept>
#include <cstdio>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>

#if !defined(_WIN32) && !defined(WIN32)
# include <unistd.h>
#endif

#include <string>

namespace noseek_fd_adapter
{


/***********************************************************************
 *
 *  NoSeekFile definition
 * 
 **********************************************************************/

class NoSeekFile
{

public:

	/// Open a stream for the specified filedes
	//
	/// @param fd
	///	A filedescriptor for a stream opened for reading
	///
	/// @param filename
	///	An optional filename to use for storing the cache.
	///	If NULL the cache would be an unnamed file and
	///	would not be accessible after destruction of this 
	///	instance.
	///
	NoSeekFile(int fd, char* filename=NULL);

	~NoSeekFile();

	/// Read 'bytes' bytes into the given buffer.
	//
	/// Return number of actually read bytes
	///
	size_t read_cache(void *dst, size_t bytes);

	/// Return true if EOF has been reached
	bool eof();

	/// Report global position within the file
	size_t tell();

	/// Put read pointer at given position
	bool seek(size_t pos);

private:

	// Open either a temporary file or a named file
	// (depending on value of _cachefilename)
	void openCacheFile();

	// Use this file to cache data
	FILE* _cache;

	// _cache file descriptor
	int _cachefd;

	// the input file descriptor
	int _fd;

	// transfer in progress
	int _running;

	// cache filename
	char* _cachefilename;

	// Attempt at filling the cache up to the given size.
	void fill_cache(size_t size);

	// Append sz bytes to the cache
	size_t cache(void *from, size_t sz);

	void printInfo();

};

/***********************************************************************
 *
 *  NoSeekFile implementation
 * 
 **********************************************************************/


/*private*/
size_t
NoSeekFile::cache(void *from, size_t sz)
{
	// take note of current position
	long curr_pos = ftell(_cache);

	// seek to the end
	fseek(_cache, 0, SEEK_END);

	size_t wrote = fwrite(from, 1, sz, _cache);
	if ( wrote < 1 )
	{
		char errmsg[256];
	
		snprintf(errmsg, 255,
			"writing to cache file: requested %lu, wrote %lu (%s)",
			static_cast<unsigned long>(sz), static_cast<unsigned long>(wrote), strerror(errno));
		fprintf(stderr, "%s\n", errmsg);
		throw gnash::GnashException(errmsg);
	}

	// reset position for next read
	fseek(_cache, curr_pos, SEEK_SET);

	return wrote;
}


/*private*/
void
NoSeekFile::fill_cache(size_t size)
{
#ifdef GNASH_NOSEEK_FD_VERBOSE
	fprintf(stderr, "fill_cache(%d) called\n", size);
#endif

	struct stat statbuf;

	// See how big is the cache
	fstat(_cachefd, &statbuf);
	if ( (size_t)statbuf.st_size >= size ) 
	{
#ifdef GNASH_NOSEEK_FD_VERBOSE
		fprintf(stderr,
			" big enough (%d), returning\n",
			statbuf.st_size);
#endif
		return;
	}

	// Let's see how many bytes are left to read
	size_t bytes_needed = size-statbuf.st_size;


	char* buf = new char[bytes_needed];
	ssize_t bytes_read = read(_fd, (void*)buf, bytes_needed);
	if ( bytes_read == -1 )
	{
		fprintf(stderr, "Error reading %lu bytes from input stream",
			static_cast<unsigned long>(bytes_needed));
	}

	if ( (size_t)bytes_read < bytes_needed )
	{
		if ( bytes_read == 0 )
		{
#ifdef GNASH_NOSEEK_FD_VERBOSE
			fprintf(stderr, "EOF reached\n");
#endif
			_running = false;
			return;
		}
		//delete [] buf;
		//throw gnash::GnashException("short read in fill_cache. EOF ?");
	}

	cache(buf, bytes_read);

	delete [] buf;
}

/*private*/
void
NoSeekFile::printInfo()
{
	fprintf(stderr, "_cache.tell = %u\n", tell());
}

/*private*/
void
NoSeekFile::openCacheFile()
{
	if ( _cachefilename )
	{
		_cache = fopen(_cachefilename, "w");
		if ( ! _cache )
		{
			throw gnash::GnashException("Could not create cache file " + std::string(_cachefilename));
		}
	}
	else
	{
		_cache = tmpfile();
		if ( ! _cache ) {
			throw gnash::GnashException("Could not create temporary cache file");
		}
	}
	_cachefd = fileno(_cache);

}

/*public*/
NoSeekFile::NoSeekFile(int fd, char* filename)
	:
	_fd(fd),
	_running(1),
	_cachefilename(filename)
{
	// might throw an exception
	openCacheFile();
}

/*public*/
NoSeekFile::~NoSeekFile()
{
	fclose(_cache);
}

/*public*/
size_t
NoSeekFile::read_cache(void *dst, size_t bytes)
{
	if ( eof() ) return 0;

#ifdef GNASH_NOSEEK_FD_VERBOSE
	fprintf(stderr, "read_cache(%d) called\n", bytes);
#endif

	fill_cache(tell()+bytes);

#ifdef GNASH_NOSEEK_FD_VERBOSE
	printInfo();
#endif

	return fread(dst, 1, bytes, _cache);

}

/*public*/
bool
NoSeekFile::eof()
{
	bool ret = ( ! _running && feof(_cache) );
	
#ifdef GNASH_NOSEEK_FD_VERBOSE
	fprintf(stderr, "eof() returning %d\n", ret);
#endif
	return ret;

}

/*public*/
size_t
NoSeekFile::tell()
{
	long ret =  ftell(_cache);

#ifdef GNASH_NOSEEK_FD_VERBOSE
	fprintf(stderr, "tell() returning %ld\n", ret);
#endif

	return ret;

}

/*public*/
bool
NoSeekFile::seek(size_t pos)
{
#ifdef GNASH_NOSEEK_FD_WARN_SEEKSBACK
	if ( pos < tell() ) {
		fprintf(stderr,
			"Warning: seek backward requested (%ld from %ld)\n",
			pos, tell());
	}
#endif

	fill_cache(pos);

	if ( fseek(_cache, pos, SEEK_SET) == -1 ) {
		fprintf(stderr, "Warning: fseek failed\n");
		return false;
	} else {
		return true;
	}

}

/***********************************************************************
 *
 * Adapter calls
 * 
 **********************************************************************/


// Return number of bytes actually read.
static int
read(void* dst, int bytes, void* appdata)
{
	NoSeekFile* stream = (NoSeekFile*) appdata;
	return stream->read_cache(dst, bytes);
}

static bool
eof(void* appdata)
{
	NoSeekFile* stream = (NoSeekFile*) appdata;
	return stream->eof();
}

static int
write(const void* /*src*/, int /*bytes*/, void* /*appdata*/)
{
	assert(0); // not supported
	return 0;
}

static int
seek(int pos, void* appdata)
{
	NoSeekFile* stream = (NoSeekFile*) appdata;
	if ( stream->seek(pos) ) return 0;
	else return TU_FILE_SEEK_ERROR;
}

static int
seek_to_end(void* /*appdata*/)
{
	assert(0); // not supported
	return 0;
}

static int
tell(void* appdata)
{
	NoSeekFile* stream = (NoSeekFile*) appdata;
	return stream->tell();
}

static int
close(void* appdata)
{
	NoSeekFile* stream = (NoSeekFile*) appdata;

	delete stream;

	//return TU_FILE_CLOSE_ERROR;
	return 0;
}

// this is the only exported interface
tu_file*
make_stream(int fd, char* cachefilename)
{
#ifdef GNASH_NOSEEK_FD_VERBOSE
	fprintf(stderr, "making NoSeekFile stream for fd %d\n", fd);
#endif

	NoSeekFile* stream = NULL;

	try {
		stream = new NoSeekFile(fd, cachefilename);
	} catch (const std::exception& ex) {
		fprintf(stderr, "NoSeekFile stream: %s\n", ex.what());
		delete stream;
		return NULL;
	}

	return new tu_file(
		(void*)stream, // opaque user pointer
		read, // read
		write, // write
		seek, // seek
		seek_to_end, // seek_to_end
		tell, // tell
		eof, // get eof
		close);
}

} // namespace noseek_fd_adapter



// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
