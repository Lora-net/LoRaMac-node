cmake_minimum_required(VERSION 3.23)

project(loramac-radio C)

if (NOT TARGET ${PROJECT_NAME})
    
    add_library(${PROJECT_NAME} STATIC)
    
    target_sources(
            ${PROJECT_NAME}
            PUBLIC
            ${LORAMAC_SOURCE_DIR}/radio/radio.h
            ${LORAMAC_SOURCE_DIR}/radio/sx1276/sx1276.h
            ${LORAMAC_SOURCE_DIR}/radio/sx1276/sx1276Regs-Fsk.h
            ${LORAMAC_SOURCE_DIR}/radio/sx1276/sx1276Regs-LoRa.h

            PRIVATE
            ${LORAMAC_SOURCE_DIR}/radio/sx1276/sx1276.c
    )
    
    target_include_directories(
            ${PROJECT_NAME} 
            PUBLIC 
            ${LORAMAC_SOURCE_DIR}/radio
    )
    
    target_link_libraries(
            ${PROJECT_NAME}
            PUBLIC
            loramac-system
    )
    
endif()
