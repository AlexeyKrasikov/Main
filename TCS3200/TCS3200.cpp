#include <TCS3200.h>

uint16_t TCS3200::counter;

void ISR_INTO()
{
  TCS3200::counter++;
}

TCS3200::TCS3200(uint8_t S0, uint8_t S1, uint8_t S2, uint8_t S3, uint8_t OBSERVATION_TIME)
                  : m_S0(S0), m_S1(S1), m_S2(S2), m_S3(S3), m_OBSERVATION_TIME(OBSERVATION_TIME)
{
	summRGBC[0] = 0;
	summRGBC[1] = 0;
	summRGBC[2] = 0;
	summRGBC[3] = 0;
  currentColor = CLEAR;
  previousColor = currentColor;
  p_counterRGBC[0] = &counterRGBC[CLEAR][0];
  p_counterRGBC[1] = &counterRGBC[RED]  [0];
  p_counterRGBC[2] = &counterRGBC[GREEN][0];
  p_counterRGBC[3] = &counterRGBC[BLUE] [0];
}

void TCS3200::begin()
{
	pinMode(m_S0, OUTPUT);
	pinMode(m_S1, OUTPUT); 
	pinMode(m_S2, OUTPUT);
	pinMode(m_S3, OUTPUT);

	setFrequency(DEFAULT_FREQUENCY);
	setColor(CLEAR);

	attachInterrupt(0, ISR_INTO, CHANGE);
	saveMillis = millis() + 1;
	while (millis() != saveMillis) ;
	resetCounter();
  while ((millis() - (WINDOW_WIDTH*4*m_OBSERVATION_TIME+1)) <= saveMillis)
  {
    loop();             // заполняем массив значениями.
  }
}

void TCS3200::loop()
{
  currentMillis = millis();

  if ((currentMillis - saveMillis) >= m_OBSERVATION_TIME) {
    saveMillis = currentMillis;
    slidingWindow(counter);
    previousColor = currentColor;
    currentColor = nextColor();
    getRGBrelativClearWithCorrection();
  }
}

void TCS3200::setFrequency (Frequency frequency)
{
  switch (frequency) {
    case POWER_DOWN:
      digitalWrite(m_S0, LOW);
      digitalWrite(m_S1, LOW);
      break;
    case _12KHZ:
      digitalWrite(m_S0, LOW);
      digitalWrite(m_S1, HIGH);
      break;
    case _120KHZ:
      digitalWrite(m_S0, HIGH);
      digitalWrite(m_S1, LOW);
      break;
    case _600KHZ:
      digitalWrite(m_S0, HIGH);
      digitalWrite(m_S1, HIGH);
      break;
    default: while(1) ;     // ошибка
  }
}

void TCS3200::setColor (Colors color)
{
  switch (color) {
    case CLEAR: 
      digitalWrite(m_S2, HIGH);
      digitalWrite(m_S3, LOW);
      break;
    case RED:
      digitalWrite(m_S2, LOW);
      digitalWrite(m_S3, LOW);
      break;
    case GREEN:
      digitalWrite(m_S2, HIGH);
      digitalWrite(m_S3, HIGH);
      break;
    case BLUE:
      digitalWrite(m_S2, LOW);
      digitalWrite(m_S3, HIGH);
      break;
    default: while(1) ;       // ошибка
  }
}

Colors TCS3200::nextColor ()
{
  switch (currentColor) {
    case CLEAR: 
      setColor(RED);
      resetCounter();
      return RED;
    case RED: 
      setColor(GREEN);
      resetCounter();
      return GREEN;
    case GREEN: 
      setColor(BLUE);
      resetCounter();
      return BLUE;
    case BLUE: 
      setColor(CLEAR);
      resetCounter();
      return CLEAR;
    default: while(1) ;
  }
}

void TCS3200::slidingWindow (uint16_t counter)
{
  summRGBC[currentColor] -= *p_counterRGBC[currentColor];
  summRGBC[currentColor] += counter;
  *p_counterRGBC[currentColor] = counter;
  if (++p_counterRGBC[currentColor] == &counterRGBC[currentColor][WINDOW_WIDTH]) {
    p_counterRGBC[currentColor] = &counterRGBC[currentColor][0];
  }
}

void TCS3200::getRGBrelativClearWithCorrection ()
{
  uint16_t  temp,
            divider = summRGBC[CLEAR];
  if (divider == 0) divider++;

  if (currentColor == RED) {
    temp = ( ((uint32_t)summRGBC[currentColor]) << (8 + 3) )   /   (5 * divider);  // коррекция *8/5 для R канала
  }

  if (currentColor == GREEN) {      
    temp = ( (((uint32_t)summRGBC[currentColor]) << 8) * 5)   /   (divider * 3);  // коррекция *5/3 для G канала    
  }

  if (currentColor == BLUE) {      
    temp = ( (((uint32_t)summRGBC[currentColor]) << (8 - 1)) * 3)   /   divider;  // коррекция *3/2 для B канала    
  }  

  if (currentColor != CLEAR) {
    if (temp > 255) temp = 255;
    RGBvalue[currentColor-1] = (uint8_t)temp;    // Присваивание переменным класса значений RGB.
  }
}

void TCS3200::getRGBtoMaxCorrection (uint8_t (&RGB)[3])
{ 
  uint8_t         maxRGB;
  uint16_t        temp;
  
  maxRGB = maximum();

  if (maxRGB == 0) maxRGB++;

  for (uint8_t i = 0; i < 3; i++) {
    temp = (((uint16_t)RGBvalue[i]) << 8) / maxRGB;
    if (temp > 255) temp = 255;

    RGB[i] = temp;
  }
}

uint8_t TCS3200::maximum ()
{
  uint8_t temp;
  RGBvalue[0] >= RGBvalue[1] ? temp = RGBvalue[0] : temp = RGBvalue[1];
  temp >= RGBvalue[2] ? temp : temp = RGBvalue[2];
  return temp;
}