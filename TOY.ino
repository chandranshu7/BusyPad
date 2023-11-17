#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncDNSServer.h>
#include <ESPAsyncWiFiManager.h>
#include <Adafruit_ILI9341.h>
#include <Update.h>
#include <HTTPClient.h>
#include "FS.h"
#include <SPIFFS.h>
#include <FastLED.h>

#define LED_PIN     13   
#define NUM_LEDS    16   
#define BUTTON_NEXT_PIN 27
#define BUTTON_BACK_PIN 22  

CRGB leds[NUM_LEDS];

char alphabet[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12', '13', '14', '15', '16', '17', '18', '19', '20'};

int currentAlphabetIndex = 0;

int currentDigitIndex = 0;


// Pin configuration for the ILI9341 display
#define TFT_CS 17
#define TFT_DC 4
#define TFT_RST -1  


const int port = 443; // HTTPS port

bool alphabetGameRunning = false;

bool countingGameRunning = false;

bool backButtonPressed = false;

// Create an instance of the ILI9341 display
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// Create an instance of AsyncWebServer
AsyncWebServer server(80);

void setup() {

  Serial.begin(115200);

  delay(1000);
  

  // Initialize the LED strip
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  randomSeed(analogRead(0)); 

  // Initialize the ILI9341 display
  tft.begin();

  // Clear the display
  tft.fillScreen(ILI9341_BLACK);

  // Create an instance of DNSServer
  DNSServer dns;

  // Create an instance of AsyncWiFiManager with both server and dns
  AsyncWiFiManager wifiManager(&server, &dns);

  // Try to auto-connect to the last saved Wi-Fi credentials
  if (!wifiManager.autoConnect("ToyAP")) {
    // Failed to connect or no saved credentials, reboot to try again
    ESP.restart();
  }

  Serial.println("Connected to Wi-Fi: " + WiFi.SSID());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  

  
  uint16_t bgColor = tft.color565(255, 253, 208); // Cream background color
  uint16_t textColor = tft.color565(0, 0, 0);

  tft.fillScreen(bgColor);
  tft.setCursor(40, 120);
  tft.setTextColor(textColor);
  tft.setRotation(3);
  tft.setTextSize(3);
  tft.print("Connected to Wi-Fi: ");
  tft.println(WiFi.SSID());
  

  Serial.println("Setup complete.");



// Route to serve the HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", getHomePage());
  });

  // Route to handle alphabet game click
  server.on("/start-alphabet-game", HTTP_GET, [](AsyncWebServerRequest *request){
    startAlphabetGame();
    request->send(200, "text/plain", "Alphabet Game Loaded");
  });

  // Route to handle counting digit game click
  server.on("/start-counting-digit-game", HTTP_GET, [](AsyncWebServerRequest *request){
    startCountingDigitGame();
    request->send(200, "text/plain", "Counting Digit Game Loaded");
  });

  // Serve static files (e.g., stylesheets, scripts)
server.serveStatic("/", SPIFFS, "/");




  // Start server
  server.begin();
}

void stopRunningGame() {
  alphabetGameRunning = false;
  countingGameRunning = false;
}

void startAlphabetGame() {
   stopRunningGame();
  alphabetGameRunning = true;
  Serial.println("Starting Alphabet Game");

  tft.begin();
  tft.setRotation(3);
  pinMode(BUTTON_NEXT_PIN, INPUT);
  pinMode(BUTTON_BACK_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

  showWelcomeAlphabetScreen();
  displayAlphabet();
  
}


void showWelcomeAlphabetScreen() {
  uint16_t bgColor = tft.color565(255, 253, 208); // Cream background color
  uint16_t textColor = tft.color565(0, 0, 0);

  tft.fillScreen(bgColor);
  tft.setCursor(40, 120);
  tft.setTextColor(textColor);
  tft.setTextSize(3);
  tft.print("Welcome to the Alphabet Game");
  delay(2000); 
}

void showWelcomeCountingScreen() {
  uint16_t bgColor = tft.color565(255, 253, 208); // Cream background color
  uint16_t textColor = tft.color565(0, 0, 0);

  tft.fillScreen(bgColor);
  tft.setCursor(40, 120);
  tft.setTextColor(textColor);
  tft.setTextSize(3);
  tft.print("Welcome to the Counting Digit");
  delay(2000); 
}

void changeAlphabet(int direction) {
  currentAlphabetIndex = (currentAlphabetIndex + direction) % (sizeof(alphabet) / sizeof(alphabet[0]));
  if (currentAlphabetIndex < 0) {
    currentAlphabetIndex = sizeof(alphabet) / sizeof(alphabet[0]) - 1;
  }
}

void changeDigit(int direction) {
  currentDigitIndex = (currentDigitIndex + direction) % (sizeof(digits) / sizeof(digits[0]));
  if (currentDigitIndex < 0) {
    currentDigitIndex = sizeof(digits) / sizeof(digits[0]) - 1;
  }
}

void flashLEDs() {
  for (int i = 0; i < 3; ++i) {  
    for (int j = 0; j < NUM_LEDS; ++j) {
      leds[j] = CHSV(random(255), 255, 255); 
    }
    FastLED.show();
    delay(250);
    FastLED.clear();
    delay(250);
  }
}

void displayAlphabet() {
  uint16_t bgColor = tft.color565(255, 253, 208); // Cream background color
  uint16_t textColor = tft.color565(0, 0, 0);
  
  tft.fillScreen(bgColor);
  tft.setCursor(80, 90);
  tft.setTextColor(textColor);
  tft.setTextSize(5);
  tft.print(alphabet[currentAlphabetIndex]);
  delay(500);  
}

void displayDigit() {
  uint16_t bgColor = tft.color565(255, 253, 208); // Cream background color
  uint16_t textColor = tft.color565(0, 0, 0);
  
  tft.fillScreen(bgColor);
  tft.setCursor(80, 90);
  tft.setTextColor(textColor);
  tft.setTextSize(5);
  tft.print(digits[currentDigitIndex]);
  delay(500);  
}


void startCountingDigitGame() {
   stopRunningGame();

   countingGameRunning= true;
  Serial.println("Starting Counting Digit");

  tft.begin();
  tft.setRotation(3);
  pinMode(BUTTON_NEXT_PIN, INPUT);
  pinMode(BUTTON_BACK_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

  showWelcomeCountingScreen();
  displayDigit();
  

}

void loop() {
  

  if (alphabetGameRunning) {
    // Check and update game state here
    if (digitalRead(BUTTON_NEXT_PIN) == HIGH) {
      changeAlphabet(1);
      flashLEDs();
      displayAlphabet();
    } else if (digitalRead(BUTTON_BACK_PIN) == HIGH && !backButtonPressed) {
      changeAlphabet(-1);
      flashLEDs();
      displayAlphabet();
    } else if (digitalRead(BUTTON_BACK_PIN) == LOW) {
      backButtonPressed = false;
    }

    
  
}

  if (countingGameRunning) {
    // Check and update game state here
    if (digitalRead(BUTTON_NEXT_PIN) == HIGH) {
      changeDigit(1);
      flashLEDs();
      displayDigit();
    } else if (digitalRead(BUTTON_BACK_PIN) == HIGH && !backButtonPressed) {
       changeDigit(-1);
      flashLEDs();
      displayDigit();
    } else if (digitalRead(BUTTON_BACK_PIN) == LOW) {
      backButtonPressed = false;
    }


  }

}

String getHomePage() {
  return R"(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Games</title>
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css">
    <link rel="stylesheet" href="styles.css">
    <style>
        body {
            background-color: #f8f9fa;
        }

        .navbar {
            background-color: #fff;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
        }

        .navbar-brand {
            margin-right: 2rem;
            font-weight: bold;
            font-size: 24px;
        }

        .navbar-toggler {
            border: none;
        }

        .navbar-toggler-icon {
            background-color: #343a40;
        }

        .navbar-nav {
            margin-left: auto;
        }

        .nav-link {
            color: #343a40 !important;
            font-weight: bold;
            margin-right: 1rem;
        }

        .content {
            min-height: calc(100vh - 120px);
        }

        .card {
            border: none;
            transition: transform 0.3s;
        }

        .card:hover {
            transform: scale(1.05);
        }

        .card-title {
            font-size: 18px;
            margin-top: 10px;
        }

        .footer {
            background-color: #343a40;
            color: #fff;
            padding: 20px 0;
            text-align: center;
            position: absolute;
            bottom: 0;
            width: 100%;
        }

        .container {
            max-width: 800px;
        }

        h1 {
            color: #343a40;
        }
    </style>
    <script>
        function startAlphabetGame() {
            fetch('/start-alphabet-game');
        }

        function startDigitGame() {
            fetch('/start-counting-digit-game');
        }
    </script>
</head>

<body>
    <!-- Navbar -->
    <nav class="navbar navbar-expand-lg navbar-light">
        <div class="container">
            <a class="navbar-brand" href="#">BusyPad</a>
            <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarNav"
                aria-controls="navbarNav" aria-expanded="false" aria-label="Toggle navigation">
                <span class="navbar-toggler-icon"></span>
            </button>
            <div class="collapse navbar-collapse" id="navbarNav">
                <ul class="navbar-nav">
                    <li class="nav-item">
                        <a class="nav-link" href="index.html">Home</a>
                    </li>
                    <li class="nav-item">
                        <a class="nav-link" href="about.html">About</a>
                    </li>
                    <li class="nav-item">
                        <a class="nav-link" href="contact.html">Contact Us</a>
                    </li>
                    <li class="nav-item">
                        <a class="nav-link" href="TOY.html">My Toy</a>
                    </li>
                </ul>
            </div>
        </div>
    </nav>
    <!-- Content Section -->
    <section class="content">
        <div class="container">
            <h1 class="text-center">Select a Game</h1>
            <div class="row">
                <div class="col-md-6 col-lg-4 mb-4">
                    <a href="#" onclick="startAlphabetGame(); return false;">
                        <div class="card">
                            <div class="card-body">
                                <h5 class="card-title">Alphabet Game</h5>
                            </div>
                        </div>
                    </a>
                </div>
                <div class="col-md-6 col-lg-4 mb-4">
                    <a href="#" onclick="startDigitGame(); return false;">
                        <div class="card">
                            <div class="card-body">
                                <h5 class="card-title">Counting Digit</h5>
                            </div>
                        </div>
                    </a>
                </div>
            </div>
        </div>
    </section>

    <!-- Footer -->
    <footer class="footer">
        <div class="container">
            <p>&copy; 2023 Busy Pad. All rights reserved.</p>
        </div>
    </footer>
</body>

</html>

  )";
}








