#include <cox.h>

Timer tPrint;

static void printTask(void *) {
  System.ledToggle();
  struct timeval t;
  gettimeofday(&t, NULL);
  printf("[%lu.%06lu] Hi!\n", (uint32_t) t.tv_sec, t.tv_usec);
}

static void keyboard(SerialPort&) {
  System.ledToggle();
  printf("[%lu usec] Keyboard input\n", micros());
}

void setup() {
  System.ledOn();
  Serial.begin(115200);
  printf("\n*** Basic functions test for Arduino-M0 ***\n");
  struct tm t;
  t.tm_year = 2018 - 1900;
  t.tm_mon = 5 - 1;
  t.tm_mday = 23;
  t.tm_hour = 0;
  t.tm_min = 0;
  t.tm_sec = 0;
  System.setDateTime(t);

  tPrint.onFired(printTask, NULL);
  tPrint.startPeriodic(100);

  Serial.listen();
  Serial.onReceive(keyboard);
}
