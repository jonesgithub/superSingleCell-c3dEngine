//
//  plane.h
//  HelloOpenGL
//
//  Created by wantnon (yang chao) on 14-2-2.
//
//

#ifndef __HelloOpenGL__plane__
#define __HelloOpenGL__plane__

#include <iostream>
using namespace std;
#include "c3dVector.h"
class Cplane final//用点法式描述
{
protected:
	Cc3dVector4 m_point;
	Cc3dVector4 m_norm;
public:
	void init(const Cc3dVector4 &point,const Cc3dVector4&norm)
	//由点法构造平面，norm须确保为单位向量
	{
        assert(point.w()==1);
        assert(norm.w()==0);
        m_point=point;
        m_norm=norm;
	}
    Cc3dVector4 getPoint()const{return m_point;}
    Cc3dVector4 getNorm()const{return m_norm;}
    
};


#endif /* defined(__HelloOpenGL__plane__) */
