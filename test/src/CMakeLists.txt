# Copyright (c) 2021-2023, University of Colorado Denver. All rights reserved.
#
# This file is part of <T>LAPACK.
# <T>LAPACK is free software: you can redistribute it and/or modify it under
# the terms of the BSD 3-Clause license. See the accompanying LICENSE file

# Configurations
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../include")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Common libraries
add_library(testutils testutils.cpp)
target_link_libraries(testutils PUBLIC Catch2::Catch2WithMain)
target_link_libraries(testutils PUBLIC tlapack)

if(TLAPACK_TEST_EIGEN)
  find_package(Eigen3 REQUIRED)
  target_link_libraries(testutils PUBLIC Eigen3::Eigen)
  target_compile_definitions(testutils PUBLIC TLAPACK_TEST_EIGEN)
endif()

if(TLAPACK_TEST_MDSPAN)
  find_package(mdspan REQUIRED)
  target_link_libraries(testutils PUBLIC std::mdspan)
  target_compile_definitions(testutils PUBLIC TLAPACK_TEST_MDSPAN)
endif()

if(TLAPACK_TEST_MPFR)
  find_package(MPFR 2.3.1 REQUIRED)
  find_package(GMP 4.2.1 REQUIRED)
  target_include_directories(testutils PUBLIC ${MPFR_INCLUDES} ${GMP_INCLUDES})
  target_link_libraries(testutils PUBLIC ${MPFR_LIBRARIES} ${GMP_LIBRARIES})
  target_compile_definitions(testutils PUBLIC TLAPACK_TEST_MPFR)
endif()

if(TLAPACK_TEST_QUAD)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_definitions(testutils PUBLIC TLAPACK_TEST_QUAD)
    target_compile_options(testutils PUBLIC -fext-numeric-literals)
    target_link_libraries(testutils PUBLIC -lquadmath)
  endif()
endif()

# Use this library on all tests
link_libraries(testutils)

# Set the directory where to put testers
set(TLAPACK_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test")

# Testers
add_executable(test_lasy2 test_lasy2.cpp)
add_executable(test_schur_move test_schur_move.cpp)
add_executable(test_transpose test_transpose.cpp)
add_executable(test_unmhr test_unmhr.cpp)
add_executable(test_hessenberg test_hessenberg.cpp)
add_executable(test_optBLAS test_optBLAS.cpp)
add_executable(test_schur_swap test_schur_swap.cpp)
add_executable(test_qr_algorithm test_qr_algorithm.cpp)
add_executable(test_utils test_utils.cpp)
# add_executable(test_gelqf test_gelqf.cpp)
add_executable(test_gelqt test_gelqt.cpp)
# add_executable(test_gelq2 test_gelq2.cpp)
add_executable(test_householder_QR test_householder_QR.cpp)
# add_executable(test_gerqf test_gerqf.cpp)
# add_executable(test_geql2 test_geql2.cpp)
# add_executable(test_geqlf test_geqlf.cpp)
# add_executable(test_gerq2 test_gerq2.cpp)
add_executable(test_trtri test_trtri.cpp)
add_executable(test_bidiag test_bidiag.cpp)
add_executable(test_lu_mult test_lu_mult.cpp)
add_executable(test_getrf test_getrf.cpp)
add_executable(test_getri test_getri.cpp)
add_executable(test_ul_mult test_ul_mult.cpp)
add_executable(test_unmr2 test_unmr2.cpp)
add_executable(test_unm2r test_unm2r.cpp)
add_executable(test_unmlq test_unmlq.cpp)
add_executable(test_unmql test_unmql.cpp)
add_executable(test_unmqr test_unmqr.cpp)
add_executable(test_unmrq test_unmrq.cpp)
add_executable(test_unml2 test_unml2.cpp)
add_executable(test_unm2l test_unm2l.cpp)
add_executable(test_lauum test_lauum.cpp)
add_executable(test_potrf test_potrf.cpp)
add_executable(test_larf test_larf.cpp)
add_executable( test_rscl test_rscl.cpp )
add_executable( test_ladiv test_ladiv.cpp )
add_executable(test_concepts test_concepts.cpp)

if(TLAPACK_TEST_EIGEN)
  add_executable(test_eigenplugin test_eigenplugin.cpp)
endif()

if(TLAPACK_TEST_MDSPAN)
  add_executable(test_mdspanplugin test_mdspanplugin.cpp)
endif()

# Reset the output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${TLAPACK_RUNTIME_OUTPUT_DIRECTORY}")

# Add tests to CTest
include(Catch)

get_property(DIRECTORY_BUILDSYSTEM_TARGETS DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)

foreach(target ${DIRECTORY_BUILDSYSTEM_TARGETS})
  if(NOT target MATCHES "^test_")
    continue()
  endif()
  catch_discover_tests(${target})
endforeach()

if( BUILD_STANDALONE_TESTS)
  # Now, build tests as standalone executable without using Catch2

  foreach(target ${DIRECTORY_BUILDSYSTEM_TARGETS})
    if(NOT target MATCHES "^test_")
      continue()
    elseif(target MATCHES "test_eigenplugin")
      continue()
    elseif(target MATCHES "test_mdspanplugin")
      continue()
    elseif(target MATCHES "test_optBLAS")
      continue()
    elseif(target MATCHES "test_utils")
      continue()
    elseif(target MATCHES "test_concepts")
      continue()
    elseif(target MATCHES "test_larf")
      continue()
    elseif(target MATCHES "test_ladiv")
      continue()
    elseif(target MATCHES "test_transpose")
      continue()
    endif()
    add_executable( standalone_${target} ${target}.cpp )
    target_compile_definitions(standalone_${target} PUBLIC TLAPACK_BUILD_STANDALONE_TESTS)
  endforeach()

endif()