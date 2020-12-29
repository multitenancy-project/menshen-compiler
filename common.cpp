#include <iostream>

#include "common.h"


namespace FPGA {

std::ostream& operator<<(std::ostream& os, const struct PHVContainer& phv_con)
{
	os << "phv_con type: " << unsigned(phv_con.type) << " pos: " << unsigned(phv_con.pos);
	return os;
}

std::ostream& operator << (std::ostream& os, const struct KeyExtractConf& key_con) {
	if (key_con.flag) {
		if (key_con.validity_flag & 0b000001)
			os << "2B 1 valid\n";
		if (key_con.validity_flag & 0b000010)
			os << "4B 1 valid\n";
		if (key_con.validity_flag & 0b000100)
			os << "6B 1 valid\n";
		os << "2B selected: " << +key_con.op_2B_1 << std::endl;
		os << "4B selected: " << +key_con.op_4B_1 << std::endl;
		os << "6B selected: " << +key_con.op_6B_1 << std::endl;
	}
	return os;
}

std::ostream& operator << (std::ostream& os, const struct LookupCAMConf& cam_conf) {
	if (cam_conf.flag) {
		os << "2B val first: " << cam_conf.op_2B_1 << std::endl;
		os << "4B val first: " << cam_conf.op_4B_1 << std::endl;
		os << "6B val first: " << cam_conf.op_6B_1 << std::endl;
	}
	return os;
}


std::ostream& operator << (std::ostream& os, const struct LookupRAMConf& ram_conf) {
	if (ram_conf.flag) {
		switch (ram_conf.op_type) {
			case OP_ADD_BIN: os << "ADD\n"; break;
			case OP_SUB_BIN: os << "SUB\n"; break;
			case OP_ADDI_BIN: os << "ADDI\n"; break;
			case OP_SUBI_BIN: os << "SUBI\n"; break;
			case OP_LOAD_BIN: os << "LOAD\n"; break;
			case OP_STORE_BIN: os << "STORE\n"; break;
			case OP_LOADD_BIN: os << "LOADD\n"; break;
			case OP_PORT_BIN: os << "PORT\n"; break;
			case OP_DISCARD_BIN: os << "DISCARD\n"; break;
		}
		if (ram_conf.op_type == OP_PORT_BIN ||
				ram_conf.op_type == OP_DISCARD_BIN) {
			// print nothing now
		}
		else {
			auto pos_1 = ram_conf.op_a & 0x7;
			auto type_1 = (ram_conf.op_a >> 3) & 0x3;
			os << "opernd A, type: " << type_1 << " pos :" << pos_1 << std::endl;
			auto pos_2 = (ram_conf.op_b >> 11) & 0x7;
			auto type_2 = (ram_conf.op_b >> 14) & 0x3;
			switch (ram_conf.op_type) {
				case OP_SUB_BIN:
				case OP_ADD_BIN: os << "opernd B, type: " << type_2 << " pos :" << pos_2 << std::endl; break;
				case OP_SUBI_BIN:
				case OP_ADDI_BIN: os << "operand B, val: " << +ram_conf.op_b << std::endl; break;
			}
		}
	}
	return os;
}

std::ostream& operator << (std::ostream& os, const struct StageConf& stg_conf) {
	if (stg_conf.flag) {
		os << "KeyExtractConf\n" << stg_conf.keyconf;
		os << "CAMConf\n";
		for (auto cam_conf : stg_conf.camconf) {
			os << cam_conf;
		}
		os << "RAMConf\n";
		for (auto ram_conf : stg_conf.ramconf) {
			int ind = 0;
			for (auto per_phv_conf : ram_conf) {
				if (per_phv_conf.flag) {
					os << "phv_ind " << ind << std::endl;
					os << per_phv_conf;
				}
				ind++;
			}
		}
	}
	return os;
}

} // namespace FPGA
