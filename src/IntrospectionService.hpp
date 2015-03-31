#ifndef INTROSPECTIONSERVICE_H
#define INTROSPECTIONSERVICE_H

#include <rtt/Service.hpp>
#include "IntrospectionTypes.hpp"

namespace RTT
{
namespace introspection
{
    
class IntrospectionService : public RTT::Service
{
public:
    static const std::string ServiceName;
    IntrospectionService(RTT::TaskContext* owner = 0);
    
private:
    TaskData getIntrospectionInformation();
};

}
}

#endif // INTROSPECTIONSERVICE_H

