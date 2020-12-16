#ifndef _BACKENDS_FPGA_CONTROL_H_
#define _BACKENDS_FPGA_CONTROL_H_

#include <map>
#include <set>
#include <vector>

#include "ir/ir.h"

#include "parser-graphs.h"
#include "fpga-table.h"
#include "fpga-program.h"
#include "common.h"

namespace FPGA {

static const int MAX_NUM_SLOTS = 4;

class FPGAControl;

class ControlBodyProcess : public Inspector {
public:
	FPGAControl* control;

	ControlBodyProcess(FPGAControl *ctrl) :
		control(ctrl) {
	}

	bool preorder(const IR::MethodCallExpression* expression) override;
	bool preorder(const IR::Declaration_Instance* decl_ins) override;
	virtual void processApply(const P4::ApplyMethod* method);
};


class FPGAControl {
public:
	const FPGAProgram *program; // fpga program
	const IR::ControlBlock *controlBlock;
	HdrFieldsAccess *hdrAccess;

	std::map<cstring, const IR::Type_Specialized*> applied_regs;
	std::map<cstring, FPGATable*> tables;
	std::vector<FPGATable*> applied_tables;

	FPGAControl(const FPGAProgram *program, const IR::ControlBlock *ctrlBlock,
					HdrFieldsAccess *access);


	FPGATable* getTable(cstring name) const {
		auto res = ::get(tables, name);
		BUG_CHECK(res != nullptr, "No table named %1%", name);
		return res;
	}

	virtual bool build();
	//
	struct StageConf stg_conf[5];
};


} // namespace FPGA





#endif // _BACKENDS_FPGA_CONTROL_H_
