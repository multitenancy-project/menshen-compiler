#ifndef _BACKENDS_FPGA_PARSER_GRAPHS_H_
#define _BACKENDS_FPGA_PARSER_GRAPHS_H_



#include "frontends/common/resolveReferences/referenceMap.h"
#include "ir/ir-generated.h"
#include "ir/visitor.h"
#include "ir/ir.h"
#include "lib/cstring.h"
#include "lib/nullstream.h"
#include "lib/path.h"
#include "lib/safe_vector.h"
#include "p4/typeMap.h"

#include "common.h"
#include "options.h"

namespace FPGA {

class ParserGraphs : public Inspector {
	P4::ReferenceMap* refMap;
	P4::TypeMap*		typeMap;

protected:
	struct TransitionEdge {
		const IR::ParserState* srcState;
		const IR::ParserState* dstState;

		TransitionEdge (const IR::ParserState* src,
						const IR::ParserState* dst) :
							srcState(src), dstState(dst) {
		}
	};

	std::map<const IR::P4Parser*, safe_vector<const TransitionEdge*>> transitions;
	std::map<const IR::P4Parser*, safe_vector<const IR::ParserState*>> states;

public:

	ParserGraphs(P4::ReferenceMap *refMap, P4::TypeMap *typeMap) :
					refMap(refMap), typeMap(typeMap) {
		CHECK_NULL(refMap);
		CHECK_NULL(typeMap);
		setName("ParserGraphs");
	}

	void postorder(const IR::ParserState* state) override;
	void postorder(const IR::PathExpression* expression) override;
	void postorder(const IR::MethodCallStatement* mcs) override;

	// 
	std::vector<const IR::Type_Header*> state_extracted_type;
};

//====================================================================================

class HdrFieldsAccess : public Inspector {
public:
	HdrFieldsAccess(const FPGAOptions* options, 
					std::vector<const IR::Type_Header*> extracted_hdrs) :
					 	options(options), extracted_hdrs(extracted_hdrs) {
		setName("HdrFieldsAccess");
	}

	bool preorder(const IR::Member* member) override;
	void analyze();
	//
	const FPGAOptions* options;
	std::vector<const IR::Type_Header*> extracted_hdrs;
	std::set<cstring> visited_fields;
	std::map<cstring, int> visited_fields_bitsize_from_start;
	std::vector<cstring> hdr_fields;
	std::map<cstring, int> hdr_fields_bitsize;

	std::map<cstring, struct PHVContainer> hdr_phv_allocation;
};



} // namespace FPGA




#endif
