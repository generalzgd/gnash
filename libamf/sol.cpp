// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/cstdint.hpp>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>

#include "network.h"
#include "amf.h"
#include "buffer.h"
#include "sol.h"
#include "log.h"
#include "GnashException.h"

#if defined(HAVE_WINSOCK_H) && !defined(__OS2__)
# include <winsock2.h>
# include <windows.h>
# include <sys/stat.h>
# include <io.h>
# include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

#include <boost/scoped_array.hpp>

using namespace std;
using namespace amf;
using namespace gnash;

// It comprises of a magic number, followed by the file length, a
// filetype, which appears to always be "TCSO", and what appears to be
// a marker at the end of the header block.
// After the SOL header, the rest is all AMF objects.

// Magic Number - 2 bytes (always 0x00bf)
// Length       - 4 bytes (the length of the file including the Marker bytes)
// Marker       - 10 bytes (always "TCSO0x000400000000")
// Object Name  - variable (the name of the object as an AMF encoded string)
// Padding      - 4 bytes
// After this is a series of AMF objects
const short SOL_MAGIC = 0x00bf;	// is in big-endian format, this is the first
				// two bytes. of the .sol file.
//char *SOL_FILETYPE = "TCSO";
const short SOL_BLOCK_MARK = 0x0004;

#define ENSUREBYTES(from, toofar, size) { \
	if ( from+size >= toofar ) \
		throw ParserException("Premature end of AMF stream"); \
}

namespace amf
{

SOL::SOL() 
    : _filesize(0)
{
//    GNASH_REPORT_FUNCTION;
}

SOL::~SOL()
{
//    GNASH_REPORT_FUNCTION;
    vector<amf::Element *>::iterator it;
    for (it = _amfobjs.begin(); it != _amfobjs.end(); it++) {
	amf::Element *el = (*(it));
	if (el) {
	    delete el;
	}
    }
}

bool
SOL::extractHeader(const std::string & /*filespec*/)
{
//    GNASH_REPORT_FUNCTION;
      return false;
}

bool
SOL::extractHeader(const vector<unsigned char> & /*data*/)
{
//    GNASH_REPORT_FUNCTION;
      return false;
}

void
SOL::addObj(amf::Element *el)
{
//    GNASH_REPORT_FUNCTION;
    _amfobjs.push_back(el);
//    _filesize += el->getName().size() + el->getLength() + 5;
}

bool
SOL::formatHeader(const vector<unsigned char> & /*data*/)
{
//    GNASH_REPORT_FUNCTION;
      return false;
}

// name is the object name
bool
SOL::formatHeader(const std::string &name)
{
    return formatHeader(name, _filesize);
}

bool
SOL::formatHeader(const std::string &name, int filesize)
{
//    GNASH_REPORT_FUNCTION;
    boost::uint32_t i;

    // First we add the magic number. All SOL data is in big-endian format,
    // so we swap it first.
    boost::uint16_t swapped = SOL_MAGIC;
    swapped = htons(swapped);
    Network::byte_t *ptr = reinterpret_cast<Network::byte_t *>(&swapped);
    for (i=0; i<sizeof(boost::uint16_t); i++) {
        _header.push_back(ptr[i]);
    }

    // Next is the file size to be created. We adjust it as the filesize
    // includes the padding in the header, the mystery bytes, and the
    // padding, plus the length of the name itself.
    filesize += name.size() + 16;
    boost::uint32_t len = filesize;
    len = htonl(len);
    ptr = reinterpret_cast<Network::byte_t *>(&len);
    for (i=0; i<sizeof(boost::uint32_t); i++) {
        _header.push_back(ptr[i]);
    }

    // Then the mystery block, but as the value never seems to every change,
    // we just built it the same way it always is.
    // first is the TCSO, we have no idea what this stands for.
//    ptr = reinterpret_cast<uint8_t *>(const_cast<uint8_t *>("TCSO");
    ptr = (uint8_t *)"TCSO";
    for (i=0; i<sizeof(boost::uint32_t); i++) {
        _header.push_back(ptr[i]);
    }
    // then the 0x0004 bytes, also a mystery
    swapped = SOL_BLOCK_MARK;
    swapped = htons(swapped);
    ptr = reinterpret_cast<Network::byte_t *>(&swapped);
    for (i=0; i<sizeof(boost::uint16_t); i++) {
        _header.push_back(ptr[i]);
    }
    // finally a bunch of zeros to pad things for this field
    for (i=0; i<sizeof(boost::uint32_t); i++) {
        _header.push_back('\0');
    }

    // Encode the name. This is not a string object, which has a type field
    // one byte field precedding the length as a file type of AMF::STRING.
    //  First the length in two bytes
    swapped = name.size();
    swapped = htons(swapped);
    ptr = reinterpret_cast<Network::byte_t *>(&swapped);
    for (i=0; i<sizeof(boost::uint16_t); i++) {
        _header.push_back(ptr[i]);
    }
    // then the string itself
    ptr = (Network::byte_t *)name.c_str();
    for (i=0; i<name.size(); i++) {
        _header.push_back(ptr[i]);
    }
    
    // finally a bunch of zeros to pad things at the end of the header
    for (i=0; i<sizeof(boost::uint32_t); i++) {
        _header.push_back('\0');
    }

#if 0
    unsigned char *hexint;
    hexint = new unsigned char[(_header.size() + 3) *3];
    
    hexify(hexint, (unsigned char *)_header, _header.size(), true);
    log_debug (_("%s: SOL file header is: \n%s"), __FUNCTION__, (char *)hexint);
    delete hexint;
#endif    
    
    return true;
}    

// write the data to disk as a .sol file

bool
SOL::writeFile(const string &filespec, const string &name)
{
//    GNASH_REPORT_FUNCTION;
    ofstream ofs(filespec.c_str(), ios::binary);
    if ( ! ofs ) {
        log_error("Failed opening file '%s' in binary mode", filespec.c_str());
        return false;
    }
    
    vector<Network::byte_t>::iterator it;
    vector<amf::Element *>::iterator ita; 
    AMF amf_obj;
    char *ptr;
    int size = 0;
    
    if (filespec.empty()) {
	return false;
    }

    for (ita = _amfobjs.begin(); ita != _amfobjs.end(); ita++) {
        amf::Element *el = (*(ita));
	size += el->getNameSize() + el->getLength() + 7;
    }
    _filesize = size;
    
    boost::scoped_array<char> body ( new char[size + 20] );
    memset(body.get(), 0, size);
    ptr = body.get();
    char* endPtr = ptr+size+20; // that's the amount we allocated..

    for (ita = _amfobjs.begin(); ita != _amfobjs.end(); ita++) {
        amf::Element *el = (*(ita));
        Buffer *var = amf_obj.encodeProperty(el); 
        //  Network::byte_t *var = amf_obj.encodeProperty(el, outsize); 
        if (!var) {
            continue;
        }
        size_t outsize = 0;
        switch (el->getType()) {
	  case Element::BOOLEAN_AMF0:
	      outsize = el->getNameSize() + 3;
	      memcpy(ptr, var->reference(), outsize); 
	      ptr += outsize;
	      break;
	  case Element::OBJECT_AMF0:
	      outsize = el->getNameSize() + 5;
              assert(ptr+outsize < endPtr);
	      outsize = el->getNameSize() + 5;
	      memcpy(ptr, var->reference(), outsize);
	      ptr += outsize;
	      *ptr++ = Element::OBJECT_END_AMF0;
	      *ptr++ = 0;	// objects are terminated too!
	      break;
	  case Element::NUMBER_AMF0:
	      outsize = el->getNameSize() + AMF0_NUMBER_SIZE + 2;
              assert(ptr+outsize < endPtr);
	      memcpy(ptr, var->reference(), outsize);
	      ptr += outsize;
	      *ptr++ = 0;	// doubles are terminated too!
	      *ptr++ = 0;	// doubles are terminated too!
	      break;
	  case Element::STRING_AMF0:
	      if (el->getLength() == 0) {
              	  assert(ptr+outsize+1 < endPtr);
		  memcpy(ptr, var, outsize+1);
		  ptr += outsize+1;
	      } else {		// null terminate the string
                  assert(ptr+outsize < endPtr);
		  memcpy(ptr, var->reference(), outsize);
		  ptr += outsize;
		  *ptr++ = 0;
	      }
	      break;
	  default:
              assert(ptr+outsize < endPtr);
	      memcpy(ptr, var->reference(), outsize);
	      ptr += outsize;
	}
	delete var;
    }
    
    _filesize = ptr - body.get();
    int len = name.size() + sizeof(uint16_t) + 16;
    boost::scoped_array<char> head ( new char[len + 4] );
    memset(head.get(), 0, len);
    ptr = head.get();
    formatHeader(name);
    for (it = _header.begin(); it != _header.end(); it++) {
        *ptr++ = (*(it));
    }
    
    if ( ofs.write(head.get(), _header.size()).fail() )
    {
        log_error("Error writing %d bytes of header to output file %s", _header.size(), filespec.c_str());
        return false;
    }

//    ofs.write(body, (ptr - body));
    if ( ofs.write(body.get(), _filesize).fail() )
    {
        log_error("Error writing %d bytes of body to output file %s", _filesize, filespec.c_str());
        return false;
    }
    ofs.close();

    return true;
}

// read the .sol file from disk
bool
SOL::readFile(std::string &filespec)
{
//    GNASH_REPORT_FUNCTION;
    struct stat st;
    boost::uint16_t size;
    Network::byte_t *buf = 0;
    Network::byte_t *ptr = 0;
    size_t bodysize;

    // Make sure it's an SOL file
    if (stat(filespec.c_str(), &st) == 0) {

	try {
	    ifstream ifs(filespec.c_str(), ios::binary);
	    _filesize = st.st_size;
	    buf = new Network::byte_t[_filesize + sizeof(int)];
	    ptr = buf;
	    Network::byte_t* tooFar = buf+_filesize+sizeof(int);
	    
	    bodysize = st.st_size - 6;
	    _filespec = filespec;
	    ifs.read(reinterpret_cast<char *>(ptr), _filesize);
	    
#ifndef GNASH_TRUST_AMF
	    ENSUREBYTES(ptr, tooFar, 2+4+10); // magic number, file size, file marker
#endif
	    
	    // skip the magic number (will check later)
	    ptr += 2;
	    
	    // extract the file size
	    boost::uint32_t length = *(reinterpret_cast<boost::uint32_t *>(ptr));
	    length = ntohl(length);
	    ptr += 4;
	    
	    // skip the file marker field
	    ptr += 10;
	    
	    // consistency check
	    if ((buf[0] == 0) && (buf[1] == 0xbf)) {
		if (bodysize == length) {
		    log_debug("%s is an SOL file", filespec.c_str());
		} else {
		    log_error("%s looks like an SOL file, but the length is wrong. Should be %d, got %d",
			      filespec.c_str(), (_filesize - 6), length);
		}
		
        } else {
		log_error("%s isn't an SOL file", filespec.c_str());
	    }
	    
#ifndef GNASH_TRUST_AMF
	    ENSUREBYTES(ptr, tooFar, 2); 
#endif
	    
	    // 2 bytes for the length of the object name, but it's also null terminated
	    size = *(reinterpret_cast<boost::uint16_t *>(ptr));
	    size = ntohs(size);
	    ptr += 2;
	    
#ifndef GNASH_TRUST_AMF
	    ENSUREBYTES(ptr, tooFar, size+4);  // 4 is the padding below
#endif
	    
	    // TODO: make sure there's the null-termination, or
	    //       force if if it's there by definition
	    _objname = reinterpret_cast<const char *>(ptr);
	    ptr += size;
	    
	    // Go past the padding
	    ptr += 4;
	    
	    AMF amf_obj;
	    amf::Element *el;
	    while ( ptr < tooFar) {
		if (ptr) {
		    el = amf_obj.extractProperty(ptr, tooFar);
		    if (el != 0) {
			// Unlike RTMP, SOL files tack an extra
			// zero byte after every property, so we
			// want to skip past this one too.
			ptr += amf_obj.totalsize() + 1;
			_amfobjs.push_back(el);
		    } else {
			break;
		    }
//		log_debug("Bodysize is: %d size is: %d for %s", bodysize, size, el->getName());
		} else {
		    break;
		}
	    }
	    delete[] buf;
	    
	    ifs.close();
	    return true;
	} catch (std::exception& e) {
	    log_error("Reading SharedObject %s: %s", filespec, e.what());
	    return false;
	}
    }
    
//    log_error("Couldn't open file: %s", strerror(errno));
    return false;
}

void
SOL::dump()
{
    vector<amf::Element *>::iterator it;

    cerr << "Dumping SOL file" << endl;
    cerr << "The file name is: " << _filespec << endl;
    cerr << "The size of the file is: " << _filesize << endl;
    cerr << "The name of the object is: " << _objname << endl;
    for (it = _amfobjs.begin(); it != _amfobjs.end(); it++) {
	amf::Element *el = (*(it));
        cerr << el->getName() << ": ";
        if (el->getType() == Element::STRING_AMF0) {
            if (el->getLength() != 0) {
                cerr << el->getData();
            } else {
                cerr << "null";
            }
        }
        if (el->getType() == Element::NUMBER_AMF0) {
            double ddd = *((double *)el->getData());
	    swapBytes(&ddd, sizeof(double));
	    cerr << ddd << " ";

            cerr << "( " << hexify(el->getData(), 8, false) << ")";
        }
        if (el->getType() == Element::BOOLEAN_AMF0) {
            if (el->to_bool() == true) {
                cerr << "true";
            }
            if (el->to_bool() == false) {
                cerr << "false";
            }
        }
        if (el->getType() == Element::OBJECT_AMF0) {
            cerr << "is an object";
        }
        cerr << endl;
    }
}


} // end of amf namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
