#ifndef _BACKENDS_FPGA_PARSE_INPUT_FILES_H_
#define _BACKENDS_FPGA_PARSE_INPUT_FILES_H_

#include "frontends/common/options.h"
#include "frontends/parsers/parserDriver.h"
#include "frontends/p4/fromv1.0/converters.h"
#include "frontends/p4/frontend.h"
#include "frontends/common/parseInput.h"
#include "lib/error.h"
#include "lib/source_file.h"

#include "options.h"


namespace FPGA {

template <typename C = P4V1::Converter>
const IR::P4Program* parseFile(cstring file, FPGAOptions& options);

std::vector<const IR::P4Program*> parseInputFiles(FPGAOptions& options);

} // namespace FPGA



#endif // _BACKENDS_FPGA_PARSE_INPUT_FILES_H_
