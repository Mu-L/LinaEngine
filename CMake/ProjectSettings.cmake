
# Language standard
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

if(MSVC)
add_definitions("/MP")
endif()

#--------------------------------------------------------------------
# Platform
#--------------------------------------------------------------------

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC LINA_PLATFORM_WINDOWS=1)
endif()
if(APPLE)
    # for MacOS X or iOS, watchOS, tvOS (since 3.10.3)
    target_compile_definitions(${PROJECT_NAME} PUBLIC LINA_PLATFORM_APPLE=1)
endif()
if(UNIX AND NOT APPLE)
    # for Linux, BSD, Solaris, Minix
    target_compile_definitions(${PROJECT_NAME} PUBLIC LINA_PLATFORM_UNIX=1)
endif()
if(NOT WIN32 AND NOT APPLE AND NOT UNIX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC LINA_PLATFORM_UNKNOWN=1)
endif()

#--------------------------------------------------------------------
# Properties
#--------------------------------------------------------------------

set_target_properties(
    ${PROJECT_NAME}
      PROPERTIES 
        CXX_STANDARD 20 
        CXX_STANDARD_REQUIRED YES 
        CXX_EXTENSIONS NO
)
   
target_compile_definitions(${PROJECT_NAME} PUBLIC LINA_BUILD=0)
target_compile_definitions(${PROJECT_NAME} PUBLIC LINA_MAJOR=1)
target_compile_definitions(${PROJECT_NAME} PUBLIC LINA_MINOR=0)
target_compile_definitions(${PROJECT_NAME} PUBLIC LINA_PATCH=0)
   
#--------------------------------------------------------------------
# Compiler
#--------------------------------------------------------------------
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   target_compile_definitions(${PROJECT_NAME} PUBLIC LINA_COMPILER_CLANG=1)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
   target_compile_definitions(${PROJECT_NAME} PUBLIC LINA_COMPILER_GNU=1)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
   target_compile_definitions(${PROJECT_NAME} PUBLIC LINA_COMPILER_MSVC=1)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
   target_compile_definitions(${PROJECT_NAME} PUBLIC LINA_COMPILER_INTEL=1)
else()
   target_compile_definitions(${PROJECT_NAME} PUBLIC LINA_COMPILER_OTHER=1)
endif()

#--------------------------------------------------------------------
# Lina
#--------------------------------------------------------------------


