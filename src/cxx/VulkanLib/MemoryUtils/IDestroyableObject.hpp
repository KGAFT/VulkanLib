//
// Created by kgaft on 11/7/23.
//
#pragma once

class IDestroyableObject{
protected:
    bool destroyed = false;
    virtual void destroy() = 0;
public:
    IDestroyableObject(){
        if(!destroyed){
            destroy();
        }
    }

};