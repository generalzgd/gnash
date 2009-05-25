// LoaderInfo_as.hx:  ActionScript 3 "LoaderInfo" class, for Gnash.
//
// Generated by gen-as3.sh on: 20090514 by "rob". Remove this
// after any hand editing loosing changes.
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

// This test case must be processed by CPP before compiling to include the
//  DejaGnu.hx header file for the testing framework support.

#if flash9
import flash.display.LoaderInfo;
import flash.display.MovieClip;
#else
import flash.LoaderInfo;
import flash.MovieClip;
#end
import flash.Lib;
import Type;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class LoaderInfo_as {
    static function main() {
        var x1:LoaderInfo = new LoaderInfo();

        // Make sure we actually get a valid class        
        if (x1 != null) {
            DejaGnu.pass("LoaderInfo class exists");
        } else {
            DejaGnu.fail("LoaderInfo lass doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
// 	if (ActionScriptVersion == 0) {
// 	    DejaGnu.pass("LoaderInfo::actionScriptVersion property exists");
// 	} else {
// 	    DejaGnu.fail("LoaderInfo::actionScriptVersion property doesn't exist");
// 	}
// 	if (x1.applicationDomain == applicationDomain) {
// 	    DejaGnu.pass("LoaderInfo::applicationDomain property exists");
// 	} else {
// 	    DejaGnu.fail("LoaderInfo::applicationDomain property doesn't exist");
// 	}
// 	if (x1.bytes == ByteArray) {
// 	    DejaGnu.pass("LoaderInfo::bytes property exists");
// 	} else {
// 	    DejaGnu.fail("LoaderInfo::bytes property doesn't exist");
// 	}
	if (x1.bytesLoaded == 0) {
	    DejaGnu.pass("LoaderInfo::bytesLoaded property exists");
	} else {
	    DejaGnu.fail("LoaderInfo::bytesLoaded property doesn't exist");
	}
	if (x1.bytesTotal == 0) {
	    DejaGnu.pass("LoaderInfo::bytesTotal property exists");
	} else {
	    DejaGnu.fail("LoaderInfo::bytesTotal property doesn't exist");
	}
	if (x1.childAllowsParent == false) {
	    DejaGnu.pass("LoaderInfo::childAllowsParent property exists");
	} else {
	    DejaGnu.fail("LoaderInfo::childAllowsParent property doesn't exist");
	}
// 	if (x1.childSandboxBridge == Object) {
// 	    DejaGnu.pass("LoaderInfo::childSandboxBridge property exists");
// 	} else {
// 	    DejaGnu.fail("LoaderInfo::childSandboxBridge property doesn't exist");
// 	}
// 	if (x1.content == content) {
// 	    DejaGnu.pass("LoaderInfo::content property exists");
// 	} else {
// 	    DejaGnu.fail("LoaderInfo::content property doesn't exist");
// 	}
	if (x1.contentType == null) {
	    DejaGnu.pass("LoaderInfo::contentType property exists");
	} else {
	    DejaGnu.fail("LoaderInfo::contentType property doesn't exist");
	}
	if (x1.frameRate == 0) {
	    DejaGnu.pass("LoaderInfo::frameRate property exists");
	} else {
	    DejaGnu.fail("LoaderInfo::frameRate property doesn't exist");
	}
	if (x1.height == 0) {
	    DejaGnu.pass("LoaderInfo::height property exists");
	} else {
	    DejaGnu.fail("LoaderInfo::height property doesn't exist");
	}
// 	if (x1.loader == loader) {
// 	    DejaGnu.pass("LoaderInfo::loader property exists");
// 	} else {
// 	    DejaGnu.fail("LoaderInfo::loader property doesn't exist");
// 	}
	if (x1.loaderURL == null) {
	    DejaGnu.pass("LoaderInfo::loaderURL property exists");
	} else {
	    DejaGnu.fail("LoaderInfo::loaderURL property doesn't exist");
	}
// 	if (x1.parameters == Object) {
// 	    DejaGnu.pass("LoaderInfo::parameters property exists");
// 	} else {
// 	    DejaGnu.fail("LoaderInfo::parameters property doesn't exist");
// 	}
	if (x1.parentAllowsChild == false) {
	    DejaGnu.pass("LoaderInfo::parentAllowsChild property exists");
	} else {
	    DejaGnu.fail("LoaderInfo::parentAllowsChild property doesn't exist");
	}
// 	if (x1.parentSandboxBridge == Object) {
// 	    DejaGnu.pass("LoaderInfo::parentSandboxBridge property exists");
// 	} else {
// 	    DejaGnu.fail("LoaderInfo::parentSandboxBridge property doesn't exist");
// 	}
	if (x1.sameDomain == false) {
	    DejaGnu.pass("LoaderInfo::sameDomain property exists");
	} else {
	    DejaGnu.fail("LoaderInfo::sameDomain property doesn't exist");
	}
// 	if (x1.sharedEvents == EventDispatcher) {
// 	    DejaGnu.pass("LoaderInfo::sharedEvents property exists");
// 	} else {
// 	    DejaGnu.fail("LoaderInfo::sharedEvents property doesn't exist");
// 	}
// 	if (x1.SWFVersion == 0) {
// 	    DejaGnu.pass("LoaderInfo::swfVersion property exists");
// 	} else {
// 	    DejaGnu.fail("LoaderInfo::swfVersion property doesn't exist");
// 	}
	if (x1.url == null) {
	    DejaGnu.pass("LoaderInfo::url property exists");
	} else {
	    DejaGnu.fail("LoaderInfo::url property doesn't exist");
	}
	if (x1.width == 0) {
	    DejaGnu.pass("LoaderInfo::width property exists");
	} else {
	    DejaGnu.fail("LoaderInfo::width property doesn't exist");
	}

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
// 	if (x1.getLoaderInfoByDefinition == LoaderInfo) {
// 	    DejaGnu.pass("LoaderInfo::getLoaderInfoByDefinition() method exists");
// 	} else {
// 	    DejaGnu.fail("LoaderInfo::getLoaderInfoByDefinition() method doesn't exist");
// 	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
