#ifndef _FPGA_BACKEND_H_
#define _FPGA_BACKEND_H_

#include "common/resolveReferences/referenceMap.h"
#include "ir/ir-generated.h"
#include "ir/ir.h"
#include "p4/typeMap.h"


//
#include "options.h"
#include "fpga-program.h"
#include "fpga-control.h"
#include "fpga-table.h"

namespace FPGA {

class FPGAProgram;
class FPGAControl;
class FPGATable;

void run_generate_stateful_conf(const FPGAOptions& options);

void run_fpga_backend(const FPGAOptions& options, const IR::ToplevelBlock* toplevel,
						P4::ReferenceMap* refMap, P4::TypeMap* typeMap);

} // namspace FPGA


#endif
