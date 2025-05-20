
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/../include
    ${PROJECT_SOURCE_DIR}/include
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    PyInteract.cpp
    PyResource.cpp
    TypeConversion.cpp
)
