#include <Adafruit_NeoPixel.h>
#include <cstdlib>
#define COLS 12
#define LED_PIN D3
#define LED_COUNT 144
#define LEN_ARRAY 5
/*
#define UP_ESQ D1
#define DOWN_ESQ D7
#define UP_DIR D5
#define DOWN_DIR D6
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define MQTTPORT 1883
#define UP_ESQ "pong/upesq"
#define DOWN_ESQ "pong/downesq"
#define UP_DIR "pong/updir"
#define DOWN_DIR "pong/downdir"

//long randNumber;


//const char* ssid = "moto g(7) 4767";
//const char* password = "arduino351";

const char* ssid = "dpi-mestrado2";
const char* password = "vbmuGDxY1e";
const char* mqtt_server = "broker.mqtt-dashboard.com";  // Public Broker, pay attention in topics to avoid conflicts

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
/*****
 * Add two pushbutton subscribe. You could add more controls....
 */
      // ... and resubscribe
    client.subscribe(UP_ESQ);
    client.subscribe(DOWN_ESQ);
    client.subscribe(UP_DIR);
    client.subscribe(DOWN_DIR);
      
/*******************************************************/      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

byte mqtt_input=66;
byte input;
long lastMsg;

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    String Topic(topic);
    Serial.print(Topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    mqtt_input = 64;

    if ( Topic.equals(UP_DIR) ) {
    mqtt_input = 65;
    }
    if ( Topic.equals(DOWN_DIR) ) {
    mqtt_input = 66;
    }
    if ( Topic.equals(UP_ESQ) ) {
    mqtt_input = 68;
    }
    if ( Topic.equals(DOWN_ESQ) ) {
    mqtt_input = 67;
    }

    if (mqtt_input != 64) Serial.println(mqtt_input);

}


int map_px ( int l, int c ) {
	return l *  COLS + ((l % 2)?COLS-c-1:c);
}

void move_up (int *plat, int *aux, int nplat, int *ball) {
    int ball_pos = map_px(ball[0], ball[1]);
    if (plat[1]-plat[0]==1) {
      if (plat[0]-23==ball_pos) return;
    }
    else if (plat[0]-1==ball_pos) return;
    if (nplat == 1) {
        if (plat[0] == 0)
            return;
    }
    else
        if (plat[0] == 11)
            return;
    for (int i = 0; i < LEN_ARRAY; i++)
        if (i == LEN_ARRAY-1)
            if (aux[LEN_ARRAY-2]-aux[LEN_ARRAY-3] == 1)
                aux[i] = aux[LEN_ARRAY-2] + 23;
            else
                aux[i] = aux[LEN_ARRAY-2] + 1;
        else
            if (plat[i+1]-plat[i] == 23)
                aux[i] = plat[i] - 1;
            else
                aux[i] = plat[i] - 23;
    for (int i = 0; i < LEN_ARRAY; i++)
        plat[i] = aux[i];
}

void move_down (int *plat, int *aux, int nplat, int *ball) {
    int ball_pos = map_px(ball[0], ball[1]);
    if (plat[4]-plat[3]==1) {
      if (plat[4]+23==ball_pos) return;
    }
    else if (plat[4]+1==ball_pos) return;
    if (nplat == 1) {
        if (plat[LEN_ARRAY-1] == LED_COUNT-1)
            return;
    }
    else
        if (plat[LEN_ARRAY-1] == LED_COUNT-12)
            return;
    for (int i = 0; i < LEN_ARRAY; i++)
        if (i == LEN_ARRAY-1)
            if (aux[LEN_ARRAY-2]-aux[LEN_ARRAY-3] == 1)
                aux[i] = aux[LEN_ARRAY-2] + 23;
            else
                aux[i] = aux[LEN_ARRAY-2] + 1;
        else
            if (plat[i+1]-plat[i] == 23)
                aux[i] = plat[i] + 23;
            else
                aux[i] = plat[i] + 1;
    for (int i = 0; i < LEN_ARRAY; i++)
        plat[i] = aux[i];
}

bool move_ball (int *ball, int *ball_speed, int *plat_esq, int *plat_dir) {
    if (ball[0] == 11 || ball[0] == 0) {
        ball_speed[0] *= -1;
    }
    if (ball[1] == 1 || ball[1] == 10) {
        int next_pos = map_px(ball[0]+ball_speed[0], ball[1]+ball_speed[1]);
        for (int i = 0; i < LEN_ARRAY; i++) {
            if (plat_dir[i] == next_pos || plat_esq[i] == next_pos) {
                ball_speed[1] *= -1;
                if ((ball_speed[0]>0 && i==0) || (ball_speed[0]<0 && i==LEN_ARRAY-1)) ball_speed[0] *= -1;
                break;
            }
        }
    }
    else {
        if (ball[1] == 0 || ball[1] == 11)
            return true;
    }
    ball[0] += ball_speed[0];
    ball[1] += ball_speed[1];
    return false;
}

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
int aux[LEN_ARRAY];
int plat_esq[LEN_ARRAY] = {24,47,48,71,72};
int plat_dir[LEN_ARRAY] = {35,36,59,60,83};
int plat_esq_backup[LEN_ARRAY] = {24,47,48,71,72};
int plat_dir_backup[LEN_ARRAY] = {35,36,59,60,83};
int ball[2] = {5,5};
int ball_speed[2];
uint32_t ball_color = 0x0000FF;
bool game_over = false;
unsigned long myTime;
bool refresh = false;
bool upesq = false;
bool downesq = false;
bool updir = false;
bool downdir = false;
int n_refresh = 0;
bool post_over = false;
int poi = 0;

void setup()
{
    if (rand()%2==0) ball_speed[0] = 1;
    else ball_speed[0] = -1;
    if (rand()%2==0) ball_speed[1] = 1;
    else ball_speed[1] = -1;
    strip.begin();
    strip.show();
    strip.clear();
    for(int i = 0; i < LEN_ARRAY; i++) {
    strip.setPixelColor(plat_esq[i], 255, 0, 0);
    strip.setPixelColor(plat_dir[i], 255, 0, 0);
    }
    strip.setPixelColor(map_px(ball[0], ball[1]), ball_color);
    strip.show();
    myTime = millis();
    /*
    pinMode(D5,INPUT_PULLUP);
    pinMode(D6,INPUT_PULLUP);
    pinMode(D7,INPUT_PULLUP);
    pinMode(D1,INPUT_PULLUP);
    */
    Serial.begin(115200);
    /*
    Serial.println("inicio teste botoes");
    for (int i = 0; i < 1000; i++) {
    delay(50);
    if ( digitalRead(D5) == LOW) Serial.println("D5");
    if ( digitalRead(D6) == LOW) Serial.println("D6");
    if ( digitalRead(D7) == LOW) Serial.println("D7");
    if ( digitalRead(D1) == LOW) Serial.println("D1");
    }
    Serial.println("fim teste botoes");
    */
    setup_wifi();
    client.setServer(mqtt_server, MQTTPORT);
    client.setCallback(callback);
    lastMsg = 0;
    while (true) {
        if (!client.connected()) reconnect();
        client.loop();

        if (millis()-myTime > 50) {
            refresh = true;
            myTime = millis();
        }

        if (mqtt_input == 65) updir = true;
        if (mqtt_input == 66) downdir = true;
        if (mqtt_input == 67) downesq = true;
        if (mqtt_input == 68) upesq = true;
        mqtt_input = 64;

        if (refresh) {
        if (!(upesq && downesq)) {
            if (upesq) move_up(plat_esq, aux, 1, ball);
            else if (downesq) move_down(plat_esq, aux, 1, ball);
        }
        if (!(updir && downdir)) {
            if (updir) move_up(plat_dir, aux, 2, ball);
            else if (downdir) move_down(plat_dir, aux, 2, ball);
        }
        if (n_refresh%15==0) {
        if (!game_over) {
            game_over = move_ball(ball, ball_speed, plat_esq, plat_dir);
        }
        else {
            if (!post_over) {
                ball_color = 0x00FF00;
                post_over = true;
                poi = n_refresh+80;
            }
        }
        }
        if (post_over && n_refresh==poi) {
        ball_color = 0x0000FF;
        for (int i=0; i<LEN_ARRAY; i++) plat_esq[i] = plat_esq_backup[i];
        for (int i=0; i<LEN_ARRAY; i++) plat_dir[i] = plat_dir_backup[i];
        ball[0] = 5;
        ball[1] = 5;
        if (rand()%2==0) ball_speed[0] = 1;
        else ball_speed[0] = -1;
        if (rand()%2==0) ball_speed[1] = 1;
        else ball_speed[1] = -1;
        } else if (n_refresh==poi+20){
        game_over = false;
        post_over = false;
        }

        strip.clear();
        for(int i = 0; i < LEN_ARRAY; i++) {
            strip.setPixelColor(plat_esq[i], 255, 0, 0);
            strip.setPixelColor(plat_dir[i], 255, 0, 0);
        }
        strip.setPixelColor(map_px(ball[0], ball[1]), ball_color);
        strip.show();
        upesq = downesq = updir = downdir = refresh = false;
        n_refresh++;
        }
    }
}

void loop()
{

}