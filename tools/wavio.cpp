
#include "wavio.h"
#include "wave/file.h"
#include "logger.h"
#include <iostream>
#include <system_error>
#include <filesystem>

std::vector<float> readWav(std::string& filePath)
{
	wave::File read_file;
	wave::Error err = read_file.Open(filePath, wave::kIn);
	if (err) {
		std::cout << "Error, cannot open file: " << filePath << std::endl;
        std::cout << "Error code: " << err << std::endl;
		exit(EXIT_FAILURE);
	}
	std::vector<float> content;
	err = read_file.Read(&content);
	if (err) {
		std::cout << "Error, could not read file: " << filePath << std::endl;
        std::cout << "Error code: " << err << std::endl;
		exit(EXIT_FAILURE);
	}
	std::vector<float> monoContent(content.size()/2, 0.0);
	if (read_file.channel_number() == 2)
	{
		for (int i = 0; i < monoContent.size(); i++)
		{
			monoContent[i] = content[i * 2];
		}
		return monoContent;
	}
	return content;
}

void writeWav(std::vector<float>& audio, std::string& outputFilePath, int sampleRate = 44100, int bitsPerSample = 16)
{

	wave::File  write_file;
    wave::Error err = write_file.Open(outputFilePath, wave::kOut);
	if (err) {
		std::cout << "Error, couldn't open output file: " << outputFilePath << std::endl;
        exit(EXIT_FAILURE);
	}

	write_file.set_sample_rate(sampleRate);
	write_file.set_bits_per_sample(bitsPerSample);
	write_file.set_channel_number(1);

	err = write_file.Write(audio);
	if (err) {
		std::cout << "Error, couldn't write to output file: " << outputFilePath << std::endl;
        exit(EXIT_FAILURE);
	}
}
