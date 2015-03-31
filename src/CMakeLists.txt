rock_library(rtt_introspection
    SOURCES IntrospectionService.cpp
    HEADERS IntrospectionService.hpp
        IntrospectionTypes.hpp
    DEPS_PKGCONFIG
        rtt_typelib-${OROCOS_TARGET}
        orocos-rtt-${OROCOS_TARGET}
        orocos-rtt-mqueue-${OROCOS_TARGET}
        orocos-rtt-corba-${OROCOS_TARGET}
        orocos_cpp_base
)

rock_library(matcher
    SOURCES ConnectionMatcher.cpp
    HEADERS ConnectionMatcher.hpp
    DEPS_PKGCONFIG
        
)

rock_executable(rtt_introspection_bin Main.cpp
    DEPS_PKGCONFIG
        orocos_cpp
    DEPS 
        rtt_introspection
        matcher)

