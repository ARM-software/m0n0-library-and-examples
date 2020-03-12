
/*
 * Copyright (c) 2020, Arm Limited
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include "m0n0_printf.h"
#include "minilibs.h"

#include <stdlib.h>

// For softmax (should be on nn?)
#include "arm_nnfunctions.h"

#include "kws.h"
#include "kws_constants.h"

#include "mfcc.h"
#include "nn.h"

// GPIO indicators
// Race condition error 
const uint8_t RC_ERROR = 15;
// new classification about to take place
const uint8_t NEW_CLASSIFICATION = 14;

// MFCC
struct mfcc_config mfcc_conf;
// buffers
// input
volatile q15_t audio_buffer[NUM_AUDIO_WIN_INPUT][FRAME_LEN];
// mfcc
q7_t mfcc_buffer[MFCC_BUFFER_SIZE];
// output
q7_t output[OUT_DIM];


// State variables
// printf available
uint8_t verbose;
// window counter, to point to the buffer area
// required for the next  mfcc computation
volatile uint8_t input_window_counter;
volatile uint8_t compute_window_counter;
volatile uint8_t mfcc_window_counter;
// audio_counter, to point to the next place to store the data
// in the audio_buffer
volatile uint32_t buff_counter;
// wether process mfcc in next operation
volatile uint8_t process_mfcc;
// wether the processing of the mfcc is running
volatile uint8_t running_mfcc;
volatile uint8_t race_condition;
// wether process nn in next operation
volatile uint8_t process_classify;

// methods
void reset_system();
int stack_info();
void turn_off_rom_blocks();
void enable_data_from_sensor();
void process_audio_window();
void classify();

void run_kws(uint8_t global_verbose)
{
	// set verbosity
	verbose = global_verbose;
	// reset buffers and constants
	reset_system();

	if (verbose) m0n0_printf("\n\n** STARTING KWS **\n\n");
	// enable data from sensor
	enable_data_from_sensor();

	// main loop
	while(1) {
        //temp += 1;
		if(process_mfcc){
			process_audio_window();
			if(process_classify){
				__NVIC_DisableIRQ(Interrupt1_IRQn);
				classify();
				__NVIC_EnableIRQ(Interrupt1_IRQn);
			}
		}
	}
}

// Prints memory addresses of stack and heap
int stack_info()
{

	#ifdef M0N0_KWS_DEBUG
	int stack = 1;
	if (verbose) m0n0_printf("Stack Address: 0x%x\n", (long unsigned int)&stack);
	int * heap_ptr;
	heap_ptr = (int *) malloc(sizeof(int));
	if (verbose) m0n0_printf("Heap Address 0x%x\n", (uint32_t) heap_ptr);
	free(heap_ptr);

	if (verbose) m0n0_printf("PSPLIM ProcStackPtrLimit 0x%x\n", __get_PSPLIM());
	if (verbose) m0n0_printf("MSPLIM MainStackPtrLimit 0x%x\n", __get_MSPLIM());
	#endif // M0N0_KWS_DEBUG

	return 0;
}

void turn_off_rom_blocks()
{
    return;
}

void reset_system()
{
	// initialize state variables
	mfcc_window_counter = 0;
	input_window_counter = 0;
	compute_window_counter = 0;
	buff_counter = 0;
	process_mfcc = 0;
	running_mfcc = 0;
	race_condition = 0;
	process_classify = 0;
	// mfcc initialization
	mfcc_init(&mfcc_conf);

	// clear buffers to known values
	memset((q15_t*)audio_buffer, 1, sizeof(audio_buffer));
	memset(mfcc_buffer, 2, sizeof(mfcc_buffer));
	memset(output, 3, sizeof(output));

	// nn buffers
	clear_nn_buffers();
	write_gpio(0);
	// turn off rom blocks
	turn_off_rom_blocks();
}

void enable_data_from_sensor()
{
	// set up interruptions
	// Clear loop timer in case it has been running
    m0n0_printf("A");
	clear_loop_timer();
	// Setup a looping timer interrupt in PCSM
	set_loop_timer(4);
	// enable interruptions
	__NVIC_EnableIRQ(Interrupt1_IRQn);

	// Configures SPI Slave Select (SS2)
	spi_set_ss_active_low_ss2();
	// Configures SPI Slave Select (SS2)
	spi_select_slave(2);

	// Enable polling bit
	spi_enable_adc_polling();
    m0n0_printf("B");
}

// Method called after a 40ms window is filled
void process_audio_window()
{
	// flag mfcc as running
	running_mfcc = 1;
	#ifdef M0N0_KWS_DEBUG
	m0n0_printf("\nMFCC %d \n", mfcc_window_counter);
	#endif

	// compute mfcc
	mfcc_compute(&mfcc_conf,
		(q15_t *)audio_buffer[compute_window_counter],
		&mfcc_buffer[mfcc_window_counter * NUM_MFCC_COEFFS]);

	#ifdef M0N0_KWS_DEBUG
	if (verbose) m0n0_printf("\nprocessed %u wc\n", mfcc_window_counter);
	#endif // M0N0_KWS_DEBUG
	// window counter
	if( ++mfcc_window_counter >= (uint8_t)NUM_AUDIO_WIN_TOTAL ){
		mfcc_window_counter = 0;
		// stop interruptions?
		process_classify = 1;
	}
	// clean flag
	process_mfcc = 0;
	// release mfcc flag
	running_mfcc = 0;
}


// Method called after all mfcc features have been processed
void classify()
{
	#ifdef M0N0_KWS_DEBUG
	if (verbose) m0n0_printf("\nclassify\n");
	#endif // M0N0_KWS_DEBUG
	
	// notify new classification
	write_gpio(NEW_CLASSIFICATION);
	if (!race_condition){
		
		// emulation of nn
		run_nn(mfcc_buffer, output);
		// softmax: could be removed
		// should only the class be required
		arm_softmax_q7(output, OUT_DIM, output);
	
		// compute best class
		int8_t max_prob = -128;
		
		uint8_t classification_result = 0;
		for (uint32_t j = 0; j < (uint32_t)OUT_DIM; j++) {
			if (output[j] > max_prob){
				max_prob = output[j];
				classification_result = j;
			}
		}
		
		// debug
		#ifdef M0N0_KWS_DEBUG
		if (verbose){
			m0n0_printf("classification done\n");
			m0n0_printf("\n");
			for (uint32_t j = 0; j < (uint32_t)OUT_DIM; j++) {
				m0n0_printf("\nClass %d, arg: %d\n", j, output[j]);
			}
			m0n0_printf("\n");
		}
		#endif // M0N0_KWS_DEBUG
		
		// report
		if (verbose){
			m0n0_printf("\nC-%d, %d\n", classification_result, output[classification_result]);
		}
		write_gpio(classification_result);
	}else{
		// report
		if (verbose){
			m0n0_printf("\nRC error\n");
		}
		write_gpio(RC_ERROR);
	}
	
	// reset pointers
	buff_counter = 0;
	input_window_counter = 0;
	// reset process_classify flag and clean other flags, just in case
	process_classify = 0;
	race_condition = 0;
	running_mfcc = 0;
	
	// turn off rom blocks
	turn_off_rom_blocks();
}


//
// Handlers
//
void Interrupt1_Handler()
{
	////////////////////////////////
	// 8b input values
	////////////////////////////////
    uint32_t sensor_data = get_sensor_data();
	audio_buffer[input_window_counter][buff_counter++] = (q15_t)((*((q7_t *)&sensor_data + 3)) << 8);
	audio_buffer[input_window_counter][buff_counter++] = (q15_t)((*((q7_t *)&sensor_data + 2)) << 8);
	audio_buffer[input_window_counter][buff_counter++] = (q15_t)((*((q7_t *)&sensor_data + 1)) << 8);
	audio_buffer[input_window_counter][buff_counter++] = (q15_t)((*((q7_t *)&sensor_data + 0)) << 8);

	// update partial counter
	// check partial counter for process_mfcc updating
	if (buff_counter >= (uint32_t)FRAME_LEN){
		if (running_mfcc){
			race_condition = 1;
			write_gpio(RC_ERROR);
		}
		compute_window_counter = input_window_counter;
		process_mfcc = 1;
		buff_counter = 0;
		input_window_counter++;
		if (input_window_counter>=NUM_AUDIO_WIN_INPUT){
		  input_window_counter = 0;
		}
	}
}
