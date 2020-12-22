#ifndef _BACKENDS_FPGA_EMITPKT_H_
#define _BACKENDS_FPGA_EMITPKT_H_

#include <cstdint>
#include <cstdio>
#include <utility>
#include <iostream>
#include <fstream>
#include <sys/cdefs.h>

#include "common.h"
#include "options.h"
#include "fpga-program.h"


namespace FPGA {

class EmitConfPkt {
public:
	EmitConfPkt (int vid, cstring filename, cstring outputfile,
					std::map<cstring, int> fields,
					std::map<cstring, struct PHVContainer> allocation,
					struct StageConf stgs[5]) : 
				vid(vid), confFilename(filename), outputfile(outputfile),
				fields_bitsize_from_start(fields),
				hdr_phv_allocation(allocation) {
		memcpy(stg_conf, stgs, 5*sizeof(struct StageConf));

		outStream.open(outputfile, std::ios::out);
		if (!outStream.is_open()) {
			BUG("can not open %1%", outputfile);
		}
	}

	EmitConfPkt (cstring filename, cstring outputfile) :
					confFilename(filename), outputfile(outputfile) {

		outStream.open(outputfile, std::ios::out);
		if (!outStream.is_open()) {
			BUG("can not open %1%", outputfile);
		}
	}

	// functions to generate configuration packets
	virtual void emitConfPkt ();
	virtual void emitParserConf();
	virtual void emitStageConf();

	virtual void buildConfIdx();
	// generate stateful conf only
	virtual void emitStatefulConf();

	int vid;
	cstring confFilename;
	cstring outputfile;
	std::ofstream outStream;
	//
	std::map<int, std::pair<int, int>> lkup_vid_to_idxrange;
	std::map<int, std::pair<int, int>> stateful_vid_to_idxrange;
	// 
	std::map<cstring, int> fields_bitsize_from_start;
	std::map<cstring, struct PHVContainer> hdr_phv_allocation;
	struct StageConf stg_conf[5];
};




} // namespace FPGA




#endif // _BACKENDS_FPGA_EMITPKT_H_
