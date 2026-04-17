add_subdirectory(dummy_algorithm)

set(NAME dummy)
desbordante_add_lib(NAME OBJECT)
target_sources(${NAME} PRIVATE dummy_metric.cpp)
target_link_libraries(
    ${NAME} PRIVATE ${DESBORDANTE_PREFIX}::model::types better-enums spdlog::spdlog_header_only
)
