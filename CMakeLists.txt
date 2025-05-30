cmake_minimum_required(VERSION 2.8)
project(Phase4)

# set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-std=c++11")
# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_CXX_FLAGS "-std=c++11 -g -fno-omit-frame-pointer")

# Here, create an executable file named 'simulator'

add_executable(simulator main.cpp circuit.cpp parser.cpp levelizer.cpp logicsim.cpp rtpg.cpp rfl.cpp pfs.cpp dfs.cpp tpfc.cpp scoap.cpp atpg.cpp dalgo.cpp podem.cpp tpg.cpp dtpfc.cpp)


# mkdir build && cd build
# cmake ..
# make
# cd ..

# ./build/simulator
