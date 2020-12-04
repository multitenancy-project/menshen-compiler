#include <iostream>


#include "ir/ir.h"

#include "lib/nullstream.h"
#include "lib/gc.h"
#include "lib/error.h"

#include "frontends/common/resolveReferences/resolveReferences.h"
#include "frontends/p4/typeMap.h"
#include "frontends/p4/parseAnnotations.h"
#include "frontends/p4/validateParsedProgram.h"
#include "frontends/p4/createBuiltins.h"
#include "frontends/p4/frontend.h"
#include "frontends/p4/evaluator/evaluator.h"

#include "frontends/p4/fromv1.0/v1model.h"

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
#include "fpga-backend.h"

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

	if (options.vid==-1) {
		::error("vid is not set");
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

	P4::P4COptionPragmaParser optionsPragmaParser;
	program->apply(P4::ApplyOptionsPragmas(optionsPragmaParser));

	P4::FrontEnd frontend;
	frontend.addDebugHook(hook);
	program = frontend.run(options, program);

	if (::errorCount() > 0) {
		::error("frontend error");
		return 1;
	}

	P4::ReferenceMap refMap;
	P4::TypeMap typeMap;
	P4::ParseAnnotations parseAnnotations;
	refMap.setIsV1(options.isv1());

	auto evaluator = new P4::EvaluatorPass(&refMap, &typeMap);
	PassManager passes = {
		// new P4::ParseAnnotationBodies(&parseAnnotations, &typeMap),
		// new P4::ValidateParsedProgram(),
		// new P4::CreateBuiltins(),
		new P4::ResolveReferences(&refMap),
		new P4::TypeInference(&refMap, &typeMap, false),
		evaluator};

	program = program->apply(passes);
	LOG3(refMap);
	LOG3(typeMap);
	LOG3(evaluator->getToplevelBlock());
	toplevel = evaluator->getToplevelBlock();

	// parser graphs
	FPGA::run_fpga_backend(options, toplevel, &refMap, &typeMap);

	// auto main = evaluator->getToplevelBlock()->getMain();
	// std::cout << main << std::endl;
	// std::cout << v1model.sw.ingress.name << std::endl;
	// auto ingress = main->findParameterValue(v1model.sw.ingress.name);
	// std::cout << ingress << std::endl;
	// auto ingress_name = ingress->to<IR::ControlBlock>()->container->name;
	// std::cout << ingress_name << std::endl;

    return 0;
}
