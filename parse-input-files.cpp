#include "parse-input-files.h"

namespace FPGA {

template <typename C = P4V1::Converter>
const IR::P4Program* parseFile(cstring file, FPGAOptions& options) {
	FILE* in = nullptr;
	if (options.doNotPreprocess) {
		in = fopen(file, "r");
		if (in == nullptr) {
			::error("%s: No such file or directory.", file);
			return nullptr;
		}
	} else {
		in = options.preprocess(file);
		if (::errorCount() > 0 || in == nullptr)
			return nullptr;
	}

	auto result = options.isv1()
		? P4::parseV1Program<FILE*, C>(in, file, 1, options.getDebugHook())
		: P4::P4ParserDriver::parse(in, file);
	options.closeInput(in, true);

	if (::errorCount() > 0) {
		::error("%1% errors encountered, aborting compilation", ::errorCount());
		return nullptr; 
	}
	BUG_CHECK(result != nullptr, "Parsing failed, but we didn't report an error");
	return result;
}

std::vector<const IR::P4Program*> parseInputFiles(FPGAOptions& options) {
	BUG_CHECK(&options == &P4CContext::get().options(),
			"Parsing using options that don't match the current "
			"compiler context");
	std::vector<const IR::P4Program*> ret;
	for (auto &file : options.files) {
		auto result = parseFile(file, options);
		ret.push_back(result);
	}
	return ret;
}

} // namespace FPGA
