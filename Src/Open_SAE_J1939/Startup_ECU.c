/*
 * Startup_ECU.c
 *
 *  Created on: 25 sep. 2021
 *      Author: Daniel Mårtensson
 */

#include "Open_SAE_J1939.h"
#include "../SAE_J1939/SAE_J1939DA/SAE_J1939DA.h"

/* Layers */
#include "../Hardware/Hardware.h"

#include "Tools.h"

/* Load our ECU parameters into J1939 structure. Very useful if you want your ECU remember its NAME + address + identifications at startup. */
bool Open_SAE_J1939_init_and_Load_ECU_info(J1939* j1939, uint32_t identityNumber) {
	uint32_t ECU_information_length = sizeof(Information_this_ECU);
	uint8_t ECU_information_data[sizeof(Information_this_ECU)];
	
	memset(ECU_information_data, 0, ECU_information_length);
	if(!Load_Struct(ECU_information_data, ECU_information_length, (char*)INFORMATION_THIS_ECU)){
		return false; /* Problems occurs */
	}
	memcpy(&j1939->information_this_ECU, (Information_this_ECU*)ECU_information_data, ECU_information_length);

	j1939->information_this_ECU.this_name.identity_number = identityNumber;		/* Specify the ECU serial ID - 0 to 2097151 */

	/* If we are going to send and receive the ECU identification and component identification, we need to specify the size of them */
	j1939->information_this_ECU.this_identifications.ecu_identification.length_of_each_field = MAX_IDENTIFICATION;
	j1939->information_this_ECU.this_identifications.component_identification.length_of_each_field = MAX_IDENTIFICATION;
	j1939->from_other_ecu_identifications.ecu_identification.length_of_each_field = MAX_IDENTIFICATION;
	j1939->from_other_ecu_identifications.component_identification.length_of_each_field = MAX_IDENTIFICATION;

	/* If we are going to send and receive Proprietary, we need to specify the size of them */
	j1939->this_proprietary.proprietary_A.total_bytes = MAX_PROPRIETARY_A;
	j1939->from_other_ecu_proprietary.proprietary_A.total_bytes = MAX_PROPRIETARY_A;
	for (int i = 0; i < MAX_PROPRIETARY_B_PGNS; ++i)
	{
		j1939->this_proprietary.proprietary_B[i].total_bytes = MAX_PROPRIETARY_B;
		j1939->from_other_ecu_proprietary.proprietary_B[i].total_bytes = MAX_PROPRIETARY_B;
	}

	/* Clear other ECU addresses by setting the broadcast address to them */
	memset(j1939->other_ECU_address, 0xFF, 0xFF);
	j1939->number_of_cannot_claim_address = 0;
	j1939->number_of_other_ECU = 0;
	j1939->current_ECU_address_in_use_we_lost = false;
	
	// seed rand with unique id, to get random time
	srand(identityNumber);

	SAE_J1939DA_init_Request_Fuel_Economy(j1939);
	
	j1939->operational_status_of_this_ecu = INIT_FINISHED;
	/* OK */
	return true;
}

static uint32_t timestampAddressClaimStarted = 0;
static uint8_t random_time_to_wait = 153;
bool SAE_J1936_handle_address_claim(J1939 *j1939)
{

	if ((j1939->operational_status_of_this_ecu == INIT_FINISHED) || (j1939->operational_status_of_this_ecu == ADDRESS_CLAIM_AGAIN))
	{
		j1939->current_ECU_address_in_use_we_lost = false;
		timestampAddressClaimStarted = TOO_sysNow();

		random_time_to_wait = 10+rand() % 153;   // 0..153ms
		SAE_J1939_logger("random_time_to_wait %u", random_time_to_wait);
		j1939->operational_status_of_this_ecu = ADDRESS_CLAIM_START;
		return false;
	}
	else if ((j1939->operational_status_of_this_ecu == ADDRESS_CLAIM_START))
	{
		if (j1939->current_ECU_address_in_use_we_lost == true)
		{
			j1939->operational_status_of_this_ecu = ADDRESS_CLAIM_PENDING;
			return false;
		}
		else
		{
			if (TOO_isMilliSecondsElapsed(timestampAddressClaimStarted, random_time_to_wait) == true )
			{
				SAE_J1939_logger("Claim for address %u will be send", j1939->information_this_ECU.this_ECU_address);
				/* This broadcast out this ECU NAME + address to all other ECU:s */
				SAE_J1939_Response_Request_Address_Claimed(j1939);
				j1939->operational_status_of_this_ecu = ADDRESS_CLAIM_PENDING;
				return false;
			}
			else
				return false;
		}
	}
	else if ( ((j1939->operational_status_of_this_ecu == ADDRESS_CLAIM_PENDING) ||
			   (j1939->operational_status_of_this_ecu == ADDRESS_CLAIM_FINISHED) ||
			   (j1939->operational_status_of_this_ecu == OPERATIONAL)) && 
			   (j1939->current_ECU_address_in_use_we_lost == true) )
	{
		j1939->current_ECU_address_in_use_we_lost=false;
		uint8_t prevAddtress = j1939->information_this_ECU.this_ECU_address;
		j1939->information_this_ECU.this_ECU_address = (j1939->information_this_ECU.this_ECU_address + 1) % 254;   // 0..253
		if (j1939->information_this_ECU.this_ECU_address == ECU_START_ADDRESS )
		{
			// give up
			SAE_J1939_Send_Address_Not_Claimed(j1939);
			j1939->operational_status_of_this_ecu = ADDRESS_CLAIM_FAILED;
			return false;
		}

		j1939->operational_status_of_this_ecu = ADDRESS_CLAIM_AGAIN;
		SAE_J1939_logger("Address %u in use, we need to try next one: %u", prevAddtress, j1939->information_this_ECU.this_ECU_address);
		return false;
	}
	else if ((j1939->operational_status_of_this_ecu == ADDRESS_CLAIM_PENDING) && 
			 TOO_isMilliSecondsElapsed(timestampAddressClaimStarted, 250) == true ) // claim expire time normally 250ms
	{
		j1939->operational_status_of_this_ecu = ADDRESS_CLAIM_FINISHED;
		SAE_J1939_logger("Claim succesfull my address = %u", j1939->information_this_ECU.this_ECU_address);
	}
	else if (j1939->operational_status_of_this_ecu == ADDRESS_CLAIM_FINISHED)
	{
		/* This asking all ECU about their NAME + address */
		SAE_J1939_Send_Request_Address_Claimed(j1939, 0xFF);
		j1939->operational_status_of_this_ecu = OPERATIONAL;
	}







	/* OK */
	return (j1939->operational_status_of_this_ecu == OPERATIONAL);

}
