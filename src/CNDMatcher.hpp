#pragma once
#include "IntrospectionTypes.hpp"
#include <cnd/model/Network.hpp>

namespace RTT {
namespace introspection {

class CNDMatcher{
public:
    static cnd::model::Network generateCND(std::vector<TaskData>& taskData);
};
}

}
