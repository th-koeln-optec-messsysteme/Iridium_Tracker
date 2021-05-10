#include <TinyGPS++.h>
// GPS Recieve über RX1 => Serial1
// Uart arbeitet byteweise!

//variablen:
//  static const int RX1 = 19, TX1 = 18;
    static double LG_d = 51.027519;             // zu erwartende Werte zwischen 47 und 54 ( Nord und Süddeutschland)
    static double BG_d = 7.685340;              // zu erwartende Werte zwischen 4 und 13  (Ecke Brüssel bis Berlin)
    static double Alt_diff_d = 45565.9;
    static double Speed_d = 10.5;               
    static double COG_d = 355.98;               // Course over Ground, als Winkel zu TN aus Course() fkt von GPS TINY, mit 2 NK Signifikanz
    static double CR_d = 1.1;
    static double HDOP_d = 20.1;                // Horizontal Dilution of Precision - Messunsicherheit mit 1NK Signifikanz  
    uint16_t data [3][7];
    static int col = 0, row = 0;

    //Funktionen
    int GPStoint(double);
    int x10(double);   // wandelt double mit 1 signifikanten NK in integer
    int x100(double);  // für 2 signifikante Nachkommastellen

    double getdata(bool);
    bool compressData(double,double,double,double,double,double,double);
    bool done = false;


    TinyGPSPlus gps;  //objekt
    

void setup()
{ 
    Serial.begin(115200);                             // für Console, später nicht mehr nötig
    Serial1.begin(9600);    //Baud Rate = 9600 bit/s
    
  //delay (120000);         // settling time 2 min 
  
    Serial.println("GPS Start..");
}

/***************************************************************************************************************************************************************************************************************************/

// TO DO: über Print Befehlt wird sofort gesendet, muss noch paketweise abgefangen werden oder
//        senden wir an Iridium und geben da immer erst Blockweise frei?

// Was macht: (gps.location.isUpdated()) - ?                                                                                                            NOCH Machen!!

void loop() //LAT/LONG/ALT wird empfangen   //RX1 an GPS und TX1 an Iridium

{
  while( Serial1.available()>0)  // RX1

  //  CLIMBRATE ist noch nicht dabei!                                                                                                                   NOCH Machen!!
  // Müssen Grenzen von isValid Funktion noch definiert werden?
  
  { 
    if(gps.encode(Serial1.read()))
    { if(gps.location.isValid()==true)
      {   LG_d = gps.location.lng();
          BG_d = gps.location.lat();
      }
      if (gps.altitude.isValid()==true)
      {   Alt_diff_d = gps.altitude.meters();
      }
      if (gps.speed.isValid()==true)
      {   Speed_d = gps.speed.mps();
      }
      if (gps.course.isValid()==true)
      {   COG_d = gps.course.deg();
      }
      // hier kommt noch die Climbrate hin: Differenz aus alten Höhendaten mitteln
      
      if (gps.hdop.isValid()==true)
      {   HDOP_d = gps.hdop.hdop();
      }

      done = compressData(LG_d,BG_d,Alt_diff_d,Speed_d,COG_d,CR_d,HDOP_d);
    }
      
      if (done == true)
      {
        done =false;
        col =0;
        if (row < 2)
        row++;   
        else if (row >=2)
        { row =0; col =0;
        
          // DATA OUTPUT, ganzes Array [3][7] an Iridium                                                                                                NOCH Machen!!
          // DATA Array Reset
        }  
        delay (30000);
      }
  }
  

}
  
 

 bool compressData(double,double,double,double,double,double,double)
 {
    int LGint = GPStoint(LG_d);
    int BGint = GPStoint(BG_d);  
    int Alt_diff = x10(Alt_diff_d);        
    int SpeedOG = x10(Speed_d);                   //  8 bit ( auf 1 NK)
    int COG = x100(COG_d);                        //  16 bit (2NK)
    int CR = x10(CR_d);                           //  8 bit  (1 NK)
    int HDOP = x10(HDOP_d);                       //  8 bit  (1 NK)


    col = 0;
    data[row][col] = LGint >> 12;               // [0][0]: 16 MSB von LG

    col = 1;
    LGint &= 0x0FFF;    
    int BG1 = BGint >> 20;
    data[row][col] = (LGint <<4) + BG1 ;        // [0][1]: 12 LSB von LG + 4 MSB von BG

    col = 2;
    int BG2 = BGint & 0x0FFFF0;
    BG2 = BG2 >> 4; 
    int BG3 = BGint & 0x00000F;
    data[row][col] = BG2;  

    col = 3;
    //Alt zerteilen:
    int Alt1 = Alt_diff >>8; // 12 MSB (Ges 20 bit)
    int Alt2 = Alt_diff & 0x000FF; // 8 LSB
    data[row][col] = (BG3<<12) + Alt1;

    col = 4;
    // Speed over Ground sind 8 bit
    SpeedOG = SpeedOG & 0x00FF;  // nur zur Sicherheit.
    data[row][col] = (Alt2 << 8) + SpeedOG;

    col = 5;
    //COG
    data[row][col] = COG;
    
    col = 6;
    // CR + HDOP
    CR = CR & 0x00FF;       // nur zur Sicherheit.
    data[row][col] = (CR << 8) + (HDOP);
  
  return true;
 }


 int GPStoint(double d)                          // convertiert zu 4 Byte
{
    double buffer = d* 10e5;
    int newGps = (int)buffer;
    return newGps;
}

int x10(double f)              
{
    double buffer = f * 10;
    int i= (int)buffer;
    return i; 
}

int x100(double d)
{
    double buffer = d * 100;
    int i = (int)buffer;
    return i;
}
