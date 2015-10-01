#ifndef DEBUG_H__
#define DEBUG_H__

#ifdef DEBUG
#define ass(x)						x
#define assert(x)                   Serial.print(x)
#define assertln(x)                 Serial.println(x)
#define assert2(x, y)               Serial.print(x, y)
#define assert2ln(x, y)             Serial.println(x, y)
#define assertF(x)                  Serial.print(F(x))
#define assertlnF(x)                Serial.println(F(x))
#define assert2F(x, y)              Serial.print(F(x), y)
#define assert2lnF(x, y)            Serial.println(F(x), y)
#define assertInit(x)               Serial.begin(x)
#define assertFlush 				Serial.flush()
#else
#define ass(x)
#define assert(x) 
#define assertln(x)  
#define assert2(x, y)
#define assert2ln(x, y) 
#define assertF(x)                  
#define assertlnF(x)                
#define assert2F(x, y)              
#define assert2lnF(x, y)                           
#define assertInit(x)
#define assertFlush               
#endif

#endif
