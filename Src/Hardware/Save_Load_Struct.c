/*
 * Save_Load_Struct.c
 *
 *  Created on: 22 sep. 2021
 *      Author: Daniel Mårtensson
 */

#include "Hardware.h"
#include "Structs.h"

/* Layers */
#include <stdio.h>

bool Save_Struct(uint8_t data[], uint32_t data_length, char file_name[]){
#if OPENSAE_J1939_TARGET_PLATFORM == STM32
	/* Save it to SD card */
	if(STM32_PLC_SD_Mont_Card() != FR_OK){
		return false;
	}
	STM32_PLC_SD_Create_File_With_Write(file_name);
	STM32_PLC_SD_Write_Data(data, data_length);
	STM32_PLC_SD_Close_File();
	STM32_PLC_SD_Unmount_Card();
	return true;
#elif OPENSAE_J1939_TARGET_PLATFORM == ARDUINO
	/* Implement your memory handler function for the Arduino platform */
#elif OPENSAE_J1939_TARGET_PLATFORM == PIC
	/* Implement your memory handler function for the PIC platform */
#elif OPENSAE_J1939_TARGET_PLATFORM == AVR
	/* Implement your memory handler function for the AVR platform */
#else
	// /* Write a file */
	// FILE *file = NULL;
	// file = fopen(file_name, "wb");
	// if (file == NULL) {
	// 	return false;
	// }
	// fwrite(data, 1, data_length, file);
	// fclose(file);
	return true;
#endif
}

bool Load_Struct(uint8_t data[], uint32_t data_length, char file_name[]){
#if OPENSAE_J1939_TARGET_PLATFORM == STM32
	/* Load it from SD card */
	if(STM32_PLC_SD_Mont_Card() != FR_OK){
		return false;
	}
	STM32_PLC_SD_Open_File_With_Read(file_name);
	STM32_PLC_SD_Read_Data(data, data_length);
	STM32_PLC_SD_Close_File();
	STM32_PLC_SD_Unmount_Card();
	return true;
#elif OPENSAE_J1939_TARGET_PLATFORM == ARDUINO
	/* Implement your memory handler function for the Arduino platform */
#elif OPENSAE_J1939_TARGET_PLATFORM == PIC
	/* Implement your memory handler function for the PIC platform */
#elif OPENSAE_J1939_TARGET_PLATFORM == AVR
	/* Implement your memory handler function for the AVR platform */
#else
	Information_this_ECU *thisEcu = (Information_this_ECU*)data;

	thisEcu->this_ECU_address = OPENSAE_J1939_ECU_ADRESS;

	thisEcu->this_name.identity_number 				= 1111;		/* Specify the ECU serial ID - 0 to 2097151 */
	thisEcu->this_name.manufacturer_code 			= 1111;		/* Specify the ECU manufacturer code - 0 to 2047 */
	thisEcu->this_name.function_instance			= 0;		/* Specify the ECU function number - 0 to 31 */
	thisEcu->this_name.ECU_instance					= 0;		/* Specify the ECU number - 0 to 7 */
	thisEcu->this_name.function						= 0;		/* Specify the ECU function - 0 to 255 */
	thisEcu->this_name.vehicle_system				= 0;		/* Specify the type of vehicle where ECU is located - 0 to 127 */
	thisEcu->this_name.arbitrary_address_capable	= 1;		/* Specify if the ECU have right to change address if addresses conflicts - 0 to 1 */
	thisEcu->this_name.industry_group				= 0;		/* Specify the group where this ECU is located - 0 to 7 */
	thisEcu->this_name.vehicle_system_instance		= 0;		/* Specify the vehicle system number - 0 to 15 */
	thisEcu->this_name.from_ecu_address				= 0;		/* From which ECU came this message */

	const char idName[] = "J1939 Data Colector";
	thisEcu->this_identifications.software_identification.number_of_fields = sizeof(idName);					/* How many numbers contains in the identifications array */
	memcpy(thisEcu->this_identifications.software_identification.identifications, idName, sizeof(idName));		/* This can be for example ASCII */
	thisEcu->this_identifications.software_identification.from_ecu_address = 0;
	
	memset(&thisEcu->this_identifications.ecu_identification, 0, sizeof(ECU_identification));
	const char ecu_location[] = "location1";
	const char ecu_part_number[] = "1";
	memcpy(thisEcu->this_identifications.ecu_identification.ecu_location, ecu_location, sizeof(ecu_location));
	memcpy(thisEcu->this_identifications.ecu_identification.ecu_part_number, ecu_part_number, sizeof(ecu_part_number));
	
	
	memset(&thisEcu->this_identifications.component_identification, 0, sizeof(Component_identification));
	// memcpy(thisEcu->this_name, "NameOTthisEcu", sizeof("NameOTthisEcu"));

// struct {
// 	struct Name this_name;
// 	uint8_t this_ECU_address;
// 	struct Identifications this_identifications;
// } Information_this_ECU;


// /* Read a file */
	// FILE *file = NULL;
	// file = fopen(file_name, "rb");
	// if(file == NULL){
	// 	file = fopen(file_name, "wb");
	// 	if (file == NULL) {
	// 		return false;
	// 	}
	// }
	// fread(data, 1, data_length, file);
	// fclose(file);
	return true;
#endif
}
