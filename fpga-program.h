#ifndef _FPGA_BACKEND_PROGRAM_H_
#define _FPGA_BACKEND_PROGRAM_H_

#include "common/resolveReferences/referenceMap.h"
#include "ir/ir-generated.h"
#include "ir/ir.h"
#include "p4/typeMap.h"

//
//
#include "parser-graphs.h"
#include "fpga-model.h"
#include "options.h"

namespace FPGA {

class FPGAProgram;
class FPGAControl;
class FPGATable;

class FPGAProgram {
public:

	const FPGAOptions* options;
	P4::ReferenceMap* refMap;
	P4::TypeMap* typeMap;
	const IR::P4Program* program;
	const IR::ToplevelBlock* toplevel;

	FPGAModel &model;
	FPGAControl *control;

	FPGAProgram(const FPGAOptions *options, P4::ReferenceMap *refMap, P4::TypeMap *typeMap, 
				const IR::P4Program *program,
				const IR::ToplevelBlock *toplevel) : 
				options(options),
				refMap(refMap), 
				typeMap(typeMap), 
				program(program),
				toplevel(toplevel),
				model(FPGAModel::instance) {
	}

	virtual bool build();
	virtual void pr_conf();
};



} // namespace FPGA



#endif // _FPGA_BACKEND_PROGRAM_H_
