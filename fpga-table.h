#ifndef _BACKENDS_FPGA_TABLE_H_
#define _BACKENDS_FPGA_TABLE_H_


#include "fpga-program.h"
#include "ir/ir-generated.h"
#include "ir/ir.h"
#include "ir/visitor.h"

#include "common.h"

namespace FPGA {

class FPGATable;

class ActionBodyProcess : public Inspector {
public:
	FPGATable *table;
	const IR::P4Action *action;
	bool if_in_ifstatement;

	ActionBodyProcess(FPGATable *table) :
		table(table) {
		if_in_ifstatement = false;
		setName("ActionBodyProcess");
	}

	virtual int process_expr(const IR::Expression* expr, cstring &field);
	virtual void process_expr(const IR::Operation_Binary* expr, struct Operation &op);
	virtual IfstatementOperation* process(const IR::IfStatement* ifs);
	virtual NormalOperation* process(const IR::AssignmentStatement* assignstat);

	bool preorder(const IR::BlockStatement* blkstat) override;

	bool preorder(const IR::P4Action *act) override {
		action = act;
		// get parameters
		std::vector<cstring> act_args;
		for (auto it = action->parameters->parameters.begin();
				it != action->parameters->parameters.end(); it++) {
			act_args.push_back((*it)->name.name);
		}
		action_args.emplace(action->name.name, act_args);

		visit(act->body);
		return false;
	}

	// map action (cstring name) to its parameters (cstring names)
	std::map<cstring, std::vector<cstring>> action_args;
	std::vector<BaseOperation *> operations;
};


class FPGATable {
public:
	const FPGAProgram *program;
	const IR::TableBlock *table;

	const IR::Key* keyGenerator;
	const IR::ActionList* actionList;
	const IR::EntriesList* entryList;

	FPGATable(const FPGAProgram *program, const IR::TableBlock *table);
	void build();

	bool emitKeyConf();
	struct LookupCAMConf emitCAMConf(const IR::ListExpression* list_expr);
	int emitRAMConf(const IR::PathExpression* act, struct StageConf *stg_conf, int st_stg);
	bool emitConf(struct StageConf * stg_conf, int st_stg, int &nxt_st_stg);
	// --> generate entry in KeyExtractor
	std::vector<cstring> keys;
	// --> generate entry in CAM in lookup (i.e., Matched Key)
	std::map<const IR::ListExpression*, const IR::PathExpression*> key_entries;
	// --> generate entry in RAM in lookup (i.e., VLIW actions)
	std::map<const IR::PathExpression*, std::vector<BaseOperation *>> act_operations;

	// configurations
	struct KeyExtractConf keyConf;
	std::map<cstring, int> keyInConf; // key to position in conf
};



} // namespace FPGA


#endif // _BACKENDS_FPGA_TABLE_H_
