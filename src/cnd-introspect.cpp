#include <iostream>
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <rtt/transports/corba/TaskContextProxy.hpp>
#include <orocos_cpp/PluginHelper.hpp>
#include <rtt/OperationCaller.hpp>
#include "CNDMatcher.hpp"
#include <orocos_cpp/CorbaNameService.hpp>
#include "IntrospectionService.hpp"
#include <boost/filesystem.hpp>
#include <fstream>
#include <getopt.h>


using namespace orocos_cpp;

bool loadPlugin(RTT::corba::TaskContextProxy *task)
{
    std::string servicePath="rtt_introspection";
    std::string taskName = task->getName();

    RTT::OperationInterface* oi = task->operations();
    std::vector<std::string> onames = oi->getNames();
    if(std::find(onames.begin(), onames.end(), RTT::introspection::IntrospectionService::OperationName) == onames.end())
    {
        RTT::OperationCaller< bool (const std::string &)> loadPlugin(task->getOperation("loadPlugin"));
        RTT::OperationCaller< bool (const std::string &)> loadService(task->getOperation("loadService"));

        if(!loadPlugin(servicePath))
        {
            return false;
        }

        if(!loadService(RTT::introspection::IntrospectionService::ServiceName)){
            return false;
        }
        usleep(10000);
        delete task;
        task = RTT::corba::TaskContextProxy::Create(taskName);

        std::cout << "Loaded introspection plug-in on task " << taskName << std::endl;
    }else{
        //std::cout << "Operation " << RTT::introspection::IntrospectionService::OperationName << " present on task " << taskName << std::endl;
    }
    return true;
}

cnd::model::Network getCND(CorbaNameService& ns){
    //Retrive all Tasks running on NameService
    std::vector<std::string> tasks = ns.getRegisteredTasks();

    //Iterate over tasks adn request Introspection-Information
    std::vector<RTT::introspection::TaskData> tasksData;
    for(const std::string &taskName: tasks)
    {
        //Retrieve Task Context and load introspection plugin
        RTT::corba::TaskContextProxy *task = RTT::corba::TaskContextProxy::Create(taskName);
        bool st = loadPlugin(task);
        if(!st){
            std::clog << "WARNING: Failed to load introspection plugin on task " << task->getName() << " task will not be present in CND!" << std::endl;
            continue;
        }

        //Request Introspection Data and add to matcher
        RTT::OperationInterfacePart *op = task->getOperation(RTT::introspection::IntrospectionService::OperationName);
        RTT::OperationCaller<RTT::introspection::TaskData ()> fc(op);
        tasksData.push_back(fc());
        delete task;
    }

    return RTT::introspection::CNDMatcher::generateCND(tasksData);
}

CosNaming::NamingContext_var getNameService(const std::string name_service_ip, const std::string name_service_port)
{
    if(CORBA::is_nil(RTT::corba::ApplicationServer::orb))
        throw std::runtime_error("Corba is not initialized. Call Orocos.initialize first.");

    CosNaming::NamingContext_var rootContext;

    // Obtain reference to Root POA.
    CORBA::Object_var obj_poa = RTT::corba::ApplicationServer::orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj_poa);
    if(CORBA::is_nil(root_poa))
        throw std::runtime_error("Failed to narrow poa context.");

    // activate poa manager
    root_poa->the_POAManager()->activate();

    // Obtain reference to NameServiceClient
    CORBA::Object_var obj;
    if(!name_service_ip.empty())
    {
        std::string temp("corbaloc::");
        temp = temp + name_service_ip;
        if(!name_service_port.empty())
            temp = temp + ":" + name_service_port;
        temp = temp +"/NameService";
        obj = RTT::corba::ApplicationServer::orb->string_to_object(temp.c_str());
    }
    else
        obj = RTT::corba::ApplicationServer::orb->resolve_initial_references("NameService");


    try{
        rootContext = CosNaming::NamingContext::_narrow(obj.in());
    }catch(CORBA::TRANSIENT ex){
         std::cerr << "CORBA::TRANSIENT received while trying to obtain reference to NameService Client" << std::endl;
    }
    if(CORBA::is_nil(rootContext)){
        std::cerr << "Failed to narrow NameService context." << std::endl;
        return nullptr;
    }

    return rootContext;
}

bool validateNameServiceClient(std::string hostname, std::string port=""){
    return getNameService(hostname, port);
}

bool set_corba_ns_host(std::string hostname_or_ip){
    char* envi = strdup(("ORBInitRef=NameService=corbaname::"+hostname_or_ip).c_str());
    int ret = putenv(envi);
    return ret;
}

bool initializeCORBA(int argc, char**argv, std::string host="")
{
    if(!host.empty()){
        set_corba_ns_host(host);
    }
    bool orb_st = RTT::corba::ApplicationServer::InitOrb(argc, argv);
    if(!orb_st){

        std::cerr << "\nError initializing CORBA application server" <<std::endl;
        return false;
    }
    if(!validateNameServiceClient(host, "")){
        std::cerr << "\nCould not connect to name service '"<<host<<"'" <<std::endl;
        return false;
    }
    return true;
}

void usage(std::string name){
    std::cout << "Usage: " << name << " <options> CNDFILE\n\n"
              << "\tCNDFILE\t\tPath to store CND file at\n"
              << "Options:\n"
              << "\t-h, --host=HOSTNAME     CORBA host the CND Handler is registered to. Default: 'localhost'\n"
              << "\t-?,--help               Show this help message\n" <<std::endl;
}
int main(int argc, char** argv)
{
    //Command line parsing
    int c;
    std::string host = "";
    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
        {"help",        no_argument,       0, '?'},
        {"host",        required_argument, 0, 'h'},
        {0,         0,                     0,  0 }
    };

        c = getopt_long(argc, argv, "?h:0",  //<-- id's for switch statement for each line in long_options. ':' means, that an argument is used by the option
                        long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 0:
            break;
        case '?':
            usage(argv[0]);
            return(EXIT_SUCCESS);
        case 'h':
            host = optarg;
            break;
        default:
            printf("?? getopt returned character code 0%o (%s) ??\n", c, std::string(1, c).c_str());
            break;
        }
    }

    int n_nonopt_args = argc-optind;
    if(n_nonopt_args < 1){
        std::cout << "ERROR: no CND file given!\n"<<std::endl;
        usage(argv[0]);
        return(EXIT_FAILURE);
    }
    std::string cnd_filepath = argv[optind++];

    //Init CORBA
    std::cout << "Initializing CORBA" << std::endl;
    if(!initializeCORBA(argc, argv, host)){
        return(EXIT_FAILURE);
    }
    std::cout << "Loading Typekits and Transports" << std::endl;
    PluginHelper::loadAllTypekitAndTransports();

    std::cout << "Performing introspection..." << std::endl;
    CorbaNameService ns;
    ns.connect();
    cnd::model::Network cnd = getCND(ns);
    std::cout << "Introspection done." << std::endl;

    std::ofstream of(cnd_filepath);
    of << cnd.getYAMLstring() << std::endl;
    of.close();
    std::cout << "\nOutput written to " << cnd_filepath << std::endl;
    
    return 0;
}
