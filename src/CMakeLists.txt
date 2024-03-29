find_package(Rock)
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++0x" )
# set(OROCOS_TARGET "gnulinux")
rock_init(rtt_introspection 0.1)

rock_library(rtt_introspection
    SOURCES IntrospectionService.cpp
    HEADERS IntrospectionService.hpp
        IntrospectionTypes.hpp
    DEPS_PKGCONFIG
        rtt_typelib-${OROCOS_TARGET}
        orocos-rtt-${OROCOS_TARGET}
        orocos-rtt-mqueue-${OROCOS_TARGET}
        orocos-rtt-corba-${OROCOS_TARGET}
        orocos_cpp
        orocos_cpp_base
)

rock_library(rtt_introspection_matcher
    SOURCES ConnectionMatcher.cpp
    HEADERS ConnectionMatcher.hpp
    DEPS_PKGCONFIG
        orocos-rtt-${OROCOS_TARGET}
)

rock_executable(rtt_introspection_bin Main.cpp
    DEPS_PKGCONFIG
        rtt_typelib-${OROCOS_TARGET}
        orocos-rtt-${OROCOS_TARGET}
        orocos-rtt-mqueue-${OROCOS_TARGET}
        orocos-rtt-corba-${OROCOS_TARGET}
        orocos_cpp
        orocos_cpp_base
	logger-proxies
    DEPS 
        rtt_introspection
        rtt_introspection_matcher)

install(TARGETS rtt_introspection
        LIBRARY DESTINATION lib/orocos/plugins/rtt_introspection/plugins)
