///////////////////////////////////////////////////////////////////////////////
// glInfo.cpp
// ==========
// get GL vendor, version, supported extensions and other states using glGet*
// functions and store them glInfo struct variable
//
// To get valid OpenGL infos, OpenGL rendering context (RC) must be opened
// before calling glInfo::getInfo(). Otherwise it returns false.
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2005-10-04
// UPDATED: 2009-10-06
//
// Copyright (c) 2005 Song Ho Ahn
//
// Modified by Ohannessian H. Gorune on 24/12/2010
// -Uses "set" to store the extensions, for speed
//  so, isExtensionSupported method is modified to use sets
///////////////////////////////////////////////////////////////////////////////

//#include <windows.h> //might not need this
#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "glInfo.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// extract openGL info
// This function must be called after GL rendering context opened.
///////////////////////////////////////////////////////////////////////////////
bool glInfo::getInfo()
{
	char* str = 0;
	char* tok = 0;

	// get vendor string
	str = (char*)glGetString(GL_VENDOR);
	if(str) this->vendor = str;                  // check NULL return value
	else return false;

	// get renderer string
	str = (char*)glGetString(GL_RENDERER);
	if(str) this->renderer = str;                // check NULL return value
	else return false;

	// get version string
	str = (char*)glGetString(GL_VERSION);
	if(str) this->version = str;                 // check NULL return value
	else return false;
	// get all extensions as a string
	str = (char*)glGetString(GL_EXTENSIONS);

	// split extensions
	if(str)
	{
		tok = strtok(str, " ");
		while (tok)
		{
			extensionsSet.insert(tok);
			tok = strtok(0, " ");               // next token
		}
	}
	else
	{
		return false;
	}

	// get number of color bits
	glGetIntegerv(GL_RED_BITS, &this->redBits);
	glGetIntegerv(GL_GREEN_BITS, &this->greenBits);
	glGetIntegerv(GL_BLUE_BITS, &this->blueBits);
	glGetIntegerv(GL_ALPHA_BITS, &this->alphaBits);

	// get depth bits
	glGetIntegerv(GL_DEPTH_BITS, &this->depthBits);

	// get stecil bits
	glGetIntegerv(GL_STENCIL_BITS, &this->stencilBits);

	// get max number of lights allowed
	glGetIntegerv(GL_MAX_LIGHTS, &this->maxLights);

	// get max texture resolution
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &this->maxTextureSize);

	// get max number of clipping planes
	glGetIntegerv(GL_MAX_CLIP_PLANES, &this->maxClipPlanes);

	// get max modelview and projection matrix stacks
	glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &this->maxModelViewStacks);
	glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &this->maxProjectionStacks);
	glGetIntegerv(GL_MAX_ATTRIB_STACK_DEPTH, &this->maxAttribStacks);

	// get max texture stacks
	glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, &this->maxTextureStacks);

	return true;
}



///////////////////////////////////////////////////////////////////////////////
// check if the video card support a certain extension
///////////////////////////////////////////////////////////////////////////////
bool glInfo::isExtensionSupported(const string& ext)
{
	set<string>::iterator it = this->extensionsSet.find(ext);

	if ( it != extensionsSet.end() )
		return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// print OpenGL info to screen and save to a file
///////////////////////////////////////////////////////////////////////////////
void glInfo::printSelf()
{
    stringstream ss;

    ss << endl; // blank line
    ss << "OpenGL Driver Info" << endl;
    ss << "==================" << endl;
    ss << "Vendor: " << this->vendor << endl;
    ss << "Version: " << this->version << endl;
    ss << "Renderer: " << this->renderer << endl;

    ss << endl;
    ss << "Color Bits(R,G,B,A): (" << this->redBits << ", " << this->greenBits
       << ", " << this->blueBits << ", " << this->alphaBits << ")\n";
    ss << "Depth Bits: " << this->depthBits << endl;
    ss << "Stencil Bits: " << this->stencilBits << endl;

    ss << endl;
    ss << "Max Texture Size: " << this->maxTextureSize << "x" << this->maxTextureSize << endl;
    ss << "Max Lights: " << this->maxLights << endl;
    ss << "Max Clip Planes: " << this->maxClipPlanes << endl;
    ss << "Max Modelview Matrix Stacks: " << this->maxModelViewStacks << endl;
    ss << "Max Projection Matrix Stacks: " << this->maxProjectionStacks << endl;
    ss << "Max Attribute Stacks: " << this->maxAttribStacks << endl;
    ss << "Max Texture Stacks: " << this->maxTextureStacks << endl;

    ss << endl;
    ss << "Total Number of Extensions: " << this->extensionsSet.size() << endl;
    ss << "==============================" << endl;
	for(set<string>::iterator it = this->extensionsSet.begin(); it != this->extensionsSet.end(); it++)
        ss << *it << endl;

    ss << "======================================================================" << endl;

    cout << ss.str() << endl;
}