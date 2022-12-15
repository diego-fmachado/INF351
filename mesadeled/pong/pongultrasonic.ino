#include <Adafruit_NeoPixel.h>
#include <cstdlib>
#include <bits/stdc++.h>
#define COLS 12
#define LED_PIN D3
#define LED_COUNT 144
#define LEN_ARRAY 5
#define UP_ESQ D8
#define DOWN_ESQ D7
#define UP_DIR D5
#define DOWN_DIR D6
#define echoPin D4
#define trigPin D1
#define MOVE_FACTOR 1


int calc_dist(int* data) {
    int array_len = 0;
    int temp1[500];
    for (int i=0; i<500; i++) {
        if (data[i]>0) {
            temp1[array_len] = data[i];
            array_len++;
        }
    }
    if (array_len==0) return -1;
    int temp2[array_len];
    for (int i=0; i<array_len; i++) {
        temp2[i]=temp1[i];
    }
    std::sort(temp2, temp2+array_len);
    Serial.println("Array De Dados:");
    
    for (int i=0; i<array_len; i++) {
        Serial.print(temp2[i]);
        Serial.print(" ");
    }
    
    if (array_len%2==1) return temp2[(int)array_len/2];
    else return (temp2[(array_len/2)-1]+temp2[array_len/2])/2;
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
unsigned long myDiff;
bool refresh = false;
bool upesq = false;
bool downesq = false;
bool updir = false;
bool downdir = false;
int n_refresh = 0;
bool post_over = false;
int poi = 0;
int ball_factor = 5;
long duration;
int distance;
int data[500];

int m_dist;
int center_target;
bool direction = false;
int dsequence = 0;





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

    pinMode(UP_DIR,INPUT_PULLUP);
    pinMode(DOWN_DIR,INPUT_PULLUP);
    pinMode(DOWN_ESQ,INPUT_PULLUP);
    pinMode(UP_ESQ,INPUT_PULLUP);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    Serial.begin(115200);
}

void loop()
{

    //delay(500);

    for (int i=0; i<500; i++) {
        data[i] = 0;
    }

    myTime = millis();
    for (int i=0; millis()-myTime < 44; i++) {
        if (i>499) break;
        digitalWrite(trigPin, LOW);
        delayMicroseconds(5);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);
        noInterrupts();
        duration = pulseIn(echoPin, HIGH, 6000);
        interrupts();
        distance = duration * 0.034 / 2;
        data[i] = distance;

        //if(digitalRead(UP_ESQ) == LOW) upesq = true;
        //if(digitalRead(DOWN_ESQ) == LOW) downesq = true;
        if(digitalRead(UP_DIR) == LOW) updir = true;
        if(digitalRead(DOWN_DIR) == LOW) downdir = true;
    }


    m_dist = calc_dist(data);
    
    /*
    Serial.print("Distancia mediana: ");
    Serial.print(m_dist);
    Serial.println("cm");
    */

    while (millis()-myTime < 50) {
        //if(digitalRead(UP_ESQ) == LOW) upesq = true;
        //if(digitalRead(DOWN_ESQ) == LOW) downesq = true;
        if(digitalRead(UP_DIR) == LOW) updir = true;
        if(digitalRead(DOWN_DIR) == LOW) downdir = true;
    }


    if (m_dist > 0) {
        if (m_dist>100) m_dist=100;
        center_target = map_px((m_dist/12.5)+2, 0);
        ///*
        Serial.print("Distance: ");
        Serial.print(m_dist);
        Serial.print("cm Target X: ");
        Serial.print((int)(m_dist/12.5)+2);
        Serial.print(" LED target: ");
        Serial.println(center_target);
        //*/
        if (plat_esq[2] != center_target) {
            if (plat_esq[2] > center_target) {
                if (!direction) dsequence = 0;
                direction = true;
            }
            else {
                if (direction) dsequence = 0;
                direction = false;
            }
            dsequence++;
        }
        //
        //
    }

    if (dsequence > MOVE_FACTOR) {
        if (direction) move_up(plat_esq, aux, 1, ball);
        else move_down(plat_esq, aux, 1, ball);
        dsequence = 0;
    }


    if (!(upesq && downesq)) {
        if (upesq) move_up(plat_esq, aux, 1, ball);
        else if (downesq) move_down(plat_esq, aux, 1, ball);
    }
    if (!(updir && downdir)) {
        if (updir) move_up(plat_dir, aux, 2, ball);
        else if (downdir) move_down(plat_dir, aux, 2, ball);
    }
    if (n_refresh%ball_factor==0) {
    if (!game_over) {
        game_over = move_ball(ball, ball_speed, plat_esq, plat_dir);
    }
    else {
        if (!post_over) {
            ball_color = 0x00FF00;
            post_over = true;
            poi = n_refresh+40;
        }
    }
    }
    if (post_over && n_refresh==poi) {
    ball_factor = 5;
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
    n_refresh = 0;
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
    if (n_refresh%200==0 && ball_factor>1) ball_factor--;

}