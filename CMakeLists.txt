cmake_minimum_required(VERSION 3.15)
project(GutPuncher VERSION 2.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compiler flags
if(MSVC)
    # MSVC flags
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2")
else()
    # GCC/MinGW flags
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -s")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -DDEBUG")
endif()

# Static linking for portability (MinGW/GCC)
if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -static-libgcc")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc")
endif()

# Main executable
add_executable(gutpuncher gutpuncher.c)

# Link Windows libraries
target_link_libraries(gutpuncher
    psapi
    shlwapi
    advapi32
    kernel32
    user32
    shell32
)

# Set subsystem to console
if(WIN32)
    set_target_properties(gutpuncher PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# Install target
install(TARGETS gutpuncher
    RUNTIME DESTINATION bin
)

# Custom target for stripping symbols (release builds)
if(CMAKE_BUILD_TYPE MATCHES Release)
    add_custom_command(TARGET gutpuncher POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Stripping symbols for smaller size..."
        COMMAND ${CMAKE_STRIP} $<TARGET_FILE:gutpuncher>
        COMMENT "Stripping debug symbols from gutpuncher.exe"
        VERBATIM
    )
endif()

# Print build info
message(STATUS "===========================================")
message(STATUS "  GutPuncher - Build Configuration")
message(STATUS "===========================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "C Standard: C${CMAKE_C_STANDARD}")
message(STATUS "Generator: ${CMAKE_GENERATOR}")
if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    message(STATUS "Linking: Static (portable)")
else()
    message(STATUS "Linking: Dynamic")
endif()
message(STATUS "===========================================")
