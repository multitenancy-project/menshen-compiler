#include "options.h"


namespace FPGA {

void FPGAOptions::setInputFile() {
	if (remainingOptions.size() >= 1) {
		for (auto file : remainingOptions) {
			files.push_back(file);
		}
	}
	// we accept zero input files to generate other confs (e.g., stateful conf)
	else {
	}
}

FILE* FPGAOptions::preprocess(cstring file) {
	FILE* in = nullptr;

	if (file == "-") {
		file = "<stdin>";
		in = stdin;
	} else {
#ifdef __clang__
		std::string cmd("cc -E -x c -Wno-comment");
#else
		std::string cmd("cpp");
#endif
		// the p4c driver sets environment variables for include
		// paths.  check the environment and add these to the command
		// line for the preprocessor
		char * driverP4IncludePath =
		isv1() ? getenv("P4C_14_INCLUDE_PATH") : getenv("P4C_16_INCLUDE_PATH");
		cmd += cstring(" -C -undef -nostdinc -x assembler-with-cpp") + " " + preprocessor_options
		+ (driverP4IncludePath ? " -I" + cstring(driverP4IncludePath) : "")
		+ " -I" + (isv1() ? p4_14includePath : p4includePath) + " " + file;
		
		if (Log::verbose())
		std::cerr << "Invoking preprocessor " << std::endl << cmd << std::endl;
		in = popen(cmd.c_str(), "r");
		if (in == nullptr) {
		::error("Error invoking preprocessor");
		perror("");
		return nullptr;
		}
		// close_input = true;
	}
		
	if (doNotCompile) {
		char *line = NULL;
		size_t len = 0;
		ssize_t read;
		
		while ((read = getline(&line, &len, in)) != -1)
		printf("%s", line);
		// closeInput(in);
		closeInput(in, true);
		return nullptr;
	}
	return in;
}

void FPGAOptions::closeInput(FILE* inputStream, bool if_close) const {
	if (if_close) {
		int exitCode = pclose(inputStream);
		if (WIFEXITED(exitCode) && WEXITSTATUS(exitCode) == 4)
			::error("input file %s does not exist", file);
		else if (exitCode != 0)
			::error("Preprocessor returned exit code %d; aborting compilation", exitCode);
	}
}

} // namespace FPGA
