cmake_minimum_required(VERSION 3.1)

set(project_name my-threaded-project) ## rename your project here

project(${project_name})

set(CMAKE_CXX_STANDARD 11)

add_library(${project_name}-lib
        src/AppComponent.hpp
        src/controller/MyController.cpp
        src/controller/MyController.hpp
        src/dto/DTOs.hpp
)

## link libs

find_package(oatpp 1.3.0 REQUIRED)

target_link_libraries(${project_name}-lib
        PUBLIC oatpp::oatpp
        PUBLIC oatpp::oatpp-test
)

target_include_directories(${project_name}-lib PUBLIC src)

## add executables
if(NOT DEFINED STOP_METHOD)
    set(STOP_METHOD StopSimple)
endif(NOT DEFINED STOP_METHOD)

## Example NoStop
add_executable(NoStop-exe
        src/App_NoStop.cpp
        test/app/MyApiTestClient.hpp)
target_link_libraries(NoStop-exe ${project_name}-lib)
add_dependencies(NoStop-exe ${project_name}-lib)

## Example StopSimple
add_executable(StopSimple-exe
        src/App_StopSimple.cpp
        test/app/MyApiTestClient.hpp)
target_link_libraries(StopSimple-exe ${project_name}-lib)
add_dependencies(StopSimple-exe ${project_name}-lib)

## Example StopByConditionCheck
add_executable(StopByConditionCheck-exe
        src/App_StopByConditionCheck.cpp
        test/app/MyApiTestClient.hpp)
target_link_libraries(StopByConditionCheck-exe ${project_name}-lib)
add_dependencies(StopByConditionCheck-exe ${project_name}-lib)

## Example StopWithFullEnclosure
add_executable(StopWithFullEnclosure-exe
        src/App_StopWithFullEnclosure.cpp
        test/app/MyApiTestClient.hpp)
target_link_libraries(StopWithFullEnclosure-exe ${project_name}-lib)
add_dependencies(StopWithFullEnclosure-exe ${project_name}-lib)

## Example StopByConditionWithFullEnclosure
add_executable(StopByConditionWithFullEnclosure-exe
        src/App_StopByConditionWithFullEnclosure.cpp
        test/app/MyApiTestClient.hpp)
target_link_libraries(StopByConditionWithFullEnclosure-exe ${project_name}-lib)
add_dependencies(StopByConditionWithFullEnclosure-exe ${project_name}-lib)

## Example RunAndStopInFunctions
add_executable(RunAndStopInFunctions-exe
        src/App_RunAndStopInFunctions.cpp
        test/app/MyApiTestClient.hpp)
target_link_libraries(RunAndStopInFunctions-exe ${project_name}-lib)
add_dependencies(RunAndStopInFunctions-exe ${project_name}-lib)

add_executable(${project_name}-test
        test/tests.cpp
        test/app/TestComponent.hpp
        test/app/MyApiTestClient.hpp
        test/MyControllerTest.cpp
        test/MyControllerTest.hpp
)

target_link_libraries(${project_name}-test ${project_name}-lib)
add_dependencies(${project_name}-test ${project_name}-lib)

set_target_properties(${project_name}-lib NoStop-exe StopSimple-exe StopByConditionCheck-exe StopWithFullEnclosure-exe StopByConditionWithFullEnclosure-exe RunAndStopInFunctions-exe ${project_name}-test PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

enable_testing()
add_test(project-tests ${project_name}-test)
