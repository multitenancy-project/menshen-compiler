#ifndef _FPGA_MODEL_H_
#define _FPGA_MODEL_H_

#include "lib/cstring.h"
#include "frontends/common/model.h"
#include "frontends/p4/coreLibrary.h"
#include "frontends/p4/methodInstance.h"
#include "ir/ir.h"
#include "lib/json.h"


namespace FPGA {
struct Parser_Model : public ::Model::Elem {
	Parser_Model(Model::Type_Model headersType, Model::Type_Model userMetaType,
			Model::Type_Model standardMetadataType) :
		Model::Elem("ParserImpl"),
		packetParam("packet", P4::P4CoreLibrary::instance.packetIn, 0),
		headersParam("hdr", headersType, 1),
		metadataParam("meta", userMetaType, 2),
		standardMetadataParam("standard_metadata", standardMetadataType, 3) {
	}
	::Model::Param_Model packetParam;
	::Model::Param_Model headersParam;
	::Model::Param_Model metadataParam;
	::Model::Param_Model standardMetadataParam;
};

struct StandardMetadataType_Model : public ::Model::Type_Model {
	explicit StandardMetadataType_Model(cstring name) :
		::Model::Type_Model(name), dropBit("drop"),
		egress_spec("egress_spec") {
	}
	::Model::Elem dropBit;
	::Model::Elem egress_spec;
};

struct Control_Model : public ::Model::Elem {
	Control_Model(cstring name, Model::Type_Model headersType, Model::Type_Model metadataType,
			Model::Type_Model standardMetadataType) :
		Model::Elem(name),
		headersParam("hdr", headersType, 0),
		metadataParam("meta", metadataType, 1),
		standardMetadataParam("standard_metadata", standardMetadataType, 2) {
	}
	::Model::Param_Model headersParam;
	::Model::Param_Model metadataParam;
	::Model::Param_Model standardMetadataParam;
};

struct Switch_Model : public ::Model::Elem {
	Switch_Model() : 
		Model::Elem("FpgaSwitch"),
		parser("p"),
		ingress("ig") {
	}
	::Model::Elem parser;
	::Model::Elem ingress;
};

class FPGAModel : public ::Model::Model {
protected:
	FPGAModel() :
		Model::Model("0.1"), file("fpga.p4"),
		standardMetadata("standard_metadata"),
		headersType("headers"),
		metadataType("metadata"),
		standardMetadataType("standard_metadata_t"),
		parser(headersType, metadataType, standardMetadataType),
		ingress("ingress", headersType, metadataType, standardMetadataType),
		sw() {
	}
public:
	unsigned version = 20201120;
	::Model::Elem						file;
	::Model::Elem						standardMetadata;
	::Model::Type_Model					headersType;
	::Model::Type_Model 				metadataType;
	StandardMetadataType_Model			standardMetadataType;
	Parser_Model						parser;
	Control_Model       				ingress;
	Switch_Model						sw;


	static FPGAModel instance;
};


} // namespae FPGA

#endif
