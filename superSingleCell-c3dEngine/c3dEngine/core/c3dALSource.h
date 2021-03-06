//
//  c3dALSource.h
//  HelloOpenGL
//
//  Created by apple on 14-2-23.
//
//

#ifndef __HelloOpenGL__c3dALSource__
#define __HelloOpenGL__c3dALSource__

#include <iostream>
#include "c3dALBuffer.h"
#include "c3dMath.h"
//the max number of source play at the same time (maxSimultaneouslyPlay) have limit
//http://stackoverflow.com/questions/2871905/openal-determine-maximum-sources
//though it may allowed that to create more source than maxSimultaneouslyPlay,
//we'd better limit the number of source to maxSimultaneouslyPlay
const int maxSimultaneouslyPlay=32;//16


class Cc3dALSource:public Cc3dObject
{//多个source可以共用buffer
protected:
    ALuint m_source;
    string m_name;
    Cc3dALBuffer*m_buffer;
    bool m_isStopedByStopFunc;//是否通过调用stop函数停止了
    bool m_isPausedByPauseFunc;//是否通过调用pause函数停止了
public:
    
    Cc3dALSource(){
        m_source=0;
        m_buffer=NULL;
    }
    virtual~Cc3dALSource(){
        if(getIsValidSource()){
            stop();
            alDeleteSources(1, &m_source);
        }
        if(m_buffer)m_buffer->release();
        C3DCHECK_AL_ERROR_DEBUG();
    }
    Cc3dALBuffer*getBuffer()const{return m_buffer;}
    void setBuffer(Cc3dALBuffer*buffer){
        assert(buffer);
        if(m_buffer==NULL){
            m_buffer=buffer;
            buffer->retain();
        }else{
            m_buffer->release();
            m_buffer=buffer;
            m_buffer->retain();
        }
    }
    
    bool getIsValidSource()const{
        bool isValid=(m_source!=0&&alIsSource(m_source));
        return isValid;
    }
    void updatePosition(const Cc3dVector4&pos){
        if(m_source==0)return;
        alSourcefv(m_source, AL_POSITION,pos.getArray());
    }
    void setGain(float gain){
        if(m_source==0)return;
        alSourcef(m_source, AL_GAIN, gain);
    }
    void play(){//对于已停止或正在播放的source重新开始播放，对于暂停的source继续播放
        if(m_source==0)return;
        m_isStopedByStopFunc=false;
        m_isPausedByPauseFunc=false;
        alSourcePlay(m_source);
    }
    ALint getState()const {
        if(m_source==0){
            cout<<"source==0,不能获得状态!"<<endl;
            exit(0);
        };
        ALint sourceState;
        alGetSourcei(m_source, AL_SOURCE_STATE, &sourceState);
        return sourceState;
    }
    bool getIsPlaying()const {//是否在播放中
        if(m_source==0)return false;
        ALint sourceState=getState();
        if(sourceState==AL_PLAYING){
            return true;
        }else{
            return false;
        }
    }
    bool getIsStoped()const {//是否已停止
        if(m_source==0)return false;
        ALint sourceState=getState();
        if(sourceState==AL_STOPPED){
            return true;
        }else{
            return false;
        }
    }
    
    bool getIsPaused()const {
        if(m_source==0)return false;
        ALint sourceState=getState();
        if(sourceState==AL_PAUSED){
            return true;
        }else{
            return false;
        }
    }
    void playIfNotPlaying()
    //如果没在play，则play
    {
        if(m_source==0)return;
        m_isStopedByStopFunc=false;
        m_isPausedByPauseFunc=false;
        ALint sourceState;
        alGetSourcei(m_source, AL_SOURCE_STATE, &sourceState);
        if(sourceState!=AL_PLAYING){
            alSourcePlay(m_source);
        }
    }
    void setIsLooping(bool looping){
        if(m_source==0)return;
        if(looping){
            alSourcei(m_source, AL_LOOPING, AL_TRUE);
        }else{
            alSourcei(m_source, AL_LOOPING, AL_FALSE);
        }
    }
    
    void stop(){
        if(m_source==0)return;
        if(m_isStopedByStopFunc)return;
        if(this->getIsPlaying()){
            alSourceStop(m_source);
            
        }
        m_isStopedByStopFunc=true;
    }
    float getGain(){
        if(m_source==0)return 0;
        float gain;
        alGetSourcef(m_source, AL_GAIN, &gain);
        return gain;
    }
    void gainDec(float dGain){//减小音量
        if(m_source==0)return;
        if(m_isStopedByStopFunc)return;
        float gain;
        alGetSourcef(m_source, AL_GAIN, &gain);
        float newGain=maxf(0,gain-dGain);//音量不小于0
        alSourcef(m_source, AL_GAIN, newGain);
        if(newGain==0)stop();//如果音量等于0，则结束播放
    }
    void pause(){
        if(m_source==0)return;
        if(m_isPausedByPauseFunc)return;
        alSourcePause(m_source);
        m_isPausedByPauseFunc=true;
        
    }
    void initSource(Cc3dALBuffer*pBuffer);
    string getName()const {return m_name;}
    void setName(const string&name){m_name=name;}
};


#endif /* defined(__HelloOpenGL__c3dALSource__) */
