
#include <iostream>
#include <ctime>
#include <math.h>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"
#include "Util.h"

using std::cout;
using std::endl;

#define PI 3.14;

// local function declarations
void    showClInfo ();

void testUnitImpulse(float **testSignal, CCombFilterIf *filterInstance, CCombFilterIf::CombFilterType_t filterType, float **outputData, int blockSize,int channels) {
    double gain = 0.5;
    int delayInSec = 2, sampleRateInHz = 5;
    filterInstance->init(filterType, delayInSec, sampleRateInHz, channels);
    int errorGain = filterInstance->setParam(CCombFilterIf::kParamGain, gain);
    if (errorGain) {
        throw "Exception:Invalid gain value";
    }
    int errorDelay= filterInstance->setParam(CCombFilterIf::kParamDelay, delayInSec);
    if (errorDelay) {
        throw "Exception:Invalid delay parameter";
    }
    filterInstance->process(testSignal, outputData, blockSize);
    cout << "\r" << "reading and writing";
    if (outputData[0][0] != 1) {
        throw "Exception:Test failed, gain value not equal";
    }
    if (filterType == CCombFilterIf::kCombFIR) {
        for (int c = 0; c < channels; c++)
        {
            for (int i=1; i<blockSize; i++ )
            {
                if (outputData[c][i] != 0) {
                    if (i != 9) {
                        throw "Exception:Test failed, filter coefficient not equal";
                    }
                    if (outputData[c][i] != gain) {
                        throw "Exception:Test failed, filter coefficient not equal";
                    }
                }
                
               // std::cout << outputData[c][i] << "\t";
            }
            
        }
    }
    else if (filterType == CCombFilterIf::kCombIIR) {
        int delaySamples = delayInSec * sampleRateInHz;
        if (outputData[0][0] != 1) {
            throw "Exception:Test failed, gain value not equal";
        }
        double current, previous;
        previous = outputData[0][0];
        double sum = 0.0;
        for (int c = 0; c < channels; c++) {
            for (int i = 0; i < blockSize; i++) {
                sum += outputData[c][i];
            }
        }
        if (sum != 2) {
            throw "Exception: Test failed, gain value not equal";
        }
        
        for (int c = 0; c < channels; c++)
        {   //Hard coded to check the cumulative sum with limited set of values
            for (int i=0; i< 90; i++ )
            {
                if (outputData[c][i] != 0) {
                    if ((i+1) % delaySamples == 0) {
                        current = outputData[c][i];
                        if (current != (previous*gain)) {
                            throw "Exception: Test failed, gain value not equal";
                        }
                        previous = current;
                    }
                }
                
                
            }
            
        }
    }
    else {
        throw "Exception:Invalid filter type";
    }
    std::cout << "Test passed" << endl;
}

/*This test sets a threshold to the value achieved with the specific parameters given.This test works with IIR filter*/
void testSinusoidalInput(float **testSignal, CCombFilterIf *filterInstance, CCombFilterIf::CombFilterType_t filterType, float **outputData, int blockSize, int channels) {
    double gain = 1;
    float delayInSec = 0.0417  , sampleRateInHz = 10000;
    filterInstance->init(filterType, delayInSec, sampleRateInHz, channels);
    filterInstance->setParam(CCombFilterIf::kParamGain, gain);
    filterInstance->setParam(CCombFilterIf::kParamDelay, delayInSec);
    filterInstance->process(testSignal, outputData, blockSize);
    for (int c = 0; c < channels; c++)
    {
        for (int i = 1; i<blockSize; i++)
        {
            if (outputData[c][i] < -0.1) {
                throw "Exception:FIR comb effect not achieved";
            }
            //std::cout << outputData[c][i] << endl;
        }
        
    }
    std::cout << "Test passed";
    
}
void testUnitStepSignal(float **testSignal, CCombFilterIf *filterInstance, CCombFilterIf::CombFilterType_t filterType, float **outputData, int blockSize, int channels) {
    double gain = 1;
    float delayInSec = 2, sampleRateInHz = 100;
    filterInstance->init(filterType, delayInSec, sampleRateInHz, channels);
    filterInstance->setParam(CCombFilterIf::kParamGain, gain);
    filterInstance->setParam(CCombFilterIf::kParamDelay, delayInSec);
    filterInstance->process(testSignal, outputData, blockSize);
    for (int c = 0; c < channels; c++)
    {
        for (int i = 1; i<blockSize; i++)
        {
            if (outputData[c][i] < outputData[c][i-1]) {
                throw "Exception:FIR comb effect not achieved";
            }
            //std::cout << outputData[c][i] << endl;
        }
        
    }
    std::cout << "Test passed";
    
}
/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string             sInputFilePath,                 //!< file paths
    sOutputFilePath;
    
    static const int        kBlockSize = 1024;
    
    clock_t                 time = 0;
    
    float                   **ppfAudioData = 0;
    float                   **ppfOutputData = 0, **outputData=0;
    float                   **testUnit=0;
    float                   **testStep = 0;
    float                   **testSinusoid = 0;
    float                   **sinOutputData = 0;
    CAudioFileIf            *phAudioFile = 0;
    
    CAudioFileIf            *phOutputFile = 0;
    
    std::fstream            hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;
    CCombFilterIf::CombFilterType_t filterTypeEnum;
    CCombFilterIf   *pInstance = 0, *testInstance = 0;
    CCombFilterIf::create(pInstance);
    CCombFilterIf::create(testInstance);
    float gain = 0, delay = 0;
    int filterType=0; ///say the default filter type is FIR
    showClInfo();
    
    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    std::cout << "Enter input audiopath, output audio path, filter type, delay and gain parameters\n";
    if (argc < 2)
    {
        cout << "Missing audio input path!";
        return -1;
    }
    else if(argc==6)
    {
        sInputFilePath = argv[1];
        sOutputFilePath = argv[2];
        filterType = atoi(argv[3]);
        delay = atof(argv[4]);
        gain = atof(argv[5]);
        if (gain < -1 || gain>1) {
            cout << "Invalid gain value";
            throw "Exception:Invalid gain value";
        }
    }
    
    else{
		cout << "Enter correct arguments" << endl;
		return -1;
        
    }
    //////////////////////////////////////////////////////////////////////////////
    // open the input wave file
    CAudioFileIf::create(phAudioFile);
    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    if (!phAudioFile->isOpen())
    {
        cout << "File open error!";
        return -1;
    }
    phAudioFile->getFileSpec(stFileSpec);
    
    //////////////////////////////////////////////////////////////////////////////
    // open the output wav file
    CAudioFileIf::create(phOutputFile);
    phOutputFile->openFile(sOutputFilePath, CAudioFileIf::kFileWrite, &stFileSpec);
    if (!phOutputFile->isOpen())
    {
        cout << "Wav file open error!";
        return -1;
    }
    
    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfAudioData[i] = new float[kBlockSize];
    ppfOutputData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfOutputData[i] = new float[kBlockSize];
    outputData = new float*[1];
    for (int i = 0; i < 1; i++) {
        outputData[i] = new float[512];
    }
    sinOutputData = new float*[1];
    for (int i = 0; i < 1; i++) {
        sinOutputData[i] = new float[10000];
    }
    
    time = clock();
    
    //Allocate memory to a test signal with a single channel
    int channels = 1, testBlockSize = 512;
    
    testUnit = new float*[channels];
    for (int k = 0; k < channels; k++) {
        testUnit[k] = new float[testBlockSize];
    }
    for (int i = 0; i < channels; i++) {
        for (int j = 0; j < testBlockSize; j++) {
            testUnit[i][j] = 0;
        }
        
    }
    testUnit[0][0] = 1;
    //Allocate memory to the step function
    testStep = new float*[channels];
    for (int i = 0; i < 1; i++) {
        testStep[i] = new float[testBlockSize];
    }
    for (int i = 0; i < channels; i++) {
        for (int j = 0; j < testBlockSize; j++) {
            testStep[i][j] = 1;
        }
    }
   
    //Allocate memory for the sinusoidal input. A random signal
   
    testSinusoid = new float*[channels];
    for (int k = 0; k < channels; k++) {
        testSinusoid[k] = new float[5000];
    }
    
    for (int k = 0; k < 5000;  k++) {
        double value = static_cast<double>((2*3.14 * 6 * k)/10000);
        testSinusoid[0][k] = sin(value);
        
    }
    
    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output file
    
    /*Initialize the comb filter depending on the filter type given as a parameter*/
    switch (filterType)
    {
        case 0:filterTypeEnum = CCombFilterIf::kCombFIR; break;
        case 1:filterTypeEnum = CCombFilterIf::kCombIIR; break;
    }
    /*Initialize and set filter parameters based on the given input*/
    pInstance->init(filterTypeEnum, delay, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    pInstance->setParam(CCombFilterIf::kParamGain, gain);
    pInstance->setParam(CCombFilterIf::kParamDelay, delay);
    
    
    while (!phAudioFile->isEof())
    {
        long long iNumFrames = kBlockSize;
        phAudioFile->readData(ppfAudioData, iNumFrames);
        pInstance->process(ppfAudioData, ppfOutputData, iNumFrames);
        cout << "\r" << "reading and writing";
        phOutputFile->writeData(ppfOutputData, iNumFrames);
        
     
    }
	cout << "Running test cases now" << endl;
    //Test case 1. Take a unit impulse signal and test FIR/IIR results. This test works with both FIR as well as IIR type of filter
    testUnitImpulse(testUnit, testInstance, CCombFilterIf::kCombFIR, outputData, testBlockSize,channels);
    
    //Test case 2. Take a sinusoidal signal and see the comb like effect with the data. The delay with this signal is set in such a fashion that
    //it results in mostly positive values with very small negative values. A threshold is set to check the negative values.
    testSinusoidalInput(testSinusoid, testInstance, CCombFilterIf::kCombIIR, sinOutputData, 5000, channels);
    
    //Test case 3: Check for magnitude increase with IIR filter here. This test runs for FIR filter as well.
    testUnitStepSignal(testStep, testInstance, CCombFilterIf::kCombIIR, outputData, testBlockSize, channels);
    cout << "\nreading/writing done in: \t" << (clock() - time)*1.F / CLOCKS_PER_SEC << " seconds." << endl;
    
    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    CAudioFileIf::destroy(phAudioFile);
    CCombFilterIf::destroy(pInstance);
    CCombFilterIf::destroy(testInstance);
    CAudioFileIf::destroy(phOutputFile);
    
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfAudioData[i];
    delete[] ppfAudioData;
    ppfAudioData = 0;
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfOutputData[i];
    delete[] ppfOutputData;
    ppfOutputData = 0;
    for (int i = 0; i < 1; i++)
        delete[] outputData[i];
    delete[] outputData;
    for (int i = 0; i < 1; i++)
        delete[] testUnit[i];
    delete[] testUnit;
    for (int i = 0; i < 1; i++)
        delete[] testSinusoid[i];
    delete[] testSinusoid;
    for (int i = 0; i < 1; i++)
        delete[] testStep[i];
    delete[] testStep;
    for (int i = 0; i < 1; i++)
        delete[] sinOutputData[i];
    delete[] sinOutputData;
    return 0;
    
}


void     showClInfo()
{
    cout << "GTCMT MUSI6106 Executable" << endl;
    cout << "(c) 2014-2018 by Alexander Lerch" << endl;
    cout  << endl;
    
    return;
}
