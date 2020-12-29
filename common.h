#ifndef _BACKENDS_FPGA_COMMON_H_
#define _BACKENDS_FPGA_COMMON_H_

#include <ostream>
#define TYPE_IF_OP 1
#define TYPE_NORMAL_OP 2

#define PHV_CON_2B 1
#define PHV_CON_4B 2
#define PHV_CON_6B 3

#define COND_LARGER 1
#define COND_LARGER_EQ 2
#define COND_EQ 3

#define OP_ADD 1
#define OP_SUB 2
#define OP_ADDI 3
#define OP_SUBI 4
#define OP_LOAD 5
#define OP_STORE 6
#define OP_LOADD 7
#define OP_PORT 8
#define OP_DISCARD 9

#define OP_ADD_BIN 0b0001
#define OP_SUB_BIN 0b0010
#define OP_ADDI_BIN 0b1001
#define OP_SUBI_BIN 0b1010
#define OP_LOAD_BIN 0b1011
#define OP_STORE_BIN 0b1000
#define OP_LOADD_BIN 0b0111
#define OP_PORT_BIN 0b1100
#define OP_DISCARD_BIN 0b1101

#define OP_TYPE_PHV 1
#define OP_TYPE_INT 2
#define OP_TYPE_ARG 3
#define OP_TYPE_MD 4

#define MAX_NUM_STAGES 5

#include <sys/types.h>
#include <iostream>
#include <string>

#include "ir/ir.h"

namespace FPGA {

const std::set<cstring> const_std_metadata = {"port","discard"};
const std::set<cstring> const_sys_tbls = {"sys_fwd_on_ip_dst_0"};

struct PHVContainer {
	uint8_t type: 2;
	uint8_t pos : 3;
	friend std::ostream& operator << (std::ostream& os, const struct PHVContainer& phv_con);
};

struct Condition {
	int type;
	int op_a_type;
	union op_a {
		struct PHVContainer phv_op;
		int int_op;
	};
	int op_b_type;
	// for op_b
	struct PHVContainer phv_op;
	int int_op;
};

struct Operation {
	int type;
	int op_res_type;
	struct PHVContainer op_res;
	struct PHVContainer op_a;
	int op_b_type;
	// for op_b
	struct PHVContainer phv_op;
	int int_op;
	cstring arg_op;
	uint32_t metadata_op;
};

class BaseOperation {
public:
	int type; // 
	bool op_valid;

	BaseOperation() {
	}
};

class IfstatementOperation : public BaseOperation {
public:
	IfstatementOperation () {
		type = TYPE_IF_OP;
		op_valid = false;
	}
	struct Condition cond;
	std::vector<BaseOperation *> Iftrue;
	std::vector<BaseOperation *> Iffalse;
};

class NormalOperation : public BaseOperation {
public:
	NormalOperation () {
		type = TYPE_NORMAL_OP;
		op_valid = false;
	}
	struct Operation op;
};

/*
 *  for configuration generation
 */
// Tao: we seperate CondConf and ParserConf here
// TODO: the cond we have now can only support 1 action
struct CondConf {
	uint8_t flag : 1;
	uint8_t type;
	uint8_t op_a;
	uint8_t op_b;
};

struct ParserConf {
	uint8_t flag : 1;
	uint16_t parse_action[10];
};

// 18-bit, each 3-bit represents container index
struct KeyExtractConf {
	uint8_t flag : 1;
	uint8_t validity_flag;
	uint8_t op_2B_1;
	uint8_t op_4B_1;
	uint8_t op_6B_1;
	friend std::ostream& operator << (std::ostream& os, const struct KeyExtractConf& key_con);
};

// (2+4+6)*8*2=192, plus 5-bit conditional flags
struct LookupCAMConf {
	uint8_t flag : 1;
	uint16_t op_2B_1;
	uint32_t op_4B_1;
	uint64_t op_6B_1;
	uint8_t  cond;
	friend std::ostream& operator << (std::ostream& os, const struct LookupCAMConf& cam_conf);
};

// 25*25=625 bit VLIW
struct LookupRAMConf {
	uint8_t flag : 1;
	uint8_t op_type;
	uint8_t op_a; // type+index
	uint16_t op_b;
	friend std::ostream& operator << (std::ostream& os, const struct LookupRAMConf& ram_conf);
};

struct StageConf {
	uint8_t flag : 1; // indicate whether this stage is set or not
	CondConf condconf; // not useful now
	KeyExtractConf keyconf; // only 1 key per stage, limited by our HW design
	std::vector<struct LookupCAMConf> camconf;
	std::vector<std::array<struct LookupRAMConf, 25>> ramconf; // 8*3(6B, 4B, 2B), plus metadata
	friend std::ostream& operator << (std::ostream& os, const struct StageConf& stg_conf);
};

} // namespace FPGA


#endif // _BACKENDS_FPGA_COMMON_H_
