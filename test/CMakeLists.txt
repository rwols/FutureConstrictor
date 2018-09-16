find_package(Boost REQUIRED COMPONENTS unit_test_framework)
set(testexe Test${PROJECT_NAME})
add_executable(${testexe}
    Module.cpp
    BoundedConstrictor.cpp
    SingleConstrictor.cpp
    UnboundedConstrictor.cpp
)
set_target_properties(${testexe}
    PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
)
target_link_libraries(${testexe} PRIVATE ${PROJECT_NAME} Boost::unit_test_framework)
target_compile_definitions(${testexe} PRIVATE BOOST_TEST_DYN_LINK)

function(unit_tests)
    foreach(testname ${ARGN})
        add_test(NAME ${PROJECT_NAME}.${testname}
                 COMMAND ${testexe} --run_test=${testname}
                                    --catch_system_error=yes)
    endforeach(testname)
endfunction(unit_tests)

unit_tests(
    BoundedConstrictorSuite
    SingleConstrictorSuite
    UnboundedConstrictorSuite
)
