rock_library(rtt_introspection
    SOURCES Dummy.cpp
    HEADERS Dummy.hpp)

rock_executable(rtt_introspection_bin Main.cpp
    DEPS rtt_introspection)

