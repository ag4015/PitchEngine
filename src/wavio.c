
#include "wavio.h"

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
void printHeader(FILE* wavFile, wav_hdr wavHeader, size_t bytesRead){

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

int16_t* readWav(unsigned long *sizeData, char* filePath){

    int16_t* audio;
    wav_hdr wavHeader;
    FILE *wavFile;
    int headerSize = sizeof(wav_hdr);
    
    wavFile = fopen( filePath , "r" );

    if(wavFile == NULL){
        printf("Could not open wave file\n");
        exit(EXIT_FAILURE);
    }

    fread(&wavHeader,headerSize,1,wavFile);
#ifdef DEBUG
    // Read the header
    size_t bytesRead = fread(&wavHeader,headerSize,1,wavFile);
    printHeader(wavFile, wavHeader, bytesRead);
#endif
	fclose(wavFile);

    // Re-open file to load all the data
    wavFile = fopen(filePath , "rb");

    if(wavFile == NULL){
        printf("Can not able to open wave file\n");
        exit(EXIT_FAILURE);
    }
    
    *sizeData = wavHeader.ChunkSize - 36;

    // Allocate SIZE_DATA plus headerSize number of bytes to store the wav file
    int8_t* data = (int8_t*)calloc((headerSize + *sizeData), sizeof(int8_t));
    fread(data,(headerSize + *sizeData),1,wavFile);

    // Separate header from audio data
    audio = (int16_t*)(data + 44);

    // If audio is stereo separate the channels
    if(wavHeader.NumOfChan == 2){
        // Allocate memory for right and left channels
        int16_t* left_channel = (int16_t*)calloc((*sizeData / 2), 1);
        // int16_t* right_channel = (int16_t*)calloc((*sizeData / 2), 1);
        
        // Get two audio arrays for each stereo channel
        separateChannels(audio, left_channel/*, right_channel*/, *sizeData);

        // The audio is now mono so the size of the data is halved
        *sizeData = *sizeData/2;

        // Ignore right_channel
        // audio = left_channel;

        // free(right_channel);
        free(data);
		fclose(wavFile);
        return left_channel;
    }
    else
    {
        int16_t* mono = (int16_t*)calloc(*sizeData, sizeof(int16_t));
        for (unsigned int i = 0; i < *sizeData/2; i++){
            mono[i] = audio[i];
        }
        free(data);
		fclose(wavFile);
        return mono;
    }
}

void writeWav(int16_t* audio, char* inputFilePath, char* outputFilePath){

    wav_hdr wavHeader;
    wav_hdr outWavHeader;
    FILE *wavFile;
    FILE *outWavFile;
    int headerSize = sizeof(wav_hdr);

    // const char* outputFilePath = "/mnt/c/Users/alexg/Google Drive/Projects/Denoiser/output.wav";
	printf("Output file: %s\n", outputFilePath);

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