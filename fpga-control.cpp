#include "fpga-control.h"
#include "FPGA-backend/common.h"
#include "FPGA-backend/parser-graphs.h"
#include "ir/visitor.h"
#include "lib/exceptions.h"
#include "p4/methodInstance.h"
#include <math.h>
#include <unistd.h>


namespace FPGA {

bool ControlBodyProcess::preorder(const IR::MethodCallExpression* expression) {
	auto mi = P4::MethodInstance::resolve(expression, control->program->refMap, 
													control->program->typeMap);
	auto apply = mi->to<P4::ApplyMethod>();
	if (apply != nullptr) {
		processApply(apply);
		return false;
	}
	::error(ErrorType::ERR_UNSUPPORTED, "Unsupported method invocation %1%", expression);
	return false;
}

void ControlBodyProcess::processApply(const P4::ApplyMethod* method) {
	auto table = control->getTable(method->object->getName().name);
	BUG_CHECK(table!=nullptr, "no table for %1%", method->expr);
	control->applied_tables.push_back(table);
}



FPGAControl::FPGAControl(const FPGAProgram* program, const IR::ControlBlock* ctrlBlock, 
							HdrFieldsAccess* access) :
				program(program), controlBlock(ctrlBlock), hdrAccess(access) {

	for (auto c : controlBlock->constantValue) {
		auto b = c.second;
		if (!b->is<IR::Block>()) continue;

		if (b->is<IR::TableBlock>()) {
			auto tblblk = b->to<IR::TableBlock>();
			auto tbl = new FPGATable(program, tblblk);
			tables.emplace(tblblk->container->name, tbl);
		}
	}
}

bool FPGAControl::build() {
	auto ctrlBodyProcess = new ControlBodyProcess(this);
	controlBlock->container->body->apply(*ctrlBodyProcess);
	
	for (auto k : applied_tables) {
		std::cout << k->table->container->getName().name << std::endl;
		k->build();
	}

	int st_stg = 0;
	int nxt_st_stg = -1;
	for (auto k : applied_tables) {
		k->emitConf(stg_conf, st_stg, nxt_st_stg);
		st_stg = nxt_st_stg;
	}
	return true;
}

} // namespace FPGA
