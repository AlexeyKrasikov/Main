#ifndef __TCS3200_H__
#define __TCS3200_H__

#include <arduino.h>

#define DEFAULT_FREQUENCY 	_12KHZ    // POWER_DOWN, _12KHZ, _120KHZ, _600KHZ
#define WINDOW_WIDTH 		20

typedef enum Colors {CLEAR, RED, GREEN, BLUE} Colors;
typedef enum Frequency {POWER_DOWN, _12KHZ, _120KHZ, _600KHZ} Frequency;

class TCS3200
{
	friend void ISR_INTO();
public:
	TCS3200(uint8_t S0 = 3, uint8_t S1 = 4, uint8_t S2 = 5, uint8_t S3 = 6, uint8_t OBSERVATION_TIME = 10);

	void 
		begin(),	
		loop(),
		getRGBtoMaxCorrection (uint8_t (&RGB)[3]);
		
	void getRGB (uint8_t (&RGB)[3])
	{
		RGB[0] = RGBvalue[0];
		RGB[1] = RGBvalue[1];
		RGB[2] = RGBvalue[2];
	}

private:
	static uint16_t counter;

	Colors 		currentColor,
                previousColor;

	uint8_t 	m_S0, m_S1, m_S2, m_S3,
				m_OBSERVATION_TIME,
				RGBvalue[3];
	
    uint16_t   	summRGBC[4],
    			counterRGBC[4][WINDOW_WIDTH],
    			*p_counterRGBC[4];
	uint32_t    saveMillis,
            	currentMillis;

    void
    	setColor(Colors color),
		setFrequency(Frequency frequency),
		slidingWindow(uint16_t counter),
		getRGBrelativClearWithCorrection();
	Colors nextColor();
	uint8_t maximum ();

    void resetCounter() 
	{
	  asm("cli");
	  counter = 0;
	  asm("sei");
	}
};

#endif