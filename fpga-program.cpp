#include "fpga-program.h"
#include "fpga-control.h"
#include "lib/error.h"


namespace FPGA {


bool FPGAProgram::build() {
	// build parser dependencies
	auto parserGraphs = new ParserGraphs(refMap, typeMap);
	toplevel->getProgram()->apply(*parserGraphs);
	// build accesed hdr and allocate PHV containers
	auto hdrFieldsAccess = new HdrFieldsAccess(parserGraphs->state_extracted_type);
	toplevel->getProgram()->apply(*hdrFieldsAccess);
	hdrFieldsAccess->analyze();
	// build the control part
	auto pack = toplevel->getMain();
	auto cb = pack->findParameterValue(model.sw.ingress.name)->to<IR::ControlBlock>();
	BUG_CHECK(cb!=nullptr, "no control block found");

	control = new FPGAControl(this, cb, hdrFieldsAccess);
	control->build();

	return ::errorCount()==0;
}

// [for debug use]
void FPGAProgram::pr_conf() {
	auto &hdr_allocation = control->hdrAccess->hdr_phv_allocation;
	auto stg_conf = control->stg_conf; // 5 stages
	// print out phv allocation
	for (auto &k : hdr_allocation) {
		std::cout << k.first << " " << k.second << std::endl;
	}

	for (auto i=0; i<5; i++) {
		std::cout << "[[[stage]]] " << i << std::endl;
		std::cout << stg_conf[i];
	}
}

	
} // namespace FPGA
