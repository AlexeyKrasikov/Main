#ifndef DEBUG_H__
#define DEBUG_H__
	 
int freeRam ();
	 	 
#ifdef DEBUG 
#define debugDo(x)					x 
#define debugPrint(x)               Serial.print(x)
#define debugPrintF(x) 				Serial.print(F(x))
#define debugPrintln(x)             Serial.println(x); delay(5)
#define debugPrintlnF(x)            Serial.println(F(x)); delay(5)
#define debugPrint2arg(x, y)        Serial.print(x, y)
#define debugPrintln2arg(x, y)      Serial.println(x, y); delay(5)
#define debugInit(x)                Serial.begin(x)
#define debugFlush 				    Serial.flush();
#else
#define debugDo(x)					
#define debugPrint(x)               
#define debugPrintF(x) 				
#define debugPrintln(x)             
#define debugPrintlnF(x)            
#define debugPrint2arg(x, y)        
#define debugPrintln2arg(x, y)      
#define debugInit(x)                
#define debugFlush 				            
#endif
	 
#endif