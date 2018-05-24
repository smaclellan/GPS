#include <SD.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);

#define PMTK_SET_NMEA_UPDATE_1HZ  "$PMTK220,1000*1F"
#define PMTK_SET_NMEA_UPDATE_5HZ  "$PMTK220,200*2C"
#define PMTK_SET_NMEA_UPDATE_10HZ "$PMTK220,100*2F"

// turn on only the second sentence (GPRMC)
#define PMTK_SET_NMEA_OUTPUT_RMCONLY "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"
// turn on ALL THE DATA
#define PMTK_SET_NMEA_OUTPUT_ALLDATA "$PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28"

#define MAXLINELENGTH 120

// we double buffer: read one line in and leave one for the main program
volatile char line1[MAXLINELENGTH];
volatile char line2[MAXLINELENGTH];
// our index into filling the current line
volatile uint8_t lineidx=0;
// pointers to the double buffers
volatile char *currentline;
volatile char *lastline;
volatile boolean recvdflag;
volatile boolean inStandbyMode;
volatile float lat;
volatile float lon;


void setup() {
 Serial.begin(115200);
 Serial.println("Adafruit MTK3329 NMEA test!");

  // 9600 NMEA is the default baud rate
  mySerial.begin(38400);
  
  // uncomment this line to turn on only the "minimum recommended" data for high update rates!
  //mySerial.println(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  mySerial.println( "$PGCMD,16,1,1,1,1,1*6B\r\n" );
  // uncomment this line to turn on all the available data - for 9600 baud you'll want 1 Hz rate
  mySerial.println(PMTK_SET_NMEA_OUTPUT_ALLDATA);
 
  mySerial.println("$PMTK251,38400*27\r\n");
  
  // Set the update rate
  // 1 Hz update rate
  mySerial.println(PMTK_SET_NMEA_UPDATE_1HZ);
  //parse(PMTK_SET_NMEA_UPDATE_1HZ);
  //mySerial.println(latitude);
  //mySerial.println(longitude);
  // 5 Hz update rate- for 9600 baud you'll have to set the output to RMC only (see above)
  //mySerial.println(PMTK_SET_NMEA_UPDATE_5HZ);
  // 10 Hz update rate - for 9600 baud you'll have to set the output to RMC only (see above)
 // mySerial.println(PMTK_SET_NMEA_UPDATE_10HZ);
 
  SD.begin(4);
  File datafile = SD.open("test.txt");
  Serial.println("getting data from SD card...");
  while(datafile.available()){
    Serial.write(datafile.read());
  }
  datafile.close();
  SD.remove("test.txt");
}
void parse(char *nmea){
  //Serial.println("parsing ");
    
  int32_t degree;
  long minutes;
  char degreebuff[10];
  // look for a few common sentences
  if (strstr(nmea, "$GPGGA")) {
    //Serial.println("GPGGA found");
    // found GGA
    char *p = nmea;
    // get time
    p = strchr(p, ',')+1;
    float timef = atof(p);
    uint32_t time = timef;
    int hour = time / 10000;
    int minute = (time % 10000) / 100;
    int seconds = (time % 100);

    int milliseconds = fmod(timef, 1.0) * 1000;
    
    // parse out latitude
    p = strchr(p, ',')+1;
    if (',' != p)
    {
      //Serial.print("we here");
      strncpy(degreebuff, p, 2);
      p += 2;
      degreebuff[2] = '\0';
      degree = atol(degreebuff) * 10000000;
      strncpy(degreebuff, p, 2); // minutes
      p += 3; // skip decimal point
      strncpy(degreebuff + 2, p, 4);
      degreebuff[6] = '\0';
      minutes = 50 * atol(degreebuff) / 3;
      int latitude_fixed = degree + minutes;
      float latitude = degree / 100000 + minutes * 0.000006F;
      float latitudeDegrees = (latitude-100*int(latitude/100))/60.0;
      latitudeDegrees = latitudeDegrees + int(latitude/100);
      //Serial.println((latitudeDegrees));
      lat = latitudeDegrees;
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      if (p[0] == 'S') int latitudeDegrees = -1.0;
      if (p[0] == 'N') char lat = 'N';
      else if (p[0] == 'S') char lat = 'S';
      else if (p[0] == ',') char lat = 0;
      else return false;
    }
    
    // parse out longitude
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      strncpy(degreebuff, p, 3);
      p += 3;
      degreebuff[3] = '\0';
      degree = atol(degreebuff) * 10000000;
      strncpy(degreebuff, p, 2); // minutes
      p += 3; // skip decimal point
      strncpy(degreebuff + 2, p, 4);
      degreebuff[6] = '\0';
      minutes = 50 * atol(degreebuff) / 3;
      float longitude_fixed = degree + minutes;
      float longitude = degree / 100000 + minutes * 0.000006F;
      float longitudeDegrees = (longitude-100*int(longitude/100))/60.0;
      longitudeDegrees = longitudeDegrees + int(longitude/100);
      //Serial.println((longitudeDegrees));
      lon = longitudeDegrees;
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      if (p[0] == 'W') float longitudeDegrees = -1.0;
      if (p[0] == 'W') char lon = 'W';
      else if (p[0] == 'E') char lon = 'E';
      else if (p[0] == ',') char lon = 0;
      else return false;
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      int fixquality = atoi(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      int satellites = atoi(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      int HDOP = atof(p);
    }
    
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      int altitude = atof(p);
    }
    
    p = strchr(p, ',')+1;
    p = strchr(p, ',')+1;
    if (',' != *p)
    {
      int geoidheight = atof(p);
    }
    return true;
  }
}

void loop() {
  char nmea[100];
  int p=0;
  if (mySerial.available()) {
for(int i=0; i<100; i++){
    char n = mySerial.read();
    delay(3);
    //Serial.print((int)n);
    //Serial.print(" ");
    if (n=='\n') break;
    nmea[p++]=n;
}
    parse(nmea);
    SD.begin(4);
    File datafile = SD.open("test.txt", FILE_WRITE);
    if(SD.exists("test.txt")) {
        //Serial.println("file exists");
        datafile.print(lat);
        datafile.print('\n');
        datafile.print(lon);
        datafile.print('\n');
    }
    datafile.close();
    //Serial.println("done parsing");
    //Serial.print((nmea));
   
  }
  if (Serial.available()) {
     mySerial.print((char)Serial.read());
     char c = mySerial.read();
   // parse(c);
  }
  
}
