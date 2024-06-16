function(_SET_CONANOPT OUT_VAR OPT_NAME OPT_VALUE)
    if (${OPT_VALUE})
        set(PY_OPT_VALUE "True")
    else ()
        set(PY_OPT_VALUE "False")
    endif ()
    set(${OUT_VAR} "${${OUT_VAR}};${OPT_NAME}=${PY_OPT_VALUE}" PARENT_SCOPE)
endfunction()

function(_SET_CONANSETTING OUT_VAR SET_NAME SET_VALUE)
    set(${OUT_VAR} "${${OUT_VAR}};${SET_NAME}=${SET_VALUE}" PARENT_SCOPE)
endfunction()

_SET_CONANOPT(CONAN_OPTS "tests" BUILD_TESTING)
_SET_CONANOPT(CONAN_OPTS "emulator" BUILD_EMULATOR)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set (CMAKE_CXX_STANDARD 20)
endif ()

_SET_CONANSETTING(CONAN_SETTINGS "compiler.cppstd" ${CMAKE_CXX_STANDARD})

# This is neccessary to ensure RC files compile with our custom toolchain
# Otherwise dirty builds will break. This code basically patches a cmake bug.
if (CMAKE_GENERATOR MATCHES "Ninja")
    # Compiling RC files needs to set this. The Ninja generator calls cmcldeps.exe to extract dependencies for RC
    # files. But it configurates DepType to "gcc" so cmake cannot filter show includes prefix and the dependencies
    # are stored in .ninja_deps with the prefix and cause next run of ninja to fail. The DepType cannot be forced
    # to "msvc" otherwise the dependency changes won't trigger recompilation of RC files.
    # The workaround is to set CMAKE_CL_SHOWINCLUDES_PREFIX to the exact English string "Note: including file: ".
    # function CMAKE_DETERMINE_MSVC_SHOWINCLUDES_PREFIX may be used if any locale issue occurs.
    set(CMAKE_CL_SHOWINCLUDES_PREFIX "Note: including file: " CACHE INTERNAL "AMD WORKAROUND")
endif()
