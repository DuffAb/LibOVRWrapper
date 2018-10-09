#pragma once

#include "OVR_CAPI.h"

#include <atomic>

#define REV_HAPTICS_SAMPLE_RATE 320
#define REV_HAPTICS_MAX_SAMPLES 256

class HapticsBuffer
{
public:
	HapticsBuffer();
	~HapticsBuffer() { }

	void AddSamples(const ovrHapticsBuffer* buffer);
	void SetConstant(float frequency, float amplitude);
	float GetSample();
	ovrHapticsPlaybackState GetState();

private:
	// Lock-less circular buffer
	std::atomic_uint8_t m_ReadIndex;
	std::atomic_uint8_t m_WriteIndex;
	uint8_t m_Buffer[REV_HAPTICS_MAX_SAMPLES];

	// Constant feedback
	std::atomic_uint16_t m_ConstantTimeout;
	float m_Frequency;
	float m_Amplitude;
};
