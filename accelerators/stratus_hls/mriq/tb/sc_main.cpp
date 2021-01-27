// Copyright (c) 2011-2019 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include "system.hpp"

#define RESET_PERIOD (30 * CLOCK_PERIOD)

system_t * testbench = NULL;

//std::string inputData_path = "/home/esp2020/pl2748/esp/accelerators/stratus_hls/mriq/stratus/32_32_32_dataset.bin";
//
//#ifdef LESS
//std::string outputData_path = "/home/esp2020/pl2748/esp/accelerators/stratus_hls/mriq/stratus/gold.txt";
//#else
//std::string outputData_path = "/home/esp2020/pl2748/esp/accelerators/stratus_hls/mriq/stratus/32_32_32_dataset.out";
//#endif
//
//std::string outint_path = "/home/esp2020/pl2748/esp/accelerators/stratus_hls/mriq/stratus/in.h";
////
//
//
//std::string inputData_path = "32_32_32_dataset.bin";
//std::string outputData_path = "32_32_32_dataset.out";
//std::string outint_path = "ih.h";
//
extern void esc_elaborate()
{
	// Creating the whole system
  testbench = new system_t("testbench");
}

extern void esc_cleanup()
{
	// Deleting the system
	delete testbench;
}

int sc_main(int argc, char *argv[])
{
	// Kills a Warning when using SC_CTHREADS
	//sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
	sc_report_handler::set_actions (SC_WARNING, SC_DO_NOTHING);

	esc_initialize(argc, argv);

//
//#ifdef CADENCE
//
//	fprintf(stderr, "[INFO] argc: %d\n" , argc);
//	if (argc == 3){
//	  inputData_path = argv[1];
//	  outputData_path = argv[2];
//	  outint_path = argv[3];
//
//	}else{
//	  fprintf(stderr, "Not enough arguments");
//        }
//#endif
//

	esc_elaborate();

	sc_clock        clk("clk", CLOCK_PERIOD, SC_PS);
	sc_signal<bool> rst("rst");

	testbench->clk(clk);
	testbench->rst(rst);
	rst.write(false);

	sc_start(RESET_PERIOD, SC_PS);

	rst.write(true);

	sc_start();

	esc_log_pass();
	//        esc_cleanup();

	return 0;
}
