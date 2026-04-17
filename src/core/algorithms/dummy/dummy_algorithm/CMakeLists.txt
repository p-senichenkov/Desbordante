set(NAME dummy.dummy_algorithm)
desbordante_add_lib(NAME OBJECT)
target_sources(${NAME} PRIVATE dummy_algorithm.cpp)
target_link_libraries(
    ${NAME} PRIVATE ${DESBORDANTE_PREFIX}::dummy spdlog::spdlog_header_only Boost::headers
                    better-enums
)
