#include <cstdlib>

#include "FPGA-backend/common.h"
#include "ir/ir-generated.h"
#include "lib/exceptions.h"

#include "fpga-table.h"
#include "fpga-control.h"


namespace FPGA {

int ActionBodyProcess::process_expr(const IR::Expression* expr, cstring &field) {
	int val = -1;

	if (expr->is<IR::Member>()) {
		field = expr->to<IR::Member>()->member.name;
	}
	else if (expr->is<IR::PathExpression>()) {
		field = expr->to<IR::PathExpression>()->path->name.name;
	}
	else if (expr->is<IR::Constant>()) {
		val = expr->to<IR::Constant>()->value.convert_to<int>();
	}
	else {
		BUG("not supported %1% expr ", expr);
	}
	return val;
}

void ActionBodyProcess::process_expr(const IR::Operation_Binary* expr, struct Operation &op) {
	auto &hdr_phv_allocation = table->program->control->hdrAccess->hdr_phv_allocation;
	cstring l_field, r_field;
	int l_val, r_val;
	int expr_type;

	const int EXPR_ADD = 1;
	const int EXPR_SUB = 2;

	if (expr->is<IR::Add>()) {
		expr_type = EXPR_ADD;
	}
	else if (expr->is<IR::Sub>()) {
		expr_type = EXPR_SUB;
	}
	else {
		BUG("not supported %1%", expr);
	}

	// process left
	l_val = process_expr(expr->left, l_field);
	if (l_val == -1) {
		auto hdr_itr = hdr_phv_allocation.find(l_field);
		if (hdr_itr != hdr_phv_allocation.end()) {
			op.op_a = hdr_phv_allocation[l_field];
		}
		else {
			BUG("operation not supported");
		}
	}
	else {
			BUG("operation not supported");
	}
	// process right
	// set default type to ADDI or SUBI
	if (expr_type == EXPR_ADD) {
		op.type = OP_ADDI;
	}
	else {
		op.type = OP_SUBI;
	}
	r_val = process_expr(expr->right, r_field);
	if (r_val == -1) {
		auto hdr_itr = hdr_phv_allocation.find(r_field);
		if (hdr_itr != hdr_phv_allocation.end()) {
			op.op_b_type = OP_TYPE_PHV;
			op.phv_op = hdr_phv_allocation[r_field];

			if (expr_type == EXPR_ADD) {
				op.type = OP_ADD;
			}
			else {
				op.type = OP_SUB;
			}
		}
		else {
			op.op_b_type = OP_TYPE_ARG;
			op.arg_op = r_field;
		}
	}
	else {
		op.op_b_type = OP_TYPE_INT;
		op.int_op = r_val;
	}
}

NormalOperation* ActionBodyProcess::process(const IR::AssignmentStatement* assignstat) {
	auto normal_op = new NormalOperation();
	struct Operation op;
	// process left
	cstring l_field;
	auto l_val = process_expr(assignstat->left, l_field);
	if (l_val == -1) {
		auto &hdr_phv_allocation = table->program->control->hdrAccess->hdr_phv_allocation;
		auto hdr_itr = hdr_phv_allocation.find(l_field);
		BUG_CHECK(hdr_itr!=hdr_phv_allocation.end(), "no hdr field exists");
		op.op_res = hdr_phv_allocation[l_field];
	}
	else {
		BUG("not possible left side");
	}
	// process right
	auto r_expr = assignstat->right;
	if (!r_expr->is<IR::Operation_Binary>()) {
		BUG("not supported %1%", r_expr);
	}
	process_expr(r_expr->to<IR::Operation_Binary>(), op);
	normal_op->op = op;
	return normal_op;
}

IfstatementOperation* ActionBodyProcess::process(const IR::IfStatement* ifs) {
	auto if_stat_op = new IfstatementOperation();

	// process condition part
	BUG("not implemented now");

	return if_stat_op;
}

bool ActionBodyProcess::preorder(const IR::BlockStatement *blkstat) {
	for (auto component : blkstat->components) {
		if (component->is<IR::IfStatement>()) {
			operations.push_back(process(component->to<IR::IfStatement>()));
		}
		else if (component->is<IR::AssignmentStatement>()) {
			operations.push_back(process(component->to<IR::AssignmentStatement>()));
		}
		// not supported
		else {
			BUG("%1% in action not supported", component);
		}
	}
	return false;
}

//==============================================================================

FPGATable::FPGATable(const FPGAProgram *program, const IR::TableBlock *table) : 
					program(program), table(table) {
	
	keyGenerator = table->container->getKey();
	actionList = table->container->getActionList();
	entryList = table->container->getEntries();
}

void FPGATable::build() {
	// get table keys
	for (auto key : keyGenerator->keyElements) {
		auto key_expr = key->expression;
		if (key_expr->is<IR::Member>()) {
			keys.push_back(key_expr->to<IR::Member>()->member.name);
		}
		else {
			BUG("not supported key type %1%", key);
		}
	}
	// process entries in tables
	for (auto entry : entryList->entries) {
		// TODO: we do not process arguments now
		auto mce = entry->action->to<IR::MethodCallExpression>()->method;
		auto action_path_expr = mce->to<IR::PathExpression>();
		BUG_CHECK(action_path_expr!=nullptr, "error");
		//
		key_entries.emplace(entry->keys, action_path_expr);
		//
		auto action_decl = program->refMap->getDeclaration(action_path_expr->path, true);
		auto action = action_decl->getNode()->to<IR::P4Action>();
		auto actionBodyProcess = new ActionBodyProcess(this);
		action->apply(*actionBodyProcess);

		act_operations.emplace(action_path_expr, actionBodyProcess->operations);
	}
}

bool FPGATable::emitKeyConf() {
	bool ret = true;
	auto hdr_phv_allocation = program->control->hdrAccess->hdr_phv_allocation;
	int phv_2B_ind, phv_4B_ind, phv_6B_ind;
	phv_2B_ind = phv_4B_ind = phv_6B_ind = 0;

	keyConf.validity_flag = 0;
	for (auto k : keys) {
		auto k_pos = hdr_phv_allocation.find(k);
		if (k_pos == hdr_phv_allocation.end()) {
			ret = false;
			BUG("no hdr phv allocation");
		}

		auto phv_allocation = hdr_phv_allocation[k];
		if (phv_allocation.type == PHV_CON_2B) {
			if (phv_2B_ind==0) {
				keyConf.op_2B_1 = phv_allocation.pos;
				keyConf.validity_flag = keyConf.validity_flag | ((int)1 << 1 & 0b000010);
				phv_2B_ind++;
				keyInConf.emplace(k, 1);
			}
			else if (phv_2B_ind==1) {
				keyConf.op_2B_2 = phv_allocation.pos;
				keyConf.validity_flag = keyConf.validity_flag | ((int)1 << 1 & 0b000001);
				phv_2B_ind++;
				keyInConf.emplace(k, 2);
			}
			else {
				BUG("not enough space for keys");
			}
		}
		else if (phv_allocation.type == PHV_CON_4B) {
			if (phv_4B_ind==0) {
				keyConf.op_2B_1 = phv_allocation.pos;
				keyConf.validity_flag = keyConf.validity_flag | ((int)1 << 2 & 0b001000);
				phv_4B_ind++;
				keyInConf.emplace(k, 1);
			}
			else if (phv_4B_ind==1) {
				keyConf.op_4B_2 = phv_allocation.pos;
				keyConf.validity_flag = keyConf.validity_flag | ((int)1 << 2 & 0b000100);
				phv_4B_ind++;
				keyInConf.emplace(k, 2);
			}
			else {
				BUG("not enough space for keys");
			}
		}
		else if (phv_allocation.type == PHV_CON_6B) {
			if (phv_6B_ind==0) {
				keyConf.op_6B_1 = phv_allocation.pos;
				keyConf.validity_flag = keyConf.validity_flag | ((int)1 << 4 & 0b100000);
				phv_6B_ind++;
				keyInConf.emplace(k, 1);
			}
			else if (phv_6B_ind==1) {
				keyConf.op_6B_2 = phv_allocation.pos;
				keyConf.validity_flag = keyConf.validity_flag | ((int)1 << 4 & 0b010000);
				phv_6B_ind++;
				keyInConf.emplace(k, 2);
			}
			else {
				BUG("not enough space for keys");
			}
		}
		else {
			BUG("not supported phv allocation type");
		}
	}
	keyConf.flag = 1;

	return ret;
}

struct LookupCAMConf FPGATable::emitCAMConf(const IR::ListExpression* list_expr) {
	struct LookupCAMConf camConf;
	memset(&camConf, 0, sizeof(struct LookupCAMConf));
	size_t ind = 0;
	for (auto expr : list_expr->components) {
		uint64_t val = -1;
		if (expr->is<IR::PathExpression>()) {
			auto path_expr = expr->to<IR::PathExpression>();
			auto decl_expr = program->refMap->getDeclaration(path_expr->path);
			if (decl_expr->is<IR::Declaration_Constant>()) {
				// auto const_decl = decl_expr->to<IR::Declaration_Constant>();
				BUG("should not happen here");
			}
			else {
				BUG("not supported expr %1%", decl_expr);
			}
		}
		else if (expr->is<IR::Constant>()) {
			val = expr->to<IR::Constant>()->value.convert_to<int>();
		}
		else {
			BUG("not supported expr %1%", expr);
		}
		auto hdr_field = keys.at(ind);
		auto bitsize = program->control->hdrAccess->hdr_fields_bitsize[hdr_field];
		auto key_pos = keyInConf[hdr_field];
		if (bitsize == 16) {
			if (key_pos == 1) {
				camConf.op_2B_1 = val;
			}
			else if (key_pos == 2) {
				camConf.op_2B_2 = val;
			}
			else {
				BUG("impossible key_pos %d", key_pos);
			}
		}
		else if (bitsize == 32) {
			if (key_pos == 1) {
				camConf.op_4B_1 = val;
			}
			else if (key_pos == 2) {
				camConf.op_4B_2 = val;
			}
			else {
				BUG("impossible key_pos %d", key_pos);
			}
		}
		else if (bitsize == 48) {
			if (key_pos == 1) {
				camConf.op_6B_1 = val;
			}
			else if (key_pos == 2) {
				camConf.op_6B_2 = val;
			}
			else {
				BUG("impossible key_pos %d", key_pos);
			}
		}
		else {
			BUG("impossible bitsize %d", bitsize);
		}
		// 
		ind++;
	}
	camConf.flag = 1;
	return camConf;
}

static inline int get_phv_index(struct PHVContainer &phv_con) {
	int ret = -1;
	if (phv_con.type == PHV_CON_2B) {
		ret = 2*8+(7-phv_con.pos);
	}
	else if (phv_con.type == PHV_CON_4B) {
		ret = 8+(7-phv_con.pos);
	}
	else if (phv_con.type == PHV_CON_6B) {
		ret = 7-phv_con.pos;
	}
	else {
		BUG("no such PHV type");
	}
	return ret;
}

int FPGATable::emitRAMConf(const IR::PathExpression* act, struct StageConf *stg_conf, int st_stg) {
	int ed_stg = st_stg;

	if (act_operations.find(act) == act_operations.end()) {
		BUG("no action operations");
	}
	auto act_ops = act_operations[act];

	int prev_stg_conf[25];
	for (int i=0; i<25; i++) {
		prev_stg_conf[i] = st_stg-1;
	}

	/*
	 *  The logic here is simple,
	 *  every action can span across at-most 5 stages,
	 *  we just keep VLIW at each stage and push them back
	 *  to the stg_conf if it is set
	 */
	std::array<std::array<struct LookupRAMConf, 25>, 5> ram_conf;
	bool modified_stg[5];
	// zero out
	for (size_t i=0; i<ram_conf.size(); i++) {
		for (size_t j=0; j<ram_conf[i].size(); j++) {
			ram_conf[i][j].flag = 0;
		}
	}
	memset(modified_stg, false, sizeof(modified_stg));

	for (auto op : act_ops) {
		if (op->type == TYPE_NORMAL_OP) {
			int res_pos, op_a_pos, op_b_pos=-1;
			int res_stg, op_a_stg, op_b_stg=-1;
			auto normal_op = static_cast<NormalOperation *>(op);
			res_pos = get_phv_index(normal_op->op.op_res);
			res_stg = prev_stg_conf[res_pos]+1;
			op_a_pos = get_phv_index(normal_op->op.op_a);
			op_a_stg = prev_stg_conf[op_a_pos]+1;
			if (normal_op->op.type==OP_ADD ||
					normal_op->op.type==OP_SUB) {
				op_b_pos = get_phv_index(normal_op->op.phv_op);
				op_b_stg = prev_stg_conf[op_b_pos]+1;
			}

			//
			int stg_op = res_stg;
			stg_op = stg_op<op_a_stg?op_a_stg:stg_op;
			stg_op = stg_op<op_b_stg?op_b_stg:stg_op;

			auto one_ram_conf = &ram_conf[stg_op][res_pos];
			modified_stg[stg_op] = true;
			one_ram_conf->flag = 1;
			switch (normal_op->op.type) {
				case OP_ADD:
					one_ram_conf->op_type = OP_ADD_BIN;
					break;
				case OP_SUB:
					one_ram_conf->op_type = OP_SUB_BIN;
					break;
				case OP_ADDI:
					one_ram_conf->op_type = OP_ADDI_BIN;
					break;
				case OP_SUBI:
					one_ram_conf->op_type = OP_SUBI_BIN;
					break;
				default:
					break;
			}
			// op_a is a phv
			one_ram_conf->op_a = normal_op->op.op_a.type << 3 | normal_op->op.op_a.pos;
			// op_b
			if (normal_op->op.op_b_type == OP_TYPE_PHV) {
				one_ram_conf->op_b = (normal_op->op.phv_op.type << 3 | normal_op->op.phv_op.pos)<<11;
			}
			else if (normal_op->op.op_b_type == OP_TYPE_INT) {
				one_ram_conf->op_b = normal_op->op.int_op;
			}
			else {
				BUG("wrong op_b type");
			}
			// update prev stage
			prev_stg_conf[res_pos] = stg_op;
			ed_stg = stg_op;
		}
		else {
			BUG("not supported op now");
		}
	}

	// push to stg_conf
	for (int i=0; i<5; i++) {
		if (modified_stg[i]) {
			stg_conf[i].ramconf.push_back(ram_conf[i]);
		}
	}

	return ed_stg+1;
}


bool FPGATable::emitConf(struct StageConf *stg_conf, int st_stg, int &nxt_st_stg) {
	bool ret = true;

	if (!emitKeyConf()) {
		ret = false;
	}

	if (st_stg > 4) {
		BUG("not feasible start stage");
	}

	for (auto entry : key_entries) {
		// get CAM conf
		auto cam_conf = emitCAMConf(entry.first);
		// get RAM conf
		auto ed_stg = emitRAMConf(entry.second, stg_conf, st_stg);
		if (nxt_st_stg < ed_stg) {
			nxt_st_stg = ed_stg;
		}
		for (auto stg=st_stg; stg<ed_stg; stg++) {
			stg_conf[stg].flag = 1;
			stg_conf[stg].keyconf = keyConf;
			stg_conf[stg].camconf.push_back(cam_conf);
			// RAM conf is already pushed
		}
	}

	return ret;
}


} // namespace FPGA
