
#include "DSPConfig.h"
#include "wavio.h"
#include "main.h"
#include <iostream>
#include <system_error>
#include "wave/file.h"


// Find the file size 
int getFileSize(FILE *inFile){
    int fileSize = 0;
    fseek(inFile,0,SEEK_END);
    fileSize=ftell(inFile);
    fseek(inFile,0,SEEK_SET);
    return fileSize;
}

// If audio is stereo separate the channels
void separateChannels(int16_t* audio, int16_t* left_channel,/* int16_t* right_channel,*/ unsigned long size){

    for (unsigned int i = 0; i < size/4; i++){
        left_channel[i] = audio[2*i];
        // right_channel[i] = audio[2*i + 1];
    }
    return;
}

// Make header from wav file contain mono information
wav_hdr makeHeaderMono(wav_hdr header){

    wav_hdr outWavHeader;
    outWavHeader = header;
    outWavHeader.NumOfChan = 1;
    outWavHeader.Subchunk2Size = header.Subchunk2Size/2;
    outWavHeader.bytesPerSec = header.bytesPerSec/2;
    outWavHeader.blockAlign = header.blockAlign/2;
    outWavHeader.ChunkSize = 36 + outWavHeader.Subchunk2Size;

    return outWavHeader;
}

// Print header information
void printHeader(wav_hdr wavHeader){

    printf("Data size: %d.\n", wavHeader.ChunkSize);
    printf("Sampling Rate              : %d Hz. \n", wavHeader.SamplesPerSec);
    printf("bits/sample                : %d. \n", wavHeader.bitsPerSample);
    printf("Number of channels         : %d. \n", wavHeader.NumOfChan);
    printf("Number of bytes per second : %d. \n", wavHeader.bytesPerSec);
    printf("Data length                : %d. \n", wavHeader.Subchunk2Size);
    printf("Audio Format               : %d. \n", wavHeader.AudioFormat);
    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM 

    printf("Block align                : %d. \n", wavHeader.blockAlign);
    printf("**************************************\n");
}

float* readWav(uint32_t *numSamp, char* filePath)
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
    int numChannels = read_file.channel_number();
    if (numChannels == 2)
    {
        *numSamp = *numSamp/2;
        PRINT_LOG1("Converting stereo to mono.\n");
        // Average the two channels
        for (size_t k = 0; k < content.size(); k += 2)
        {
            content[k] = (content[k] + content[k + 1]) / 2;
        }
        *numSamp = content.size()/2;
    }
    else if(numChannels > 2)
    {
        std::cout << "Error: Unsupported number of channels: " << numChannels << std::endl;
		exit(EXIT_FAILURE);
    }
    float* audio = (float*) calloc(*numSamp, sizeof(float));
	for (uint32_t i = 0; i < *numSamp; i++) {
		audio[i] = (numChannels == 2) ? content[i*2] : content[i];
	}
	PRINT_LOG2("Number of samples: %i\n", *numSamp);
    return audio;
}

#ifndef USE_WAVE_LIBRARY
void writeWav(float* audio, char* inputFilePath, char* outputFilePath, uint32_t numSamp) {
    
    wav_hdr wavHeader;
    wav_hdr outWavHeader;
    FILE *wavFile;
    FILE *outWavFile;
    int headerSize = sizeof(wav_hdr);

    wavFile = fopen( inputFilePath , "r" );

    if(wavFile == NULL){
        printf("Couldn't open wave file\n");
        exit(EXIT_FAILURE);
    }

    // Read the header
#ifdef DEBUG
    size_t bytesRead = fread(&wavHeader,headerSize,1,wavFile);
#else
    fread(&wavHeader,headerSize,1,wavFile);
#endif

    wavHeader.Subchunk2Size = wavHeader.ChunkSize - 36;
    wavHeader.Subchunk2ID[0] = 'd';
    wavHeader.Subchunk2ID[1] = 'a';
    wavHeader.Subchunk2ID[2] = 't';
    wavHeader.Subchunk2ID[3] = 'a';
#ifdef DEBUG
    printHeader(wavFile, wavHeader, bytesRead);
#endif
    fclose(wavFile);

    // Re-open file to load all the data
    wavFile = fopen(inputFilePath , "rb");

    if(wavFile == NULL)
		{
        printf("Couldn't open wave file\n");
        exit(EXIT_FAILURE);
    }

    // Open and write to output wav file
    outWavFile = fopen(outputFilePath , "w");

    if(outWavFile == NULL)
	{
        printf("Couldn't open output wave file\n");
        exit(EXIT_FAILURE);
    }

    const unsigned long SIZE_DATA = wavHeader.Subchunk2Size;

    // Make header information mono
    if (wavHeader.NumOfChan == 2)
	{
        outWavHeader = makeHeaderMono(wavHeader);
    }
    else
	{
        outWavHeader = wavHeader;
    }

    // Write output wav data to file
    fwrite(&outWavHeader, (headerSize), 1, outWavFile);
    fclose(outWavFile);

    outWavFile = fopen(outputFilePath, "a");
    if (wavHeader.NumOfChan == 2)
	{
        fwrite(audio, (SIZE_DATA/2), 1, outWavFile);
    }
    else
	{
        fwrite(audio, SIZE_DATA, 1, outWavFile);
    }

    // Close wav files
    fclose(wavFile);
    fclose(outWavFile);

    return;
}
#else
void writeWav(float* audio, char* inputFilePath, char* outputFilePath, uint32_t numSamp) {

	wave::File read_file;
	wave::Error err = read_file.Open(inputFilePath, wave::kIn);

	wave::File write_file;
    err = write_file.Open(outputFilePath, wave::kOut);
	if (err) {
		std::cout << "Error, couldn't open output file: " << outputFilePath << std::endl;
        exit(EXIT_FAILURE);
	}

	write_file.set_sample_rate(read_file.sample_rate());
	write_file.set_bits_per_sample(read_file.bits_per_sample());
	write_file.set_channel_number(1);

	std::vector<float> content;
    content.reserve(numSamp);
    for (uint32_t i = 0; i < numSamp; i++) {
        content.push_back(audio[i]);
    }
	err = write_file.Write(content);
	if (err) {
		std::cout << "Error, couldn't write to output file: " << outputFilePath << std::endl;
        exit(EXIT_FAILURE);
	}
}
#endif