#include "parser-graphs.h"
#include "FPGA-backend/common.h"
#include "ir/ir-generated.h"

namespace FPGA {

static const int NUM_PHV_TYPE = 3;
static const int NUM_PHV_PER_TYPE = 8;
static const int MAX_NUM_PHV_PARSE_ACTIONS = 10;

static bool PHVAllocation(std::map<cstring, int> &visited_hdr_fields,
		std::map<cstring, int> &hdr_fields_bitsize,
		std::map<cstring, struct PHVContainer> &allocation) {

	bool ret = true;
	int cnt = 0;
	uint8_t ind_2B, ind_4B, ind_6B;
	/*
	 * Tao: here we simply alloate PHV from index 0
	 *		but remember that load/store will overwrite
	 *		this allocation since only alu_4B_7 can only
	 *		support stateful memory operations
	 */
	ind_2B = 0;
	ind_4B = 1;
	ind_6B = 0;

	for (auto hdr_field : visited_hdr_fields) {
		cnt++;
		auto bitsize = hdr_fields_bitsize[hdr_field.first];
		if (bitsize == 16) {
			struct PHVContainer con = {PHV_CON_2B, ind_2B};
			allocation.emplace(hdr_field.first, con);
			ind_2B++;
		}
		else if (bitsize == 32) {
			if (hdr_field.first == "ip_dst_addr") {
				// make sure we allocate "ip_dst_addr" to PHV 4B 0
				struct PHVContainer con = {PHV_CON_4B, 0};
				allocation.emplace(hdr_field.first, con);
			}
			else {
				struct PHVContainer con = {PHV_CON_4B, ind_4B};
				allocation.emplace(hdr_field.first, con);
				ind_4B++;
			}
		}
		else if (bitsize == 48) {
			struct PHVContainer con = {PHV_CON_6B, ind_6B};
			allocation.emplace(hdr_field.first, con);
			ind_6B++;
		}
		else {
			BUG("do not support bitsize %1%", bitsize);
		}
	}

	if (cnt>MAX_NUM_PHV_PARSE_ACTIONS ||
			ind_2B > NUM_PHV_PER_TYPE-1 ||
			ind_4B > NUM_PHV_PER_TYPE-1 ||
			ind_6B > NUM_PHV_PER_TYPE-1) {
		ret = false;
	}

	return ret;
}


void ParserGraphs::postorder(const IR::ParserState* state) {
	auto parser = findContext<IR::P4Parser>();
	CHECK_NULL(parser);
	states[parser].push_back(state);
}

void ParserGraphs::postorder(const IR::PathExpression* expression) {
    auto state = findContext<IR::ParserState>();
    if (state != nullptr) {
        auto parser = findContext<IR::P4Parser>();
        CHECK_NULL(parser);
        auto decl = refMap->getDeclaration(expression->path);
        if (decl != nullptr && decl->is<IR::ParserState>()) {
            transitions[parser].push_back(
                new TransitionEdge(state, decl->to<IR::ParserState>()));
        }
	}
}

void ParserGraphs::postorder(const IR::MethodCallStatement* mcs) {
	auto state = findContext<IR::ParserState>();
	if (state != nullptr) {
		auto method = mcs->methodCall->method;
		if (method->is<IR::Member>() && 
				method->to<IR::Member>()->member=="extract") {
			auto argument = mcs->methodCall->arguments->at(0);
			auto extract_type = typeMap->getType(argument);
			// can not be nullptr here
			CHECK_NULL(extract_type->to<IR::Type_Header>());
			state_extracted_type.push_back(extract_type->to<IR::Type_Header>());
		}
	}
}


//====================================================================================
// Tao: get visited header fields, some may be metadata
bool HdrFieldsAccess::preorder(const IR::Member* member) {
	if (visited_fields.find(member->member.name) == visited_fields.end()) {
		visited_fields.insert(member->member.name);
	}
	return false;
}

void HdrFieldsAccess::analyze() {
	// calculate hdr fields and correspoding sizes
	for (auto hdr : extracted_hdrs) {
		for (auto sf : hdr->fields) {
			hdr_fields.push_back(sf->name.name);
			auto type_bits = sf->type->to<IR::Type_Bits>();
			CHECK_NULL(type_bits);
			hdr_fields_bitsize[sf->name.name] = type_bits->size;
		}
	}
	//
	for (auto field : visited_fields) {
		auto pos = std::find(hdr_fields.begin(), hdr_fields.end(), field);
		if (pos != hdr_fields.end()) {
			auto st = hdr_fields.begin();
			int bit_length = 0;
			while (st != pos) {
				bit_length = bit_length+hdr_fields_bitsize[*st];
				st++;
			}
			visited_fields_bitsize_from_start[field] = bit_length;
		}
	}
	//
	auto res = PHVAllocation(visited_fields_bitsize_from_start,
								hdr_fields_bitsize,
								hdr_phv_allocation);

	if (res != true) {
		BUG("no feasible PHV container allocation");
	}
}




} // namespace FPGA

