set(NAME ${DESBORDANTE_PREFIX}.memory_bench)
add_executable(${NAME})
target_sources(${NAME} PRIVATE main.cpp)
target_include_directories(${NAME} PRIVATE ${DESBORDANTE_COMMON_INCLUDE_DIRS})
target_link_libraries(
    ${NAME}
    PRIVATE ${DESBORDANTE_PREFIX}::pac::verifier::domain_pac_verifier
            ${DESBORDANTE_PREFIX}::pac::verifier::fd_pac_verifier
            ${DESBORDANTE_PREFIX}::pac::verifier::ucc_pac_verifier
            ${DESBORDANTE_PREFIX}.compile_feats
            ${DESBORDANTE_PREFIX}::testlib::common
)
