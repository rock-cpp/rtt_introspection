#include "IntrospectionTypes.hpp"
#include "IntrospectionService.hpp"
#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>
#include <rtt/transports/mqueue/MQChannelElement.hpp>
#include <rtt/plugin/ServicePlugin.hpp>
#include <orocos_cpp_base/OrocosHelpers.hpp>


namespace RTT
{
namespace introspection
{

const std::string IntrospectionService::ServiceName = std::string("Introspection");
    
IntrospectionService::IntrospectionService(RTT::TaskContext* owner): Service(ServiceName, owner)
{
    std::cout << "I am loaded " << std::endl;
    
    OrocosHelpers::loadTypekitAndTransports("rtt_introspection_typekit");
    
    RTT::Operation<TaskData ()> *op = new RTT::Operation<TaskData ()>("getIntrospectionInformation", boost::bind(&IntrospectionService::getIntrospectionInformation, this));
    
    owner->addOperation(*op);
    
    std::cout << "Operation added" << std::endl;
}


TaskData IntrospectionService::getIntrospectionInformation()
{
    std::cout << "Introspection test..." << std::endl;
    RTT::TaskContext* task = getOwner();
    
    TaskData taskData;
    taskData.taskName = task->getName();
    
    RTT::DataFlowInterface *dfif = task->ports();
    for(RTT::base::PortInterface *port : dfif->getPorts())
    {
        PortData portData;
        portData.portName = port->getName();

        std::cout << "Port " << portData.portName << std::endl;
        
        if(dynamic_cast<RTT::base::InputPortInterface *>(port))
        {
            portData.type = PortData::INPUT;
            std::cout << "Port Type is INPUT" << std::endl;
        }
        else
        {
            portData.type = PortData::OUTPUT;
            std::cout << "Port Type is OUTPUT" << std::endl;
        }

        
        
        const RTT::internal::ConnectionManager *conManager = port->getManager();
        if(!conManager)
            throw std::runtime_error("Error, not connection manager found");
        
        for(const RTT::internal::ConnectionManager::ChannelDescriptor &desc : conManager->getChannels())
        {
            ConnectionData connData;
            
            RTT::base::ChannelElementBase *elem = boost::get<1>(desc).get();
            RTT::base::ChannelElementBase *lastElem = elem;
            
//             if(portData.type == PortData::INPUT)
//             {
//                 //we first go though the list and look for the input point
//                 while(elem->getInput())
//                     elem = elem->getInput().get();
//             }
//             
            std::cout << "Inspection Channels" << std::endl;
            int cnt = 0;
            while(elem)
            {
                ElementData elemdata;
                elemdata.type = elem->getElementName();
                if(elemdata.type == "ChannelBufferElement")
                {
                    const RTT::internal::ChannelBufferElementBase *buffer = dynamic_cast<const RTT::internal::ChannelBufferElementBase *>(elem);
                    elemdata.bufferSize = buffer->getBufferSize();
                    elemdata.droppedSamples = buffer->getNumDroppedSamples();
                    elemdata.numSamples = buffer->getBufferFillSize();
                    
                }

                std::cout << "Elem is " << elemdata.type << std::endl; 

                elemdata.remoteElement = elem->isRemoteElement();
                elemdata.localURI = elem->getLocalURI();
                elemdata.remoteURI = elem->getRemoteURI();
                
                std::cout << "Is remote, localURI " << elemdata.localURI << " remoteURI " << elemdata.remoteURI << std::endl;
                connData.elementData.push_back(elemdata);

                lastElem = elem;
                
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

                std::cout << "Next Elem " << elem << " last is" << lastElem << std::endl;
                cnt++;
            }
            
            portData.connectionData.push_back(connData);
        }
        
        taskData.portData.push_back(portData);
    }
    
    std::cout << "Done " << std::endl;
    return taskData;
}

}
}

ORO_SERVICE_NAMED_PLUGIN(RTT::introspection::IntrospectionService, RTT::introspection::IntrospectionService::ServiceName)