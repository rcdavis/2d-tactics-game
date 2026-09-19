#include "CLI/CLI.hpp"

#include "AssetBuilder.h"

int main(int argc, char** argv) {
	std::string inputDir;
	std::string outputDir;
	std::string generatedDir;

	CLI::App app("Program for building and converting game assets");
	app.set_version_flag("-V,--version", "1.0.0");
	app.add_option("-i,--input", inputDir, "Input directory containing assets")->required();
	app.add_option("-o,--output", outputDir, "Output directory for built assets")->required();
	app.add_option("-g,--generated", generatedDir, "Output directory for generated source code")->required();

	CLI11_PARSE(app, argc, argv);

	AssetBuilder builder;
	builder.BuildAssets(inputDir, outputDir, generatedDir);

	return 0;
}
