#include <string>
#include "CNDMatcher.hpp"
#include <iostream>

namespace RTT
{
namespace introspection
{

bool isValid(const cnd::model::PortRef& p){
    return (p.getPortName() != "" && p.getTaskId() != "");
}

void setPolicy(cnd::model::Connection& conn, RTT::ConnPolicy policy){
    switch(policy.type){
    case policy.UNBUFFERED:
        std::cerr << "Did not expect to see 'UNBUFFERED' connection policy! Storing it as a 'DATA' connection" << std::endl;
        conn.setType(cnd::model::ConPolicy::DATA);
        conn.setSize(0);
        break;
    case policy.DATA:
        conn.setType(cnd::model::ConPolicy::DATA);
        conn.setSize(0);
        break;
    case policy.BUFFER:
        conn.setType(cnd::model::ConPolicy::BUFFER);
        conn.setSize(policy.size);
        break;
    case policy.CIRCULAR_BUFFER:
        conn.setType(cnd::model::ConPolicy::CIRCULARBUFFER);
        conn.setSize(policy.size);
        break;
    default:
        throw std::runtime_error(std::string()+"Did not expect policy-type: "+std::to_string(policy.type));
    }
}

std::vector<cnd::model::Connection> generateConnections(std::vector<TaskData>& taskData)
{
    typedef std::pair<std::string, std::string> ChainElem;
    typedef std::vector<ElementData> Chain;
    //For Debugging: port-id (task.port) is mapped to the Chain of ElementData attached to the port
    std::map<std::string, std::vector<Chain>> ochains; //Output ports
    std::map<std::string, std::vector<Chain>> ichains; //Input ports

    typedef std::string ChannelURI;
    //Maps the URI of a Channel*Element to the corresponding connection
    std::map<ChannelURI, cnd::model::Connection> connections;

    for(const TaskData& td : taskData){
        for(const PortData& pd : td.portData){
            std::string pname = td.taskName+"."+pd.portName;
            if(ochains.find(pname) != ochains.end()){
                throw(std::runtime_error("Ports exsits two times? that is surprising"));
            }
            if(ichains.find(pname) != ichains.end()){
                throw(std::runtime_error("Ports exsits two times? that is surprising"));
            }
            for(const ConnectionData& cd : pd.connectionData){
                Chain chain;
                for(const ElementData& ed : cd.elementData){
                    //chain.push_back(std::make_pair(ed.localURI, ed.remoteURI));
                    chain.push_back(ed);


                    if(ed.type == "ChannelBufferElement" || ed.type == "ChannelDataElement"){
                        //A connection INTRA-Process connection in RTT consists of:
                        //    InputEndPoint,
                        //    ChannelDataElement (unbuffered) or ChannelBufferElement (buffered)
                        //    OutputEndpoint
                        //Thereby the input and output port refer to the same Channel*Element with the
                        //remoteURI field.
                        //I.e. for a valid (local) connection the same remoteURI of a Channel*Element
                        //always appears twice: onnce on the corresponding input port and once
                        //on the corresponding port.
                        //
                        //Thus we create a map of cnd::model:Connectiosn indexed by Channel*Element-remoteURIs
                        //and insert the missing input/output
                        //port information when the same URI is seen a second time.
                        if(pd.type == PortData::INPUT){
                            connections[ed.remoteURI].setTo(cnd::model::PortRef(td.taskName, pd.portName));
                        }else{
                            connections[ed.remoteURI].setFrom(cnd::model::PortRef(td.taskName, pd.portName));
                        }
                        //Set Connection Policy
                        setPolicy(connections[ed.remoteURI], cd.policy);
                    }
                    else if(ed.type == "CorbaRemoteChannelElement" || ed.type == "MQChannelElement"){
                        //A connection INTER-Process connection in RTT consists of:
                        //    ChannelDataElement (unbuffered) or ChannelBufferElement (buffered)
                        //    CorbaRemoteChannelElement
                        //Here the CorbaRemoteChannelElement is the element where a correspondence
                        //of the connection can be created: The remoteURI of the connection's output
                        //port correspnds with thelocalURI of the input port
                        if(pd.type == PortData::INPUT){
                            connections[ed.localURI].setTo(cnd::model::PortRef(td.taskName, pd.portName));
                        }else{
                            connections[ed.remoteURI].setFrom(cnd::model::PortRef(td.taskName, pd.portName));
                        }
                        //Set Connection Policy
                        setPolicy(connections[ed.remoteURI], cd.policy);

                        if(ed.type == "MQChannelElement"){
                            std::cerr << "WARNING: A MessageQueue Connection was found. Exporting MessageQueue connects to CND was not tested." << std::endl;
                        }
                    }
                    else if(ed.type == "ConnInputEndpoint"){
                        //Ignore
                    }
                    else if(ed.type == "ConnOutputEndpoint"){
                        //Ignore
                    }
                    else{
                        throw(std::runtime_error("Unexpected ElementDataType: "+ed.type));
                    }
                }
                if(pd.type == PortData::INPUT){
                    ichains[pname].push_back(chain);
                }else if(pd.type == PortData::OUTPUT){
                    ochains[pname].push_back(chain);
                }else{
                    throw(std::runtime_error("Should not happen"));
                }
            }
        }
    }

    std::vector<cnd::model::Connection> ret;
    for(std::pair<ChannelURI, cnd::model::Connection> c : connections){
        if(isValid(c.second.getFrom()) && isValid(c.second.getTo()))
            ret.push_back(c.second);
    }
    return ret;
}


cnd::model::Network CNDMatcher::generateCND(std::vector<TaskData>& taskData)
{
    //Return value
    cnd::model::Network net;

    // Use task uid (which coincides with its name) to resolve deployment membership
    std::map<std::string,cnd::model::Deployment> deploymentMap;

    //Iterate TaskData to extract tasks
    for(const TaskData &t: taskData)
    {
        cnd::model::Task cndTask;
        cndTask.setUID(t.taskName);
        cndTask.setTaskState(t.taskState);
        cndTask.setModelType(t.taskType);
        cnd::model::Activity ac = cnd::model::Activity();
        ac.setType(cnd::model::ListTools::activityTypeFromString(t.taskActivity.type));
        ac.setPeriod(t.taskActivity.period);
        ac.setPriority(t.taskActivity.period);
        ac.setRealTime(t.taskActivity.realTime);
        cndTask.setActivity(ac);

        net.addTask(cndTask);

        // Create or find deployment for the task
        std::string depUID(t.taskHost + std::to_string(t.taskPid));
        std::map<std::string,cnd::model::Deployment>::iterator it = deploymentMap.find(depUID);
        if(it == deploymentMap.end())
        {
            // When we have distributed deployments, we use the hostname,pid pair to generate a UID for deployments
            cnd::model::Deployment depl = cnd::model::Deployment(depUID);
            depl.setDeployer("orogen");
            depl.setHostID(t.taskHost);
            depl.setProcessName(t.taskDeployment);
            std::map<std::string, std::string> taskList;
            taskList[t.taskName] = t.taskCommand;
            depl.setTaskList(taskList);
            deploymentMap.insert(std::make_pair(depUID,  depl));
        }
        else
        {
            std::map<std::string, std::string> taskList = it->second.getTaskList();
            if(taskList.find(t.taskName) == taskList.end())
            {
                taskList[t.taskName] = t.taskCommand;
                it->second.setTaskList(taskList);
            }
        }
    }

    //Add deploymetns to network
    for(auto depl:deploymentMap)
    {
        net.addDeployment((const cnd::model::Deployment) depl.second);
    }

    //Add Connections
    std::vector<cnd::model::Connection> connections = generateConnections(taskData);
    for(const cnd::model::Connection& c : connections){
        net.addConnection(c);
    }

    return net;
}
}
}
