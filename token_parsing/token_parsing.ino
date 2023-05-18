#include <string.h>
#include <stdlib.h>

char str[80] = "T95P121P144P96";
float psi[4];
//short i = 0;
float pressure;

void setup() {
  Serial.begin(9600);
}

void loop() {
  
  parseTPMS(str, psi);

  for (int i = 0; i < 4; i++) {
    Serial.println(psi[i]);
  }
  Serial.println();
  delay(1E3);
}

void parseTPMS(char str[], float out[] ) {
  char *tokenPtr = strtok(&str[1], "P");

  for (int i = 0; tokenPtr != NULL; i++) {
    out[i] = atoi(tokenPtr) * 0.1 + 25.6;
    tokenPtr = strtok(NULL, "P");
  }
}