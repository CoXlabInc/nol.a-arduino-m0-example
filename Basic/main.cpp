#include <cox.h>

Timer tPrint;

static void printTask(void *) {
  ledToggle();
  printf("[%lu usec] Hi!\n", micros());
}

static void keyboard(SerialPort&) {
  ledToggle();
  printf("[%lu usec] Keyboard input\n", micros());
}

void setup() {
  ledOn();
  Serial.begin(115200);
  printf("\n*** Basic functions test for Arduino-M0 ***\n");

  tPrint.onFired(printTask, NULL);
  tPrint.startPeriodic(1000);

  Serial.listen();
  Serial.onReceive(keyboard);
}
