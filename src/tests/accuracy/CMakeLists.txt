set(NAME ${DESBORDANTE_PREFIX}.accuracy)
add_executable(${NAME})
target_sources(${NAME} PRIVATE main.cpp)
target_include_directories(${NAME} PRIVATE ${DESBORDANTE_COMMON_INCLUDE_DIRS})
target_link_libraries(
    ${NAME}
    PRIVATE
            ${DESBORDANTE_PREFIX}::pac::verifier::ucc_pac_verifier
            ${DESBORDANTE_PREFIX}::pac::verifier::dim_ucc_pac_verifier
            ${DESBORDANTE_PREFIX}.compile_feats
            ${DESBORDANTE_PREFIX}::testlib::common
            Boost::program_options
            magic_enum::magic_enum
            spdlog::spdlog_header_only
)
add_dependencies(${NAME} Desbordante.zip.datasets)
