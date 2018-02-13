
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"
#include "Util.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string             sInputFilePath,                 //!< file paths
        sOutputFilePath;

    static const int        kBlockSize = 1024;

    clock_t                 time = 0;

    float                   **ppfAudioData = 0;
	float                   **ppfOutputData = 0;
	float **testUnit=0;

    CAudioFileIf            *phAudioFile = 0;
    std::fstream            hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;
	CCombFilterIf::CombFilterType_t filterTypeEnum;
    CCombFilterIf   *pInstance = 0;
    CCombFilterIf::create(pInstance);
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
        sOutputFilePath = sInputFilePath + ".txt";
		filterType = atoi(argv[3]);	
		delay = atof(argv[4]);
		gain = atof(argv[5]);

    }
	else if (argc == 5) {
		sInputFilePath = argv[1];
		sOutputFilePath = sInputFilePath + ".txt";
		filterType = atoi(argv[3]);
		delay = atof(argv[4]);
	}
	else if (argc == 4) {
		sInputFilePath = argv[1];
		sOutputFilePath = sInputFilePath + ".txt";
		filterType = atoi(argv[3]);
	}
	else{
		sInputFilePath = argv[1];
		sOutputFilePath = sInputFilePath + ".txt";
		
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
    hOutputFile.open(sOutputFilePath.c_str(), std::ios::out);
    if (!hOutputFile.is_open())
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

    time = clock();
	testUnit = new float*[1];
	for (int k = 0; k < 1; k++) {
		testUnit[k] = new float[kBlockSize];
	}
	for (int i = 0; i < 1; i++) {
		for (int j = 0; j < kBlockSize; j++) {
			testUnit[i][j] = 0;
		}
		
	}
	testUnit[0][0] = 1;
	//////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output file
	//pInstance->init(filterType,)
	switch (filterType)
	{
	case 0:filterTypeEnum = CCombFilterIf::kCombFIR; break;
	case 1:filterTypeEnum = CCombFilterIf::kCombIIR; break;
	}
	//filterTypeEnum = (CCombFilterIf::FilterParam_t)filterType;
	//pInstance->init(filterTypeEnum, delay, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
	//pInstance->setParam(CCombFilterIf::kParamGain, gain);
	//pInstance->setParam(CCombFilterIf::kParamDelay, delay);
	pInstance->init(CCombFilterIf::kCombFIR, 2, 5, 1);
	pInstance->setParam(CCombFilterIf::kParamGain, 0.5);
	pInstance->setParam(CCombFilterIf::kParamDelay, 2);

	/*while (!phAudioFile->isEof())
    {
        long long iNumFrames = kBlockSize;
        phAudioFile->readData(ppfAudioData, iNumFrames);
		pInstance->process(ppfAudioData, ppfOutputData, iNumFrames);
        cout << "\r" << "reading and writing";

        for (int i = 0; i < iNumFrames; i++)
        {
            for (int c = 0; c < stFileSpec.iNumChannels; c++)
            {
                hOutputFile << ppfOutputData[c][i] << "\t";
            }
            hOutputFile << endl;
        }
    }*/
	int k = 0;
	while (k<100)
	{
	long long iNumFrames = 100;
	//phAudioFile->readData(ppfAudioData, iNumFrames);
	pInstance->process(testUnit, ppfOutputData, iNumFrames);
	cout << "\r" << "reading and writing";

	for (int i = 0; i < iNumFrames; i++)
	{
	for (int c = 0; c < stFileSpec.iNumChannels; c++)
	{
	hOutputFile << ppfOutputData[c][i] << "\t";
	}
	hOutputFile << endl;
	}
	k++;
	}
    cout << "\nreading/writing done in: \t" << (clock() - time)*1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    CAudioFileIf::destroy(phAudioFile);
	CCombFilterIf::destroy(pInstance);
    hOutputFile.close();

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfAudioData[i];
    delete[] ppfAudioData;
    ppfAudioData = 0;
	for (int i = 0; i < stFileSpec.iNumChannels; i++)
		delete[] ppfOutputData[i];
	delete[] ppfOutputData;
	ppfOutputData = 0;
    return 0;

}


void     showClInfo()
{
    cout << "GTCMT MUSI6106 Executable" << endl;
    cout << "(c) 2014-2018 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

