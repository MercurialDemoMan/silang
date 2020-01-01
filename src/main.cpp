#include "Scanner.hpp"
#include "Parser.hpp"
#include "Preprocessor.hpp"

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		return 0;
	}
	
	FILE* in = fopen(argv[1], "rb");
	if(in == NULL)
	{
		return 0;
	}
	FILE* out = fopen(argv[2], "wb");
	if(out == NULL)
	{
		return 0;
	}

	Preprocessor* preprocessor = new Preprocessor();

	FILE* preprocessed_file = fopen("/tmp/tmp.sil", "w+b");

	if(preprocessed_file == NULL)
	{
		std::printf("FUCK\n");
	}

	bool succ = preprocessor->preprocess(in, preprocessed_file);

	fseek(preprocessed_file, 0, SEEK_SET);

	if(succ == true)
	{
		Parser* parser = new Parser();
		parser->parse(preprocessed_file, out);
		delete parser;
	}

	delete preprocessor;

	fclose(preprocessed_file);
	fclose(in);
	fclose(out);
}