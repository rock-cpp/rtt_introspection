#pragma once

#include <string>
#include <vector>
#include <rtt/ConnPolicy.hpp>

namespace RTT
{
namespace introspection
{

class ElementData
{
public:
    bool remoteElement;
    std::string localURI;
    std::string remoteURI;
    
    std::string type;

    //only used in buffer case
    size_t bufferSize;
    size_t droppedSamples;
    size_t numSamples;
};

class ConnectionData
{
public:
    std::vector<ElementData> elementData;
    RTT::ConnPolicy policy;
};

class PortData
{
public:
    enum TYPE
    {
        INPUT,
        OUTPUT,
    };
    std::string portName;
    
    enum TYPE type;
    
    std::vector<ConnectionData> connectionData;
};


class TaskActivity
{
public:
    std::vector<std::string> eventPorts;
    
    std::string fileDescriptor;
    
    int priority;
    
    double period;
    
    std::string type;
    
    bool realTime;
    
};

class TaskData
{
public:
    std::string taskName;
    
    std::string taskCommand;
    
    std::string taskType;
    
    std::string taskState;

    int taskStateInternal; //Internal "Rock" state as communicated via state port
    
    int taskPid;
    
    std::vector<PortData> portData;
    
    TaskActivity taskActivity;
    
    std::string taskDeployment;

    std::string taskHost;
};

}
}
