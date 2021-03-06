//
//  collisionWithModelAtTriLevel.h
//  HelloOpenGL
//
//  Created by wantnon (yang chao) on 13-1-20.
//
//

#ifndef __HelloOpenGL__collisionWithModelAtTriLevel__
#define __HelloOpenGL__collisionWithModelAtTriLevel__

#include <iostream>
#include <vector>
using namespace std;
#include "c3dModel.h"
//#include "circleGenerator.h"
#include "triangleWithNorm.h"
//----------------------------------------------------------------------
#define IDTRI_WALL 0//IDtri是墙壁
#define IDTRI_CEIL 1//IDtri是天花板
#define IDTRI_FLOOR 2//IDtri是地板
static inline int IDtriExIsWhichPartOfHouse(const CtriangleWithNorm&tri){//0.5=sin30，0.86=sin60
    if(tri.getNorm().y()>=0.5){//如果比0.5再小，相机跟随时会产生万向锁
        return IDTRI_FLOOR;
    }else if(tri.getNorm().y()<=-0.86){
        return IDTRI_CEIL;
    }else{
        return IDTRI_WALL;
    }
}

static inline bool collisionTest_lineSeg_common(const vector<CtriangleWithNorm>&triWithNormList,
                                                const Cc3dVector4&p1,const Cc3dVector4&p2,const Cc3dVector4&c,float R,Cc3dVector4&p,Cc3dVector4&collisionFaceNorm)
//c为p1,p2中点，R为p1p2距离的一半
//判断线段p1p2是否与this相交，同时得到碰撞点坐标
//前端返回是否相交
//p带会交点坐标
{
    assert(p1.w()==1);
    assert(p2.w()==1);
    //将线段与各三角面逐一作相交检测
    int nTri=(int)triWithNormList.size();
    for(int i=0;i<nTri;i++){
        const CtriangleWithNorm&tri=triWithNormList[i];
        //求IDtri所在平面
        Cplane plane;
        plane.init(tri.getVert(0),tri.getNorm());
        //求线段p1p2与IDtri所在平面的交点
        //先判断p1,p2是否在IDtri异侧或IDri上
        float side1=PND_point_plane(plane,p1);
        float side2=PND_point_plane(plane,p2);
        if(side1*side2>0){//不在异侧或IDtri上
            //p1p2与IDtri不可能相交，可能与其它IDtri相交
            continue;//尝试下一个IDtri
        }
        //至此p1,p2在IDtri所在平面异侧或之上
        int flg=getPointOfIntersect_line_plane_np(p1,p2,plane,p);
     //   assert(p.w()==1);
        switch(flg){
            case 0:
                //不可能
                break;
            case 2://与IDtri所在平面有无穷多个交点
                //认为不相交
                break;
            case 1://与IDtri所在平面相交
                //检测交点是否在IDtri的volum内
                if(point_in_triVolum(tri.getNorm(),tri.getVert(0),tri.getVert(1),tri.getVert(2),p)){//在volum内，与IDtri相交
                    collisionFaceNorm=tri.getNorm();
                    return true;//碰撞
                }else{//不在volum内，与IDtri不相交
                    //可能与其它IDtri相交
                    continue;//尝试下一个IDtri
                }
                break;
        }
    }
    //与所有IDtri都未检测到相交
    collisionFaceNorm.init(0,0,0,0);
    return false;
}

static inline float getH_floor_common(const vector<CtriangleWithNorm>&triWithNormList,
                                      const Cc3dVector4&c,const float Rc,Cc3dVector4&houseFloorNorm)
//求点c处house地板高度，并带回c处地板法向
//如果返回-inf，则houseFloorNorm为(0,1,0)
{
    //与每个三角面检测碰撞
    float Hmax=-c3d_INF;//低于c[Y]最高高度
    CtriangleWithNorm*pTri=NULL;//取得Hmax的地板三角面
    int nTri=(int)triWithNormList.size();
    for(int i=0;i<nTri;i++){
        const CtriangleWithNorm&tri=triWithNormList[i];
        //如果不是地板面，则跳过
        if(IDtriExIsWhichPartOfHouse(tri)!=IDTRI_FLOOR)continue;
        //如果是地板面，还必须检测c的投影是落在它上面
        vector<Cc3dVector4> triPolygon;
        triPolygon.push_back(tri.getVert(0));
        triPolygon.push_back(tri.getVert(1));
        triPolygon.push_back(tri.getVert(2));
        //判断c是否在以tri为底dir={0,1,0}为方向向量的斜棱柱内
        float dir[4]={0,1,0,0};
        if(isPointInLeanedVolum(dir, triPolygon, c) ){//c在以tri为底dir为方向向量的斜棱柱内
            //计算c在IDtri所在平面上的投影高度
            //IDtri上取点P
            const Cc3dVector4&P=tri.getVert(0);
            //IDtri法向量
            const Cc3dVector4&norm=tri.getNorm();
            //c在IDtri所在平面上的投影高度为
            //H=(norm[X]*(P[X]-c[X])+norm[Z]*(P[Z]-c[Z]))/norm[Y]+P[Y]
            //由于前面已跳过非地板面，所以这里norm[Y]一定不为0
            float H=(norm.x()*(P.x()-c.x())+norm.z()*(P.z()-c.z()))/norm.y()+P.y();
            if(H<=c.y()+8.0){//必须判断是否H<=c[Y]
                //加10是因为，由于误差，有时主角中心可能会到地面之下，也不应错过检测
                if(H>Hmax){
                    Hmax=H;
                    pTri=(CtriangleWithNorm*)&tri;
                }
            }
        }
    }//得到Hmax和pIDtri
    if(pTri!=NULL){
        houseFloorNorm=pTri->getNorm();
    }else{
        houseFloorNorm.init(0, 1, 0, 0);
    }
    return Hmax;//如果c处不存在地板，则返回Hmax的初始值-inf（一个足够低的值）
}
static inline float getH_ceil_common(const vector<CtriangleWithNorm>&triWithNormList,
                                     const Cc3dVector4&c,const float Rc,Cc3dVector4&houseCeilNorm)
//求点c处天花板高度，并带回c处天花板法向
//如果返回inf，则houseCeilNorm为(0,-1,0)
{
    //与每个三角面检测碰撞
    float Hmin=c3d_INF;//高于c[Y]最低高度
    CtriangleWithNorm*pTri=NULL;//取得Hmin的天花板三角面
    int nTri=(int)triWithNormList.size();
    for(int i=0;i<nTri;i++){
        const CtriangleWithNorm&tri=triWithNormList[i];
        //如果不是天花板面，则跳过
        if(IDtriExIsWhichPartOfHouse(tri)!=IDTRI_CEIL)continue;
        //如果是天花板面，还必须检测c的投影是落在它上面
        vector<Cc3dVector4> triPolygon;
        triPolygon.push_back(tri.getVert(0));
        triPolygon.push_back(tri.getVert(1));
        triPolygon.push_back(tri.getVert(2));
        //判断c是否在以tri为底dir={0,1,0}为方向向量的斜棱柱内
        float dir[4]={0,-1,0,0};//注意dir要向下
        if(isPointInLeanedVolum(dir, triPolygon, c) ){//c在以tri为底dir为方向向量的斜棱柱内
            //计算c在IDtri所在平面上的投影高度
            //IDtri上取点P
            const Cc3dVector4&P=tri.getVert(0);
            //IDtri法向量
            const Cc3dVector4&norm=tri.getNorm();
            //c在IDtri所在平面上的投影高度为
            //H=(norm[X]*(P[X]-c[X])+norm[Z]*(P[Z]-c[Z]))rm[Y]+P[Y]
            //由于前面已跳过非地板面，所以这里norm[Y]一定不为0
            float H=(norm.x()*(P.x()-c.x())+norm.z()*(P.z()-c.z()))/norm.y()+P.y();
            if(H>=c.y()-1.5){//必须判断是否H>=c[Y]
                //减10是因为，由于误差，有时主角中心可能会到天花板之上，也不应错过检测
                if(H<Hmin){
                    Hmin=H;
                    pTri=(CtriangleWithNorm*)&tri;
                }
            }
        }
    }//得到Hmin和pIDtri
    if(pTri!=NULL){
        houseCeilNorm=pTri->getNorm();
    }else{
        houseCeilNorm.init(0, -1, 0, 0);
    }
    return Hmin;//如果c处不存在天花板，则返回Hmax的初始值inf（一个足够高的值）
}

static inline vector<float> collisionTestWithWall_multiPoint_common(const vector<CtriangleWithNorm>&triWithNormList,
                                                                    const Cc3dVector4&c,const float Rc,const float Rc_small,const Cc3dVector4&up,vector<Cc3dVector4>&e_backList,
                                                                    const int intended_nSP)
//以_c为球心，Rc为长半径的扁球体与*this进行多点碰撞检测（_c为世界坐标）
//RH为半短轴高度
//up为球体的向上方向
//前端返回打入深度列表，其中数值皆为正数，如果没有碰撞，则此列表为空
//e_backList返回单位回退向量列表，如果没有碰撞，返回列表为空
//e_backList输入必须为空
//intended_nSP为意向sample point数量，最小取3，精确些取5，更大的值计算量就太大了
{
    vector<float> dList;
    //----预计算若干数值
    float Rc_smallDivN=Rc_small/4;
    float bottomY=c.y()-Rc_small;
    float topY=c.y()+Rc_small;
    //----预生成采样点模板
    int nSP=max(intended_nSP,3);//但sample point数量不能少于3，且必须是奇数
    if(nSP%2==0)nSP++;//nSP必须是奇数
    vector<Cc3dVector4> SPlist(nSP);//采样点列表
    float SPRlist[nSP];//采样点半径列表
    //填充SPRlist，使中心采样点半径为Rc_small，向两边递减
    int imid=nSP/2;
    int d=Rc_small/imid;//公差
    int SPR=-d;
    for(int i=0;i<nSP;i++){
        if(i<imid){
            SPR+=d;
        }else{
            SPR-=d;
        }
        SPRlist[i]=SPR;
    }//得到SPRlist
    const float startPercent=-0.8;//起点百分比
    const float endPercent=0.8;//终点百分比
    //在[startPercent,endPercent]中等距插入nSP个点，其中包含这两个端点
    const float d_percent=(startPercent-endPercent)/(nSP-1);
    float percentList[nSP];
    percentList[0]=startPercent;
    for(int j=1;j<nSP;j++){//从1开始
        percentList[j]=percentList[j-1]+d_percent;
    }//得到percentList
    //----碰撞检测
    //与pIDtriList中每个三角面检测碰撞
    //为了得到正确的打入深度和反弹向量，必须检测到所有碰撞（而不能检测到一个就跳出循环）
    int nTri=(int)triWithNormList.size();
    for(int i=0;i<nTri;i++){
        const CtriangleWithNorm&tri=triWithNormList[i];
        //看IDtri是地板面还是墙壁或天花板
        if(IDtriExIsWhichPartOfHouse(tri)!=IDTRI_WALL){
            //跳过
            continue;
        }
        //看球体是否与IDtri相交
        //获得三个顶点
        //注意，要用vlist_share
        const Cc3dVector4&p0=tri.getVert(0);
        const Cc3dVector4&p1=tri.getVert(1);
        const Cc3dVector4&p2=tri.getVert(2);
        //先粗略检测
        //看c是否比p0,p1,p2都高过RH
        if(bottomY>p0.y()&&bottomY>p1.y()&&bottomY>p2.y()){//如果bottomY比p0,p1,p2都高，则不碰撞，跳过
            continue;
        }
        //看c是否比p0,p1,p2都低过RH
        if(topY<p0.y()&&topY<p1.y()&&topY<p2.y()){//如果topY比p0,p1,p2都低，则不碰撞，跳过
            continue;
        }
        //看球体中心是否在IDtri正面及球体是否与IDtriEx所在平面是否相交     
        Cplane plane;
        plane.init(p0,tri.getNorm());
        float PND=PND_point_plane(plane,c);
        if(PND<-Rc_smallDivN){//球中在IDtri背面大于一定深度，认为不相交
            continue;
        }
        if(PND>Rc){//球体到平面距离过大，球与IDtri所在平面不相交
            //那么也一定不与IDtri相交
            continue;//跳过
        }
        //否则有可能相交，作进一步检测
        //求pos到IDtri的最短距离及取得此距离的点
        Cc3dVector4 minp;//取得最小距离的点
        float minDis=minDis_point_Tri(c,p0,p1,p2,tri.getNorm(),minp);//最小距离
        if(minDis<Rc){//碰撞
            //检查IDtri在竖直平面上的最大投影是否与球体赤道线相交
            {
                //求使IDtri在竖直方向上取得最大投影的投影向量
                //就是将IDtri.norm去掉Y分量
                float projv[4]={tri.getNorm().getArray()[0],0,tri.getNorm().getArray()[2],0};//得到projv,未单位化
                //projv和iDtri构成一个斜锥体，看c是否在此锥体之内
                //制作锥体的三个侧壁（法向量指向内）
                //将p0,p1,p2作成polygon
                vector<Cc3dVector4> polygon(3);
                polygon[0]=p0;
                polygon[1]=p1;
                polygon[2]=p2;
                vector<Cplane> planeList=getLeanedVolum(projv, polygon);
                //球体赤道线是否有在planeList内的部分
                //为了简单，我们先求赤道线在projv方向的投影
                //然后在此投影（一条直径线）上取若干个采样点
                //检查这些采样点是否有落在锥体内者
                //求这些采样点
                //求赤道直径的单位方向向量
                Cc3dVector4 dir;//赤道直径的单位方向向量
                dir=cross(up, projv);
                dir=normalize(dir);//得到dir
                //填充SPlist
                for(int j=0;j<nSP;j++){
                    Cc3dVector4 temp=dir*percentList[j];
                    SPlist[j]=c+temp;
                }//得到SPlist
                //看各中心采样球（SPlist+SPRlist）是否有在volume内的部分
                bool haveSomeOneInVolume=false;//假设没有
                for(int j=0;j<nSP;j++){//遍历SPlist
                    Cc3dVector4&sp=SPlist[j];//当前采样点
                    //判断sp为中心的采样球是否在锥体内部
                    bool inVolume=isPointInVolum(planeList, sp,SPRlist[j]);//得到inVolume
                    if(inVolume){//只要发现一个在锥体内部，就可得到haveSomeOneInVolume的值了，可跳出
                        haveSomeOneInVolume=true;
                        break;
                    }
                }//得到haveSomeOneInVolume
                if(haveSomeOneInVolume){//有在锥体内部的采样点，碰撞
                    //放行
                }else{//没有在锥体内部的采样点，不碰撞
                    continue;//跳过
                }
            }
            
            //求反弹向量
            Cc3dVector4 v=tri.getNorm();//得到反弹向量
            e_backList.push_back(v);
            //求打入深度d
            float d;
            //如果c在IDtri正面，则d=Rc-minDis
            //如果c在IDtri反面，则d=Rc+minDis
            if(PND>=0){
                d=Rc-minDis;
            }else{
                d=Rc+minDis;
            }
            dList.push_back(d);
        }
        
    }
    return dList;
}

static inline float collisionTestWithWall_multiPoint_common(const vector<CtriangleWithNorm>&triWithNormList,
                                                            const Cc3dVector4&c,const float Rc,const float RH,const Cc3dVector4&up,Cc3dVector4&e_back,
                                                            const int intended_nSP)
//以_c为球心，Rc为长半径的扁球体与*this进行多点碰撞检测（_c为世界坐标）
//up为球体的向上方向
//若碰撞，前端返回打入深度；若不碰撞，前端返回-1
//若发生碰撞，则e_back带回反弹方向（单位向量，世界坐标），否则e_back无效
//intended_nSP为意向sample point数量
{
     vector<Cc3dVector4> e_backList;
     vector<float> dList=collisionTestWithWall_multiPoint_common(triWithNormList,c,Rc,RH,up,e_backList,intended_nSP);
    //已得到dList和e_backList
    int nd=(int)dList.size();
    if(nd>0){//碰撞
        //求d和e_back
        //d取最大值
        float maxd=dList[0];
        for(int i=1;i<nd;i++){
            if(dList[i]>maxd){
                maxd=dList[i];
            }
        }//得到maxd
        //e_back取加权平均值
        e_back.init(0,0,0,0);
        for(int i=0;i<nd;i++){
            e_backList[i]=e_backList[i]*dList[i];
            e_back=e_back+e_backList[i];
        }
        e_back=normalize(e_back);//得到e_back
        return maxd;
    }else{//不碰撞
        return -1;
    }
}









static inline float collisionTestWithWall_singlePoint_common(const vector<CtriangleWithNorm>&triWithNormList,
                                                                    const Cc3dVector4&c,const float Rc,const float Rc_small,const Cc3dVector4&up,Cc3dVector4&e_back,
                                                             const int intended_nSP)
//以_c为球心，Rc为长半径的扁球体与*this进行多点碰撞检测（_c为世界坐标）
//RH为半短轴高度
//up为球体的向上方向
//若碰撞，前端返回打入深度；若不碰撞，前端返回-1
//若发生碰撞，则e_back带回反弹方向（单位向量，世界坐标），否则e_back无效
//intended_nSP为意向sample point数量
{
    //----预计算若干数值
    float Rc_smallDivN=Rc_small/4;
    float bottomY=c.y()-Rc_small;
    float topY=c.y()+Rc_small;
    //----预生成采样点模板
    int nSP=max(intended_nSP,3);//但sample point数量不能少于3，且必须是奇数
    if(nSP%2==0)nSP++;//nSP必须是奇数
    vector<Cc3dVector4> SPlist(nSP);//采样点列表
    float SPRlist[nSP];//采样点半径列表
    //填充SPRlist，使中心采样点半径为Rc_small，向两边递减
    int imid=nSP/2;
    int d=Rc_small/imid;//公差
    int SPR=-d;
    for(int i=0;i<nSP;i++){
        if(i<imid){
            SPR+=d;
        }else{
            SPR-=d;
        }
        SPRlist[i]=SPR;
    }//得到SPRlist
    const float startPercent=-0.8;//起点百分比
    const float endPercent=0.8;//终点百分比
    //在[startPercent,endPercent]中等距插入nSP个点，其中包含这两个端点
    const float d_percent=(startPercent-endPercent)/(nSP-1);
    float percentList[nSP];
    percentList[0]=startPercent;
    for(int j=1;j<nSP;j++){//从1开始
        percentList[j]=percentList[j-1]+d_percent;
    }//得到percentList
    //----碰撞检测
    //与pIDtriList中每个三角面检测碰撞
    //为了得到正确的打入深度和反弹向量，检测到一个就跳出循环
    int nTri=(int)triWithNormList.size();
    for(int i=0;i<nTri;i++){
        const CtriangleWithNorm&tri=triWithNormList[i];
        //看IDtri是地板面还是墙壁或天花板
        if(IDtriExIsWhichPartOfHouse(tri)!=IDTRI_WALL){
            //跳过
            continue;
        }
        //看球体是否与IDtri相交
        //获得三个顶点
        //注意，要用vlist_share
        const float*p0=tri.getVert(0).getArray();
        const float*p1=tri.getVert(1).getArray();
        const float*p2=tri.getVert(2).getArray();
        //先粗略检测
        //看c是否比p0,p1,p2都高过RH
        if(bottomY>p0[1]&&bottomY>p1[1]&&bottomY>p2[1]){//如果bottomY比p0,p1,p2都高，则不碰撞，跳过
            continue;
        }
        //看c是否比p0,p1,p2都低过RH
        if(topY<p0[1]&&topY<p1[1]&&topY<p2[1]){//如果topY比p0,p1,p2都低，则不碰撞，跳过
            continue;
        }
        //看球体中心是否在IDtri正面及球体是否与IDtriEx所在平面是否相交
        Cplane plane;
        plane.init(p0,tri.getNorm());
        float PND=PND_point_plane(plane,c);
        if(PND<-Rc_smallDivN){//球中在IDtri背面大于一定深度，认为不相交
            continue;
        }
        if(PND>Rc){//球体到平面距离过大，球与IDtri所在平面不相交
            //那么也一定不与IDtri相交
            continue;//跳过
        }
        //否则有可能相交，作进一步检测
        //求pos到IDtri的最短距离及取得此距离的点
        Cc3dVector4 minp;//取得最小距离的点
        float minDis=minDis_point_Tri(c,p0,p1,p2,tri.getNorm(),minp);//最小距离
        if(minDis<Rc){//碰撞
            //检查IDtri在竖直平面上的最大投影是否与球体赤道线相交
            {
                //求使IDtri在竖直方向上取得最大投影的投影向量
                //就是将IDtri.norm去掉Y分量
                float projv[4]={tri.getNorm().getArray()[0],0,tri.getNorm().getArray()[2],0};//得到projv,未单位化
                //projv和iDtri构成一个斜锥体，看c是否在此锥体之内
                //制作锥体的三个侧壁（法向量指向内）
                //将p0,p1,p2作成polygon
                vector<Cc3dVector4> polygon(3);
                polygon[0]=Cc3dVector3(p0).toV4(1);
                polygon[1]=Cc3dVector3(p1).toV4(1);
                polygon[2]=Cc3dVector3(p2).toV4(1);
                vector<Cplane> planeList=getLeanedVolum(projv, polygon);
                //球体赤道线是否有在planeList内的部分
                //为了简单，我们先求赤道线在projv方向的投影
                //然后在此投影（一条直径线）上取若干个采样点
                //检查这些采样点是否有落在锥体内者
                //求这些采样点
                //求赤道直径的单位方向向量
                Cc3dVector4 dir;//赤道直径的单位方向向量
                dir=cross(up, projv);
                dir=normalize(dir);//得到dir
                //填充SPlist
                for(int j=0;j<nSP;j++){
                    Cc3dVector4 temp=dir*percentList[j];
                    SPlist[j]=c+temp;
                }//得到SPlist
                //看各中心采样球（SPlist+SPRlist）是否有在volume内的部分
                bool haveSomeOneInVolume=false;//假设没有
                for(int j=0;j<nSP;j++){//遍历SPlist
                    Cc3dVector4&sp=SPlist[j];//当前采样点
                    //判断sp为中心的采样球是否在锥体内部
                    bool inVolume=isPointInVolum(planeList, sp,SPRlist[j]);//得到inVolume
                    if(inVolume){//只要发现一个在锥体内部，就可得到haveSomeOneInVolume的值了，可跳出
                        haveSomeOneInVolume=true;
                        break;
                    }
                }//得到haveSomeOneInVolume
                if(haveSomeOneInVolume){//有在锥体内部的采样点，碰撞
                    //放行
                }else{//没有在锥体内部的采样点，不碰撞
                    continue;//跳过
                }
            }
            
            //求反弹向量
            e_back=tri.getNorm();//得到反弹向量
            //求打入深度d
            float d;
            //如果c在IDtri正面，则d=Rc-minDis
            //如果c在IDtri反面，则d=Rc+minDis
            if(PND>=0){
                d=Rc-minDis;
            }else{
                d=Rc+minDis;
            }
            return d;
        }
        
    }
    return -1;
}

#endif /* defined(__HelloOpenGL__collisionWithModelAtTriLevel__) */
