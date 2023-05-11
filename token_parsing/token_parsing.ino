#include <string.h>
#include <stdlib.h>

char str[80] = "T120P129P144P96";
float psi[4];
char *token;
//short i = 0;
float pressure;

void setup() {
  Serial.begin(9600);
}

void loop() {
  token = strtok(&str[1], "P");
  /*while(token != NULL) {
        //Serial.println(atoi(token));
        psi[i] = atoi(token) * 0.1 + 25.6;
        token = strtok(NULL, "P");
        i++;
    }*/
  
  for (int i = 0; token != NULL; i++) {
    psi[i] = atoi(token) * 0.1 + 25.6;
    token = strtok(NULL, "P");
  }

  for (int i = 0; i < 4; i++) {
    Serial.println(psi[i]);
  }
  Serial.println();
  delay(1E3);
}
