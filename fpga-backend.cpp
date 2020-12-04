#include "fpga-backend.h"
#include "fpga-program.h"
#include "fpga-control.h"
#include "fpga-table.h"

#include "emit-pkt.h"

namespace FPGA {


void run_fpga_backend(const FPGAOptions& options, const IR::ToplevelBlock* toplevel,
						P4::ReferenceMap* refMap, P4::TypeMap* typeMap) {

	if (nullptr == toplevel) {
		::error("toplevel null");
		return ;
	}

	auto fpga_program = new FPGAProgram(refMap, typeMap, toplevel->getProgram(),
											toplevel);

	fpga_program->build();
	fpga_program->pr_conf();


	auto emit_pkt_conf = new EmitConfPkt(options.vid, options.confFilename, options.outputfile,
									fpga_program->control->hdrAccess->visited_fields_bitsize_from_start,
									fpga_program->control->hdrAccess->hdr_phv_allocation,
									fpga_program->control->stg_conf);

	emit_pkt_conf->emitConfPkt();
}


} // namespace FPGA
