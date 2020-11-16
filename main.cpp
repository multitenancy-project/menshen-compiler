#include <iostream>


#include "ir/ir.h"

#include "lib/nullstream.h"
#include "lib/gc.h"
#include "lib/error.h"


#include "ir/ir.h"
#include "control-plane/p4RuntimeSerializer.h"
#include "frontends/common/applyOptionsPragmas.h"
#include "frontends/common/parseInput.h"
#include "frontends/p4/frontend.h"
#include "lib/error.h"
#include "lib/exceptions.h"
#include "lib/gc.h"
#include "lib/log.h"
#include "lib/nullstream.h"
#include "ir/json_loader.h"
#include "fstream"

// 
#include "options.h"

int main(int argc, char *const argv[]) {
	setup_gc_logging();

	AutoCompileContext autoFPGAContext(new FPGA::FPGAContext);
	auto& options = FPGA::FPGAContext::get().options();

	options.langVersion = CompilerOptions::FrontendVersion::P4_16;

	if (options.process(argc, argv) != nullptr) {
		options.setInputFile();
	}

	if (::errorCount() > 0) {
		return 1;
	}

	auto hook = options.getDebugHook();

	//
	const IR::P4Program* program = nullptr;
	const IR::ToplevelBlock* toplevel = nullptr;

	// parse input p4 program
	program = P4::parseP4File(options);

	if (program == nullptr || ::errorCount() > 0) {
		::error("can not parse input p4 program!\n");
		return 1;
	}

    return 0;
}
