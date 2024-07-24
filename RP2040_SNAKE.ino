#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <vector>
#include <random>
#include <iostream>
#include <ctime>

#define joyStickPowerPin 2
#define joyStickX A1
#define joyStickY A2
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
using namespace std;

void setup() {
	// Initialization and flashscreen
	Serial.begin(9600);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.setTextColor(1);
    display.setTextSize(2);
	display.setCursor(36, 16);
	display.print("SNAKE    by Jens");
    display.display();

	// Prepares the gameloop
	delay(2000);
	for (int i = 3; i > 0; i--) {
		display.clearDisplay();
		display.setCursor(60, 22);
		display.print(i);
		display.drawRect(0, 0, 128, 64, 1);
		display.display();
		delay(1000);
	}
    display.clearDisplay();
	display.drawRect(0, 0, 128, 64, 1);
    display.display();

    pinMode(joyStickPowerPin, OUTPUT);
    pinMode(joyStickX, INPUT);
    pinMode(joyStickY, INPUT);
    digitalWrite(joyStickPowerPin, HIGH);
}

int length = 6;
float speed = 10;
int direction = 1; // [Up, 0]  [Right, 1]  [Down, 2]  [Left, 3]
int position[2] = {64, 32};
int xVal;
int yVal;
int joyStickThreshold = 200;
int appleBonus = 10;
int count = 0; // This count is for adding passive length..


class body {
	public:
		int lifeTime;
		int position[2];
		body(auto& inlifeTime, int inposition[2]){
            lifeTime = inlifeTime;
            position[1] = inposition[1];
            position[0] = inposition[0];
            // This draws the pixel bodypart
            display.drawPixel(position[0], position[1], 1);
		}
	
	void tick() {
		if (lifeTime <= 0) {
			// This kills the pixel
			display.drawPixel(position[0], position[1], 0);
		}
	}
};

vector<body> snake;
vector<vector<int>> apples;

vector<int> spawnApple() {
    mt19937 gen(static_cast<unsigned int>(time(0)));
    mt19937 gan(static_cast<unsigned int>(time(0)));
    uniform_int_distribution<> dis(1, 127);
    uniform_int_distribution<> das(1, 63);
    int randomX = dis(gen);
    int randomY = das(gan);
    // apples.push_back({randomX, randomY});
    display.drawPixel(randomX, randomY, 1);
    return {randomX, randomY};
}

// Handles GAME OVER
void gameOver() {
    display.clearDisplay();
	display.setCursor(5, 20);
	display.print("GAME OVER!");
	display.setTextSize(1);
	display.setCursor(28, 40);
	display.print("points: ");
	display.print(length);
	display.display();
	for(;;);
}


void loop() {
	body bodyPart(length, position);
	snake.insert(snake.begin(), bodyPart);
	switch (direction)
	{
	case 0:
		position[1]--;
		break;
	case 1:
		position[0]++;
		break;
	case 2:
		position[1]++;
		break;
	case 3:
		position[0]--;
		break;
	default:
		break;
	}

    // BODYLOOP
	for (body& elem : snake) {
        elem.lifeTime = elem.lifeTime - 1;
		elem.tick();
        if (elem.lifeTime <= 0) {
            snake.pop_back();
        }

        // Handles colliding with self
        if (elem.position[0] == position[0] && elem.position[1] == position[1]) {
            gameOver();
        }

        // Handles catching apples
        for (vector<int> apple : apples) {
            if (apple[0] == position[0] && apple[1] == position[1]) {
                // We caught an apple
                length += appleBonus;
                apples.pop_back();
            }
        }
	}
	display.display();
	
	// Ends game if we hit border
	if (position[0] < 1 or position[0] > 127 or position[1] < 1 or position[1] > 63) {
		gameOver();
	}
	delay(1000/speed);


// RANDOM MOVEMENT
    // if (millis() % 10 == 0) {
    // int temperature = analogRead(A0);  // Read from the internal temperature sensor
    // unsigned long seed = millis() ^ temperature;  // Combine millis() with temperature reading

    // // Initialize the random number generator with this seed
    // randomSeed(seed);

    // int randomDir = random(0, 3);  // Generate a random number between 1 and 127
    // direction = randomDir;
    // }

    // Handles joystick movement
    xVal = analogRead(joyStickX);
    yVal = analogRead(joyStickY);
    if (abs(xVal - 512) > joyStickThreshold && abs(xVal - 512) > abs(yVal - 512)) {
        if (xVal > 512) {
            direction = direction != 1 ? 3 : 1;
        } else {
            direction = direction != 3 ? 1 : 3;
        }
    } else if (abs(yVal - 512) > joyStickThreshold && abs(yVal - 512) > abs(xVal - 512)) {
        if(yVal > 512) {
            direction = direction != 0 ? 2 : 0;
        } else {
            direction = direction != 2 ? 0 : 2;
        }
    }
    
    // Adds length every 5th loop, for some reason
    count++;
    if (count > 3) {
        count = 0;
        length++;
    }


    // Handles spawning apples
    if (apples.size() < 3) {
        apples.insert(apples.begin(), spawnApple());
    }
    Serial.println(length);
}

