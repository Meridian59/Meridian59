# libm59win32: the Win32 subset the client and its modules use, shared by all of
# them (one handle table, one message queue).  core/ is platform-independent;
# gtk/ (Linux) and appkit/ (macOS) put windows, controls and drawing on screen.

file(GLOB core_sources CONFIGURE_DEPENDS core/*.cpp core/*/*.cpp gfx/*.cpp)
if (APPLE)
  file(GLOB backend_sources CONFIGURE_DEPENDS appkit/*.mm appkit/*.cpp)
else()
  file(GLOB backend_sources CONFIGURE_DEPENDS gtk/*.cpp)
endif()

add_library(m59win32 SHARED ${core_sources} ${backend_sources})
set_target_properties(m59win32 PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${M59_RUN_DIR}" CXX_VISIBILITY_PRESET hidden)

# Wine 10.0's source release, downloaded at configure time (pinned by hash): its
# sources and headers are built as they are; what adapts them is ours, in
# wine-port/.  Offline, -DFETCHCONTENT_SOURCE_DIR_WINE=<unpacked wine-10.0> uses
# a local copy.  SOURCE_SUBDIR names no directory: Wine has no CMake build.
include(FetchContent)
FetchContent_Declare(wine
  URL https://dl.winehq.org/wine/source/10.0/wine-10.0.tar.xz
  URL_HASH SHA256=c5e0b3f5f7efafb30e9cd4d9c624b85c583171d33549d933cd3402f341ac3601
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SOURCE_SUBDIR m59-no-cmake)
FetchContent_MakeAvailable(wine)
set(WINE "${wine_SOURCE_DIR}")

# widl, from Wine's tools: the headers Wine generates from its IDL files (objidl.h,
# oaidl.h ...), which its tree doesn't hold.  Bison 3 and flex build it; macOS's
# bison is 2.3, Homebrew's is used.
find_program(M59_BISON bison PATHS /opt/homebrew/opt/bison/bin /usr/local/opt/bison/bin NO_DEFAULT_PATH)
if (M59_BISON)
  set(BISON_EXECUTABLE "${M59_BISON}" CACHE FILEPATH "bison" FORCE)
endif()
find_package(BISON 3.0 REQUIRED)
find_package(FLEX REQUIRED)
set(widl_gen "${CMAKE_CURRENT_BINARY_DIR}/widl")
file(MAKE_DIRECTORY "${widl_gen}")
file(WRITE "${widl_gen}/config.h"
  "#ifndef __WINE_CONFIG_H\n#define __WINE_CONFIG_H\n#define PACKAGE_VERSION \"10.0\"\n#define HAVE_SIGPROCMASK 1\n#endif\n")
BISON_TARGET(widl_parser "${WINE}/tools/widl/parser.y" "${widl_gen}/parser.tab.c" DEFINES_FILE "${widl_gen}/parser.tab.h")
FLEX_TARGET(widl_lexer "${WINE}/tools/widl/parser.l" "${widl_gen}/parser.yy.c")
BISON_TARGET(wpp_parser "${WINE}/tools/wrc/ppy.y" "${widl_gen}/ppy.tab.c" DEFINES_FILE "${widl_gen}/ppy.tab.h")
FLEX_TARGET(wpp_lexer "${WINE}/tools/wrc/ppl.l" "${widl_gen}/ppl.yy.c")
file(GLOB widl_sources "${WINE}/tools/widl/*.c")
add_executable(m59_widl ${widl_sources} "${WINE}/tools/wrc/wpp.c" ${BISON_widl_parser_OUTPUTS} ${FLEX_widl_lexer_OUTPUTS}
  ${BISON_wpp_parser_OUTPUTS} ${FLEX_wpp_lexer_OUTPUTS})
target_include_directories(m59_widl PRIVATE "${widl_gen}" "${WINE}/tools/widl" "${WINE}/tools/wrc" "${WINE}/tools"
  "${WINE}/include")
target_compile_definitions(m59_widl PRIVATE INCLUDEDIR="/nonexistent" BINDIR="/nonexistent" LIBDIR="/nonexistent")
target_compile_options(m59_widl PRIVATE -w "SHELL:-include ${widl_gen}/config.h")

# wrc, Wine's resource compiler, compiles the .rc files (M59Resources.cmake)
# against Wine's headers; it shares widl's preprocessor.
file(MAKE_DIRECTORY "${widl_gen}/wrc")
BISON_TARGET(wrc_parser "${WINE}/tools/wrc/parser.y" "${widl_gen}/wrc/parser.tab.c"
  DEFINES_FILE "${widl_gen}/wrc/parser.tab.h")
FLEX_TARGET(wrc_lexer "${WINE}/tools/wrc/parser.l" "${widl_gen}/wrc/parser.yy.c")
add_executable(m59_wrc "${WINE}/tools/wrc/genres.c" "${WINE}/tools/wrc/newstruc.c" "${WINE}/tools/wrc/po.c"
  "${WINE}/tools/wrc/utils.c" "${WINE}/tools/wrc/wpp.c" "${WINE}/tools/wrc/wrc.c" ${BISON_wrc_parser_OUTPUTS}
  ${FLEX_wrc_lexer_OUTPUTS} ${BISON_wpp_parser_OUTPUTS} ${FLEX_wpp_lexer_OUTPUTS})
target_include_directories(m59_wrc PRIVATE "${widl_gen}/wrc" "${widl_gen}" "${WINE}/tools/wrc" "${WINE}/tools"
  "${WINE}/include")
target_compile_definitions(m59_wrc PRIVATE __WINESRC__ INCLUDEDIR="/nonexistent" BINDIR="/nonexistent"
  DATADIR="/nonexistent")
target_compile_options(m59_wrc PRIVATE -w "SHELL:-include ${widl_gen}/config.h")
set(M59_WINE "${WINE}" CACHE INTERNAL "Wine's source, for wrc: its headers and NLS tables")

# sfnt2fon, Wine's raster font builder, makes the .fon files from the bitmap
# strikes in Wine's fonts/*.ttf (FreeType reads them).
find_package(Freetype REQUIRED)
add_executable(m59_sfnt2fon "${WINE}/tools/sfnt2fon/sfnt2fon.c")
target_include_directories(m59_sfnt2fon PRIVATE "${widl_gen}" "${WINE}/tools" "${WINE}/include")
target_compile_definitions(m59_sfnt2fon PRIVATE HAVE_FREETYPE HAVE_FT2BUILD_H)
target_compile_options(m59_sfnt2fon PRIVATE -w "SHELL:-include ${widl_gen}/config.h")
target_link_libraries(m59_sfnt2fon PRIVATE Freetype::Freetype)

set(wine_generated "${CMAKE_CURRENT_BINARY_DIR}/wine-include")
file(MAKE_DIRECTORY "${wine_generated}")
set(wine_idl_headers commoncontrols msxml oaidl objidlbase objidl ocidl oleacc oleidl propidl servprov shtypes
  uiautomationclient uiautomationcore unknwn urlmon wtypes)
set(wine_generated_headers)
foreach(idl IN LISTS wine_idl_headers)
  add_custom_command(OUTPUT "${wine_generated}/${idl}.h"
    COMMAND m59_widl -I "${WINE}/include" -D__WINESRC__ -h -o "${wine_generated}/${idl}.h" "${WINE}/include/${idl}.idl"
    DEPENDS m59_widl "${WINE}/include/${idl}.idl" VERBATIM)
  list(APPEND wine_generated_headers "${wine_generated}/${idl}.h")
endforeach()
add_custom_target(m59_wine_headers DEPENDS ${wine_generated_headers})
add_dependencies(m59win32 m59_wine_headers)

# Windows headers (Wine's), and the MSVC C runtime headers glibc and libc lack.
# Wine marks imported API hidden, for PE DLLs; here it lives in this shared
# library, so it is default visibility, which also exports the definitions.
target_compile_definitions(m59win32 PUBLIC "DECLSPEC_IMPORT=__attribute__((visibility(\"default\")))")
# native/ wraps a few of them; it comes first.
target_include_directories(m59win32 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/native"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/crt" "${CMAKE_CURRENT_SOURCE_DIR}/wine-port/include" "${wine_generated}"
  "${WINE}/include")
target_include_directories(m59win32 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
# For the Windows sources: MSVC's C runtime extras (strcpy_s, _stricmp ...), which
# MSVC declares in its own standard headers, and their file calls redirected
# through the layer.  Not for the layer itself, which makes the real calls.
target_compile_options(m59win32 INTERFACE "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/include/crt/msvcrt.h")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # Wine's rpcasync.h names a member HWND.
  target_compile_options(m59win32 PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-Wno-changes-meaning>)
endif()

# Wine's comctl32 (dlls/comctl32): Windows DLL code in C, built with 16-bit
# wchar_t as on Windows; their wide C runtime calls go to the layer's 16-bit
# versions (wine-port/wine_prelude.h).  No wcslen builtin: optimized, clang turns
# their 16-bit length loops into calls to the C library's, whose wchar_t is 32-bit.
set(comctl32_files animate button combo commctrl dpa dsa edit header imagelist listbox listview progress propsheet
  smoothscroll static string tab tooltips trackbar updown)
list(TRANSFORM comctl32_files PREPEND "${WINE}/dlls/comctl32/")
list(TRANSFORM comctl32_files APPEND ".c")
set(wine_sources ${comctl32_files} wine-port/m59_guids.c wine-port/m59_weak.c)
target_sources(m59win32 PRIVATE ${wine_sources})
set_source_files_properties(${wine_sources} PROPERTIES
  COMPILE_OPTIONS "-fshort-wchar;-fno-builtin-wcslen;-include;${CMAKE_CURRENT_SOURCE_DIR}/wine-port/wine_prelude.h;-Wno-unused-parameter"
  COMPILE_DEFINITIONS "WINE_UNICODE_NATIVE;__WINESRC__;_COMCTL32_"
  INCLUDE_DIRECTORIES "${WINE}/dlls/comctl32")
# Each DLL's DllMain under its own name, called at startup.
set_source_files_properties("${WINE}/dlls/comctl32/commctrl.c" PROPERTIES COMPILE_DEFINITIONS
  "WINE_UNICODE_NATIVE;__WINESRC__;_COMCTL32_;DllMain=COMCTL32_DllMain")
# comctl32's resources (property sheet templates, bitmaps, cursors, icons),
# under its module handle.
m59_add_resources(m59win32 "${WINE}/dlls/comctl32/comctl32.rc"
  INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/wine-port/include" "${WINE}/include" MODULE "comctl32.dll")
# user32's system icons (IDI_HAND ...), Wine's, under its module handle.
set(user32_rc "${CMAKE_CURRENT_BINARY_DIR}/res-src/user32.rc")
file(RELATIVE_PATH ICONS "${CMAKE_CURRENT_BINARY_DIR}/res-src" "${WINE}/dlls/user32/resources")
configure_file(res/user32.rc.in "${user32_rc}" @ONLY)
m59_add_resources(m59win32 "${user32_rc}" MODULE "user32.dll")

# Wine's raster fonts (Windows' MS Sans Serif ...), in fonts/ next to the layer:
# file, source, and sfnt2fon's arguments, as Wine's fonts/*.sfd give them
# (#pragma makedep font).
set(raster_fonts
  "sserife.fon|ms_sans_serif|-d 129 13,1252,5 16,1252,7 20,1252,8"
  "vgasys.fon|system|-d 128 16,1252,7"
  "smalle.fon|small_fonts|-d 128 11,1252,5"
  "coure.fon|courier|-d 128 13,1252,8"
  "vgafix.fon|fixedsys|-d 128 15,1252,8")
set(raster_font_files)
foreach(entry IN LISTS raster_fonts)
  string(REPLACE "|" ";" entry "${entry}")
  list(GET entry 0 font)
  list(GET entry 1 ttf)
  list(GET entry 2 args)
  separate_arguments(args)
  add_custom_command(OUTPUT "${M59_RUN_DIR}/fonts/${font}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${M59_RUN_DIR}/fonts"
    COMMAND m59_sfnt2fon -q -o "${M59_RUN_DIR}/fonts/${font}" "${WINE}/fonts/${ttf}.ttf" ${args}
    DEPENDS m59_sfnt2fon "${WINE}/fonts/${ttf}.ttf" VERBATIM)
  list(APPEND raster_font_files "${M59_RUN_DIR}/fonts/${font}")
endforeach()
add_custom_target(m59_raster_fonts ALL DEPENDS ${raster_font_files})
add_dependencies(m59win32 m59_raster_fonts)

# WinINet's HTTP(S).
find_package(CURL REQUIRED)
target_link_libraries(m59win32 PRIVATE CURL::libcurl)

if (APPLE)
  # Manual reference counting; OpenGL is deprecated on macOS but works.
  target_compile_options(m59win32 PRIVATE $<$<COMPILE_LANGUAGE:OBJCXX>:-fno-objc-arc -Wno-deprecated-declarations>)
  target_link_libraries(m59win32 PRIVATE "-framework AppKit" "-framework Carbon" "-framework CoreText"
                        "-framework OpenGL")
  # The executable's WinMain, which the layer's main calls (weak, as on Linux).
  target_link_options(m59win32 PRIVATE "LINKER:-U,_WinMain")
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4 epoxy fontconfig harfbuzz)
  target_link_libraries(m59win32 PRIVATE PkgConfig::GTK4)
  target_link_libraries(m59win32 PUBLIC ${CMAKE_DL_LIBS})
endif()

# Small Win32 programs that exercise the layer without the game:
# win32/tests/<name>.c, built as test_<name>.
option(M59_WIN32_TESTS "Build the Win32 layer's test programs" OFF)
if (M59_WIN32_TESTS)
  file(GLOB tests CONFIGURE_DEPENDS tests/*.c)
  foreach(test IN LISTS tests)
    get_filename_component(name "${test}" NAME_WE)
    set_source_files_properties("${test}" PROPERTIES LANGUAGE CXX)
    add_executable(test_${name} "${test}")
    target_link_libraries(test_${name} PRIVATE m59win32)
    target_include_directories(test_${name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    set_target_properties(test_${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${M59_RUN_DIR}/tests")
    # ctest runs each from its directory; the interactive ones end after 2 s.
    add_test(NAME ${name} COMMAND test_${name} WORKING_DIRECTORY "${M59_RUN_DIR}/tests")
    set_tests_properties(${name} PROPERTIES ENVIRONMENT "M59_TEST_SECONDS=2" TIMEOUT 300)
    # These type keys or read GetFocus(): alone, as another test's window would
    # take the keyboard (the focus is null while it has it).
    if (name MATCHES "^(cc_listview|core_dialog|ctl_edit)$")
      set_tests_properties(${name} PROPERTIES RUN_SERIAL TRUE)
    endif()
    # tests/<name>.rc: the program's resources.
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/${name}.rc")
      m59_add_resources(test_${name} "${CMAKE_CURRENT_SOURCE_DIR}/tests/${name}.rc")
    endif()
  endforeach()
endif()
