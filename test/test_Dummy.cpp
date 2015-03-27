#include <boost/test/unit_test.hpp>
#include <rtt_introspection/Dummy.hpp>

using namespace rtt_introspection;

BOOST_AUTO_TEST_CASE(it_should_not_crash_when_welcome_is_called)
{
    rtt_introspection::DummyClass dummy;
    dummy.welcome();
}
