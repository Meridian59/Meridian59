# Windows resources (.rc) on Linux and macOS: compiled to a .res with Wine's wrc
# (built in win32/, against Wine's headers) and embedded in the binary, which
# registers it with the Win32 layer when it loads (win32/res/embed.c.in);
# FindResource and friends read it from there.

find_package(Python3 REQUIRED COMPONENTS Interpreter)

# m59_add_resources(<target> <rc-file> [INCLUDES <dir>...] [MODULE <dll>])
# MODULE: the resources of a Windows DLL the layer provides (built into the
# layer), not of the binary <target> makes.
function(m59_add_resources target rc)
  cmake_parse_arguments(ARG "" "MODULE" "INCLUDES" ${ARGN})
  get_filename_component(name "${rc}" NAME_WE)
  get_filename_component(rcdir "${rc}" DIRECTORY)
  set(dir "${CMAKE_CURRENT_BINARY_DIR}/res/${target}")
  set(prep "${dir}/${name}.rc")   # #includes resolve through -I, file names are found from here
  set(res "${dir}/${name}.res")
  set(args "")
  foreach(inc IN LISTS ARG_INCLUDES rcdir)
    list(APPEND args "-I${inc}")
  endforeach()
  add_custom_command(OUTPUT "${res}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${dir}"
    COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/win32/tools/rcprep.py" "${rc}" "${prep}"
    # US English where the script names no LANGUAGE, as Microsoft's rc.
    COMMAND m59_wrc --nostdinc "--nls-dir=${M59_WINE}/nls" -l 0x409 -I "${M59_WINE}/include" -D_WIN32 -D_WIN64
            ${args} -o "${res}" "${prep}"
    DEPENDS "${rc}" "${PROJECT_SOURCE_DIR}/win32/tools/rcprep.py" m59_wrc
    IMPLICIT_DEPENDS C "${rc}"
    COMMENT "Compiling resources ${name}.rc"
    VERBATIM)
  set(M59_RES_FILE "${res}")
  if (ARG_MODULE)
    set(M59_RES_REGISTER "M59RegisterModuleResources(\"${ARG_MODULE}\", m59_res_begin, m59_res_end)")
  else()
    set(M59_RES_REGISTER "M59RegisterResources(m59_res_begin, m59_res_end)")
  endif()
  configure_file("${PROJECT_SOURCE_DIR}/win32/res/embed.c.in" "${dir}/${name}_res.c" @ONLY)
  set_source_files_properties("${dir}/${name}_res.c" PROPERTIES OBJECT_DEPENDS "${res}" LANGUAGE C)
  target_sources(${target} PRIVATE "${dir}/${name}_res.c")
endfunction()
