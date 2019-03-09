

//Oled display
#include <Adafruit_GFX.h>
#include <ESP_Adafruit_SSD1306.h>
#define  OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);



//OLED Display 05-06-2018  -----------------------------------------------------------------
void initdisplay(int versie, char Unit) {
  display.begin(SSD1306_SWITCHCAPVCC, 0x78 >> 1);          // init done
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

//  if (App[0] == 'S')  display.println("P&J  Slim");
 // if (App[0] != 'S')   display.println("P&J Mobiel");
  display.setTextSize(1);
  display.setCursor(0, 17);  display.print("Versie:  "); display.println(versie);
  display.setCursor(0, 47);  display.print("RRSI:  ");   display.println(WiFi.RSSI());
  display.setCursor(0, 27);  display.print("SSID:  ");   display.println(WiFi.SSID());
  display.setCursor(0, 37);  display.print("IP:  ");      display.println(WiFi.localIP());
  display.setCursor(0, 57);  display.print("UNIT:  ");   display.println(Unit);
  display.display();

}
//Show een regel (bericht)
void show( String txt, String msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("--BERICHTEN--");
  display.setCursor(0, 35);
  display.print(txt);
  display.setCursor(0, 45);
  display.print(msg);
  display.display();
  delay(1000);
}

// display alle waarden van de slimme meter
void showslim() {
  display.clearDisplay();
  display.setTextSize(1);
  /*
  display.setCursor(0, 0);  display.print("EVLT: "); display.println(String(mEVLT));
  display.setCursor(0, 9); display.print("EVHT: "); display.println(String(mEVHT));
  display.setCursor(0, 18); display.print("EOLT: "); display.println(String(mEOLT));
  display.setCursor(0, 27); display.print("EOHT: "); display.println(String(mEOHT));
  display.setCursor(0, 36); display.print("EAV : "); display.println(String(mEAV));
  display.setCursor(0, 45); display.print("EAT : "); display.println(String(mEAT));
  display.setCursor(0, 54); display.print("GAS : "); display.println(String(mGAS));
  */
  display.display();
}

// display alle waarden van de slimme meter
void showwaardenl(String w1,String w2,String w3,String w4,String w5,String w6, String w7 , int v1, int v2, int v3, int v4, int v5, int v6, int v7) {
  display.clearDisplay();
  display.setTextSize(1);
  
  display.setCursor(0, 0);  display.print(w1); display.println(String(v1));
  display.setCursor(0, 9);  display.print(w2); display.println(String(v2));
  display.setCursor(0, 18); display.print(w3); display.println(String(v3));
  display.setCursor(0, 27); display.print(w4); display.println(String(v4));
  display.setCursor(0, 36); display.print(w5); display.println(String(v5));
  display.setCursor(0, 45); display.print(w6); display.println(String(v6));
  display.setCursor(0, 54); display.print(w7); display.println(String(v7));
  
  display.display();
}


void showcloudinfo(String *w) {
  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0); display.println("CLOUD INFO");
  
  display.setCursor(0, 18); display.println(w[0]);
  display.setCursor(0, 27); display.println(w[1]);
  display.setCursor(0, 36); display.println(w[2]);
  display.setCursor(0, 45); display.println(w[3]);
  display.setCursor(0, 54); display.println(w[4]);
  
  display.display();
}

