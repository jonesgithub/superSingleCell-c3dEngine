//
//  c3dDefine.h
//  HelloCpp
//
//  Created by Yang Chao (wantnon) on 14-1-2.
//
//

#ifndef __HelloCpp__c3dDefine__
#define __HelloCpp__c3dDefine__

#include <iostream>
#include <string>
using namespace std;
#include <limits>
#include <OpenGLES/ES2/gl.h>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) 
	#define C3DFINAL  //vs2010 not support final key word, so just ignor
#else
    #define C3DFINAL final
#endif

#define USED_FOR_SUBMIT_TO_VBO C3DFINAL //if used for submit to vbo, it must not contain virtual member function, so it's destructor should not be virtual, so we mark it final


enum Ec3dProjectionMode{
    ec3dOrthographicMode = 0,
    ec3dPerspectiveMode,
};

enum Ec3dCullFace{
    ec3dNotCull=0,
    ec3dCullFront,
    ec3dCullBack,
    
};


const float c3d_PI=3.14159265358979323846264338327950288;
const float c3d_INF=0.5*numeric_limits<float>::max();//为了安全，降低一些；<>中的float不要写成const float



//下面的属性位置是模仿xcode自带例子(例用GLKit)定义的
const GLint ATTRIB_LOC_position_local=0;
const GLint ATTRIB_LOC_normal_local=1;
const GLint ATTRIB_LOC_color=2;
const GLint ATTRIB_LOC_texCoord=3;
const GLint ATTRIB_LOC_texCoord2=4;

#endif /* defined(__HelloCpp__c3dDefine__) */
