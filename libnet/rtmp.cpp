// rtmp.cpp:  Adobe/Macromedia Real Time Message Protocol handler, for Gnash.
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/detail/endian.hpp>
#include <iostream>
#include <string>
#include <map>

#if ! (defined(_WIN32) || defined(WIN32))
#	include <netinet/in.h>
#endif

#include "log.h"
#include "amf.h"
#include "rtmp.h"
#include "network.h"
#include "element.h"
#include "handler.h"
#include "utility.h"
#include "buffer.h"

using namespace gnash;
using namespace std;
using namespace amf;

namespace gnash
{

extern map<int, Handler *> handlers;

const char *content_str[] = {
    "None",
    "Chunk Size",
    "Unknown",
    "Bytes Read",
    "Ping",
    "Server",
    "Client",
    "Unknown2",
    "Audio Data",
    "Video Data",
    "Unknown3",
    "Blank 0xb",
    "Blank 0xc",
    "Blank 0xd",
    "Blank 0xe",
    "Blank 0xf",
    "Blank 0x10",
    "Blank 0x11",
    "Notify",
    "Shared object",
    "Invoke"
};

const char *ping_str[] = {
    "PING_CLEAR",
    "PING_PLAY",
    "Unknown Ping 2",
    "PING_TIME",
    "PING_RESET",
    "Unknown Ping 2",
    "PING_CLIENT",
    "PONG_CLIENT"
};

const char *status_str[] = {
    "APP_GC",
    "APP_RESOURCE_LOWMEMORY",
    "APP_SCRIPT_ERROR",
    "APP_SCRIPT_WARNING",
    "APP_SHUTDOWN",
    "NC_CALL_BADVERSION",
    "NC_CALL_FAILED",
    "NC_CONNECT_APPSHUTDOWN",
    "NC_CONNECT_CLOSED",
    "NC_CONNECT_FAILED",
    "NC_CONNECT_INVALID_APPLICATION",
    "NC_CONNECT_REJECTED",
    "NC_CONNECT_SUCCESS",
    "NS_CLEAR_FAILED",
    "NS_CLEAR_SUCCESS",
    "NS_DATA_START",
    "NS_FAILED",
    "NS_INVALID_ARGUMENT",
    "NS_PAUSE_NOTIFY",
    "NS_PLAY_COMPLETE",
    "NS_PLAY_FAILED",
    "NS_PLAY_FILE_STRUCTURE_INVALID",
    "NS_PLAY_INSUFFICIENT_BW",
    "NS_PLAY_NO_SUPPORTED_TRACK_FOUND",
    "NS_PLAY_PUBLISHNOTIFY",
    "NS_PLAY_RESET",
    "NS_PLAY_START",
    "NS_PLAY_STOP",
    "NS_PLAY_STREAMNOTFOUND",
    "NS_PLAY_SWITCH",
    "NS_PLAY_UNPUBLISHNOTIFY",
    "NS_PUBLISH_BADNAME",
    "NS_PUBLISH_START",
    "NS_RECORD_FAILED",
    "NS_RECORD_NOACCESS",
    "NS_RECORD_START",
    "NS_RECORD_STOP",
    "NS_SEEK_FAILED",
    "NS_SEEK_NOTIFY",
    "NS_UNPAUSE_NOTIFY",
    "NS_UNPUBLISHED_SUCCESS",
    "SO_CREATION_FAILED",
    "SO_NO_READ_ACCESS",
    "SO_NO_WRITE_ACCESS",
    "SO_PERSISTENCE_MISMATCH"
};


// These are the textual responses
const char *response_str[] = {
    "/onStatus",
    "/onResult",
    "/onDebugEvents"
};

int
RTMP::headerSize(Network::byte_t header)
{
//    GNASH_REPORT_FUNCTION;
    
    int headersize = -1;
    
    switch (header & RTMP_HEADSIZE_MASK) {
      case HEADER_12:
          headersize = 12;
          break;
      case HEADER_8:
          headersize = 8;
          break;
      case HEADER_4:
          headersize = 4;
          break;
      case HEADER_1:
          headersize = 1;
          break;
      default:
          log_error(_("AMF Header size bits (0x%X) out of range"),
          		header & RTMP_HEADSIZE_MASK);
          headersize = 1;
          break;
    };

    return headersize;
}

RTMP::RTMP() 
    : _handshake(0), _handler(0)
{
//    GNASH_REPORT_FUNCTION;
//     _inbytes = 0;
//     _outbytes = 0;
    
//    _body = new unsigned char(RTMP_BODY_SIZE+1);
//    memset(_body, 0, RTMP_BODY_SIZE+1);
}

RTMP::~RTMP()
{
//    GNASH_REPORT_FUNCTION;
    _variables.clear();
    if (_handshake) {
	delete _handshake;
    }
    if (_handler) {
	delete _handler;
    }
//    delete _body;
}

void
RTMP::addProperty(amf::Element *el)
{
//    GNASH_REPORT_FUNCTION;
    _variables[el->getName()] = el;
}

void
RTMP::addProperty(char *name, amf::Element *el)
{ 
//    GNASH_REPORT_FUNCTION;
    _variables[name] = el;
}

amf::Element *
RTMP::getProperty(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
//    return _variables[name.c_str()];
    map<const char *, amf::Element *>::iterator it;
    for (it = _variables.begin(); it != _variables.end(); it++) {
	const char *title = it->first;
	amf::Element *el = it->second;
	if (name == title) {
// 	    log_debug("found variable in RTMP packet: %s", name);
	    return el;
	}
    }
    return 0;
}

RTMP::rtmp_head_t *
RTMP::decodeHeader(Buffer *buf)
{
    GNASH_REPORT_FUNCTION;
    return decodeHeader(buf->reference());
}

RTMP::rtmp_head_t *
RTMP::decodeHeader(Network::byte_t *in)
{
    GNASH_REPORT_FUNCTION;
    
    Network::byte_t *tmpptr = in;
    
    _header.channel = *tmpptr & RTMP_INDEX_MASK;
    log_debug (_("The AMF channel index is %d"), _header.channel);
    
    _header.head_size = headerSize(*tmpptr++);
    log_debug (_("The header size is %d"), _header.head_size);

    if (_header.head_size == 1) {
        _header.bodysize = sizeof(boost::uint16_t) * 2;
    }
    
    if (_header.head_size >= 4) {
        _mystery_word = *tmpptr++;
        _mystery_word = (_mystery_word << 12) + *tmpptr++;
        _mystery_word = (_mystery_word << 8) + *tmpptr++;

        log_debug(_("The mystery word is: %d"), _mystery_word);
    }

    if (_header.head_size >= 8) {
        _header.bodysize = *tmpptr++;
        _header.bodysize = (_header.bodysize << 12) + *tmpptr++;
        _header.bodysize = (_header.bodysize << 8) + *tmpptr++;
        _header.bodysize = _header.bodysize & 0xffffff;
        log_debug(_("The body size is: %d"), _header.bodysize);
    }

    if (_header.head_size >= 8) {
	char c = *(reinterpret_cast<char *>(tmpptr));
	_header.type = static_cast<content_types_e>(c);
        tmpptr++;
        log_debug(_("The type is: %s"), content_str[_header.type]);
    }
    
    if (_header.head_size == 12) {
        _header.src_dest = *(reinterpret_cast<RTMPMsg::rtmp_source_e *>(tmpptr));
        tmpptr += sizeof(boost::uint32_t);
        log_debug(_("The source/destination is: %x"), _header.src_dest);
    }

    return &_header;
}

/// \brief \ Each RTMP header consists of the following:
///
/// * Index & header size - The header size and amf channel index.
/// * Total size - The total size of the message
/// * Type - The type of the message
/// * Routing - The source/destination of the message
//

amf::Buffer *
RTMP::encodeHeader(int amf_index, rtmp_headersize_e head_size)
{
//    GNASH_REPORT_FUNCTION;
    amf::Buffer *buf = new Buffer(1);
    Network::byte_t *ptr = buf->reference();
    
    // Make the channel index & header size byte
    *ptr = head_size & RTMP_HEADSIZE_MASK;  
    *ptr += amf_index  & RTMP_INDEX_MASK;

    return buf;
}

// There are 3 size of RTMP headers, 1, 4, 8, and 12.
amf::Buffer *
RTMP::encodeHeader(int amf_index, rtmp_headersize_e head_size,
		       size_t total_size, content_types_e type,
		       RTMPMsg::rtmp_source_e routing)
{
    GNASH_REPORT_FUNCTION;

    amf::Buffer *buf = 0;
    switch(head_size) {
      case HEADER_1:
	  buf = new Buffer(1);
	  break;
      case HEADER_4:
	  buf = new Buffer(4);
	  break;
      case HEADER_8:
	  buf = new Buffer(8);
	  break;
      case HEADER_12:
	  buf = new Buffer(12);
	  break;
    }
    
// FIXME: this is only to make this more readeable with GDB, and is a performance hit.
    buf->clear();
    Network::byte_t *ptr = buf->reference();
    
    // Make the channel index & header size byte
//    *ptr = head_size & RTMP_HEADSIZE_MASK;
    *ptr = head_size; // & RTMP_INDEX_MASK;
    *ptr += amf_index  & RTMP_INDEX_MASK;
    ptr++;

    // Add the unknown bytes. These seem to be used by video and
    // audio, and only when the header size is 4 or more.
    if ((head_size == HEADER_4) || (head_size == HEADER_8) || (head_size == HEADER_12)) {
	memset(ptr, 0, 3);
	ptr += 3;
    }
    
    // Add the size of the message if the header size is 8 or more.
    // and add the type of the object if the header size is 8 or more.
    if ((head_size == HEADER_8) || (head_size == HEADER_12)) {
// RTMP uses a 3 byte length field, which is the total size of the packet
	boost::uint32_t length = total_size << 8;
	swapBytes(&length, 4);
	memcpy(ptr, &length, 3);
	ptr += 3;
	
	// Add the type
        *ptr = type;
        ptr++;
    }
    
    // Add the routing of the message if the header size is 12, the maximum.
    if (head_size == HEADER_12) {
        memcpy(ptr, &routing, 4);
        ptr += 4;
    }
    
    return buf;
}

bool
RTMP::packetRead(amf::Buffer *buf)
{
    GNASH_REPORT_FUNCTION;

//    int packetsize = 0;
    size_t amf_index, headersize;
    Network::byte_t *ptr = buf->reference();
    Network::byte_t *tooFar = ptr+buf->size();
    AMF amf;
    
//    \003\000\000\017\000\000%G￿%@\024\000\000\000\000\002\000\aconnect\000?%G￿%@\000\000\000\000\000\000\003\000\003app\002\000#software/gnash/tests/1153948634.flv\000\bflashVer\002\000\fLNX 6,0,82,0\000\006swfUrl\002\000\035file:///file|%2Ftmp%2Fout.swf%G￿%@\000\005tcUrl\002\0004rtmp://localhost/software/gnash/tests/1153948634
    amf_index = *buf->reference() & RTMP_INDEX_MASK;
    headersize = headerSize(*buf->reference());
    log_debug (_("The Header size is: %d"), headersize);
    log_debug (_("The AMF index is: 0x%x"), amf_index);

//     if (headersize > 1) {
// 	packetsize = decodeHeader(ptr);
//         if (packetsize) {
//             log_debug (_("Read first RTMP packet header of size %d"), packetsize);
//         } else {
//             log_error (_("Couldn't read first RTMP packet header"));
//             return false;
//         }
//     }

#if 1
    Network::byte_t *end = buf->remove(0xc3);
#else
    Network::byte_t *end = buf->find(0xc3);
    log_debug("END is %x", (void *)end);
    *end = '*';
#endif
    
//    ptr = decodeHeader(ptr);
//    ptr += headersize;
    
    amf::Element *el = amf.extractAMF(ptr, tooFar);
//    el->dump();
    el = amf.extractAMF(ptr, tooFar) + 1; // @@strk@@ : what's the +1 for ?
//    el->dump();
    log_debug (_("Reading AMF packets till we're done..."));
//    buf->dump();
    while (ptr < end) {
	amf::Element *el = amf.extractProperty(ptr, tooFar);
	addProperty(el);
//	el->dump();
    }
    ptr += 1;
    size_t actual_size = static_cast<size_t>(_header.bodysize - AMF_HEADER_SIZE);
    log_debug("Total size in header is %d, buffer size is: %d", _header.bodysize, buf->size());
//    buf->dump();
    if (buf->size() < actual_size) {
	log_debug("FIXME: MERGING");
	buf = _handler->merge(buf);
    }
    while ((ptr - buf->begin()) < static_cast<int>(actual_size)) {
	amf::Element *el = amf.extractProperty(ptr, tooFar);
	addProperty(el);
//	el->dump();		// FIXME: dump the AMF objects as they are read in
    }

//    dump();
    
    amf::Element *url = getProperty("tcUrl");
    amf::Element *file = getProperty("swfUrl");
    amf::Element *app = getProperty("app");
    
    if (file) {
	log_debug("SWF file %s", file->getData());
    }
    if (url) {
	log_debug("is Loading video %s", url->getData());
    }
    if (app) {
	log_debug("is file name is %s", app->getData());
    }
    
    return true;
}

void
RTMP::dump()
{
    cerr << "RTMP packet contains " << _variables.size() << " variables." << endl;
    map<const char *, amf::Element *>::iterator it;
    for (it = _variables.begin(); it != _variables.end(); it++) {
//	const char *name = it->first;
	amf::Element *el = it->second;
	el->dump();
    }
}

// A Ping packet has two parameters that ae always specified, and 2 that are optional.
// The first two bytes are the ping type, as in rtmp_ping_e, the second is the ping
// target, which is always zero as far as we can tell.
//
// More notes from: http://jira.red5.org/confluence/display/docs/Ping
// type 0: Clear the stream. No third and fourth parameters. The second parameter could be 0.
// After the connection is established, a Ping 0,0 will be sent from server to client. The
// message will also be sent to client on the start of Play and in response of a Seek or
// Pause/Resume request. This Ping tells client to re-calibrate the clock with the timestamp
// of the next packet server sends.
// type 1: Tell the stream to clear the playing buffer.
// type 3: Buffer time of the client. The third parameter is the buffer time in millisecond.
// type 4: Reset a stream. Used together with type 0 in the case of VOD. Often sent before type 0.
// type 6: Ping the client from server. The second parameter is the current time.
// type 7: Pong reply from client. The second parameter is the time the server sent with his
//         ping request.

// A RTMP Ping packet looks like this: "02 00 00 00 00 00 06 04 00 00 00 00 00 00 00 00 00 0",
// which is the Ping type byte, followed by two shorts that are the parameters. Only the first
// two paramters are required.
// This seems to be a ping message, 12 byte header, system channel 2
// 02 00 00 00 00 00 06 04 00 00 00 00 00 00 00 00 00 00
RTMP::rtmp_ping_t *
RTMP::decodePing(Network::byte_t *data)
{
    GNASH_REPORT_FUNCTION;
    
    Network::byte_t *ptr = reinterpret_cast<Network::byte_t *>(data);
    rtmp_ping_t *ping = new rtmp_ping_t;
    memset(ping, 0, sizeof(rtmp_ping_t));

    // All the data fields in a ping message are 2 bytes long.
    boost::uint16_t type = ntohs(*reinterpret_cast<rtmp_ping_e *>(ptr));
    ping->type = static_cast<rtmp_ping_e>(type);
    ptr += sizeof(boost::uint16_t);

    ping->target = ntohs(*reinterpret_cast<boost::uint16_t *>(ptr));
    ptr += sizeof(boost::uint16_t);
    
    ping->param1 = ntohs(*reinterpret_cast<boost::uint16_t *>(ptr));
    ptr += sizeof(boost::uint16_t);
    
//     ping->param2 = ntohs(*reinterpret_cast<boost::uint16_t *>(ptr));
//     ptr += sizeof(boost::uint16_t);

//    ping->param3 = ntohs(*reinterpret_cast<boost::uint16_t *>(ptr));
    ping->param3 = 0;

    return ping;    
}
RTMP::rtmp_ping_t *
RTMP::decodePing(amf::Buffer *buf)
{
    GNASH_REPORT_FUNCTION;
    return decodePing(buf->reference());
}

// Decode the result we get from the server after we've made a request.
//
// 03 00 00 00 00 00 81 14 00 00 00 00 02 00 07 5f  ..............._
// 72 65 73 75 6c 74 00 3f f0 00 00 00 00 00 00 05  result.?........
// 03 00 0b 61 70 70 6c 69 63 61 74 69 6f 6e 05 00  ...application..
// 05 6c 65 76 65 6c 02 00 06 73 74 61 74 75 73 00  .level...status.
// 0b 64 65 73 63 72 69 70 74 69 6f 6e 02 00 15 43  .description...C
// 6f 6e 6e 65 63 74 69 6f 6e 20 73 75 63 63 65 65  onnection succee
// 64 65 64 2e 00 04 63 6f 64 65 02 00 1d 4e 65 74  ded...code...Net
// 43 6f 6e 6e 65 63 74 69 6f 6e 2e 43 6f 6e 6e 65  Connection.Conne
// 63 74 2e 53 75 63 63 65 73 73 00 00 c3 09        ct.Success....
//
// 43 00 00 00 00 00 48 14 02 00 06 5f 65 72 72 6f  C.....H...._erro
// 72 00 40 00 00 00 00 00 00 00 05 03 00 04 63 6f  r.@...........co
// 64 65 02 00 19 4e 65 74 43 6f 6e 6e 65 63 74 69  de...NetConnecti
// 6f 6e 2e 43 61 6c 6c 2e 46 61 69 6c 65 64 00 05  on.Call.Failed..
// 6c 65 76 65 6c 02 00 05 65 72 72 6f 72 00 00 09  level...error...
//
// T 127.0.0.1:1935 -> 127.0.0.1:38167 [AP]
// 44 00 00 00 00 00 b2 14 02 00 08 6f 6e 53 74 61  D..........onSta
// 74 75 73 00 3f f0 00 00 00 00 00 00 05 03 00 08  tus.?...........
// 63 6c 69 65 6e 74 69 64 00 3f f0 00 00 00 00 00  clientid.?......
// 00 00 05 6c 65 76 65 6c 02 00 06 73 74 61 74 75  ...level...statu
// 73 00 07 64 65 74 61 69 6c 73 02 00 16 6f 6e 32  s..details...on2
// 5f 66 6c 61 73 68 38 5f 77 5f 61 75 64 69 6f 2e  _flash8_w_audio.
// 66 6c 76 00 0b 64 65 73 63 72 69 70 74 69 6f 6e  flv..description
// 02 00 27 53 74 61 72 74 65 64 20 70 6c 61 79 69  ..'Started playi
// 6e 67 20 6f 6e 32 5f 66 c4 6c 61 73 68 38 5f 77  ng on2_f.lash8_w
// 5f 61 75 64 69 6f 2e 66 6c 76 2e 00 04 63 6f 64  _audio.flv...cod
// 65 02 00 14 4e 65 74 53 74 72 65 61 6d 2e 50 6c  e...NetStream.Pl
// 61 79 2e 53 74 61 72 74 00 00 09                 ay.Start...
//
// ^^^_result^?^^^^^^^^^^^application^^^level^^^status^^description^^^Connection succeeded.^^code^^^NetConnection.Connect.Success^^^^
// 02 00 07 5f 72 65 73 75 6c 74 00 3f f0 00 00 00 00 00 00 05 03 00 0b 61 70 70 6c 69 63 61 74 69 6f 6e 05 00 05 6c 65 76 65 6c 02 00 06 73 74 61 74 75 73 00 0b 64 65 73 63 72 69 70 74 69 6f 6e 02 00 15 43 6f 6e 6e 65 63 74 69 6f 6e 20 73 75 63 63 65 65 64 65 64 2e 00 04 63 6f 64 65 02 00 1d 4e 65 74 43 6f 6e 6e 65 63 74 69 6f 6e 2e 43 6f 6e 6e 65 63 74 2e 53 75 63 63 65 73 73 00 00 c3 09 
// 10629:3086592224] 20:01:20 DEBUG: read 29 bytes from fd 3 from port 0
// C^^^^^^^^^^onBWDone^@^^^^^^^^
// 43 00 00 00 00 00 15 14 02 00 08 6f 6e 42 57 44 6f 6e 65 00 40 00 00 00 00 00 00 00 05
RTMPMsg *
RTMP::decodeMsgBody(Network::byte_t *data, size_t size)
{
    GNASH_REPORT_FUNCTION;
    AMF amf_obj;
    Network::byte_t *ptr = data;
    Network::byte_t* tooFar = ptr + size;
    bool status = false;

    // The first data object is the method name of this object.
    Element *name = amf_obj.extractAMF(ptr, tooFar);
    if (name) {
	ptr += name->getLength() + 3; // skip the length bytes too
    } else {
	log_error("Name field of RTMP Message corrupted!");
	return 0;
    }

    // The stream ID is the second data object. All messages have these two objects
    // at the minimum.
    Element *streamid = amf_obj.extractAMF(ptr, tooFar);
    if (streamid) {
	ptr += streamid->getLength() + 2;
    } else {
	log_error("Stream ID field of RTMP Message corrupted!");
	return 0;
    }

    // This will need to be deleted manually later after usage, it is not
    // automatically deallocated.
    RTMPMsg *msg = new RTMPMsg;

    msg->setMethodName(name->to_string());
    double swapped = streamid->to_number();
    msg->setStreamID(swapped);

    if ((msg->getMethodName() == "_result") || (msg->getMethodName() == "error")) {
 	status = true;
    }
    
    // Then there are a series of AMF objects, often a higher level ActionScript object with
    // properties attached.
    while (ptr < tooFar) {
	// These pointers get deleted automatically when the msg object is deleted
        amf::Element *el = amf_obj.extractAMF(ptr, tooFar);
	ptr += amf_obj.totalsize();
        if (el == 0) {
	    break;
	}
	el->dump();
	msg->addObject(el);
 	if (status) {
	    msg->checkStatus(el);
	}
    };
    
    // cleanup after ourselves
    delete name;
    delete streamid;
    
    return msg;
}

RTMPMsg *
RTMP::decodeMsgBody(amf::Buffer *buf)
{
//    GNASH_REPORT_FUNCTION;
    return decodeMsgBody(buf->reference(), buf->size());
}

amf::Buffer *
RTMP::encodeChunkSize()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
    return 0;
}

void
RTMP::decodeChunkSize()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
}
    
amf::Buffer *
RTMP::encodeBytesRead()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
    return 0;
}

void
RTMP::decodeBytesRead()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
}

amf::Buffer *
RTMP::encodeServer()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
    return 0;
}

void 
RTMP::decodeServer()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
}
    
amf::Buffer *
RTMP::encodeClient()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
    return 0;
}

void 
RTMP::decodeClient()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
}
    
amf::Buffer *
RTMP::encodeAudioData()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
    return 0;
}

void 
RTMP::decodeAudioData()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
}
    
amf::Buffer *
RTMP::encodeVideoData()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
    return 0;
}

void 
RTMP::decodeVideoData()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
}
    
amf::Buffer *
RTMP::encodeNotify()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
    return 0;
}

void 
RTMP::decodeNotify()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
}
    
amf::Buffer *
RTMP::encodeSharedObj()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
    return 0;
}

void 
RTMP::decodeSharedObj()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
}
    
amf::Buffer *
RTMP::encodeInvoke()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
    return 0;
}
void 
RTMP::decodeInvoke()
{
    GNASH_REPORT_FUNCTION;
    log_unimpl(__PRETTY_FUNCTION__);
}

// Send a message, usually a single ActionScript object. This message
// may be broken down into a series of packets on a regular byte
// interval. (128 bytes for video data). Each message main contain
// multiple packets.
bool
RTMP::sendMsg(amf::Buffer *buf)
{
    GNASH_REPORT_FUNCTION;

    size_t partial = RTMP_VIDEO_PACKET_SIZE;
    size_t nbytes = 0;
    Network::byte_t header = 0xc3;

    while (nbytes <= buf->size()) {
	if ((buf->size() - nbytes) < static_cast<signed int>(RTMP_VIDEO_PACKET_SIZE)) {
	    partial = buf->size() - nbytes;
	}    
	writeNet(buf->reference() + nbytes, partial);
	if (partial == static_cast<signed int>(RTMP_VIDEO_PACKET_SIZE)) {
	    writeNet(&header, 1);
	}
	nbytes += RTMP_VIDEO_PACKET_SIZE;	
    };
    return true;
}
    
// Send a Msg, and expect a response back of some kind.
amf::Element *
RTMP::sendRecvMsg(int amf_index, rtmp_headersize_e head_size,
		  size_t total_size, content_types_e type,
		  RTMPMsg::rtmp_source_e routing, amf::Buffer *bufin)
{
    GNASH_REPORT_FUNCTION;
//    size_t total_size = buf2->size() - 6; // FIXME: why drop 6 bytes ?
    Buffer *head = encodeHeader(amf_index, head_size, total_size,
				type, routing);
    int ret = writeNet(head);
//     if (netDebug()) {
//         cerr << __FUNCTION__ << ": " <<__LINE__ << ": " << hexify(head->reference(), headerSize(head_size), false) << endl;
//     }

    ret = sendMsg(bufin);
    if (netDebug()) {
        cerr << __FUNCTION__ << ": " << __LINE__ << ": " << hexify(head->reference(), headerSize(head_size), false) << hexify(bufin->reference(), ret, true) << endl;
    }

    Buffer buf;
    ret = readNet(&buf, 5);
    if (ret < 0) {
	log_error("Never got any data!");
	return 0;
    }
    if ((ret == 1) && (*buf.reference() == 0xff)) {
	log_error("Got an error from the server sending object of type %s",
		  content_str[type]);
	ret = readNet(&buf, 5);	
	if (ret < 0) {
	    log_error("Never got any data!");
	    return 0;
	}
	if ((ret == 1) && (*buf.reference() == 0xff)) {
	    cerr << __FUNCTION__ << ": " << __LINE__ << ": " <<
		hexify(buf.reference(), buf.size(), false) << endl;
	    log_error("Got an error from the server sending object of type %s",
		      content_str[type]);
//	exit(-1);
	}
    }

    RTMP::rtmp_head_t *rthead = decodeHeader(&buf);

    RTMPMsg *msg;
    if (rthead) {
	if (rthead->head_size == 1) {
	    log_debug("Response header: %s", hexify(buf.reference(),
						    7, false));
	} else {
	    log_debug("Response header: %s", hexify(buf.reference(),
						    rthead->head_size, false));
	}
	if (rthead->type == RTMP::PING) {
	    RTMP::rtmp_ping_t *ping = decodePing(buf.reference());
	    log_debug("FIXME: Ping type is: %d, ignored for now", ping->type);
	} else if (rthead->type != RTMP::PING) {
	    msg = decodeMsgBody(buf.reference() + rthead->head_size, rthead->bodysize);
	    if (msg) {
		log_debug("%s: Msg status is: %d: %s", __FUNCTION__,
			  msg->getStatus(), status_str[msg->getStatus()]);
	    } else {
		log_error("Couldn't decode message body for type %s!",
			  content_str[rthead->type]);
	    }
	} else {
	    log_error("Couldn't decode message header for type %s!",
		      content_str[type]);
	}
    }
    
    
//    Element *el = new Element;  
//    el.
    
    if (rthead->bodysize < ret) {
	log_debug("more bytes left to read ! %d", (rthead->bodysize < ret));
    }
    
    return 0;
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
