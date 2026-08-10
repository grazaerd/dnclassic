cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

project(dnclassic)

set(USED_CMAKE_GENERATOR "${CMAKE_GENERATOR}" CACHE STRING "Expose CMAKE_GENERATOR" FORCE)

# Create export compile commands
if(USED_CMAKE_GENERATOR MATCHES "Ninja" OR USED_CMAKE_GENERATOR MATCHES "MinGW Makefiles")
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()
# https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html
# Do not use cmake or compiler flto if doing a objdump else empty

# Disable other architectures
set(ASMJIT_NO_FOREIGN TRUE)
set(ASMJIT_NO_AARCH64 TRUE)

# -Wl,-s causes Wacatac, need to manual llvm-strip -s .dll
set(COMMON_FLAGS "-O3 -ffp-model=strict -march=x86-64-v2 -fuse-ld=lld -DNDEBUG -static -fno-delete-null-pointer-checks -fomit-frame-pointer -fdata-sections -ffunction-sections")

set(CMAKE_C_FLAGS "${COMMON_FLAGS} -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables")
set(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -fno-rtti")

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -falign-functions=16 -falign-labels=16 -falign-loops=16 -falign-jumps=16")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -falign-functions=16 -falign-labels=16 -falign-loops=16 -falign-jumps=16")
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -femulated-tls -pthread -falign-functions=16 -falign-loops=16")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -femulated-tls -pthread -falign-functions=16 -falign-loops=16")
endif()

add_library(dnclassic SHARED
            src/main.cpp
            src/dxport.cpp
            src/dxport.hpp
            src/mem.hpp
            src/proxy.cpp
            src/proxy.hpp
            src/proxy.def
            src/util.hpp
           )

set(polyhook "${CMAKE_CURRENT_SOURCE_DIR}/lib/polyhook")

add_subdirectory(${polyhook})

target_compile_options(PolyHook_2 PRIVATE -w)
target_compile_options(asmjit PRIVATE -w)
target_compile_options(asmtk PRIVATE -w)
target_compile_options(Zycore PRIVATE -w)
target_compile_options(Zydis PRIVATE -w)

target_include_directories(dnclassic SYSTEM PRIVATE ${polyhook})

target_link_libraries(dnclassic PRIVATE PolyHook_2)
target_link_libraries(dnclassic PRIVATE winmm)

set_target_properties(dnclassic PROPERTIES PREFIX "")
set_target_properties(dnclassic PROPERTIES OUTPUT_NAME "ddraw")

set(dll_path "${CMAKE_CURRENT_BINARY_DIR}/ddraw.dll")
set(zip_path "${CMAKE_CURRENT_BINARY_DIR}/dnclassic.zip")

add_custom_command(TARGET dnclassic POST_BUILD COMMAND llvm-strip -s ${dll_path})
add_custom_command(TARGET dnclassic POST_BUILD COMMAND tar -a -c -f ${zip_path} ${dll_path})
