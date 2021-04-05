// ConsoleTest_ASCIItoHEX.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <iostream>
using namespace std;
int GPStoint(double);
int Alttoint(double);
int Speedtoint(double);
int CRtoint();



int main()
{
    cout << "Hello World!\n";
    // Ascii to HEX
    static double LG_f = 51.027519;
    static double BG_f = 7.685340;
    static double Alt_ellipse_d = 12222.2;         //ellipsoid oder diff
    static double Alt_diff_d = 15555.5;

    static double SpeedoverGround = 10.5;       // nach Conv: 1.5 Byte
    static double climbrate = 1.1;
    static double COG = 120;                    //Course over Ground, Winkel zu true North
    static double EHPE = 20.1;                  // Unsicherheit in Metern
    cout << "LG_f: " << LG_f << endl;

    int LGint = GPStoint(LG_f);
    cout << "LG_int: " << LGint << endl;        // 3.5 (2 MSB können reduziert werden)
    int BGint = GPStoint(BG_f);
    cout << "BG: " << BGint << endl;            // max 3Byte (keine Reduktion)

    int Alt_el = Alttoint(Alt_ellipse_d);       //(5 MSB reduzieren)
    int Alt_diff = Alttoint(Alt_diff_d);        // (5 MSB red)
   // cout << Alt_el << endl;
   // cout << Alt_diff << endl;

    // bitweise schieben zum platz sparen
    uint16_t data[3][8];    // 16 bit integer (unsigned short)
    int col = 0, row = 0;
    data[row][col] = LGint >> 12;   // [0][0]: 16 MSB von LG
    cout << "[0][0]: " << data[0][0]<< endl;    //liefert: 12457

    col = 1;
    LGint &= 0x0FFF;    
    int BG1 = BGint >> 20;
    data[row][col] = (LGint <<4) + BG1 ;    // [0][1]: 12 LSB von LG + 4 MSB von BG
    cout << "[0][1]: " << data[0][1] << endl;

    col = 2;
    int BG2 = BGint & 0x0FFFF0;
    BG2 = BG2 >> 4; 
    int BG3 = BGint & 0x00000F;
    data[row][col] = BG2;
    cout << "[0][2]: " << data[0][2] << endl;
    //cout << "BG1: " << BG1 << "BG2: " << BG2 << endl;//
    cout << "BG3: " << BG3 << endl;

    col = 3;
    //Alt ellipse zerteilen:
    int Alt_el1 = Alt_el >>8; // 12 MSB (Ges 20 bit)
    int Alt_el2 = Alt_el& 0x000FF; // 8 LSB
    data[row][col] = (BG3<<12) + Alt_el1;
    cout << "[0][3]: " << data[0][3] << endl;
}

int GPStoint(double d)                          // convertiert zu 4 Byte
{
    double buffer = d* 10e5;
    int newGps = (int)buffer;
    return newGps;
}

int Alttoint(double f)
{
    double buffer = f * 10;
    int Alt_i = (int)buffer;
    return Alt_i; 
}

// Programm ausführen: STRG+F5 oder Menüeintrag "Debuggen" > "Starten ohne Debuggen starten"
// Programm debuggen: F5 oder "Debuggen" > Menü "Debuggen starten"

// Tipps für den Einstieg: 
//   1. Verwenden Sie das Projektmappen-Explorer-Fenster zum Hinzufügen/Verwalten von Dateien.
//   2. Verwenden Sie das Team Explorer-Fenster zum Herstellen einer Verbindung mit der Quellcodeverwaltung.
//   3. Verwenden Sie das Ausgabefenster, um die Buildausgabe und andere Nachrichten anzuzeigen.
//   4. Verwenden Sie das Fenster "Fehlerliste", um Fehler anzuzeigen.
//   5. Wechseln Sie zu "Projekt" > "Neues Element hinzufügen", um neue Codedateien zu erstellen, bzw. zu "Projekt" > "Vorhandenes Element hinzufügen", um dem Projekt vorhandene Codedateien hinzuzufügen.
//   6. Um dieses Projekt später erneut zu öffnen, wechseln Sie zu "Datei" > "Öffnen" > "Projekt", und wählen Sie die SLN-Datei aus.
