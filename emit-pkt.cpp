#include "emit-pkt.h"
#include "FPGA-backend/common.h"
#include "lib/exceptions.h"


namespace FPGA {

static const int MAX_NUM_PARSE_ACTIONS = 10;
static const int MODULE_PARSER = 0;
static const int MODULE_EXTRACTOR = 1;
static const int MODULE_LOOKUP = 2;
static const int MODULE_ACTION_ENGINE = 3;
static const int MODULE_DEPARSER = 5;

// some shit helper functions

static void printBits(size_t const size, void const * const ptr, std::ostream &os) {
	unsigned char *b = (unsigned char*) ptr;
	unsigned char byte;
	int i, j;
	for (i = size-1; i >= 0; i--) {
		for (j = 7; j >= 0; j--) {
			byte = (b[i] >> j) & 1;
			os << +byte;
		}
	}
}

/*
 *  3B
 *  1B -- module ID [7:3] --> stage num, [2:0] --> module ID
 *  1B -- 4b mode, 4b reserved (used for two different components)
 *  1B -- index
 */

static void printStageInd(std::ostream &os, int module, int stg_num, int entry_ind, 
							int if_mask, int if_ram) {
	uint8_t module_id = (((uint8_t)stg_num)<<3 & 0b11111000) | (((uint8_t)module) & 0b00000111);
	uint8_t reserved = 0;
	uint8_t index = entry_ind;

	if (MODULE_EXTRACTOR==module && if_mask) {
		reserved = 0xf;
	}

	if (MODULE_LOOKUP==module && if_ram) {
		reserved = 0xf;
	}

	// output
	int j;
	unsigned char *b;
	unsigned char byte;
	b = &module_id;
	for (j=7; j>=0; j--) {
		byte = (b[0] >> j) & 1;
		os << +byte;
	}
	b = &reserved;
	for (j=7; j>=0; j--) {
		byte = (b[0] >> j) & 1;
		os << +byte;
	}
	b = &index;
	for (j=7; j>=0; j--) {
		byte = (b[0] >> j) & 1;
		os << +byte;
	}
	if (MODULE_PARSER==module)
		os << " ";
	else 
		os << std::endl;
}

static void printCondConf(struct CondConf &cond_conf, std::ostream &os) { 
	int j;
	unsigned char *b;
	unsigned char byte;
	// print type
	b = &cond_conf.type;
	for (j=3; j>=0; j--) {
		byte = (b[0] >> j) & 1;
		os << +byte;
	}

	b = (unsigned char *)&cond_conf.op_a;
	for (j=7; j>=0; j--) {
		byte = (b[0] >> j) & 1;
		os << +byte;
	}

	b = (unsigned char *)&cond_conf.op_b;
	for (j=7; j>=0; j--) {
		byte = (b[0] >> j) & 1;
		os << +byte;
	}
}

static void printExtractorConf(struct KeyExtractConf &key_conf, std::ostream &os, int stage, int vid) {
	
	int j;
	unsigned char *b;
	unsigned char byte;
	os << "KeyExtractConf ";
	printStageInd(os, MODULE_EXTRACTOR, stage, vid, 0, 0);
	b = &key_conf.op_6B_1;
	for (j=2; j>=0; j--) {
		byte = (b[0] >> j) & 1;
		os << +byte;
	}
	b = &key_conf.op_6B_2;
	for (j=2; j>=0; j--) {
		byte = (b[0] >> j) & 1;
		os << +byte;
	}
	b = &key_conf.op_4B_1;
	for (j=2; j>=0; j--) {
		byte = (b[0] >> j) & 1;
		os << +byte;
	}
	b = &key_conf.op_4B_2;
	for (j=2; j>=0; j--) {
		byte = (b[0] >> j) & 1;
		os << +byte;
	}
	b = &key_conf.op_2B_1;
	for (j=2; j>=0; j--) {
		byte = (b[0] >> j) & 1;
		os << +byte;
	}
	b = &key_conf.op_2B_2;
	for (j=2; j>=0; j--) {
		byte = (b[0] >> j) & 1;
		os << +byte;
	}
	os << "000000\n"; // append to 24b = 3B

	// Mask conf
	os << "CAMMaskConf ";
	printStageInd(os, MODULE_EXTRACTOR, stage, vid, 1, 0);
	if (key_conf.validity_flag & 0b100000) 
		os << "000000000000000000000000000000000000000000000000";
	else
		os << "111111111111111111111111111111111111111111111111";
	if (key_conf.validity_flag & 0b010000)
		os << "000000000000000000000000000000000000000000000000";
	else
		os << "111111111111111111111111111111111111111111111111";
	if (key_conf.validity_flag & 0b001000)
		os << "00000000000000000000000000000000";
	else 
		os << "11111111111111111111111111111111";
	if (key_conf.validity_flag & 0b000100)
		os << "00000000000000000000000000000000";
	else 
		os << "11111111111111111111111111111111";
	if (key_conf.validity_flag & 0b000010)
		os << "0000000000000000";
	else 
		os << "1111111111111111";
	if (key_conf.validity_flag & 0b000001)
		os << "0000000000000000";
	else 
		os << "1111111111111111";
	os << "11111"; // last 5 cond bits
	os << "000\n"; // append to 200bit = 25B
}

static void printCAMConf(struct LookupCAMConf &cam_conf, std::ostream &os, int stage, 
							int entry_ind, uint32_t vid) {
	int i, j;
	unsigned char *b;
	unsigned char byte;
	os << "CAMConf ";
	printStageInd(os, MODULE_LOOKUP, stage, entry_ind, 0, 0);
	// print vid, 4 bit
	b = (unsigned char *)&vid;
	for (i=3; i>=0; i--) {
		byte = (b[0] >> i) & 1;
		os << +byte;
	}
	// print data
	b = (unsigned char *)&cam_conf.op_6B_1;
	for (i=5; i>=0; i--) {
		for(j=7; j>=0; j--) {
			byte = (b[i] >> j) & 1;
			os << +byte;
		}
	}
	b = (unsigned char *)&cam_conf.op_6B_2;
	for (i=5; i>=0; i--) {
		for(j=7; j>=0; j--) {
			byte = (b[i] >> j) & 1;
			os << +byte;
		}
	}
	printBits(sizeof(uint32_t), &cam_conf.op_4B_1, os);
	printBits(sizeof(uint32_t), &cam_conf.op_4B_2, os);
	printBits(sizeof(uint16_t), &cam_conf.op_2B_1, os);
	printBits(sizeof(uint16_t), &cam_conf.op_2B_2, os);
	os << "11111"; // default cond configurations
	os << "0000000\n"; // append to 208bit = 26B
}

/*
 *  type 4, op_a 5, op_b 16
 *  25b in total
 */
static void printPerRAMConf(struct LookupRAMConf &ram_conf, std::ostream &os) {
	int j;
	unsigned char *b;
	unsigned char byte;
	b = (unsigned char *)&ram_conf.op_type;
	for (j=3; j>=0; j--) {
		byte = (b[0] >> j) & 1;
		os << +byte;
	}
	b = (unsigned char *)&ram_conf.op_a;
	for (j=4; j>=0; j--) {
		byte = (b[0] >> j) & 1;
		os << +byte;
	}
	b = (unsigned char *)&ram_conf.op_b;
	printBits(sizeof(uint16_t), b, os);
}

/*
 *  25*25=625b in total
 */
static void printRAMConf(std::array<struct LookupRAMConf, 25> &ram, 
							std::ostream &os,
							int stg,
							int entry_ind) {
	struct LookupRAMConf empty_ram_conf;
	memset(&empty_ram_conf, 0, sizeof(empty_ram_conf));
	empty_ram_conf.flag = 0;
	empty_ram_conf.op_type = 0b1111;
	empty_ram_conf.op_a = empty_ram_conf.op_b = 0;
	os << "RAMConf ";
	printStageInd(os, MODULE_LOOKUP, stg, entry_ind, 0, 1);
	for (auto &ram_conf : ram) {
		if (ram_conf.flag) {
			printPerRAMConf(ram_conf, os);
		}
		else {
			printPerRAMConf(empty_ram_conf, os);
		}
	}
	os << "0000000\n"; // append to 632bit = 79B
}


/*
 *  Parser action format, 16 bit each
 *  [15:13] reserved
 *  [12:6]	byte number for 0
 *  [5:4]	container type, 01 for 2B, 10 for 4B, 11 for 6B
 *  [3:1]	pos
 *  [0]		validity bit
 *
 *  16*10+20*5 = 260
 */

void EmitConfPkt::emitParserConf() {
	struct ParserConf par_conf;
	// zero out 
	for (auto i=0; i<MAX_NUM_PARSE_ACTIONS; i++) {
		par_conf.parse_action[i] = 0;
	}

	int par_ind = 0;
	outStream << "Parser ";
	printStageInd(outStream, MODULE_PARSER, 0, vid, 0, 0);
	printStageInd(outStream, MODULE_DEPARSER, 0, vid, 0, 0);
	for (auto k : hdr_phv_allocation) {
		if (fields_bitsize_from_start.find(k.first) ==
				fields_bitsize_from_start.end()) {
			BUG("no such field %1%", k.first);
		}

		int bytes_from_zero = fields_bitsize_from_start[k.first]/8;
		auto par_act = 0b1;
		uint8_t phv_pos = k.second.pos;
		uint8_t phv_type = k.second.type;

		par_act = par_act | (bytes_from_zero<<6 & 0b0001111111000000)
					| (phv_type << 4 & 0b00110000)
					| (phv_pos << 1 & 0b00001110);

		par_conf.parse_action[par_ind++] = par_act;
	}

	// output to outStream
	for (auto i=0; i<MAX_NUM_PARSE_ACTIONS; i++) {
		printBits(sizeof(uint16_t), &par_conf.parse_action[i], outStream);
	}

	// default cond configuration, since we do not use it now
	struct CondConf cond;
	cond.type = 3;
	cond.op_a = cond.op_b = 0;
	printCondConf(cond, outStream);
	printCondConf(cond, outStream);
	printCondConf(cond, outStream);
	printCondConf(cond, outStream);
	printCondConf(cond, outStream);
	outStream << "0000\n"; // append to 264b = 33B
}

void EmitConfPkt::emitStageConf() {
	// get idx allocation for one vid
	if (lkup_vid_to_idxrange.find(vid) == lkup_vid_to_idxrange.end()) {
		BUG("no conf for vid %1%", vid);
	}
	int stg_idx = lkup_vid_to_idxrange[vid].first;
	int stg_max = lkup_vid_to_idxrange[vid].second;
	int stg_ind[5];
	for (int i=0; i<5; i++) {
		stg_ind[i] = stg_idx;
	}
	for(auto stg=0; stg<5; stg++) {
		if (stg_conf[stg].flag) { // valid stg conf
			if (if_sys != -1) {
				for (auto lkup_vid : lkup_vid_to_idxrange) {
					printExtractorConf(stg_conf[stg].keyconf, outStream, stg, lkup_vid.first);
				}
			}
			else {
				printExtractorConf(stg_conf[stg].keyconf, outStream, stg, vid);
			}
			if (stg_conf[stg].camconf.size() != stg_conf[stg].ramconf.size()) {
				BUG("size of cam and ram conf should be equal stg[%1%], %2%, %3%",
						stg,
						stg_conf[stg].camconf.size(),
						stg_conf[stg].ramconf.size());
			}
			for (size_t i=0; i<stg_conf[stg].camconf.size(); i++) {
				printCAMConf(stg_conf[stg].camconf.at(i), outStream, stg, stg_ind[stg], vid);
				printRAMConf(stg_conf[stg].ramconf.at(i), outStream, stg, stg_ind[stg]);
				stg_ind[stg]++;
				if (stg_ind[stg] > stg_max) {
					BUG("stg ind larger than allocated %1%", stg_max);
				}
			}
		}
	}
}

void EmitConfPkt::buildConfIdx() {
	std::ifstream confin;
	confin.open(confFilename, std::ios::in);
	if (!confin.is_open()) {
		BUG("can not open conf file %1%", confFilename);
	}

	std::string type;
	int n, vid, st, offset;

	while (confin >> type) {
		if (type == "lkupconf") {
			confin >> n;
			for (int i=0; i<n; i++) {
				confin >> vid >> st >> offset;
				lkup_vid_to_idxrange.emplace(vid, std::make_pair(st, offset));
			}
		}
		else if (type == "statefulmem") {
			confin >> n;
			for (int i=0; i<n; i++) {
				confin >> vid >> st >> offset;
				stateful_vid_to_idxrange.emplace(vid, std::make_pair(st, offset));
			}
		}
		else {
			BUG("not supported conf %1%", type);
		}
	}

	confin.close();
}

void EmitConfPkt::emitStatefulConf() {
	// first read index info
	buildConfIdx();
	for (auto k : stateful_vid_to_idxrange) {
		uint8_t st, offset;
		outStream << "StatefulIdxConf ";
		printStageInd(outStream, MODULE_ACTION_ENGINE, 2, k.first, 0, 0);
		st = k.second.first;
		offset = k.second.second;
		printBits(sizeof(uint8_t), &offset, outStream);
		printBits(sizeof(uint8_t), &st, outStream);
		outStream << "\n";
	}

	outStream.close();
}

void EmitConfPkt::emitConfPkt() {
	// first build index for lkup cam and ram
	buildConfIdx();
	// then emit
	if (if_sys == -1) {
		emitParserConf();
	}
	emitStageConf();

	//
	outStream.close();
}


} // namespace FPGA
