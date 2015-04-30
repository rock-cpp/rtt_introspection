#include "IntrospectionTypes.hpp"
#include "IntrospectionService.hpp"
#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>
#include <rtt/transports/mqueue/MQChannelElement.hpp>
#include <rtt/plugin/ServicePlugin.hpp>
#include <orocos_cpp/PluginHelper.hpp>


namespace RTT
{
namespace introspection
{

const std::string IntrospectionService::ServiceName = std::string("Introspection");
const std::string IntrospectionService::OperationName = std::string("getIntrospectionInformation");
    
IntrospectionService::IntrospectionService(RTT::TaskContext* owner): Service(ServiceName, owner)
{
    PluginHelper::loadTypekitAndTransports("rtt_introspection");
    RTT::Operation<TaskData ()> *op = new RTT::Operation<TaskData ()>(OperationName, boost::bind(&IntrospectionService::getIntrospectionInformation, this));
    owner->addOperation(*op);
}


TaskData IntrospectionService::getIntrospectionInformation()
{
    RTT::TaskContext* task = getOwner();
    
    TaskData taskData;
    taskData.taskName = task->getName();
    
    RTT::DataFlowInterface *dfif = task->ports();
    for(RTT::base::PortInterface *port : dfif->getPorts())
    {
        PortData portData;
        portData.portName = port->getName();

        
        if(dynamic_cast<RTT::base::InputPortInterface *>(port))
        {
            portData.type = PortData::INPUT;
        }
        else
        {
            portData.type = PortData::OUTPUT;
        }

        
        
        const RTT::internal::ConnectionManager *conManager = port->getManager();
        if(!conManager)
            throw std::runtime_error("Introspection :Error, no connection manager found");
        
        for(const RTT::internal::ConnectionManager::ChannelDescriptor &desc : conManager->getChannels())
        {
            ConnectionData connData;
            connData.policy = boost::get<2>(desc);
            
            RTT::base::ChannelElementBase *elem = boost::get<1>(desc).get();
            int cnt = 0;
            while(elem)
            {
                ElementData elemdata;
                elemdata.type = elem->getElementName();
                if(elemdata.type == "ChannelBufferElement")
                {
                    const RTT::internal::ChannelBufferElementBase *buffer = dynamic_cast<const RTT::internal::ChannelBufferElementBase *>(elem);
                    if(buffer)
                    {
                        elemdata.bufferSize = buffer->getBufferSize();
                        elemdata.droppedSamples = buffer->getNumDroppedSamples();
                        elemdata.numSamples = buffer->getBufferFillSize();
                    }
                    else
                    {
                        throw std::runtime_error("Introspection : Error could not cast to ChannelBufferElementBase but type is ChannelBufferElement");
                    }
                }

                elemdata.remoteElement = elem->isRemoteElement();
                elemdata.localURI = elem->getLocalURI();
                elemdata.remoteURI = elem->getRemoteURI();
                
                connData.elementData.push_back(elemdata);

                if(portData.type == PortData::INPUT)
                {
                    //traverse the chain
                    elem = elem->getInput().get();
                }
                else
                {
                    //traverse the chain
                    elem = elem->getOutput().get();
                }

                cnt++;
            }
            
            portData.connectionData.push_back(connData);
        }
        
        taskData.portData.push_back(portData);
    }
    
    return taskData;
}

}
}

ORO_SERVICE_NAMED_PLUGIN(RTT::introspection::IntrospectionService, RTT::introspection::IntrospectionService::ServiceName)