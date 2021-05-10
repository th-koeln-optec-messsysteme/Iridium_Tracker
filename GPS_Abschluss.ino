#include <TinyGPS++.h>
// GPS Recieve über RX1 => Serial1
// Uart arbeitet byteweise!

//Variablen:
    static double LG_d = 51.027519;             // zu erwartende Werte zwischen 47 und 54 (Nord und Süddeutschland)
    static double BG_d = 7.685340;              // zu erwartende Werte zwischen 4 und 13  (Ecke Brüssel bis Berlin)
    static double Alt_diff_d = 45565.9;
    static double Speed_d = 10.5;               
    static double COG_d = 355.98;               // Course over Ground, als Winkel zu TN aus Course() fkt von GPS TINY, mit 2 NK Signifikanz
    static double CR_d = 1.1;
    static double prev_Alt_diff_d = 0.0;
    static double HDOP_d = 20.1;                // Horizontal Dilution of Precision - Messunsicherheit mit 1NK Signifikanz  
    uint16_t data [3][7];
    static int col = 0, row = 0;

//Boolsche Variablen
    bool done = false;
    bool flughoehe = false;
    
//Funktionen
    int GPStoint(double);
    int x10(double);   // wandelt double mit 1 signifikanten NK in integer
    int x100(double);  // für 2 signifikante Nachkommastellen
    double getdata(bool);
    bool compressData(double,double,double,double,double,double,double);



    TinyGPSPlus gps;  //objekt
    

void setup()
{ 
    Serial1.begin(9600);    //Baud Rate = 9600 bit/s

    //optional zum testen ob Iridium funktioniert, ueber Putty/ HTerm (auf Uart 2),hierfür wird ein USB Adapter benoetigt:
    Serial2.begin(19200);
    Serial2.println("AT");
    Serial2.println("AT+CSQ");
}

/******** Main ****************************************************************************************************************************************************************************************************************/


void loop() //LAT/LONG/ALT wird empfangen   //RX1 an GPS und TX1 an Iridium
{
  while( Serial1.available()>0) {  // RX1

    // Müssen Grenzen von isValid Funktion noch definiert werden?
    if(gps.location.isUpdated() == true)
    { 
      if(gps.encode(Serial1.read()))
      { if(gps.location.isValid()==true)
        {   LG_d = gps.location.lng();
            BG_d = gps.location.lat();
        }
        else {
          LG_d = 0.0;
          BG_d = 0.0;
        }
        if (gps.altitude.isValid()==true)
        {   Alt_diff_d = gps.altitude.meters();
        }
        else Alt_diff_d = 0.0;
        if (gps.speed.isValid()==true)
        {   Speed_d = gps.speed.mps();
        }
        else Speed_d = 0.0;
        if (gps.course.isValid()==true)
        {   COG_d = gps.course.deg();
        }
        else COG_d = 0.0;
        // hier kommt noch die Climbrate hin: Differenz aus alten Höhendaten mitteln
        if ((prev_Alt_diff_d != Alt_diff_d) && (Alt_diff_d != 0.0)) {
          if (flughoehe == true) {
            CR_d = (Alt_diff_d - prev_Alt_diff_d)/200.00;  
          }
          else CR_d = (Alt_diff_d - prev_Alt_diff_d)/40.00;
          prev_Alt_diff_d = Alt_diff_d;
        }
        if (gps.hdop.isValid()==true)
        {   HDOP_d = gps.hdop.hdop();
        }
        else HDOP_d = 0.0;
        
        done = compressData(LG_d,BG_d,Alt_diff_d,Speed_d,COG_d,CR_d,HDOP_d);
      }
        
        if (done == true)
        {
          if (Alt_diff_d > 1000.00) {
            // Senden alle 10 Minuten
            flughoehe = true;
            delay (200000);                       // 3* 200_000 = 600_000 entspricht einem vollen Array pro 10 min
          }
          else {
            // Senden alle zwei Minuten
            flughoehe = false;
          delay (40000);                         // 3* 40_000 = 120_000 entspricht einem vollen Array pro 2 min
        }
          done = false;
          col = 0;
          if (row < 2)
            row++;   
          else if (row >=2)
          { //row =0; col =0;
            // DATA OUTPUT, ganzes Array [3][7] an Iridium
            Serial2.println("AT+SBDWT=" + 
            data[0][0] + data[0][1]+ data[0][2]+ data[0][3]+ data[0][4]+ data[0][5]+ data[0][6] +
            data[1][0] + data[1][1]+ data[1][2]+ data[1][3]+ data[1][4]+ data[1][5]+ data[1][6] +
            data[2][0] + data[2][1]+ data[2][2]+ data[2][3]+ data[2][4]+ data[2][5]+ data[2][6]);
          }  
       }
    }
  }
}
  
/***************************************************************************************************************************************************************************************************************************/

 bool compressData(double LG_d, double BG_d, double Alt_diff_d ,double Speed_d ,double COG-d ,double CR_d, double HDOP_d)
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
