#include <iostream>

#include <orocos_cpp/Spawner.hpp>
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <orocos_cpp_base/OrocosHelpers.hpp>
#include <rtt/OperationCaller.hpp>
#include "IntrospectionTypes.hpp"
#include "ConnectionMatcher.hpp"
#include <orocos_cpp/CorbaNameService.hpp>
#include "IntrospectionService.hpp"

int main(int argc, char** argv)
{

    Spawner &spawner(Spawner::getInstace());
    
    RTT::corba::TaskContextServer::InitOrb(argc, argv);

    Deployment introTest("inspection_test");
//     spawner.spawnDeployment(&introTest, false);

    spawner.spawnTask("inspection_test::Task", "task1", false);
    spawner.spawnTask("inspection_test::Task2", "task2", false);
    
    OrocosHelpers::loadTypekitAndTransports("rtt-types");
    OrocosHelpers::loadTypekitAndTransports("rtt_introspection");
    
    
    spawner.waitUntilAllReady(base::Time::fromSeconds(10.0));
    
//     introTest.loadNeededTypekits();
    
    RTT::corba::TaskContextProxy *t1 = RTT::corba::TaskContextProxy::Create("task1", false);
    RTT::corba::TaskContextProxy *t2 = RTT::corba::TaskContextProxy::Create("task2", false);
    
    t1->getPort("output")->connectTo(t2->getPort("input2"), RTT::ConnPolicy::buffer(50));
    t2->getPort("output2")->connectTo(t1->getPort("input"));

    t1->configure();
    t2->configure();
    
    t1->start();
    t2->start();
    
    delete t1;
    delete t2;
    
    std::cout << "Connect done" << std::endl;

    CorbaNameService ns;
    
    ns.connect();
    
    auto tasks = ns.getRegisteredTasks();
    
    RTT::introspection::ConnectionMatcher matcher;
    std::string str;
    std::getline(std::cin, str);
    
    for(const std::string &taskName: tasks)
    {
        std::cout << "TaSK is " << taskName << std::endl;
        RTT::corba::TaskContextProxy *task = RTT::corba::TaskContextProxy::Create(taskName, false);
        
        RTT::OperationCaller< bool (const std::string &)> loadPlugin(task->getOperation("loadPlugin"));
        RTT::OperationCaller< bool (const std::string &)> loadService(task->getOperation("loadService"));
        
        loadPlugin("/home/scotch/spacebot/install/lib/orocos/plugins/librtt_introspection.so");
        
        loadService(RTT::introspection::IntrospectionService::ServiceName);
        
//         if(!task->getOperation(RTT::introspection::IntrospectionService::OperationName))
//         {
//             task->loadPlugin();
//         }
        delete task;
    }    

    usleep(100000);
    
    for(const std::string &taskName: tasks)
    {
        std::cout << "TaSK is " << taskName << std::endl;
        RTT::corba::TaskContextProxy *task = RTT::corba::TaskContextProxy::Create(taskName, false);
        while(!task->getOperation(RTT::introspection::IntrospectionService::OperationName))
        {
            usleep(10000);
            
            delete task;
            task = RTT::corba::TaskContextProxy::Create(taskName, false);
        }

        RTT::OperationCaller<RTT::introspection::TaskData ()> fc(task->getOperation(RTT::introspection::IntrospectionService::OperationName));
        matcher.addTaskData(fc());
        
        delete task;
    }    

    matcher.createGraph();
    std::cout << "Service loaded" << std::endl;
    
    matcher.printGraph();
    
    matcher.writeGraphToDotFile("test.dot");
    
    spawner.killAll();
    
    return 0;
}
