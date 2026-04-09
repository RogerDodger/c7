# Set C++20, mostly for std::format
set(CMAKE_CXX_STANDARD 20)
# This is needed or CMake will expect a .lib file instead of .dll for our shared libs
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Function to compile a folder into a .c
function(create_resources dir output)
    # Create empty output file
    file(WRITE ${output} "")
    # Collect input files
    file(GLOB bins ${dir}/*.*)
    # Iterate through input files
    foreach(bin ${bins})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" filename ${bin})
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" filename ${filename})
        # Read hex data from file
        file(READ ${bin} filedata HEX)
        # Convert hex data for C compatibility
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
        # Append data to output file
        file(APPEND ${output} "const unsigned char ${filename}[] = {${filedata}0x00};\nconst unsigned ${filename}_size = sizeof(${filename});\n")
    endforeach()
endfunction()


set(INCLUDE_FOLDERS
                    "GameInteractions"
                    "GameInteractions/GameProcess"

                    "GameSpecific"
                    "GameSpecific/Structures"
                    "GameSpecific/Editors"
                    "GameSpecific/EditorVisuals"
                    "GameSpecific/Extractors"
                    "GameSpecific/Importers"
                    "GameSpecific/Online"
                    "GameSpecific/MovesetConverters"
                    "GameSpecific/AnimExtractors"

                    
                    "GameSpecific/EditorVisuals/EditorVisuals_t7"
                    "GameSpecific/Editors/Editor_t7"
                    
                    "Layouts"
                    "Layouts/Submenus"
                    "Layouts/EditorVisuals"

                    "LocalStorage"
                    "ResourcesClasses"
                    "Utils"

                    "Injected"
                    "Injected/SharedMemory"
                    "Injected/MovesetLoaders"

                    "${CMAKE_SOURCE_DIR}/Sig/include"
                    "${CMAKE_SOURCE_DIR}/steamapi sdk/public/steam"
                    "${CMAKE_SOURCE_DIR}/imgui_markdown"
)

# This will be included in the .exe
set(BINARY_SOURCES
                    # Main sources
                    "main/main.cpp"
                    "main/run_gui.cpp"
                    "main/arguments_handling.cpp"

                    "GameInteractions/GameInteraction.cpp"
                    "GameInteractions/GameExtract.cpp"
                    "GameInteractions/GameImport.cpp"
                    "GameInteractions/GameSharedMem.cpp"
                    "GameInteractions/MatchReporting.cpp"
                    "GameInteractions/GameProcess/GameData.cpp"
                    "GameInteractions/GameProcess/GameProcess.cpp"  

                    "LocalStorage/LocalStorage.cpp"

                    # Main Layout
                    "Layouts/MainWindow.cpp"
                    "Layouts/NavigationMenu.cpp"
                    "Layouts/SideMenu.cpp"
                    # Layout: submenus
                    "Layouts/Submenus/Submenu_Extract.cpp"
                    "Layouts/Submenus/Submenu_Import.cpp"
                    "Layouts/Submenus/Submenu_Edition.cpp"
                    "Layouts/Submenus/Submenu_OnlinePlay.cpp"
                    "Layouts/Submenus/Submenu_PersistentPlay.cpp"
                    "Layouts/Submenus/Submenu_About.cpp"
                    "Layouts/Submenus/Submenu_Status.cpp"
                    # Layout: editor window
                    "Layouts/EditorVisuals/EditorVisuals.cpp"
                    "Layouts/EditorVisuals/EditorForm.cpp"
                    "Layouts/EditorVisuals/EditorFormList.cpp"

                    # Helpers and such
                    "Utils/Animations.cpp"
                    "Utils/Animations_T_x64.cpp"
                    "Utils/helpers.cpp"
                    "Utils/imgui_extras.cpp"
                    "Utils/ThreadedClass.cpp"
                    "Utils/Compression.cpp"

                    # Resources-accessing
                    "ResourcesClasses/Keybinds.cpp"
                    "ResourcesClasses/Settings.cpp"
                    "ResourcesClasses/Localization.cpp"
                    "ResourcesClasses/EditorLabel.cpp"
                    "ResourcesClasses/GameAddressesFile.cpp"
                    "ResourcesClasses/GameAddressesWrapper.cpp"
)

# Everything game-specific is in here
set(GAMESPECIFIC_SOURCES
                    # List of games
                    "GameSpecific/Games.cpp"
                    "GameSpecific/BaseAddresses.cpp"
                    # Base classes for our game-specific stuff
                    "GameSpecific/Extractors/Extractor.cpp"
                    "GameSpecific/Importers/Importer.cpp"
                    "GameSpecific/Editors/EditorLogic.cpp"
                    "GameSpecific/Online/Online.cpp"
                    "GameSpecific/MovesetConverters/Aliases.cpp"
                    "GameSpecific/MovesetConverters/MovesetFileConverter.cpp"
                    "GameSpecific/AnimExtractors/AnimExtractors.cpp"
                    
                    # Game-specific
                    "GameSpecific/Editors/TEditor.cpp"
                    ##T8
                    "GameSpecific/Online/Online_t8.cpp"
                    ### Editor logic
                    #"GameSpecific/Editors/Editor_t8/Editor_t8.cpp"
                    #"GameSpecific/Editors/Editor_t8/Editor_t8_Forms.cpp"
                    #"GameSpecific/Editors/Editor_t8/Editor_t8_Forms_DisplayableMovelist.cpp"
                    #"GameSpecific/Editors/Editor_t8/Editor_t8_Creations_DisplayableMovelist.cpp"
                    #"GameSpecific/Editors/Editor_t8/Editor_t8_Commands.cpp"
                    #"GameSpecific/Editors/Editor_t8/Editor_t8_Creations.cpp"
                    #"GameSpecific/Editors/Editor_t8/Editor_t8_Deletion.cpp"
                    #"GameSpecific/Editors/Editor_t8/Editor_t8_Duplication.cpp"
                    #"GameSpecific/Editors/Editor_t8/Editor_t8_ListCreations.cpp"
                    #"GameSpecific/Editors/Editor_t8/Editor_t8_LiveEdition.cpp"
                    #"GameSpecific/Editors/Editor_t8/Editor_t8_LiveEdition_Displayable.cpp"
                    #"GameSpecific/Editors/Editor_t8/Editor_t8_Utils.cpp"
                    #"GameSpecific/Editors/Editor_t8/Editor_t8_References.cpp"

                    ## T7
                    "GameSpecific/Online/Online_t7.cpp"
                    "GameSpecific/Extractors/Extractor_t7/Extractor_t7.cpp"
                    "GameSpecific/Importers/Importer_t7/Importer_t7.cpp"
                    "GameSpecific/Importers/Importer_t7/Importer_t7_Utils.cpp"
                    "GameSpecific/Importers/Importer_t7/Importer_t7_OtherGames.cpp"
                    "GameSpecific/AnimExtractors/AnimExtractor_t7.cpp"
                    ### Editor logic
                    "GameSpecific/Editors/Editor_t7/Editor_t7.cpp"
                    "GameSpecific/Editors/Editor_t7/Editor_t7_Forms.cpp"
                    "GameSpecific/Editors/Editor_t7/Editor_t7_Forms_DisplayableMovelist.cpp"
                    "GameSpecific/Editors/Editor_t7/Editor_t7_Creations_DisplayableMovelist.cpp"
                    "GameSpecific/Editors/Editor_t7/Editor_t7_Commands.cpp"
                    "GameSpecific/Editors/Editor_t7/Editor_t7_Creations.cpp"
                    "GameSpecific/Editors/Editor_t7/Editor_t7_Deletion.cpp"
                    "GameSpecific/Editors/Editor_t7/Editor_t7_Duplication.cpp"
                    "GameSpecific/Editors/Editor_t7/Editor_t7_ListCreations.cpp"
                    "GameSpecific/Editors/Editor_t7/Editor_t7_LiveEdition.cpp"
                    "GameSpecific/Editors/Editor_t7/Editor_t7_LiveEdition_Displayable.cpp"
                    "GameSpecific/Editors/Editor_t7/Editor_t7_Utils.cpp"
                    "GameSpecific/Editors/Editor_t7/Editor_t7_References.cpp"
                    ### Editor layout
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/EditorVisuals_t7.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/EditorVisuals_t7_Rendering.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Subwindows/TEditor_MoveReferences.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Subwindows/TEditor_CancelReferences.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/TEditorMove.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/TEditorVoiceclip.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/TEditorExtraproperties.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/TEditorCancels.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/TEditorGroupedCancels.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/TEditorRequirements.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/TEditorHitConditions.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/TEditorReactions.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/TEditorPushback.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/TEditorPushbackExtra.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/TEditorMoveStartProperty.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/TEditorMoveEndProperty.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/TEditorInputSequence.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/TEditorInputStruct.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/TEditorProjectile.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/TEditorThrowCamera.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/DisplayableMovelist/TEditorMovelistDisplayable.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/DisplayableMovelist/TEditorMovelistPlayable.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/DisplayableMovelist/TEditorMovelistInput.cpp"
                    "GameSpecific/EditorVisuals/EditorVisuals_t7/Structures/Subwindows/TEditorMove_Animations.cpp"

                    # TTT2
                    "GameSpecific/Extractors/Extractor_ttt2/Extractor_ttt2.cpp"
                    "GameSpecific/MovesetConverters/Converter_ttt2_to_t7.cpp"
                    "GameSpecific/MovesetConverters/Converter_ttt2_to_t7_Aliases.cpp"
                    "GameSpecific/AnimExtractors/AnimExtractor_ttt2.cpp"

                    # TREV
                    "GameSpecific/MovesetConverters/Converter_trev_to_t7_Aliases.cpp"

                    # T6
                    "GameSpecific/Extractors/Extractor_t6/Extractor_t6.cpp"
                    "GameSpecific/MovesetConverters/Converter_t6_to_t7.cpp"
                    "GameSpecific/MovesetConverters/Converter_t6_to_t7_Aliases.cpp"
                    "GameSpecific/AnimExtractors/AnimExtractor_t6.cpp"

                    # T5
                    "GameSpecific/Extractors/Extractor_t5/Extractor_t5.cpp"
                    "GameSpecific/MovesetConverters/Converter_t5_to_t7.cpp"
                    "GameSpecific/MovesetConverters/Converter_t5_to_t7_Aliases.cpp"
                    "GameSpecific/AnimExtractors/AnimExtractor_t5.cpp"

                    # T5 DR
                    "GameSpecific/Extractors/Extractor_t5/Extractor_t5dr.cpp"
                    "GameSpecific/AnimExtractors/AnimExtractor_t5dr.cpp"
                    "GameSpecific/MovesetConverters/Converter_t5dr_to_t7_Aliases.cpp"
                    "GameSpecific/MovesetConverters/Converter_t5dr_to_t7.cpp"
)                   

# This will be included in the .dll and should only be related to the injected DLL
set(MOVESETLOADER_SOURCES
    "Injected/MovesetLoader.cpp"

    "Injected/InjectionUtils.cpp"
    "Injected/steamHelper.cpp"

    #T8
    "Injected/MovesetLoaders/MovesetLoader_t8.cpp"

    #T7
    "Injected/MovesetLoaders/MovesetLoader_t7.cpp"
    "Injected/MovesetLoaders/MovesetLoader_t7_Importer.cpp"
    "Injected/MovesetLoaders/MovesetLoader_t7_Networking.cpp"

    "ResourcesClasses/GameAddressesFile.cpp"
    "ResourcesClasses/GameAddressesWrapper.cpp"
    "Utils/helpers.cpp"
    "Utils/Compression.cpp"
    
    "GameSpecific/MovesetConverters/Aliases.cpp"

    # Converters
    "GameSpecific/MovesetConverters/Converter_ttt2_to_t7.cpp"
    "GameSpecific/MovesetConverters/Converter_ttt2_to_t7_Aliases.cpp"
    "GameSpecific/Importers/Importer_t7/Importer_t7_Utils.cpp"

    "GameSpecific/MovesetConverters/Converter_trev_to_t7_Aliases.cpp"

    "GameSpecific/MovesetConverters/Converter_t6_to_t7.cpp"
    "GameSpecific/MovesetConverters/Converter_t6_to_t7_Aliases.cpp"

    "GameSpecific/MovesetConverters/Converter_t5_to_t7.cpp"
    "GameSpecific/MovesetConverters/Converter_t5_to_t7_Aliases.cpp"
)

## Setup embedded resources ##
set(EMBEDDED_RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/../EmbeddedResources.c")
create_resources("${CMAKE_CURRENT_SOURCE_DIR}/Resources" ${EMBEDDED_RESOURCES})
# Build file embedder .EXE. This file embedder applies lz4 compression and can be used on files generated at compile them
add_executable (FileEmbedder "External/embedfile.cpp" "Utils/Compression.cpp" )
target_include_directories(FileEmbedder PRIVATE ${INCLUDE_FOLDERS})

# Build injected TKTKMovesetLoader.dll
add_library(TKMovesetLoader SHARED ${MOVESETLOADER_SOURCES} ${EMBEDDED_RESOURCES})
target_include_directories(TKMovesetLoader PRIVATE ${INCLUDE_FOLDERS})
target_compile_definitions(TKMovesetLoader PUBLIC DLL_EXPORTING)
add_dependencies(TKMovesetLoader FileEmbedder)
if (${CMAKE_TKM_BUILD_TYPE} MATCHES "Debug")
    target_compile_definitions(TKMovesetLoader PUBLIC BUILD_TYPE_DEBUG)
endif()


## Don't embed TKMovesetLoader into main binary ##
set(MOVESET_LOADER_EMBEDDED "")

# Build lib containing per-game Import/Export movesets functions
add_library(GameSpecific STATIC ${GAMESPECIFIC_SOURCES} ${EMBEDDED_RESOURCES})
target_include_directories(GameSpecific PRIVATE ${INCLUDE_FOLDERS} )
target_compile_definitions(GameSpecific PUBLIC DLL_EXPORTING)
if (${CMAKE_TKM_BUILD_TYPE} MATCHES "Debug")
    target_compile_definitions(GameSpecific PUBLIC BUILD_TYPE_DEBUG)
endif()

# Build program .exe
enable_language("RC")

# --- Version single-source-of-truth ---
# Parse PROGRAM_VERSION out of Utils/constants.h and use it to generate app.rc
# from app.rc.in. constants.h is the only place anyone has to edit when
# bumping the version. The RC compiler can't include constants.h directly
# because constants.h pulls in C++ types and ImGui macros, so we extract the
# version string at configure time and substitute it into the resource file.
set(_C7_CONSTANTS_H "${CMAKE_CURRENT_SOURCE_DIR}/Utils/constants.h")

# Re-run cmake configure if constants.h changes, otherwise the generated
# app.rc would silently keep using a stale version after a bump.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_C7_CONSTANTS_H}")

file(STRINGS "${_C7_CONSTANTS_H}" _C7_VERSION_LINE
     REGEX "^[ \t]*#[ \t]*define[ \t]+PROGRAM_VERSION[ \t]+\"[0-9.]+\"")
if (NOT _C7_VERSION_LINE)
    message(FATAL_ERROR
        "Could not find PROGRAM_VERSION in ${_C7_CONSTANTS_H}. "
        "Expected a line like:  # define PROGRAM_VERSION  \"1.2.0\"")
endif()
string(REGEX MATCH "\"([0-9.]+)\"" _C7_VERSION_QUOTED "${_C7_VERSION_LINE}")
set(PROGRAM_VERSION "${CMAKE_MATCH_1}")

# Split into components and pad to exactly 4 (major,minor,patch,build).
# Anything missing becomes 0; anything beyond 4 is truncated.
string(REPLACE "." ";" _C7_VERSION_PARTS "${PROGRAM_VERSION}")
list(LENGTH _C7_VERSION_PARTS _C7_VERSION_NUM_PARTS)
while(_C7_VERSION_NUM_PARTS LESS 4)
    list(APPEND _C7_VERSION_PARTS "0")
    list(LENGTH _C7_VERSION_PARTS _C7_VERSION_NUM_PARTS)
endwhile()
list(SUBLIST _C7_VERSION_PARTS 0 4 _C7_VERSION_PARTS_4)

# Two forms used by VERSIONINFO: comma-separated for FILEVERSION/PRODUCTVERSION
# (RC syntax) and dotted for the human-readable "FileVersion"/"ProductVersion"
# string entries.
string(REPLACE ";" "," PROGRAM_VERSION_COMMAS  "${_C7_VERSION_PARTS_4}")
string(REPLACE ";" "." PROGRAM_VERSION_DOTTED4 "${_C7_VERSION_PARTS_4}")

message(STATUS "c7 version: ${PROGRAM_VERSION} -> "
               "FILEVERSION=${PROGRAM_VERSION_COMMAS} "
               "FileVersion=${PROGRAM_VERSION_DOTTED4}")

# Absolute paths to the icon files. rc.exe resolves bare relative resource
# paths against the location of the .rc file being compiled — and we now
# generate app.rc into the build dir, where the icons don't live. Using
# absolute source-tree paths sidesteps that and any future build-dir moves.
# CMake variables already use forward slashes, which rc.exe accepts.
set(C7_ICON_PATH       "${CMAKE_CURRENT_SOURCE_DIR}/tkm.ico")
set(C7_ICON_DEBUG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tkm_debug.ico")

# Generate the application manifest from c7.manifest.in (substitutes
# PROGRAM_VERSION) into the build dir, then record its absolute path so
# app.rc.in can reference it as an RT_MANIFEST resource.
set(C7_MANIFEST_PATH "${CMAKE_CURRENT_BINARY_DIR}/c7.manifest")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/c7.manifest.in"
    "${C7_MANIFEST_PATH}"
    @ONLY
)

# Generate app.rc into the build dir. @ONLY restricts substitution to @VAR@
# (not ${VAR}) so we don't accidentally interpolate anything else.
set(_C7_GENERATED_RC "${CMAKE_CURRENT_BINARY_DIR}/app.rc")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/app.rc.in"
    "${_C7_GENERATED_RC}"
    @ONLY
)

# app.rc handles both debug and release variants via #ifdef BUILD_TYPE_DEBUG.
# Pass that define to rc.exe in debug builds so it picks the debug icon and
# tags VERSIONINFO with VS_FF_DEBUG.
if (${CMAKE_TKM_BUILD_TYPE} MATCHES "Debug")
    set_source_files_properties("${_C7_GENERATED_RC}" PROPERTIES COMPILE_DEFINITIONS "BUILD_TYPE_DEBUG")
endif()
add_executable (c7 WIN32 ${BINARY_SOURCES} ${EMBEDDED_RESOURCES} ${MOVESET_LOADER_EMBEDDED} "${_C7_GENERATED_RC}")
if (${CMAKE_TKM_BUILD_TYPE} MATCHES "Debug")
    target_compile_definitions(c7 PUBLIC BUILD_TYPE_DEBUG)
endif()

# MSVC-specific tweaks for the c7.exe target:
#   1) Disable auto-generated manifest so our RT_MANIFEST resource is the
#      only manifest in the final binary (see app.rc.in comment). Without
#      /MANIFEST:NO the linker generates its own and mt.exe tries to merge
#      them, which either produces a merged-but-unexpected manifest or
#      fails the build outright depending on toolchain version.
#
#   2) Hardening flags. These set bits in the PE IMAGE_DLLCHARACTERISTICS
#      field that Defender's ML classifier reads as a "modern hardened
#      binary" signal, and provide real runtime security on top of that.
#
#      /DYNAMICBASE, /HIGHENTROPYVA, /NXCOMPAT are default-on for MSVC
#      Release x64 builds, but listing them explicitly means (a) they're
#      verifiable via `dumpbin /headers c7.exe | findstr "DLL character"`
#      and (b) they can't be accidentally unset by a future toolchain or
#      CMake preset change.
#
#      /GUARD:CF (Control Flow Guard) is the only one that's NOT default
#      and is the meaningful new addition here. It adds runtime indirect-
#      call target validation. It requires matching /guard:cf on the
#      compiler side — linker-only is a no-op. Applied only to c7.exe, not
#      the injected DLL, because the DLL uses Polyhook to do runtime code
#      patching and CFG's strict indirect-call rules can conflict with
#      that; c7.exe has no such constraint.
#
#      /CETCOMPAT (Intel CET shadow stack) is deliberately NOT enabled
#      here. It's a newer flag that marks the binary as compatible with
#      hardware shadow-stack enforcement, but it requires ALL linked
#      static libraries to also be CET-aware or they risk runtime
#      crashes from legacy `ret` patterns. Without auditing every vcpkg
#      dependency, it's not safe to enable blind. Consider adding later
#      if a user reports a need for it.
if (MSVC)
    target_link_options(c7 PRIVATE
        "/MANIFEST:NO"
        "/DYNAMICBASE"
        "/HIGHENTROPYVA"
        "/NXCOMPAT"
        "/GUARD:CF"
    )
    target_compile_options(c7 PRIVATE
        "/guard:cf"
    )
endif()

target_include_directories(c7 PRIVATE ${INCLUDE_FOLDERS})
add_dependencies(c7 GameSpecific)
add_dependencies(c7 TKMovesetLoader)
target_link_libraries(c7 PRIVATE GameSpecific)

# Copy addresses file to build folder, even if it's already embedded '#
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Resources/game_addresses.ini" "game_addresses.ini" COPYONLY)
# Copies TKMInterface folder containg extra, optional interface data (editor labels, maybe more later down the line)
add_custom_command(TARGET c7 PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Resources/TKMInterface" "${CMAKE_CURRENT_BINARY_DIR}/TKMInterface")

## Extra dependencies ##
# Link the other libraries that we use #

# OpenGL (Renderer)
find_package(glad CONFIG REQUIRED)
target_link_libraries(c7 PRIVATE glad::glad)

# GLFW (Renderer)
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(c7 PRIVATE glfw)

# ImGUI (Interface)
find_package(imgui CONFIG REQUIRED)
target_link_libraries(c7 PRIVATE imgui::imgui)
target_link_libraries(GameSpecific PRIVATE imgui::imgui) # Only used for the color-defining includes

# Compression. I tried LZMA but it was too slow even on the fastest preset, so we're using LZ4 for now
## LZ4
find_package(lz4 CONFIG REQUIRED)
target_link_libraries(c7 PRIVATE lz4::lz4)
target_link_libraries(FileEmbedder PRIVATE lz4::lz4)
target_link_libraries(TKMovesetLoader PRIVATE lz4::lz4)
## LZMA
find_package(liblzma CONFIG REQUIRED)
target_link_libraries(c7 PRIVATE liblzma::liblzma)
target_link_libraries(FileEmbedder PRIVATE liblzma::liblzma)
target_link_libraries(TKMovesetLoader PRIVATE liblzma::liblzma)

# Hook (Polyhook)
find_package(PolyHook_2 CONFIG REQUIRED)
target_link_libraries(TKMovesetLoader PRIVATE PolyHook_2::PolyHook_2)

# HTTP requests (WinHTTP) - linked to GUI app, not the injected DLL
target_link_libraries(c7 PRIVATE winhttp)