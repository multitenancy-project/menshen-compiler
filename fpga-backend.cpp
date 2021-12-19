#include "fpga-backend.h"
#include "fpga-program.h"
#include "fpga-control.h"
#include "fpga-table.h"

#include "emit-pkt.h"

namespace FPGA {

inline void checkPHVAllocation(std::map<cstring, struct PHVContainer> &hdr_phv_allocation) {
	int phv_allocation[4][9];
	memset(phv_allocation, 0, sizeof(phv_allocation));
	for (auto k : hdr_phv_allocation) {
		auto type = k.second.type;
		auto pos = k.second.pos;
		if (phv_allocation[type][pos] != 0) {
			BUG("PHV allocation wrong");
		}
	}
}

void run_generate_stateful_conf(const FPGAOptions* options) {
	auto emit_pkt_conf = new EmitConfPkt(options->confFilename, options->outputfile);

	emit_pkt_conf->emitStatefulConf();
}

void run_fpga_backend(const FPGAOptions* options, const IR::ToplevelBlock* toplevel,
						P4::ReferenceMap* refMap, P4::TypeMap* typeMap) {

	if (nullptr == toplevel) {
		::error("toplevel null");
		return ;
	}

	auto fpga_program = new FPGAProgram(options, refMap, typeMap, toplevel->getProgram(),
											toplevel);

	fpga_program->build();
	// fpga_program->pr_conf();

	// do phv allocation check
	checkPHVAllocation(fpga_program->control->hdrAccess->hdr_phv_allocation);

	// emit the configuration pkts from build
	auto emit_pkt_conf = new EmitConfPkt(options->if_sys, options->vid, options->confFilename, options->outputfile,
									fpga_program->control->hdrAccess->visited_fields_bitsize_from_start,
									fpga_program->control->hdrAccess->hdr_phv_allocation,
									fpga_program->control->stg_conf);

	emit_pkt_conf->emitConfPkt();
}


} // namespace FPGA
