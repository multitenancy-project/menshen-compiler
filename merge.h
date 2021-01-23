#ifndef _BACKENDS_FPGA_MERGE_H_
#define _BACKENDS_FPGA_MERGE_H_



#include "FPGA-backend/fpga-model.h"
#include "frontends/common/applyOptionsPragmas.h"
#include "frontends/common/parseInput.h"
#include "frontends/common/resolveReferences/resolveReferences.h"
#include "frontends/p4/typeChecking/bindVariables.h"
#include "frontends/p4/typeMap.h"
#include "frontends/p4/parseAnnotations.h"
#include "frontends/p4/validateParsedProgram.h"
#include "frontends/p4/createBuiltins.h"
#include "frontends/p4/frontend.h"
#include "frontends/p4/evaluator/evaluator.h"
#include "frontends/p4/inlining.h"
#include "frontends/p4/actionsInlining.h"
#include "frontends/p4/functionsInlining.h"
#include "frontends/p4/actionsInlining.h"
#include "frontends/p4/checkConstants.h"
#include "frontends/p4/checkNamedArgs.h"
#include "frontends/p4/createBuiltins.h"
#include "frontends/p4/defaultArguments.h"
#include "frontends/p4/deprecated.h"
#include "frontends/p4/directCalls.h"
#include "frontends/p4/dontcareArgs.h"
#include "frontends/p4/evaluator/evaluator.h"
#include "frontends/common/constantFolding.h"
#include "frontends/p4/functionsInlining.h"
#include "frontends/p4/hierarchicalNames.h"
#include "frontends/p4/inlining.h"
#include "frontends/p4/localizeActions.h"
#include "frontends/p4/moveConstructors.h"
#include "frontends/p4/moveDeclarations.h"
#include "frontends/p4/parseAnnotations.h"
#include "frontends/p4/parserControlFlow.h"
#include "frontends/p4/removeReturns.h"
#include "frontends/p4/resetHeaders.h"
#include "frontends/p4/setHeaders.h"
#include "frontends/p4/sideEffects.h"
#include "frontends/p4/simplify.h"
#include "frontends/p4/simplifyDefUse.h"
#include "frontends/p4/simplifyParsers.h"
#include "frontends/p4/specialize.h"
#include "frontends/p4/strengthReduction.h"
#include "frontends/p4/structInitializers.h"
#include "frontends/p4/tableKeyNames.h"
#include "frontends/p4/toP4/toP4.h"
#include "frontends/p4/typeChecking/typeChecker.h"
#include "frontends/p4/uniqueNames.h"
#include "frontends/p4/unusedDeclarations.h"
#include "frontends/p4/uselessCasts.h"
#include "frontends/p4/validateMatchAnnotations.h"
#include "frontends/p4/validateParsedProgram.h"
#include "ir/ir-generated.h"
#include "midend/expandEmit.h"
#include "ir/visitor.h"
#include "ir/ir.h"
#include "lib/nullstream.h"
#include "lib/gc.h"
#include "lib/error.h"

#include "common.h"

#include "options.h"

#include "fpga-model.h"
#include "fpga-top4.h"

namespace FPGA {

class MergeProgs;


class Merger : public Modifier {
public:
	MergeProgs *mg;

	Merger(MergeProgs *mg) :
		mg(mg) {
	}

	// do some merging here
	bool preorder(IR::P4Control *ctrl) override;
	bool preorder(IR::BlockStatement *blk) override;
};


//
class MergeProgs {
public:

	std::vector<const IR::P4Program *> progs;
	std::vector<const IR::P4Program *> results;
	std::vector<const IR::ToplevelBlock *> toplevels;
	FPGAOptions options;
	FPGAModel &model;


	MergeProgs(std::vector<const IR::P4Program*> progs,
			FPGAOptions options) : 
			progs(progs), 
			options(options),
			model(FPGAModel::instance) {
	}

	bool merge();
	void output() {
		// write to the OUTPUT_FILENAME (merged.p4)
		auto ostream = openFile("merged.p4", false);
		FPGAToP4 top4(ostream, false);
		results[0]->apply(top4);
		ostream->flush();
	}
}; 


} // namespace FPGA



#endif // _BACKENDS_FPGA_MERGE_H_
