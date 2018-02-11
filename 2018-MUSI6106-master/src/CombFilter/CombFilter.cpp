
// standard headers
#include <limits>

// project headers
#include "MUSI6106Config.h"

#include "ErrorDef.h"
#include "Util.h"
#include "RingBuffer.h"

#include "CombFilterIf.h"
#include "CombFilter.h"


CCombFilterBase::CCombFilterBase( int iMaxDelayInFrames, int iNumChannels ) :
    m_ppCRingBuffer(0),
    m_iNumChannels(iNumChannels)
{

    assert(iMaxDelayInFrames > 0);
    assert(iNumChannels > 0);

  //Initialize the filter parameters and allocate memory for the ring buffer
	for (int i = 0; i < CCombFilterIf::kNumFilterParams; i++) {
		m_afParam[i] = 0.F;
	}
    //Initialize the range parameters as given in comments
	m_aafParamRange[CCombFilterIf::kParamGain][0] = -1; //initialize the minimum value of gain parameter to -1
	m_aafParamRange[CCombFilterIf::kParamGain][1] = 1; //Initialize the maximum value of gain parameter to +1
	m_aafParamRange[CCombFilterIf::kParamDelay][0] = 0; //Initialize the minimum delay to 0 samples
	m_aafParamRange[CCombFilterIf::kParamDelay][1] = iMaxDelayInFrames;  //Initialize the max delay to maxDelayInFrames

	//Allocate memory to ring buffer for all the channels
	m_ppCRingBuffer = new CRingBuffer<float>*[m_iNumChannels];
	for (int i = 0; i < m_iNumChannels; i++) {
		m_ppCRingBuffer[i] = new CRingBuffer<float>(iMaxDelayInFrames);
	}
}

CCombFilterBase::~CCombFilterBase()
{   
	
	for (int i = 0; i < m_iNumChannels; i++)
		delete[] m_ppCRingBuffer[i];
	delete[] m_ppCRingBuffer;
	
}
//Doubt : would it reset the contents of the ring buffer?
Error_t CCombFilterBase::resetInstance()
{

	for (int i = 0; i < m_iNumChannels; i++)
	{
		m_ppCRingBuffer[i]->reset();
		m_ppCRingBuffer[i]->setWriteIdx(CCombFilterIf::kParamDelay);
	}
	   
    return kNoError;
}

Error_t CCombFilterBase::setParam( CCombFilterIf::FilterParam_t eParam, float fParamValue )
{
    if (!isInParamRange(eParam, fParamValue))
        return kFunctionInvalidArgsError;
    //handle delay in the ring buffer - it can either be either more than the previous delay or less than the previous delay
	//If it is more than the previous delay, add 0's to the ring buffer until the point in reached.
	//If it is less than the previous delay, set the value to (read_index+delay_value)
	if (eParam == CCombFilterIf::kParamDelay) {
		int difference = fParamValue - CCombFilterIf::kParamDelay;
		if (difference > 0) {
			for (int i = 0; i < m_iNumChannels; i++) {

				{
					int incrementTimes = difference;
					while (incrementTimes > 0) {
						m_ppCRingBuffer[i]->putPostInc(0);
						incrementTimes -= 1;
					}
				}
			}
		}
		else {
			for (int i = 0; i < m_iNumChannels; i++){
				m_ppCRingBuffer[i]->setWriteIdx(m_ppCRingBuffer[i]->getReadIdx() + fParamValue);
			}
		}
	}
	m_afParam[eParam] = fParamValue;	
    return kNoError;
}

float CCombFilterBase::getParam( CCombFilterIf::FilterParam_t eParam ) const
{   

    return m_afParam[eParam];
}

bool CCombFilterBase::isInParamRange( CCombFilterIf::FilterParam_t eParam, float fValue )
{
    if (fValue < m_aafParamRange[eParam][0] || fValue > m_aafParamRange[eParam][1])
    {
        return false;
    }
    else
    {
        return true;
    }
}

//Implementation as per the code given in the question
Error_t CCombFilterFir::process( float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames )
{
	for (int i = 0; i < m_iNumChannels; i++) {
		for (int j = 0; i < iNumberOfFrames; j++) {
			ppfOutputBuffer[i][j] = ppfInputBuffer[i][j] + m_afParam[CCombFilterIf::kParamGain]*m_ppCRingBuffer[i]->getPostInc();
			m_ppCRingBuffer[i]->putPostInc(ppfInputBuffer[i][j]);
		}
	}
	return kNoError;
}


CCombFilterIir::CCombFilterIir (int iMaxDelayInFrames, int iNumChannels) : CCombFilterBase(iMaxDelayInFrames, iNumChannels)
{
	m_aafParamRange[CCombFilterIf::kParamGain][0] = -1; //initialize the minimum value of gain parameter to -1
	m_aafParamRange[CCombFilterIf::kParamGain][1] = 1; //Initialize the maximum value of gain parameter to +1

}

Error_t CCombFilterIir::process( float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames )
{
	for (int i = 0; i < m_iNumChannels; i++) {
		for (int j = 0; i < iNumberOfFrames; j++) {
			ppfOutputBuffer[i][j] = ppfInputBuffer[i][j] + m_afParam[CCombFilterIf::kParamGain] * m_ppCRingBuffer[i]->getPostInc();
			m_ppCRingBuffer[i]->putPostInc(ppfOutputBuffer[i][j]);
		}
	}

    return kNoError;
}
