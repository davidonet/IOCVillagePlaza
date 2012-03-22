/*
 * ofxIRGenerator.h
 *
 * Copyright 2011 (c) Matthew Gingold http://gingold.com.au
 * Originally forked from a project by roxlu http://www.roxlu.com/ 
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef _H_OFXIRGENERATOR
#define _H_OFXIRGENERATOR

#include "ofxOpenNIContext.h"

class ofxIRGenerator {
	
public:
	
	ofxIRGenerator();
	~ofxIRGenerator();
	
	bool setup(ofxOpenNIContext* pContext);
	void update();
	void draw(float x=0, float y=0, float w=640, float h=480);
	
	xn::IRGenerator& getXnIRGenerator();
	
//	ofxIRGenerator(ofxIRGenerator const& mom);
//	ofxIRGenerator & operator = (const ofxIRGenerator& mom);
	
private:
	
	void generateTexture();
	
	xn::IRGenerator		ir_generator;
	ofTexture			ir_texture;
	unsigned char *		ir_pixels;
	
};

#endif