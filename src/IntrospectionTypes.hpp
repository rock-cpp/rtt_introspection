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

class TaskData
{
public:
    std::string taskName;
    
    std::string taskType;
    
    int32_t taskState;
    
    std::vector<PortData> portData;
};

}
}