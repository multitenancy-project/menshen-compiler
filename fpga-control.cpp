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

bool ControlBodyProcess::preorder(const IR::Declaration_Instance* decl_ins) {
	auto type = decl_ins->type->to<IR::Type_Specialized>();

	if (type == nullptr) {
		return false;
	}

	// check register definition
	if (type->baseType->path->name.name == "register") {
		auto element_type = type->arguments->at(0); // we only have 1 arg

		// check element type
		if (element_type->is<IR::Type_Bits>()) {
			auto bit_type = element_type->to<IR::Type_Bits>();
			if (bit_type->size != 32) {
				BUG("not supported element size %1%", bit_type->size);
			}
		}
		else {
			BUG("not supported type %1%", element_type);
		}
		// check #slots
		auto arg_0 = decl_ins->arguments->at(0);
		if (arg_0->expression->is<IR::Constant>()) {
			auto num_slots = arg_0->expression->to<IR::Constant>()->value.convert_to<int>();
			if (num_slots > MAX_NUM_SLOTS) {
				BUG("not supported #slots %1%", num_slots);
			}
		}
		else {
			BUG("not supported expr %1%", arg_0->expression);
		}

		control->applied_regs.emplace(decl_ins->Name(), type);
	}

	return false;
}

void ControlBodyProcess::processApply(const P4::ApplyMethod* method) {
	auto tbl_name = method->object->getName().name;
	auto table = control->getTable(tbl_name);
	BUG_CHECK(table!=nullptr, "no table for %1%", method->expr);
	if (const_sys_tbls.find(tbl_name) != const_sys_tbls.end()) {
		if (control->program->options->if_sys != -1) {
			control->applied_tables.push_back(table);
		}
	}
	else {
		control->applied_tables.push_back(table);
	}
}

//=====================================================================================

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
	// check how many regs are used
	if (applied_regs.size() > 1) {
		BUG("#applied regs > 1");
	}
	//
	auto ctrlBodyProcess = new ControlBodyProcess(this);
	controlBlock->container->body->apply(*ctrlBodyProcess);
	for (auto ctrlLocal : controlBlock->container->controlLocals) {
		ctrlLocal->apply(*ctrlBodyProcess);
	}

	// get applied tables
	for (auto k : applied_tables) {
		std::cout << k->table->container->getName().name << std::endl;
		// build for each table
		k->build();
	}

	// emit configuration for each table
	int st_stg, nxt_st_stg;
	if (program->options->if_sys != -1) {
		st_stg = 4;
		nxt_st_stg = 5;
	}
	else {
		st_stg = 0;
		nxt_st_stg = -1;
	}
	for (auto k : applied_tables) {
		k->emitConf(stg_conf, st_stg, nxt_st_stg);
		st_stg = nxt_st_stg;
	}
	return true;
}

} // namespace FPGA
