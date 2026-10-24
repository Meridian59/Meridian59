# Meridian 59 client for Linux and macOS: the Windows sources, unchanged, built
# against the Win32 subset in win32/.  Windows builds with nmake (makefile).
cmake_minimum_required(VERSION 3.24)
project(Meridian59 C CXX)
if (APPLE)
  enable_language(OBJCXX)   # the Win32 layer's AppKit backend
endif()

if (WIN32)
  message(FATAL_ERROR "On Windows, build with nmake (see makefile); CMake builds Linux and macOS")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(M59Nmake)
include(M59Resources)

# Where the client runs from, as on Windows: meridian, its modules in resource/.
# Their run paths are relative ($ORIGIN, @loader_path), so the directory can be
# copied (makefile.linux copies it to run/localclient).
set(M59_RUN_DIR "${PROJECT_BINARY_DIR}/run")
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
enable_testing()

# The client includes <AL/al.h>, openal-soft's layout; macOS's own OpenAL
# framework has <OpenAL/al.h>, so frameworks come last there.  Homebrew keeps
# openal-soft and libarchive out of its prefix (keg-only): their opt/ paths.
if (APPLE)
  set(CMAKE_FIND_FRAMEWORK LAST)
  foreach(prefix /opt/homebrew /usr/local)
    foreach(pkg openal-soft libarchive)
      if (EXISTS "${prefix}/opt/${pkg}")
        list(APPEND CMAKE_PREFIX_PATH "${prefix}/opt/${pkg}")
      endif()
    endforeach()
  endforeach()
endif()
find_package(OpenAL REQUIRED)
find_package(LibArchive REQUIRED)

# zlib and libpng from external/, as the Windows build: the objects its nmake
# makefiles list.  Static, and hidden in the executable, which exports its own
# symbols (to the modules), not these (to GTK's libraries, which use the system's).
set(EXTERNAL "${PROJECT_SOURCE_DIR}/external")
m59_nmake_objects("${EXTERNAL}/zlib/makefile" zlib_objs)
m59_resolve_sources(zlib_sources "${zlib_objs}" DIRS "${EXTERNAL}/zlib")
add_library(m59_zlib STATIC ${zlib_sources})
target_include_directories(m59_zlib PUBLIC "${EXTERNAL}/zlib")
target_compile_definitions(m59_zlib PRIVATE HAVE_UNISTD_H)
m59_nmake_objects("${EXTERNAL}/libpng/makefile" png_objs)
m59_resolve_sources(png_sources "${png_objs}" DIRS "${EXTERNAL}/libpng")
add_library(m59_png STATIC ${png_sources})
target_include_directories(m59_png PUBLIC "${EXTERNAL}/libpng")
# Its ARM NEON code isn't in external/.
target_compile_definitions(m59_png PRIVATE PNG_ARM_NEON_OPT=0)
target_link_libraries(m59_png PUBLIC m59_zlib)
set_target_properties(m59_zlib m59_png PROPERTIES C_VISIBILITY_PRESET hidden POSITION_INDEPENDENT_CODE ON)
if (APPLE)
  # These versions take TARGET_OS_MAC for classic Mac OS: zlib would define
  # fdopen as NULL (unless defined), libpng include <fp.h> (unless <math.h> is in).
  target_compile_definitions(m59_zlib PRIVATE fdopen=fdopen)
  target_compile_options(m59_png PRIVATE "SHELL:-include math.h")
endif()

add_subdirectory(win32)

# --- Compile settings the Windows sources need everywhere -------------------
# The .c files are C++ (MSVC /TP); they see the Win32 layer's headers first.
add_library(m59_windows_sources INTERFACE)
target_link_libraries(m59_windows_sources INTERFACE m59win32)
target_compile_definitions(m59_windows_sources INTERFACE BLAKCLIENT STB_VORBIS_NO_PUSHDATA_API)
# What MSVC accepts: a function declared extern and defined static, function
# pointers converted to void *.
target_compile_options(m59_windows_sources INTERFACE
  $<$<CXX_COMPILER_ID:GNU>:-fpermissive> $<$<CXX_COMPILER_ID:Clang,AppleClang>:-fms-extensions>)
# FindOpenAL's directory is the one with al.h; <AL/al.h> needs the one above.
get_filename_component(openal_parent "${OPENAL_INCLUDE_DIR}" DIRECTORY)
target_include_directories(m59_windows_sources INTERFACE "${openal_parent}")
target_include_directories(m59_windows_sources INTERFACE
  "${PROJECT_SOURCE_DIR}/clientd3d" "${PROJECT_SOURCE_DIR}/include" "${EXTERNAL}/stb" "${EXTERNAL}/libpng"
  "${EXTERNAL}/zlib")

function(m59_windows_sources_are_cxx)
  set_source_files_properties(${ARGN} PROPERTIES LANGUAGE CXX)
endfunction()

# --- Client -------------------------------------------------------------------
set(CLIENT "${PROJECT_SOURCE_DIR}/clientd3d")
set(GEN "${PROJECT_BINARY_DIR}/gen")

add_executable(maketrig "${CLIENT}/maketrig.c")
add_executable(makepal "${CLIENT}/makepal.c")
foreach(tool maketrig makepal)
  m59_windows_sources_are_cxx("${CLIENT}/${tool}.c")
  target_link_libraries(${tool} PRIVATE m59_windows_sources)
endforeach()
add_custom_command(OUTPUT "${GEN}/trig.c" "${GEN}/trig.h"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${GEN}"
  COMMAND maketrig "${GEN}"
  DEPENDS maketrig VERBATIM)
add_custom_command(OUTPUT "${GEN}/pal.c"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${GEN}"
  COMMAND makepal "${PROJECT_SOURCE_DIR}/blakston.pal" "${GEN}"
  DEPENDS makepal "${PROJECT_SOURCE_DIR}/blakston.pal" VERBATIM)
add_custom_target(m59_generated DEPENDS "${GEN}/trig.c" "${GEN}/trig.h" "${GEN}/pal.c")

m59_nmake_objects("${CLIENT}/makefile" client_objs)
m59_resolve_sources(client_sources "${client_objs}"
  DIRS "${CLIENT}" "${PROJECT_SOURCE_DIR}/util" "${EXTERNAL}/stb"
  SKIP trig pal maketrig makepal)
m59_windows_sources_are_cxx(${client_sources} "${GEN}/trig.c" "${GEN}/pal.c")

add_executable(meridian ${client_sources} "${GEN}/trig.c" "${GEN}/trig.h" "${GEN}/pal.c")
target_include_directories(meridian PRIVATE "${GEN}")
target_link_libraries(meridian PRIVATE m59_windows_sources m59rscload
  OpenAL::OpenAL m59_png LibArchive::LibArchive)
# Modules call the functions client.def exports; they resolve against the executable.
set_target_properties(meridian PROPERTIES ENABLE_EXPORTS ON
  RUNTIME_OUTPUT_DIRECTORY "${M59_RUN_DIR}")
m59_add_resources(meridian "${CLIENT}/client.rc" INCLUDES "${CLIENT}" "${PROJECT_SOURCE_DIR}/include")

# rscload.lib
add_library(m59rscload STATIC "${PROJECT_SOURCE_DIR}/util/rscload.c")
m59_windows_sources_are_cxx("${PROJECT_SOURCE_DIR}/util/rscload.c")
target_link_libraries(m59rscload PRIVATE m59_windows_sources)

# --- Modules: <name>.dll, as the server names them ---------------------------
function(m59_add_module name)
  set(dir "${PROJECT_SOURCE_DIR}/module/${name}")
  m59_nmake_objects("${dir}/makefile" objs)
  m59_resolve_sources(sources "${objs}" DIRS "${dir}")
  m59_windows_sources_are_cxx(${sources})
  add_library(${name}_objects OBJECT ${sources})
  target_include_directories(${name}_objects PRIVATE "${dir}" "${GEN}")
  target_link_libraries(${name}_objects PRIVATE m59_windows_sources)
  add_dependencies(${name}_objects m59_generated)   # trig.h
  # The .def's exports by ordinal, which the client asks GetProcAddress for.
  set(exports "${CMAKE_CURRENT_BINARY_DIR}/exports/${name}_exports.c")
  add_custom_command(OUTPUT "${exports}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/exports"
    COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/win32/tools/defexports.py" "${dir}/${name}.def"
            "${CMAKE_NM}" "${exports}" "$<TARGET_OBJECTS:${name}_objects>"
    DEPENDS "${dir}/${name}.def" "${PROJECT_SOURCE_DIR}/win32/tools/defexports.py" "$<TARGET_OBJECTS:${name}_objects>"
    COMMAND_EXPAND_LISTS VERBATIM)
  add_library(${name} MODULE $<TARGET_OBJECTS:${name}_objects> "${exports}")
  target_link_libraries(${name} PRIVATE m59_windows_sources)
  set_target_properties(${name} PROPERTIES PREFIX "" SUFFIX ".dll"
    LIBRARY_OUTPUT_DIRECTORY "${M59_RUN_DIR}/resource")
  if (APPLE)
    target_link_options(${name} PRIVATE -undefined dynamic_lookup)
  else()
    # A module's own globals are its own, as a DLL's are: hInst in a module is
    # not the executable's hInst, which it would bind to otherwise.
    target_link_options(${name} PRIVATE "LINKER:-Bsymbolic")
  endif()
  m59_add_resources(${name} "${dir}/${name}.rc" INCLUDES "${dir}" "${CLIENT}" "${PROJECT_SOURCE_DIR}/include")
endfunction()

foreach(module admin char mailnews merintr dm chess)
  m59_add_module(${module})
endforeach()
