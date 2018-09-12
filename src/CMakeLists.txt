INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
link_directories(./)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
aux_source_directory(./ DIR_SRCS)
ADD_EXECUTABLE(CloudDE ${DIR_SRCS})

target_link_libraries(CloudDE mpi EA_CPU DE_CPU GA_CPU PSO_CPU CEC2014)

