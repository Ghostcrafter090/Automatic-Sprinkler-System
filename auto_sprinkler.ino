const double airValue = 890;   //you need to replace this value with Value_1
const double waterValue = 730;  //you need to replace this value with Value_2
double soilMoistureValue = 0;

const double openValve = 0.6;
const double closeValve = 0.85;

int debugPin = 0;

unsigned long statusTic = 0;

const int controlValves[4] = {2, 3, 4, 5};
const int moistureSensors[4] = {A0, A1, A2, A3};
const int valveCount = 4;

bool controlValveStates[4] = {false, false, false, false};
bool valveStates[2][4] = {{false, false, false, false}, {false, false, false, false}};
double moistureSensorPercents[4] = {0, 0, 0, 0};

void setup() {
  for (auto &valve : controlValves) {
    pinMode(valve, HIGH);
  }
  debugPin = controlValves[-1] + 1;
  pinMode(debugPin, INPUT);

  Serial.begin(115200); // open serial port, set the baud rate to 115200 bps
  Serial.println("started.");
}

void loop() {

  // Water Line Sensing
  int i = 0;
  double dryestSensor[2] = {1, -1};
  for (auto sensor : moistureSensors) {
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

  // Status Messaging
  if (millis() > statusTic) {
    
    // Valve Status Apply
    i = 0;
    for (auto valve : controlValves) {
      if (valveStates[0][i]) {
        if (!valveStates[1][i]) {
          digitalWrite(valve, HIGH);
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
          digitalWrite(valve, LOW);
          valveStates[1][i] = false;
          Serial.println("");
          Serial.print("{\"messageType\":\"valve_event\",\"valveId\":");
          Serial.print(i);
          Serial.print(",\"valveState\":");
          Serial.print(valveStates[1][i]);
          Serial.print("}");
        }
      }
      i = i + 1;
    }
    
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
  }
}
