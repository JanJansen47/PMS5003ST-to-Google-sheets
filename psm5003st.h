#include<SoftwareSerial.h>

SoftwareSerial Serialx(14, 12); // Software RX, TX (D5 en D6)
char col;
unsigned int PMSa = 0, PMSb = 0;
unsigned int  CR1 = 0, CR2 = 0;
unsigned char buffer_RTT[40] = {};   //
//Serial buffer;
char tempStr[40];

boolean init_psm5003()  {
  Serialx.begin(9600);
}


  
  boolean read_psm5003(float *result) {
  // request  output  from psm5003st sensor 
  char d = 0xe2, s1=0x42, s2=0x4d;
  Serialx.print(s1);
  Serialx.print(s2);
  Serialx.print(d); 
    
  while (!Serialx.available());
  while (Serialx.available() > 0)     //Data check: weather there is any Data in Serialx
  { for (int i = 0; i < 40; i++)     {
      col = Serialx.read();
      buffer_RTT[i] = (char)col;
      delay(2);
    }
    Serialx.flush();
    CR1 = (buffer_RTT[38] << 8) + buffer_RTT[39];
    CR2 = 0;
    for (int i = 0; i < 38; i++)
      CR2 += buffer_RTT[i];
    if (CR1 == CR2)                        //Check
    { PMSa = buffer_RTT[12];       //Read PM2.5 High 8-bit
      PMSb = buffer_RTT[13];       //Read PM2.5 Low 8-bit   
      result[0] = float(((PMSa << 8) + PMSb)/1.0);    //PM2.5 value
      
      PMSa = buffer_RTT[10];       //Read PM1.0 High 8-bit
      PMSb = buffer_RTT[11];       //Read PM1.0 Low 8-bit   
      result[1] = float(((PMSa << 8) + PMSb)/1.0);    //PM1.0 value
      
      PMSa = buffer_RTT[8];       //Read PM10 High 8-bit
      PMSb = buffer_RTT[9];       //Read PM10 Low 8-bit   
      result[2] = float(((PMSa << 8) + PMSb)/1.0);    //PM10 value    
       
      PMSa = buffer_RTT[28];       //Read Formaldehyde High 8-bit
      PMSb = buffer_RTT[29];       //Read Formaldehyde Low 8-bit 
      result[3] = float(((PMSa << 8) + PMSb)/1.0);    //Formaldehyde value  
      
      PMSa = buffer_RTT[30];       //Read Temperature High 8-bit
      PMSb = buffer_RTT[31];       //Read Temperature Low 8-bit
      result[4] = float(((PMSa << 8) + PMSb)/10.0);    //Temperature value 
      
      PMSa = buffer_RTT[32];       //Read Humidity High 8-bit
      PMSb = buffer_RTT[33];       //Read Humidity Low 8-bit
      result[5] = float(((PMSa << 8) + PMSb)/10.0);    //Humidity value
      return true;
    }
    else
    {
      for ( int i=0; i<6;i++) result[i]=0;
      return false;
    }

  }
  }

