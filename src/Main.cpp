#include <iostream>

#include <orocos_cpp/Spawner.hpp>
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <orocos_cpp_base/OrocosHelpers.hpp>
#include <rtt/OperationCaller.hpp>
#include "IntrospectionTypes.hpp"
#include "ConnectionMatcher.hpp"

int main(int argc, char** argv)
{

    Spawner &spawner(Spawner::getInstace());
    
    RTT::corba::TaskContextServer::InitOrb(argc, argv);

    Deployment introTest("inspection_test");
    spawner.spawnDeployment(&introTest, false);

//     spawner.spawnTask("inspection_test::Task", "task1", false);
//     spawner.spawnTask("inspection_test::Task2", "task2", false);
    
    OrocosHelpers::loadTypekitAndTransports("rtt-types");
    OrocosHelpers::loadTypekitAndTransports("rtt_introspection_typekit");
    
    
    spawner.waitUntilAllReady(base::Time::fromSeconds(2.0));
    
    introTest.loadNeededTypekits();
    
    RTT::corba::TaskContextProxy *t1 = RTT::corba::TaskContextProxy::Create("task1");
    RTT::corba::TaskContextProxy *t2 = RTT::corba::TaskContextProxy::Create("task2");
    
    t1->getPort("output")->connectTo(t2->getPort("input2"));
    t2->getPort("output2")->connectTo(t1->getPort("input"));

    std::cout << "Connect done" << std::endl;

    t1->loadService("Introspection");
    t2->loadService("Introspection");

    std::cout << "Service loaded" << std::endl;
    
    RTT::introspection::ConnectionMatcher matcher;
    
    RTT::OperationCaller<RTT::introspection::TaskData ()> fc(t1->getOperation("getIntrospectionInformation"));

    matcher.addTaskData(fc());
    
    RTT::OperationCaller<RTT::introspection::TaskData ()> fc2(t2->getOperation("getIntrospectionInformation"));
    matcher.addTaskData(fc2());

    matcher.createGraph();
    std::cout << "Service loaded" << std::endl;
    
    matcher.printGraph();
    
    return 0;
}
