# taken from https://github.com/nvpro-samples/nvpro_core/blob/master/cmake/find/FindOptix7.cmake, adjusted

# Try to find OptiX project dll/so and headers
#

# outputs
unset(OPTIX_DLL CACHE)
unset(OPTIX_LIB CACHE)
unset(OPTIX_FOUND CACHE)
unset(OPTIX_INCLUDE_DIR CACHE)

# OPTIX_LOCATION can be setup to search versions somewhere else

macro(folder_list result curdir substring)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*${substring}*)
    SET(dirlist "")
    foreach (child ${children})
        IF (IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        ENDIF ()
    ENDFOREACH ()
    SET(${result} ${dirlist})
ENDMACRO()

macro(_find_version_path targetVersion targetPath rootName searchList)
    unset(targetVersion)
    unset(targetPath)
    SET(bestver "0.0.0")
    SET(bestpath "")
    foreach (basedir ${searchList})
        folder_list(dirList ${basedir} ${rootName})
        foreach (checkdir ${dirList})
            string(REGEX MATCH "${rootName}(.*)([0-9]+)\\.([0-9]+)\\.([0-9]+)(.*)$" result "${checkdir}")
            if ("${result}" STREQUAL "${checkdir}")
                # found a path with versioning
                SET(ver "${CMAKE_MATCH_2}.${CMAKE_MATCH_3}.${CMAKE_MATCH_4}")
                if (ver VERSION_GREATER bestver)
                    SET(bestver ${ver})
                    SET(bestmajorver ${CMAKE_MATCH_2})
                    SET(bestminorver ${CMAKE_MATCH_3})
                    SET(bestpath "${basedir}/${checkdir}")
                endif ()
            endif ()
        endforeach ()
    endforeach ()
    if ("${bestver}" STREQUAL "0.0.0")
        foreach (basedir ${searchList})
            if (EXISTS "${basedir}/optix/include/optix.h")
                SET(bestver "unknown")
                SET(bestpath "${basedir}/optix")
            endif ()
        endforeach ()
    endif ()
    SET(${targetVersion} "${bestver}")
    SET(${targetPath} "${bestpath}")
endmacro()

macro(_find_files targetVar incDir dllName dllName64 folder)
    unset(fileList)
    if (ARCH STREQUAL "x86")
        file(GLOB fileList "${${incDir}}/../${folder}${dllName}")
        list(LENGTH fileList NUMLIST)
        if (NUMLIST EQUAL 0)
            file(GLOB fileList "${${incDir}}/${folder}${dllName}")
        endif ()
    else ()
        file(GLOB fileList "${${incDir}}/../${folder}${dllName64}")
        list(LENGTH fileList NUMLIST)
        if (NUMLIST EQUAL 0)
            file(GLOB fileList "${${incDir}}/${folder}${dllName64}")
        endif ()
    endif ()
    list(LENGTH fileList NUMLIST)
    if (NUMLIST EQUAL 0)
        message(STATUS "MISSING: unable to find ${targetVar} files (${folder}${dllName}, ${folder}${dllName64})")
        set(${targetVar} "NOTFOUND")
    endif ()
    list(APPEND ${targetVar} ${fileList})

    # message ( "File list: ${${targetVar}}" )		#-- debugging
endmacro()

if (DEFINED OPTIX_LOCATION OR DEFINED ENV{OPTIX_LOCATION})
    Message(STATUS "OptiX: Using OPTIX_LOCATION (${OPTIX_LOCATION})...")
    if (NOT DEFINED OPTIX_LOCATION)
        if (DEFINED ENV{OPTIX_LOCATION})
            set(OPTIX_LOCATION $ENV{OPTIX_LOCATION})
        endif ()
    endif ()
    # Locate by version failed. Handle user override for OPTIX_LOCATION.
    string(REGEX MATCH ".*([0-9]+).([0-9]+).([0-9]+)(.*)$" result "${OPTIX_LOCATION}")
    if ("${result}" STREQUAL "${OPTIX_LOCATION}")
        SET(bestver "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
        SET(bestmajorver ${CMAKE_MATCH_1})
        SET(bestminorver ${CMAKE_MATCH_2})
        Message(STATUS "OptiX: Version ${bestver}")
        SET(OPTIX_VERSION "${bestver}")
    else ()
        Message(WARNING "Could NOT extract the version from OPTIX folder : ${result}")
    endif ()
    find_path(OPTIX_INCLUDE_DIR optix.h ${OPTIX_LOCATION}/include)
    if (OPTIX_INCLUDE_DIR)
        set(OPTIX_ROOT_DIR ${OPTIX_INCLUDE_DIR}/../)
    endif ()
endif ()
if (NOT DEFINED OPTIX_ROOT_DIR)
    # Locate OptiX by version
    set(SEARCH_PATHS
            $ENV{OPTIX_LOCATION}
            ${OPTIX_LOCATION}
            ${PROJECT_SOURCE_DIR}/../LocalPackages/Optix
            ${PROJECT_SOURCE_DIR}/../../LocalPackages/Optix
            ${PROJECT_SOURCE_DIR}/../../../LocalPackages/Optix
            C:/ProgramData/NVIDIA\ Corporation
            /opt
    )

    _find_version_path(OPTIX_VERSION OPTIX_ROOT_DIR "OptiX" "${SEARCH_PATHS}")

    message(STATUS "OptiX version: ${OPTIX_VERSION}")
endif ()

if (OPTIX_ROOT_DIR)
    #-------- Locate HEADERS
    _find_files(OPTIX_HEADERS OPTIX_ROOT_DIR "optix.h" "optix.h" "include/")

    include(FindPackageHandleStandardArgs)

    SET(OPTIX_INCLUDE_DIR "${OPTIX_ROOT_DIR}/include" CACHE PATH "path")
    add_definitions("-DOPTIX_PATH=R\"(${OPTIX_ROOT_DIR})\"")
#    add_definitions("-DOPTIX_VERSION=\"${OPTIX_VERSION}\"")

else (OPTIX_ROOT_DIR)

    message(WARNING "
      OPTIX not found. 
      The OPTIX folder you would specify with OPTIX_LOCATION should contain:
      - lib[64] folder: containing the Optix[64_]*.dll or *.so
      - include folder: containing the include files"
    )
endif (OPTIX_ROOT_DIR)

find_package_handle_standard_args(Optix DEFAULT_MSG OPTIX_ROOT_DIR)
mark_as_advanced(OPTIX_FOUND)