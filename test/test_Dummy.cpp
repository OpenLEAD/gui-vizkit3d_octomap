#include <boost/test/unit_test.hpp>
#include <vizkit_3d_octomap/Dummy.hpp>

using namespace vizkit_3d_octomap;

BOOST_AUTO_TEST_CASE(it_should_not_crash_when_welcome_is_called)
{
    vizkit_3d_octomap::DummyClass dummy;
    dummy.welcome();
}
