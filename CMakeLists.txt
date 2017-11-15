cmake_minimum_required(VERSION 3.3)
project(msilo)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    cacheLogicDeprecated/hash.c
    cacheLogicDeprecated/hash.h
    ms_amqp.c
    ms_amqp.h
    ms_msg_list.c
    ms_msg_list.h
    msfuncs.c
    msfuncs.h
    msg_retry.c
    msg_retry.h
    msilo.c
    msilo.h)

add_executable(msilo ${SOURCE_FILES})