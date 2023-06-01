const double airValue = 890;   //you need to replace this value with Value_1
const double waterValue = 710;  //you need to replace this value with Value_2
const double openValve = 0.75;
const double closeValve = 0.85;

// int debugPin = 0;

unsigned long statusTic = 0;
unsigned long displayTic = 0;
unsigned long secondDisplayTic = 0;
unsigned long activeTic = 0;


const bool intToBytes[4][2] = {{0, 1}, {1, 0}, {1, 1}};

const int controlValves[4] = {2, 3, 4, 5};
const int moistureSensors[4] = {A2, A3, A4, A5};
const int valveCount = 4;

int led0 = 8;
int led1 = 9;
int led2 = 13;

bool activeBool = false;

bool controlValveStates[4] = {false, false, false, false};
bool valveStates[2][4] = {{false, false, false, false}, {false, false, false, false}};
double moistureSensorPercents[4] = {0, 0, 0, 0};

void setup() {
  for (auto &valve : controlValves) {
    pinMode(valve, OUTPUT);
  }
  // debugPin = controlValves[-1] + 1;
  //  pinMode(debugPin, INPUT);
  pinMode(led0, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  Serial.begin(115200); // open serial port, set the baud rate to 115200 bps
  Serial.println("started.");
}

int fn = 0;
int fg = 0;
bool derpf = true;

void loop() {

  // Water Line Sensing
  int i = 0;
  double dryestSensor[2] = {1, -1};
  for (auto &sensor : moistureSensors) {
    moistureSensorPercents[i] = ((moistureSensorPercents[i] * 100) + ((airValue - analogRead(sensor)) / (airValue - waterValue))) / 101;

    // Find Dryest
    if (moistureSensorPercents[i] > 1) {
      moistureSensorPercents[i] = 1;
    }
    if (moistureSensorPercents[i] < 0) {
      moistureSensorPercents[i] = 0;
    }

    // Request Water
    if (moistureSensorPercents[i] < openValve) {
      if (!controlValveStates[i]) {
        controlValveStates[i] = true;
      }
    }
    if (controlValveStates[i]) {
      if (moistureSensorPercents[i] <= dryestSensor[0]) {
        dryestSensor[0] = moistureSensorPercents[i];
        dryestSensor[1] = i;
      }
    }
    if (moistureSensorPercents[i] > closeValve) {
      if (controlValveStates[i]) {
        controlValveStates[i] = false;
      }
    }
    i = i + 1;
  }

  // Water Line Control
  i = 0;
  while (i < valveCount) {
    valveStates[0][i] = false;
    i = i + 1;
  }
  if (dryestSensor[1] != -1) {
    valveStates[0][(int)dryestSensor[1]] = true;
  }

  // Display
  if ((millis() > displayTic) && derpf) {
    fn = 0;
    fg = 0;
    derpf = false;
  }
  if ((fn < 4) && (!derpf)) {
    if (millis() > secondDisplayTic) {
      fg = 0;
      while (fg < (fn + 1)) {
        digitalWrite(led0, HIGH);
        digitalWrite(led2, HIGH);
        delay(250);
        digitalWrite(led0, LOW);
        digitalWrite(led2, LOW);
        delay(250);
        fg = fg + 1;
      }
      delay(600);
      fg = 0;
      while (fg < floor((moistureSensorPercents[fn] * 100) / 10)) {
        digitalWrite(led1, HIGH);
        digitalWrite(led2, HIGH);
        delay(250);
        digitalWrite(led1, LOW);
        digitalWrite(led2, LOW);
        delay(250);
        fg = fg + 1;
      }
      delay(600);
      fg = 0;
      while (fg < floor(((moistureSensorPercents[fn] * 100) - ((int)floor((moistureSensorPercents[fn] * 100) / 10)) * 10))) {
        digitalWrite(led0, HIGH);
        digitalWrite(led1, HIGH);
        digitalWrite(led2, HIGH);
        delay(250);
        digitalWrite(led0, LOW);
        digitalWrite(led1, LOW);
        digitalWrite(led2, LOW);
        delay(250);
        fg = fg + 1;
      }
      secondDisplayTic = millis() + 2500;
      fn = fn + 1;
    }
  }
  if (fn > 3) {
    fn = 0;
    derpf = true;
    displayTic = millis() + 10000;
  }

  if (millis() > activeTic) {
    if (activeBool) {
      digitalWrite(led2, LOW);
      activeBool = false;
    } else {
      digitalWrite(led2, HIGH);
      activeBool = true;
    }
    activeTic = millis() + 100;
  }

  // Once Per Second Timer
  if (millis() > statusTic) {
    bool valveOn = false;
    // Valve Status Apply
    i = 0;
    for (auto &sensor : moistureSensors) {
      if (i <= valveCount) {
        if (valveStates[0][i]) {
          if (!valveStates[1][i]) {
            digitalWrite(controlValves[i], LOW);
            valveStates[1][i] = true;
            Serial.println("");
            Serial.print("{\"messageType\":\"valve_event\",\"valveId\":");
            Serial.print(i);
            Serial.print(",\"valveState\":");
            Serial.print(valveStates[1][i]);
            Serial.print("}");
          }
        } else {
          if (valveStates[1][i]) {
            digitalWrite(controlValves[i], HIGH);
            valveStates[1][i] = false;
            Serial.println("");
            Serial.print("{\"messageType\":\"valve_event\",\"valveId\":");
            Serial.print(i);
            Serial.print(",\"valveState\":");
            Serial.print(valveStates[1][i]);
            Serial.print("}");
          }
        }
      }
      i = i + 1;
    }

    // Status Messaging
    Serial.println("");
    Serial.print("{\"messageType\":\"status\",\"sensorValues\":{");
    i = 0;
    while (i < valveCount) {
      Serial.print(moistureSensors[i]);
      Serial.print(":");
      Serial.print(moistureSensorPercents[i] * 100.0);
      Serial.print(",");
      i = i + 1;
    }
    Serial.print("},\"valveStates\":{");
    i = 0;
    while (i < valveCount) {
      Serial.print(controlValves[i]);
      Serial.print(":[");
      Serial.print(controlValveStates[i]);
      Serial.print(",");
      Serial.print(valveStates[0][i]);
      Serial.print(",");
      Serial.print(valveStates[1][i]);
      Serial.print("],");
      i = i + 1;
    }
    Serial.print("}");
    statusTic = millis() + 1000;
    int ig = 0;
    while (ig < 4) {
      if (moistureSensorPercents[ig] > 95) {
        digitalWrite(controlValves[ig], LOW);
      }
      ig = ig + 1;
    }
  }
}
