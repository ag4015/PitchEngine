
#ifndef WAVIO_H
#define WAVIO_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#pragma pack(push, 1) // exact fit - align at byte-boundary, no padding

typedef struct  WAV_HEADER
{
    /* RIFF Chunk Descriptor */
    uint8_t         RIFF[4];        // RIFF Header Magic header
    uint32_t        ChunkSize;      // RIFF Chunk Size
    uint8_t         WAVE[4];        // WAVE Header
    /* "fmt" sub-chunk */
    uint8_t         fmt[4];         // FMT header
    uint32_t        Subchunk1Size;  // Size of the fmt chunk
    uint16_t        AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t        NumOfChan;      // Number of channels 1=Mono 2=Sterio
    uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
    uint32_t        bytesPerSec;    // bytes per second
    uint16_t        blockAlign;     // 2=16-bit mono, 4=16-bit stereo
    uint16_t        bitsPerSample;  // Number of bits per sample
    /* "data" sub-chunk */
    uint8_t         Subchunk2ID[4]; // "data"  string
    uint32_t        Subchunk2Size;  // Sampled data length
} wav_hdr;

#pragma pack(pop)

#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

// Function prototypes 
int      getFileSize(FILE *inFile); 
void     printHeader(FILE* wavFile, wav_hdr wavHeader, size_t bytesRead);
void     separateChannels(int16_t* audio, int16_t* left_channel,/* int16_t* right_channel,*/ unsigned long size);
wav_hdr  makeHeaderMono(wav_hdr header);
int16_t* readWav(unsigned long *sizeData, char* filePath);
void     writeWav(int16_t* audio, char* inputFilePath, char* outputFilePath);

#ifdef __cplusplus
} // extern C
#endif

#endif // WAVIO_H
