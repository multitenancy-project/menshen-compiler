#include <stdio.h>

#include "lib/exceptions.h"



#include "merge.h"


namespace FPGA {

bool Merger::preorder(IR::P4Control *ctrl) {
	// auto cb = mg->toplevels[0]->getMain()->findParameterValue(mg->model.sw.ingress.name)->to<IR::ControlBlock>();
	// std::cout << ctrl << std::endl;
	// std::cout << "hhhhhhh\n";
	// std::cout << cb->container << std::endl;
	// BUG_CHECK(ctrl==cb->container, "is something wrong here?");
	// printf("%p %p %p\n", ctrl, cb->container, cb);

	for (size_t i=1; i<mg->progs.size(); i++) {
		auto prog_ctrl = mg->toplevels.at(i)->getMain()->findParameterValue(mg->model.sw.ingress.name)
								->to<IR::ControlBlock>()->container;
		BUG_CHECK(prog_ctrl!=nullptr, "impossible nullptr prog ctrl");

		// merge control locals, like declarations
		for (auto decl : prog_ctrl->controlLocals) {
			ctrl->controlLocals.push_back(decl);
		}

		// merge body part (i.e. apply {})
		visit(ctrl->body);
	}

	return false;
}

bool Merger::preorder(IR::BlockStatement *blk) {
	for (size_t i=1; i<mg->progs.size(); i++) {
		auto prog_ctrl = mg->toplevels.at(i)->getMain()->findParameterValue(mg->model.sw.ingress.name)
								->to<IR::ControlBlock>()->container;

		// merge body part (i.e. apply {})
		for (auto body_component : prog_ctrl->body->components) {
			blk->components.push_back(body_component);
		}
	}
	return false;
}

//=====================================================================================

bool MergeProgs::merge() {
	// pass frontend
	auto hook = options.getDebugHook();
	for (auto program : progs) {
		// P4::P4COptionPragmaParser optionsPragmaParser;
		// program->apply(P4::ApplyOptionsPragmas(optionsPragmaParser));

		// P4::FrontEnd frontend;
		// frontend.addDebugHook(hook);
		// program = frontend.run(options, program);

		// if (::errorCount() > 0) {
		// 	::error("frontend error");
		// 	return false;
		// }

		P4::ReferenceMap refMap;
		P4::TypeMap typeMap;
		P4::ParseAnnotations parseAnnotations;
		refMap.setIsV1(options.isv1());

		auto evaluator = new P4::EvaluatorPass(&refMap, &typeMap);
		PassManager passes = {
			new P4::ParseAnnotationBodies(&parseAnnotations, &typeMap),
			new P4::ValidateParsedProgram(),
			new P4::CreateBuiltins(),
			new P4::ResolveReferences(&refMap, true),
			// new P4::ResolveReferences(&refMap),
			// new P4::CheckNamedArgs(),
			new P4::TypeInference(&refMap, &typeMap, false),
			// new P4::DefaultArguments(&refMap, &typeMap),  // add default argument values to parameters
			// new P4::BindTypeVariables(&refMap, &typeMap),
			evaluator};

		auto result = program->apply(passes);
		auto toplevel = evaluator->getToplevelBlock();

		results.push_back(result);
		toplevels.push_back(toplevel);
	}


	// do the merging
	auto merger = new Merger(this);
	results[0] = results[0]->apply(*merger);

	return true;
}



} // namespace
