#----------------------------------------------------------------
#      Define the function ADD_TARGET_PHIS_BENCHMARKS(target_name)
#----------------------------------------------------------------
function(ADD_TARGET_PHIS_BENCHMARKS target_name)
        
        message(STATUS "-----------")

        #+++++++++++++++++++++++++
        # CUDA TARGETS           |
        #+++++++++++++++++++++++++
        if(BUILD_CUDA_TARGETS)
                message(STATUS "Adding target ${target_name} to CUDA backend. Executable file name: ${target_name}_cuda")
                 
                 cuda_add_executable("${target_name}_cuda"
                        #EXCLUDE_FROM_ALL 
                        "${CMAKE_CURRENT_SOURCE_DIR}/src/phi_s/benchmarks/${target_name}.cu"    
                        OPTIONS -Xcompiler -DHYDRA_DEVICE_SYSTEM=CUDA -DHYDRA_HOST_SYSTEM=CPP)
                    
                 target_link_libraries("${target_name}_cuda" ${ROOT_LIBRARIES} ${LIBCONFIG_CPP_LIBRARIES} ${GSL_LIBRARIES})

        endif(BUILD_CUDA_TARGETS)
    
        #+++++++++++++++++++++++++
        # TBB TARGETS            |
        #+++++++++++++++++++++++++
        if(BUILD_TBB_TARGETS)
                 message(STATUS "Adding target ${target_name} to TBB backend. Executable file name: ${target_name}_tbb")
                 
                 add_executable("${target_name}_tbb"
                        # EXCLUDE_FROM_ALL
                        "${CMAKE_CURRENT_SOURCE_DIR}/src/phi_s/benchmarks/${target_name}.cpp")
                    
                 set_target_properties("${target_name}_tbb" 
                        PROPERTIES COMPILE_FLAGS "-DHYDRA_HOST_SYSTEM=CPP -DHYDRA_DEVICE_SYSTEM=TBB")
                    
                 target_link_libraries("${target_name}_tbb" ${ROOT_LIBRARIES} ${TBB_LIBRARIES} ${LIBCONFIG_CPP_LIBRARIES} ${GSL_LIBRARIES})

        endif(BUILD_TBB_TARGETS)
         
        #+++++++++++++++++++++++++
        # CPP TARGETS            |
        #+++++++++++++++++++++++++
        if(BUILD_CPP_TARGETS)
                 message(STATUS "Adding target ${target_name} to CPP backend. Executable file name: ${target_name}_cpp")
                 
                 add_executable("${target_name}_cpp"
                        # EXCLUDE_FROM_ALL 
                        "${CMAKE_CURRENT_SOURCE_DIR}/src/phi_s/benchmarks/${target_name}.cpp")
                    
                 set_target_properties( "${target_name}_cpp" 
                        PROPERTIES COMPILE_FLAGS "-DHYDRA_HOST_SYSTEM=CPP -DHYDRA_DEVICE_SYSTEM=CPP")
                    
                 target_link_libraries("${target_name}_cpp" ${ROOT_LIBRARIES} ${TBB_LIBRARIES} ${LIBCONFIG_CPP_LIBRARIES} ${GSL_LIBRARIES})

         endif(BUILD_CPP_TARGETS)
         
          
        #+++++++++++++++++++++++++
        # OMP TARGETS            |
        #+++++++++++++++++++++++++
        if(BUILD_OMP_TARGETS)
                 message(STATUS "Adding target ${target_name} to OMP backend. Executable file name: ${target_name}_omp")
                 
                 add_executable("${target_name}_omp" 
                        #EXCLUDE_FROM_ALL
                        "${CMAKE_CURRENT_SOURCE_DIR}/src/phi_s/benchmarks/${target_name}.cpp" )
                    
                 set_target_properties( "${target_name}_omp" 
                        PROPERTIES COMPILE_FLAGS "-DHYDRA_HOST_SYSTEM=CPP -DHYDRA_DEVICE_SYSTEM=OMP ${OpenMP_CXX_FLAGS}")
                    
                 target_link_libraries("${target_name}_omp" ${ROOT_LIBRARIES} ${OpenMP_CXX_LIBRARIES} ${LIBCONFIG_CPP_LIBRARIES} ${GSL_LIBRARIES})

         endif(BUILD_OMP_TARGETS)
         
endfunction(ADD_TARGET_PHIS_BENCHMARKS)  


#----------------------------------------------------------
#      Define the function ADD_TARGET_PHIS_FIT(target_name)
#----------------------------------------------------------
function(ADD_TARGET_PHIS_FIT target_name)
        
        message(STATUS "-----------")

        #+++++++++++++++++++++++++
        # CUDA TARGETS           |
        #+++++++++++++++++++++++++
        if(BUILD_CUDA_TARGETS)
                message(STATUS "Adding target ${target_name} to CUDA backend. Executable file name: ${target_name}_cuda")
                 
                 cuda_add_executable("${target_name}_cuda"
                        #EXCLUDE_FROM_ALL 
                        "${CMAKE_CURRENT_SOURCE_DIR}/src/phi_s/fit/${target_name}.cu"
                        OPTIONS -Xcompiler -DHYDRA_DEVICE_SYSTEM=CUDA -DHYDRA_HOST_SYSTEM=CPP)
                    
                 target_link_libraries("${target_name}_cuda" ${ROOT_LIBRARIES} ${LIBCONFIG_CPP_LIBRARIES} ${GSL_LIBRARIES})

        endif(BUILD_CUDA_TARGETS)

        #+++++++++++++++++++++++++
        # TBB TARGETS            |
        #+++++++++++++++++++++++++
        if(BUILD_TBB_TARGETS)
                 message(STATUS "Adding target ${target_name} to TBB backend. Executable file name: ${target_name}_tbb")
                 
                 add_executable("${target_name}_tbb"
                        # EXCLUDE_FROM_ALL
                        "${CMAKE_CURRENT_SOURCE_DIR}/src/phi_s/fit/${target_name}.cpp")
                    
                 set_target_properties("${target_name}_tbb"
                        PROPERTIES COMPILE_FLAGS "-DHYDRA_HOST_SYSTEM=CPP -DHYDRA_DEVICE_SYSTEM=TBB")
                    
                 target_link_libraries("${target_name}_tbb" ${ROOT_LIBRARIES} ${TBB_LIBRARIES} ${LIBCONFIG_CPP_LIBRARIES} ${GSL_LIBRARIES})

        endif(BUILD_TBB_TARGETS)
         
        #+++++++++++++++++++++++++
        # CPP TARGETS            |
        #+++++++++++++++++++++++++
        if(BUILD_CPP_TARGETS)
                 message(STATUS "Adding target ${target_name} to CPP backend. Executable file name: ${target_name}_cpp")
                 
                 add_executable("${target_name}_cpp"
                        # EXCLUDE_FROM_ALL 
                        "${CMAKE_CURRENT_SOURCE_DIR}/src/phi_s/fit/${target_name}.cpp")
                    
                 set_target_properties( "${target_name}_cpp" 
                        PROPERTIES COMPILE_FLAGS "-DHYDRA_HOST_SYSTEM=CPP -DHYDRA_DEVICE_SYSTEM=CPP")
                    
                 target_link_libraries("${target_name}_cpp" ${ROOT_LIBRARIES} ${TBB_LIBRARIES} ${LIBCONFIG_CPP_LIBRARIES} ${GSL_LIBRARIES})

         endif(BUILD_CPP_TARGETS)
         
          
        #+++++++++++++++++++++++++
        # OMP TARGETS            |
        #+++++++++++++++++++++++++
        if(BUILD_OMP_TARGETS)
                 message(STATUS "Adding target ${target_name} to OMP backend. Executable file name: ${target_name}_omp")
                 
                 add_executable("${target_name}_omp" 
                        #EXCLUDE_FROM_ALL
                        "${CMAKE_CURRENT_SOURCE_DIR}/src/phi_s/fit/${target_name}.cpp" )
                    
                 set_target_properties( "${target_name}_omp" 
                        PROPERTIES COMPILE_FLAGS "-DHYDRA_HOST_SYSTEM=CPP -DHYDRA_DEVICE_SYSTEM=OMP ${OpenMP_CXX_FLAGS}")
                    
                 target_link_libraries("${target_name}_omp" ${ROOT_LIBRARIES} ${OpenMP_CXX_LIBRARIES} ${LIBCONFIG_CPP_LIBRARIES} ${GSL_LIBRARIES})

         endif(BUILD_OMP_TARGETS)
         
endfunction(ADD_TARGET_PHIS_FIT)