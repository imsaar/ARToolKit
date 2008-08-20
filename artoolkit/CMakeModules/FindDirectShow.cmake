# - Test for DirectShow on Windows.
# Once loaded this will define
#   DIRECTSHOW_FOUND        - system has DirectShow
#   DIRECTSHOW_INCLUDE_DIRS - include directory for DirectShow
#   DIRECTSHOW_LIBRARIES    - libraries you need to link to

SET(DIRECTSHOW_FOUND "NO")

# DirectShow is only available on Windows platforms
IF(MSVC)

  set(MS_PLATFORMSDK_ROOT "")
  if (MSVC71)  
	# 
	# On Visual Studio 2003 .NET (VS7.1) use the internal platform SDK
	#
	set(MS_PLATFORMSDK_ROOT "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\7.1\\Setup\\VC;ProductDir]/PlatformSDK")
  else(MSVC71)  
	#
	# Otherwise use the Windows SDK - remember you need to patch the qedit.h header!
	#   
	set(MS_PLATFORMSDK_ROOT "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows;CurrentInstallFolder]")	
	#
	# emit a message 
	#
	message(STATUS "Remember to patch (remove dxtrans.h) the qedit.h header in the Windows SDK (${MS_PLATFORMSDK_ROOT})")
  endif(MSVC71)
  
  # Find DirectX Include Directory (dshow depends on it)
  FIND_PATH(DIRECTX_INCLUDE_DIR ddraw.h
    # Visual Studio product based PlatformSDK/Windows SDK
	${MS_PLATFORMSDK_ROOT}/Include
    # Newer DirectX: dshow not included; requires Platform SDK
    "$ENV{DXSDK_DIR}/Include"
    # Older DirectX: dshow included
    "C:/DXSDK/Include"
    DOC "What is the path where the file ddraw.h can be found"
    NO_DEFAULT_PATH
  )

  # if DirectX found, then find DirectShow include directory
  IF(DIRECTX_INCLUDE_DIR)
    FIND_PATH(DIRECTSHOW_INCLUDE_DIR dshow.h
      "${DIRECTX_INCLUDE_DIR}"
      "$ENV{ProgramFiles}/Microsoft Platform SDK for Windows Server 2003 R2/Include"
      "$ENV{ProgramFiles}/Microsoft Platform SDK/Include"
      DOC "What is the path where the file dshow.h can be found"
      NO_DEFAULT_PATH
    )

    # if DirectShow include dir found, then find DirectShow libraries
    IF(DIRECTSHOW_INCLUDE_DIR)
      IF(CMAKE_CL_64)
        FIND_LIBRARY(DIRECTSHOW_STRMIIDS_LIBRARY strmiids
          "${DIRECTSHOW_INCLUDE_DIR}/../Lib/x64"
          DOC "Where can the DirectShow strmiids library be found"
          NO_DEFAULT_PATH
          )
        FIND_LIBRARY(DIRECTSHOW_QUARTZ_LIBRARY quartz
          "${DIRECTSHOW_INCLUDE_DIR}/../Lib/x64"
          DOC "Where can the DirectShow quartz library be found"
          NO_DEFAULT_PATH
          )
      ELSE(CMAKE_CL_64)
        FIND_LIBRARY(DIRECTSHOW_STRMIIDS_LIBRARY strmiids
          "${DIRECTSHOW_INCLUDE_DIR}/../Lib"
          "${DIRECTSHOW_INCLUDE_DIR}/../Lib/x86"
          DOC "Where can the DirectShow strmiids library be found"
          NO_DEFAULT_PATH
          )
        FIND_LIBRARY(DIRECTSHOW_QUARTZ_LIBRARY quartz
          "${DIRECTSHOW_INCLUDE_DIR}/../Lib"
          "${DIRECTSHOW_INCLUDE_DIR}/../Lib/x86"
          DOC "Where can the DirectShow quartz library be found"
          NO_DEFAULT_PATH
          )
      ENDIF(CMAKE_CL_64)
    ENDIF(DIRECTSHOW_INCLUDE_DIR)
  ENDIF(DIRECTX_INCLUDE_DIR)
ENDIF(MSVC)

#---------------------------------------------------------------------
SET(DIRECTSHOW_INCLUDE_DIRS
  "${DIRECTX_INCLUDE_DIR}"
  "${DIRECTSHOW_INCLUDE_DIR}"
  )

SET(DIRECTSHOW_LIBRARIES
  "${DIRECTSHOW_STRMIIDS_LIBRARY}"
  "${DIRECTSHOW_QUARTZ_LIBRARY}"
  )

#---------------------------------------------------------------------
INCLUDE (CheckCXXSourceCompiles)

SET(CMAKE_REQUIRED_INCLUDES  ${DIRECTSHOW_INCLUDE_DIRS})
SET(CMAKE_REQUIRED_LIBRARIES ${DIRECTSHOW_LIBRARIES})
CHECK_CXX_SOURCE_COMPILES("


  #include <atlbase.h>
  #include <dshow.h>
  
  // dxtrans.h is missing in latest SDKs 
  // please comment out dxtrans.h in qedit h
  // and use the following block before including qedit.h in
  // your own source code
  #define __IDxtCompositor_INTERFACE_DEFINED__
  #define __IDxtAlphaSetter_INTERFACE_DEFINED__
  #define __IDxtJpeg_INTERFACE_DEFINED__
  #define __IDxtKey_INTERFACE_DEFINED__
  
  #include <qedit.h>

  int main()
  {
    CComPtr<IFilterGraph2> filter_graph;
    filter_graph.CoCreateInstance(CLSID_FilterGraph);
    return 0;
  }
" DIRECTSHOW_SOURCE_COMPILES)
SET(CMAKE_REQUIRED_INCLUDES)
SET(CMAKE_REQUIRED_LIBRARIES)

#---------------------------------------------------------------------
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  DIRECTSHOW
  DEFAULT_MSG
  DIRECTX_INCLUDE_DIR
  DIRECTSHOW_INCLUDE_DIR
  DIRECTSHOW_STRMIIDS_LIBRARY
  DIRECTSHOW_QUARTZ_LIBRARY
  DIRECTSHOW_SOURCE_COMPILES
  )
