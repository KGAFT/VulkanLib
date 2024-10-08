//
// Created by kgaft on 11/9/23.
//
#pragma once

#include <map>
#include <vector>
#include <VulkanLib/MemoryUtils/MemoryUtils.hpp>
#include <cstdlib>
#include <iostream>

template <typename T> class SeriesObject{
private:
    std::map<T*, bool> objectInstances;
    unsigned int releasedObjectCount = 0;
public:
     T* getObjectInstance()  {
        for (auto &item: objectInstances){
            if(!item.second){
                item.second = true;
                MemoryUtils::memClear(item.first, sizeof(T));
                releasedObjectCount--;
                return item.first;
            }
        }
        T* res = new T;
        objectInstances[res] = true;
        return res;
    }
    void releaseObjectInstance( T* instance){
        objectInstances[instance] = false;
        releasedObjectCount ++;
        if(releasedObjectCount>2){
            finalizeGarbageObjects();
        }
    }
private:
    std::vector<T*> toClear;
    void finalizeGarbageObjects(){
        for (auto &item: objectInstances){
            if(!item.second){
                toClear.push_back(item.first);
            }
        }
        unsigned int counter = 0;
        while(counter<toClear.size()-1){
            try{
                delete toClear[counter];
            }catch(std::exception& exception){
                std::cerr<<exception.what()<<std::endl;
            }
            objectInstances.erase(toClear[counter]);
            counter++;
        }
        toClear.clear();
    }
public:
    ~SeriesObject(){
        for (const auto &item: objectInstances){
            try{
                delete item.first;
            }catch (std::exception& e){
                std::cerr<<e.what()<<std::endl;
            }
        }
    }
};