#ifndef _BACKENDS_FPGA_COMMON_OPTIONS_H_
#define _BACKENDS_FPGA_COMMON_OPTIONS_H_


#include <cstdlib>
#include <getopt.h>
#include "frontends/common/options.h"


namespace FPGA {

class FPGAOptions : public CompilerOptions {
public:

	int vid = -1;
	cstring confFilename;
	cstring outputfile;

	FPGAOptions() {
		registerOption("--vid", "vid", 
				[this](const char* arg) {
					vid = std::atoi(arg);
					return true;
				},
				"vlan id for this program.");

		registerOption("--confile", "confile",
				[this](const char* arg) {
					confFilename = arg;
					return true;
				},
				"configuration file for control plane.");

		registerOption("--outputfile", "outfile",
				[this](const char* arg) {
					outputfile = arg;
					return true;
				},
				"output file for control plane pkts.");
	}
};

using FPGAContext = P4CContextWithOptions<FPGAOptions>;


} // end namespace FPGA





#endif
