/*
 * Copyright (c) 2021, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>  // printf
#include <time.h>	// time, localtime

#include "scd4x_i2c.h"
#include "sensirion_common.h"
#include "sensirion_i2c_hal.h"

/**
 * TO USE CONSOLE OUTPUT (PRINTF) IF NOT PRESENT ON YOUR PLATFORM
 */
//#define printf(...)

int main(void) {
    int16_t error = 0;

    sensirion_i2c_hal_init();

    // Clean up potential SCD40 states
    scd4x_wake_up();
    scd4x_stop_periodic_measurement();
    scd4x_reinit();

    uint16_t serial_0;
    uint16_t serial_1;
    uint16_t serial_2;
    error = scd4x_get_serial_number(&serial_0, &serial_1, &serial_2);
    if (error) {
        printf("Error executing scd4x_get_serial_number(): %i\n", error);
    } else {
        printf("serial: 0x%04x%04x%04x\n", serial_0, serial_1, serial_2);
    }

    // Start Measurement

    error = scd4x_start_periodic_measurement();
    if (error) {
        printf("Error executing scd4x_start_periodic_measurement(): %i\n",
               error);
    }

	
	time_t rawtime;
	struct tm *info;
	time(&rawtime);
	info = localtime(&rawtime);
	char buffer[80];
	strftime(buffer, 80, "%d-%m-%Y %H:%M:%S", info);
	printf("Start at: %s \n", buffer);
	
	char text_data_file[80];
	char csv_data_file[80];
	strftime(text_data_file, 80, "sensor_data_%d-%m-%Y %H:%M:%S.txt", info);
	strftime(csv_data_file, 80, "sensor_data_%d-%m-%Y %H:%M:%S.csv", info);
	
	// Initialize csv file
	FILE *fdata = fopen(csv_data_file, "a");
	if (fdata == NULL)
	{
		printf("Error opening the file %s", csv_data_file);
		return -1;
	}
	fprintf(fdata, "time, co2(ppm), temperature(C), rel_humidity(percent)\n");
	fclose(fdata);

    printf("Waiting for first measurement... (5 sec)\n");		
		
    for (;;) {
		// Open files for writing
		FILE *ftext = fopen(text_data_file, "a");
		if (ftext == NULL)
		{
			printf("Error opening the file %s", text_data_file);
			return -1;
		}
		FILE *fdata = fopen(csv_data_file, "a");
		if (fdata == NULL)
		{
			printf("Error opening the file %s", csv_data_file);
			return -1;
		}
		
		
		// Get current time and read and write measurement
        sensirion_i2c_hal_sleep_usec(5000000);
        
        time_t rawtime;
		struct tm *info;
		time(&rawtime);
		info = localtime(&rawtime);
		char buffer[80];
		strftime(buffer, 80, "%d-%m-%Y %H:%M:%S", info);
		
        uint16_t co2;
        float temperature;
        float humidity;
        error = scd4x_read_measurement(&co2, &temperature, &humidity);
        if (error) {
            printf("Error executing scd4x_read_measurement(): %i\n", error);
            fprintf(ftext, "Error executing scd4x_read_measurement(): %i\n", error);
        } else if (co2 == 0) {
            printf("Invalid sample detected, skipping.\n");
            fprintf(ftext, "Invalid sample detected, skipping.\n");
        } else {
			printf("%s\n", buffer);
            printf("CO2: %u ppm\n", co2);
            printf("Temperature: %.2f °C\n", temperature);
            printf("Humidity: %.2f RH\n\n", humidity);
            fprintf(ftext, "%s\n", buffer);
            fprintf(ftext, "CO2: %u ppm\n", co2);
            fprintf(ftext, "Temperature: %.2f °C\n", temperature);
            fprintf(ftext, "Humidity: %.2f RH\n\n", humidity);
            fprintf(fdata, "%s, %u, %.2f, %.2f\n", buffer, co2, temperature, humidity);
        }
		fclose(ftext);
		fclose(fdata);
    }
	
	
    return 0;
}
