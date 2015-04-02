#ifndef CONNECTIONMATCHER_H
#define CONNECTIONMATCHER_H

#include "IntrospectionTypes.hpp"
#include <vector>
#include <map>
#include <iostream>
namespace RTT
{
namespace introspection
{

class Port;
class Task;
    
class ChannelBase
{
public:
    ChannelBase() : connectedToPort(nullptr)
    {
    };
    std::string type;
    
    std::string remoteURI;
    std::string localURI;
    
    std::vector<ChannelBase *> in;
    std::vector<ChannelBase *> out;
    
    Port *connectedToPort;
    
    virtual ~ChannelBase() {};
};

class BufferChannel : public ChannelBase
{
public:
    size_t bufferSize;
    size_t fillLevel;
    size_t samplesDropped;
};

class Connection
{
public:
    ChannelBase * firstElement;
};

class Port
{
public:
    virtual ~Port() {};
    std::string name;
    
    std::vector<Connection> connections;
    
    Task *owningTask;
};

class InputPort : public Port
{
};

class OutputPort : public Port
{
};


class Task
{
private:

public:
    std::string name;
    
    std::vector<InputPort *> inputPorts;
    std::vector<OutputPort *> outputPorts;
};

class ConnectionMatcher
{
    std::vector<TaskData> data;
    std::vector<Task> tasks;
public:
    ConnectionMatcher();
    
    void addTaskData(const TaskData &data);
    
    void createGraph();
    
    void printGraph();
    
    void printPort(const RTT::introspection::Port* port, int curIndent);
    
    void writeGraphToDotFile(const std::string &fileName);
};



}
}

inline std::ostream& operator << (std::ostream& io, RTT::introspection::ChannelBase const& channel)
{
    
    io << channel.type << "(" << channel.localURI << " / " << channel.remoteURI << ")";
    return io;
}

#endif // CONNECTIONMATCHER_H
