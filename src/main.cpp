#include "Scanner.hpp"
#include "Parser.hpp"
#include "Preprocessor.hpp"

int main(int argc, char* argv[])
{
	//check number of arguments
	if(argc < 2)
	{
		std::printf("silang [input.sil] [optional: out.silcode]\n");
		return 1;
	}
	
	//open input
	FILE* in = std::fopen(argv[1], "rb");
	if(in == NULL)
	{
		std::printf("error: cannot open input file\n");
		return 1;
	}

	//open output
	FILE* out;
	if(argc == 3)
	{
		//open specified output
		out = std::fopen(argv[2], "wb");
	}
	else
	{
		//open standard output
		out = std::fopen("out.silcode", "wb");
	}
	if(out == NULL)
	{
		std::printf("error: cannot open output file\n");
		return 1;
	}

	//preprocess the input file
	//and output the result into a temporary file
	//which will be used as parser input
	FILE* preprocessed_file = fopen("/tmp/tmp.sil", "w+b");
	if(preprocessed_file == NULL)
	{
		std::printf("error: cannot access /tmp/tmp.sil file\n");
	}
	Preprocessor* preprocessor = new Preprocessor();

	//do the preprocessing
	bool succ = preprocessor->preprocess(in, preprocessed_file);

	//if preprocessor generated no errors we can start parsing
	if(succ == true)
	{
		//reset file seeker
		std::fseek(preprocessed_file, 0, SEEK_SET);

		//start parsing
		Parser* parser = new Parser();
		parser->parse(preprocessed_file, out);
		delete parser;
	}

	//cleanup
	delete preprocessor;

	std::fclose(preprocessed_file);
	std::fclose(in);
	std::fclose(out);
}





