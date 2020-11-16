#ifndef _BACKENDS_FPGA_COMMON_OPTIONS_H_
#define _BACKENDS_FPGA_COMMON_OPTIONS_H_


#include <getopt.h>
#include "frontends/common/options.h"


namespace FPGA {

class FPGAOptions : public CompilerOptions {
public:

	FPGAOptions() {
	}
};

using FPGAContext = P4CContextWithOptions<FPGAOptions>;


} // end namespace FPGA





#endif
