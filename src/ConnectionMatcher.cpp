#include "ConnectionMatcher.hpp"
#include <boost/graph/adjacency_list.hpp>
namespace RTT
{
namespace introspection
{

    
ConnectionMatcher::ConnectionMatcher()
{

}

void ConnectionMatcher::addTaskData(const RTT::introspection::TaskData &ndata)
{
    data.push_back(ndata);
}


void ConnectionMatcher::createGraph()
{
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    
    std::cout << "BUILDING GRAPH" << std::endl;
    
    std::multimap<std::string, ChannelBase *> localURIToElementMap;
    std::multimap<std::string, ChannelBase *> remoteURIToElementMap;
    size_t curIndent = 0;
    tasks.reserve(data.size());
    for(const TaskData &taskData: data)
    {
        std::cout << "TaskData " << taskData.taskName << std::endl;
        tasks.push_back(Task());
        
        Task *task = tasks.data() + (tasks.size() - 1);
        task->name = taskData.taskName;
        curIndent += 4;
        for(const PortData &pd: taskData.portData)
        {
            std::cout << std::string(curIndent, ' ') << "PortData " << pd.portName << std::endl;
            Port *p;
            if(pd.type == PortData::INPUT)
            {
                task->inputPorts.push_back(new InputPort());
                p = task->inputPorts.back();
            }
            else
            {
                task->outputPorts.push_back(new OutputPort());
                p = task->outputPorts.back();
            }
            
            p->owningTask = task;
            p->name = pd.portName;

            curIndent += 4;

            for(const ConnectionData &cd : pd.connectionData)
            {
                std::cout << std::string(curIndent, ' ') << "Connection " << std::endl;
                Connection c;
                bool firstElem = true;
                for(const ElementData &e: cd.elementData)
                {
                    std::cout << std::string(curIndent, ' ') << "Con Element " << e.type << " localURI " << e.localURI << " remoteURI " << e.remoteURI << std::endl;
                    
                    //special case first : 
                    //in cas of a local connection, we get double channel elements
                    //one time from the input and one time from the output port
                    if(firstElem)
                    {
                        //check if the element already exists
                        auto localIt = localURIToElementMap.find(e.localURI);
                        if(localIt != localURIToElementMap.end())
                        {
                            std::cout << "Found Element that is already registered" << std::endl;

                            auto elem = localIt->second;
                            if(e.type != elem->type)
                                throw std::runtime_error("Error, type of connection mismatch");
                            elem->connectedToPort = p;
                            std::cout << "Registering Elem " << elem->type << " at port " << p->name << " of task " << p->owningTask->name << std::endl;
                            c.firstElement = elem;
                            break;
                        }
                    }

                    //from here on we should not have any duplicates
                    
                    ChannelBase *be = nullptr;
                    
                    if(e.type == "ChannelBufferElement")
                    {
                        BufferChannel *bc = new BufferChannel();
                        bc->fillLevel = e.numSamples;
                        bc->samplesDropped = e.droppedSamples;
                        bc->bufferSize = e.bufferSize;
                        be = bc;
                    }
                    else
                    {
                        be = new ChannelBase();
                    }
                    
                    be->type = e.type;
                    be->localURI = e.localURI;
                    be->remoteURI = e.remoteURI;
                    
                    //first we register at the map
                    localURIToElementMap.insert(std::make_pair(be->localURI, be));
                    if(!be->remoteURI.empty())
                    {
                        remoteURIToElementMap.insert(std::make_pair(be->remoteURI, be));
                    
                        //let's see if we got matching remotes
                        auto range = localURIToElementMap.equal_range(be->remoteURI);
                        for(auto localIt = range.first; localIt != range.second; localIt++)
                        {
                            //register all connections a the new element
                            localIt->second->in.push_back(be);
                            be->out.push_back(localIt->second);
                            
                            std::cout << "Element " << be->type << " la " << be->localURI << " ru " << be->remoteURI << " Writes into " << localIt->second->type << " la " << localIt->second->localURI << " ra " << localIt->second->remoteURI << std::endl;
                        }
                    }
                    
                    //we also need to do the reverse connection
                    auto range = remoteURIToElementMap.equal_range(be->localURI);
                    for(auto remoteIt = range.first; remoteIt != range.second; remoteIt++)
                    {
                        //register all connections a the new element
                        remoteIt->second->out.push_back(be);
                        be->in.push_back(remoteIt->second);
                        
                        std::cout << "Element " << remoteIt->second->type << " la " << remoteIt->second->localURI << " ra " << remoteIt->second->remoteURI << " Writes into " << be->type << " la " << be->localURI << " ru " << be->remoteURI << std::endl;
                    }
                    
                    
                    if(firstElem)
                    {
                        std::cout << "Registering Elem " << be->type << " at port " << p->name << " of task " << p->owningTask->name << std::endl;
                        be->connectedToPort = p;
                        c.firstElement = be;
                    }
                    
                    firstElem = false;
                }
                
                p->connections.push_back(c);
            }
            curIndent -= 4;
        }

        curIndent -= 4;

    }
    
    std::cout << "Exit " << std::endl;
    
}

void ConnectionMatcher::printPort(const Port* port, int curIndent)
{
    bool isInput = dynamic_cast<const InputPort *>(port);
    curIndent += 4;
    std::cout << std::string(curIndent, ' ') << port->name << " :" << std::endl;
    curIndent += 4;
    std::cout << std::string(curIndent, ' ')  << "Connections" << std::endl;
    for(const Connection &con: port->connections)
    {
        curIndent += 4;
        std::cout << std::string(curIndent, ' ');
        
        if(!con.firstElement->connectedToPort || con.firstElement->connectedToPort != port)
        {
            std::cout << "Port mismatch for element " << *(con.firstElement) << " for port " << port->name << "(" << port << ")";
            if(!con.firstElement->connectedToPort)
                std::cout << " not connected ";
            else
                std::cout << " connected Port " << con.firstElement->connectedToPort->name << "(" << con.firstElement->connectedToPort << ")";
            std::cout << std::endl;
            throw std::runtime_error("Error, first element is not connected to this port");
        }
        
        const ChannelBase *cb = con.firstElement;
        while(cb)
        {
            std::cout << cb->type << "(" << cb->localURI << ")->";
            if(cb->connectedToPort && cb->connectedToPort != port)
            {
                std::cout << cb->connectedToPort->name << "->" << cb->connectedToPort->owningTask->name;
            }

            if(isInput)
            {
                if(cb->in.size())
                    cb = cb->in.front();
                else
                    cb = nullptr;
            }
            else
            {
                if(cb->out.size())
                    cb = cb->out.front();
                else
                    cb = nullptr;                
            }
        }
        std::cout << std::endl;
        curIndent -= 4;
    }
    curIndent -= 4;

}


void ConnectionMatcher::printGraph()
{
    for(const Task &t: tasks)
    {
        int curIndent = 0;
        std::cout << "Task " << t.name << std::endl;
        std::cout << "Output Ports : " << std::endl;
        for(const OutputPort *op: t.outputPorts)
        {
            printPort(op, curIndent);
        }
        std::cout << "Inputs Ports : " << std::endl;
        for(const InputPort *op: t.inputPorts)
        {
            printPort(op, curIndent);
        }
    }
}


}
    
}