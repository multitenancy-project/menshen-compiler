#ifndef _FPGA_MODEL_P4_
#define _FPGA_MODEL_P4_

#include "core.p4"

@metadata @name("standard_metadata")
struct standard_metadata_t {
}

extern register<T>
{
	// create a array of 'size' identical elements, each with type T
	register(bit<32> size);
	// load a val
	void load(out T result, in bit<32> index);
	// store 
	void store(in bit<32> index, in T value);
	// load a val and store it back plus 1
	void loadd(out T result, in bit<32> index);
}

// Architecture
// M must be a strcut,
// H must be a strcut where every memeber is of type header

parser Parser<H, M>(packet_in b,
					out H parsedHdr,
					inout M meta,
					inout standard_metadata_t standard_metadata);

control Ingress<H, M>(inout H hdr,
						inout M meta,
						inout standard_metadata_t standard_metadata);

package FpgaSwitch<H, M>(Parser<H, M> p,
						Ingress<H, M> ig);


#endif // _FPGA_MODEL_P4_
