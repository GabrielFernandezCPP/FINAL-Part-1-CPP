/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <WiFi.h>
#include <PubSubClient.h>

#include <stdlib.h>
#include <time.h>

#include <stdbool.h>

#include <string.h>

#include <LiquidCrystal_I2C.h>
#include <Wire.h>


#define SDA 14
#define SCL 13

#define NO_WINNERS 0
#define PLAY_X 1
#define PLAY_O 2
#define DRAW 3

int RUN_PROGRAM = false;
int GAME[3][3] =  {{0, 0, 0},
                  { 0, 0, 0},
                  { 0, 0, 0}};

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int dirsGame[] = { 3, 2, 2, 2, 4, 1, 1, 1 };
const int places[] = { 7, 7, 8, 9, 9, 9, 6, 3 };

bool sendGameState = false;
bool FREEZE = false;
char gameState[10];

//const int BUILTIN_LED = 12;
const char* ssid = "Socalled5";
const char* password = "Ariesleo7755";
const char* mqtt_server = "104.196.229.154";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

int randomNumLocal() {
  return (rand() % 10);
}

void printLCD(const char* mess) {
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(mess);
}

bool i2CAddrTest(uint8_t addr) {
  Wire.beginTransmission(addr);

  if (Wire.endTransmission() == 0) return true;

  return false;
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");

  char* con = (char*)malloc(sizeof(char*) * strlen(strcat("CON to ", ssid)));

  if (con == NULL) printLCD("NULL");
  else
  {
    strcpy(con, "CON to ");
    strcat(con, ssid);
    printLCD(con);
  }

  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //This is for the client id//
  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");

  //char* ip = (char*)(WiFi.localIP());
  //char* ipStr = (char*)malloc(sizeof(char*) * strlen(strcat("Connected.\nIP: ", ip)));
  
  //if (ipStr == NULL) printLCD("NULL!!!");
  //else
  //{
  //  strcpy(ipStr, "Connected.\nIP: ");
  //  strcat(ipStr, ip);

  printLCD("Connected.");
  //}

  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  digitalWrite(2, HIGH);

  free(con);
}

void initGame() {
  int x = 0;
  int y = 0;

  for (x = 0; x < 3; ++x)
  {
    for (y = 0; y < 3; ++y)
    {
      GAME[x][y] = 0;
    }
  }
}

//1 for x and 2 for o
int playItem(const int item, int pos) {
  bool ok = false;
  int num = -1;
  int x, y;
  bool hasAll = true;

  Serial.print("Playing item.\n");

  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      if (GAME[i][j] == 0) hasAll = false;
    }
  }

  if (hasAll)
  {
    Serial.print("All are filled!\n");
    return DRAW;
  }

  while (!ok)
  {
    if (pos == 0) num = randomNumLocal();
    else num = pos - 1;

    y = num % 3;
    x = num / 3;

    if (GAME[x][y] == 0)
    {
      ok = true;
      GAME[x][y] = item;
      Serial.print("Found pos.\n");
    }
    else
    {
      if (pos != 0) pos = 0;
    }

    Serial.printf(". Random Pos: %d\n", num);
  }

  Serial.print("Checking game.\n");
  if (checkGame(item)) return item;

  return NO_WINNERS;
}

bool checkGame(const int item) {
  //1 = -, 2 = |, 3 = /, 4 = \ 
  //int i = 0;
  //int j = 0;
  //int k = 0;
  int prevNum = 0;
  int wInt;

  int x, y;
  int yes = 0;

  int baseNum;
  int workingNum;

  int check[] = { 0, 0, 0 };
  
  bool won = false;

  //Check dires
  for (int i = 0; i < 8; ++i)
  {
    baseNum = places[i] - 1;

    //Serial.printf("Pass: %d, Base Num: %d ------------------- \n", i, baseNum);
    //ACtually check dirs
    for (int j = 0; j < 3; ++j)
    {
      wInt = dirsGame[i];

      switch(wInt)
      {
        case 1:
        {
          workingNum = baseNum - j;
          break;
        }
        case 2:
        {
          workingNum = baseNum - (j * 3);
          break;
        }
        case 3:
        {
          workingNum = baseNum - (j * 2);
          break;
        }
        case 4:
        {
          workingNum = baseNum - (j * 4);
          break;
        }
      }

      y = workingNum % 3;
      x = workingNum / 3;

      check[j] = GAME[x][y];
      //Serial.printf("X: %d, Y: %d, Res: %d\n", x, y, check[j]);
    }

    //Check if won
    yes = 0;
    for (int k = 0; k < 3; ++k)
    {
      if (check[k] == item) yes += 1;
    }

    if (yes == 3) won = true;
  }

  return won;
}

//This function is called when ever the ESP discoveres 
void callback(char* topic, byte* payload, unsigned int length) {
  int itemCheck = NO_WINNERS;

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (length <= 0) return;

  char retPay = (char)payload[0];
  int posToPlay = 0;

  if (length > 0) posToPlay = payload[1] - '0';

  //Serial.print("Reading results\n");

  // Switch on the LED if an 1 was received as first character
  switch (retPay)
  {
    case '1':
    {
      digitalWrite(2, HIGH);
      Serial.print("Set LED on.\n");
      break;
    }
    case '2':
    {
      digitalWrite(2, LOW);  // Turn the LED off by making the voltage HIGH
      Serial.print("Set LED off.\n");
      break;
    }
    case '3':
    {
      initGame();
      printLCD("Initialized the TIC TAC TOE game.\n");
      FREEZE = false;
      break;
    }
    case '4':
    {
      if (FREEZE) return;
      itemCheck = playItem(PLAY_X, posToPlay); //X
      printLCD("Played X randomly.\n");
      break;
    }
    case '5':
    {
      if (FREEZE) return;
      itemCheck = playItem(PLAY_O, posToPlay); //O
      printLCD("Played O randomly.\n");
      break;
    }
    case '6':
    {
      if (FREEZE) return;
      sendGameState = true;
      Serial.print("Return the state of the game.\n");
      break;
    }
    case '7':
    {
      if (FREEZE) return;
      itemCheck = playItem(PLAY_X, posToPlay); //X
      printLCD("Played X at pos.\n");
      sendGameState = true;
      break;
    }
    case '8':
    {
      if (FREEZE) return;
      itemCheck = playItem(PLAY_O, posToPlay); //X
      printLCD("Played O at pos.\n");
      sendGameState = true;
      break;
    }
    case '9':
    {
      FREEZE = false;
      break;
    }
  }

  //Display if person won
  switch (itemCheck)
  {
    case PLAY_X:
    {
      Serial.print("~~~~~~~~~ X HAS WON! ~~~~~~~~~~~~\n");
      break;
    }
    case PLAY_O:
    {
      Serial.print("~~~~~~~~~ O HAS WON! ~~~~~~~~~~~~\n");
      break;
    }
    case DRAW:
    {
      Serial.print("~~~~~~~~~ DRAW! ~~~~~~~~~~~~\n");
      break;
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  srand(time(NULL));

  Wire.begin(SDA, SCL);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  //lcd.print("Hello, world!");

  pinMode(2, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  //Send game state to GCP
  if (sendGameState)
  {
    int x, y;

    for (x = 0; x < 3; x++)
    {
      for (y = 0; y < 3; y++)
      {
        gameState[y + (x * 3)] = (char)(GAME[x][y] + 48);
        //Serial.print(y + (x * 3));
      }
    }

    gameState[9] = '\0';

    //Send game state
    client.publish("tttGame", gameState);

    sendGameState = false;
  }

  //Check if game has won.
  if (!FREEZE)
  {
    if (checkGame(PLAY_X))
    {
      printLCD("X has won!");
      client.publish("tttGame", "X has won!");
      FREEZE = true;
    }

    if (checkGame(PLAY_O))
    {
      printLCD("O has won!");
      client.publish("tttGame", "O has won!");
      FREEZE = true;
    }
  }
}
