cmake_minimum_required(VERSION 3.16.0)

# Collect sources
file(GLOB_RECURSE ALL_SOURCE_FILES "Src/*.c")
set(MAIN_SRC "Src/Main.c")
list(REMOVE_ITEM ALL_SOURCE_FILES ${MAIN_SRC})

# remove translation unit "SocketCAN_Transmit_Receive.c" if not SOCKETCAN
if(NOT "${PROCESSOR_CHOICE}" STREQUAL "SOCKETCAN")
    list(REMOVE_ITEM ALL_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Src/Hardware/SocketCAN_Transmit_Receive.c")
endif()

idf_component_register(
    SRCS ${ALL_SOURCE_FILES}
    INCLUDE_DIRS 
    "."
    "Src/Open_SAE_J1939"
)

#0x80   periodic sender
#0x81   reciever

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    OPENSAE_J1939_TARGET_PLATFORM=INTERNAL_CALLBACK
    OPENSAE_J1939_ECU_ADRESS=0x80
)

if(NOT OPENSAE_J1939_TARGET_PLATFORM STREQUAL "SOCKETCAN")
    list(REMOVE_ITEM ALL_SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/Hardware/SocketCAN_Transmit_Receive.c")
endif()