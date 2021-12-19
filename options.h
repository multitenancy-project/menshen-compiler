#ifndef _BACKENDS_FPGA_COMMON_OPTIONS_H_
#define _BACKENDS_FPGA_COMMON_OPTIONS_H_


#include <cstdlib>
#include <getopt.h>
#include <vector>
#include "frontends/common/options.h"


namespace FPGA {

class FPGAOptions : public CompilerOptions {
public:

	int vid = -1;
	int if_stateful_only = -1;
	int if_sys = -1;
	cstring confFilename;
	cstring outputfile;

	std::vector<cstring> files;

	FPGAOptions() {
		registerOption("--vid", "vid", 
				[this](const char* arg) {
					vid = std::atoi(arg);
					return true;
				},
				"vlan id for this program.");

		registerOption("--statefulconf", "statefulconf",
				[this](const char* arg) {
					if_stateful_only = std::atoi(arg);
					return true;
				},
				"whether to generate stateful conf only.");

		registerOption("--onlysys", "onlysys",
				[this](const char* arg) {
					if_sys = std::atoi(arg);
					return true;
				},
				"whether processing sysem file only.");

		registerOption("--conffile", "conffile",
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

	//
	void setInputFile();
	FILE* preprocess(cstring file);
	void closeInput(FILE* inputStream, bool if_close) const;
};

using FPGAContext = P4CContextWithOptions<FPGAOptions>;


} // end namespace FPGA





#endif
