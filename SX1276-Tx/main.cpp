#include <cox.h>

SX127xChip &SX1276 = attachSX1276MB1LASModule();
Timer sendTimer;
RadioPacket *frame = NULL;
uint32_t sent = 0;

char buf[20];
int8_t modem;
Radio::LoRaSF_t sf;
Radio::LoRaCR_t cr;
Radio::LoRaBW_t bw;
int8_t txPower;
bool iq;
uint8_t syncword;

static void eventOnTxDone(void *ctx, bool success, GPIOInterruptInfo_t *) {
  printf("[%lu us] Tx %s!\n", micros(), (success) ? "SUCCESS" : "FAIL");
  delete frame;
  frame = NULL;
}

static void sendTask(void *args) {
  if (frame != NULL) {
    printf("Tx in progress...\n");
    return;
  }

  frame = new RadioPacket(125);

  if (!frame) {
    printf("Not enough memory\n");
    return;
  }

  for (uint8_t n = 2; n < frame->len; n++)
    frame->buf[n] = n;

  frame->buf[0] = (sent >> 8);
  frame->buf[1] = (sent & 0xff);

  SX1276.transmit(frame);

  printf("[%lu us] %lu Tx started...\n", micros(), sent);
  sent++;
}

static void eventKeyboardInput(SerialPort &) {
  reboot();
}

static void appStart() {
  Serial.onReceive(eventKeyboardInput);

  /* All parameters are specified. */
  SX1276.begin();

  if (modem == 0) {
    SX1276.setModemLoRa();
    SX1276.setDataRate(sf);
    SX1276.setCodingRate(cr);
    SX1276.setBandwidth(bw);
    SX1276.setIQMode(iq);
    SX1276.setSyncword(syncword);
  } else {
    SX1276.setModemFsk();
    SX1276.setDataRate(50000);
    SX1276.setBandwidth(50000);
    SX1276.setAfcBandwidth(83333);
    SX1276.setFdev(25000);
  }

  SX1276.setChannel(917100000);
  SX1276.setTxPower(txPower);
  SX1276.onTxDone = eventOnTxDone;
  SX1276.wakeup();
  SX1276.cca();

  sendTimer.onFired(sendTask, NULL);
  sendTimer.startPeriodic(5000);
}

static void inputSyncword(SerialPort &);
static void askSyncword() {
  printf("Enter syncword [0x12]:");
  Serial.onReceive(inputSyncword);
  Serial.inputKeyboard(buf, sizeof(buf));
}

static void inputSyncword(SerialPort &) {
  if (strlen(buf) == 0) {
    syncword = 0x12;
  } else {
    uint8_t sw = (uint8_t) strtoul(buf, NULL, 0);
    if (sw == 0) {
      printf("* Invalid syncword.\n");
      askSyncword();
      return;
    }

    syncword = sw;
  }

  printf("* Syncword: 0x%02X\n", syncword);
  appStart();
}

static void inputIQ(SerialPort &);
static void askIQ() {
  printf("Set IQ (0:normal, 1:inverted) [0]:");
  Serial.onReceive(inputIQ);
  Serial.inputKeyboard(buf, sizeof(buf));
}

static void inputIQ(SerialPort &) {
  if (strlen(buf) == 0 || strcmp(buf, "0") == 0) {
    printf("* Normal selected.\n");
    iq = true;
  } else if (strcmp(buf, "1") == 0) {
    printf("* inverted selected.\n");
    iq = false;
  } else {
    printf("* Unknown IQ mode\n");
    askIQ();
    return;
  }
  askSyncword();
}

static void inputTxPower(SerialPort &);
static void askTxPower() {
  printf("Set Tx power (1:-1, 2:10, 3:14 dBm) [1]:");
  Serial.onReceive(inputTxPower);
  Serial.inputKeyboard(buf, sizeof(buf));
}

static void inputTxPower(SerialPort &) {
  if (strlen(buf) == 0 || strcmp(buf, "1") == 0) {
    printf("* 1:-1 dBm selected.\n");
    txPower = -1;
  } else if (strcmp(buf, "2") == 0) {
    printf("* 2:10 dBm selected.\n");
    txPower = 10;
  } else if (strcmp(buf, "3") == 0) {
    printf("* 3:14 dBm selected.\n");
    txPower = 14;
  } else {
    printf("* Unknown Tx power.\n");
    askTxPower();
    return;
  }
  askIQ();
}

static void inputBW(SerialPort &);
static void askBW() {
  printf("Set bandwidth (0:125kHz, 1:250kHz, 2:500kHz) [0]:");
  Serial.onReceive(inputBW);
  Serial.inputKeyboard(buf, sizeof(buf));
}

static void inputBW(SerialPort &) {
  if (strlen(buf) == 0 || strcmp(buf, "0") == 0) {
    printf("* 125kHz selected.\n");
    bw = Radio::BW_125kHz;
  } else if (strcmp(buf, "1") == 0) {
    printf("* 250kHz selected.\n");
    bw = Radio::BW_250kHz;
  } else if (strcmp(buf, "2") == 0) {
    printf("* 500kHz selected.\n");
    bw = Radio::BW_500kHz;
  } else {
    printf("* Unknown SF\n");
    askBW();
    return;
  }
  askTxPower();
}

static void inputCR(SerialPort &);
static void askCR() {
  printf("Set coding rate (1:4/5, 2:4/6, 3:4/7, 4:4/8) [1]:");
  Serial.onReceive(inputCR);
  Serial.inputKeyboard(buf, sizeof(buf));
}

static void inputCR(SerialPort &) {
  if (strlen(buf) == 0 || strcmp(buf, "1") == 0) {
    printf("* (4/5) selected.\n");
    cr = Radio::CR_4_5;
  } else if (strcmp(buf, "2") == 0) {
    printf("* (4/6) selected.\n");
    cr = Radio::CR_4_6;
  } else if (strcmp(buf, "3") == 0) {
    printf("* (4/7) selected.\n");
    cr = Radio::CR_4_7;
  } else if (strcmp(buf, "4") == 0) {
    printf("* (4/8) selected.\n");
    cr = Radio::CR_4_8;
  } else {
    printf("* Unknown coding rate\n");
    askCR();
    return;
  }
  askBW();
}

static void inputSF(SerialPort &);
static void askSF() {
  printf("Set SF (7, 8, 9, 10, 11, 12) [7]:");
  Serial.onReceive(inputSF);
  Serial.inputKeyboard(buf, sizeof(buf));
}

static void inputSF(SerialPort &) {
  if (strlen(buf) == 0 || strcmp(buf, "7") == 0) {
    printf("* SF7 selected.\n");
    sf = Radio::SF7;
  } else if (strcmp(buf, "8") == 0) {
    printf("* SF8 selected.\n");
    sf = Radio::SF8;
  } else if (strcmp(buf, "9") == 0) {
    printf("* SF9 selected.\n");
    sf = Radio::SF9;
  } else if (strcmp(buf, "10") == 0) {
    printf("* SF10 selected.\n");
    sf = Radio::SF10;
  } else if (strcmp(buf, "11") == 0) {
    printf("* SF11 selected.\n");
    sf = Radio::SF11;
  } else if (strcmp(buf, "12") == 0) {
    printf("* SF12 selected.\n");
    sf = Radio::SF12;
  } else {
    printf("* Unknown SF\n");
    askSF();
    return;
  }
  askCR();
}

static void inputModem(SerialPort &);
static void askModem() {
  printf("Select modem (0: LoRa, 1:FSK) [0]:");
  Serial.onReceive(inputModem);
  Serial.inputKeyboard(buf, sizeof(buf));
}

static void inputModem(SerialPort &) {
  if (strlen(buf) == 0 || strcmp(buf, "0") == 0) {
    printf("* LoRa selected.\n");
    modem = 0;
    askSF();
  } else if (strcmp(buf, "1") == 0) {
    printf("* FSK selected.\n");
    modem = 1;
    appStart();
  } else {
    printf("* Unknown modem\n");
    askModem();
  }
}


void setup(void) {
  Serial.begin(115200);
  printf("*** SX1276 Low-Level Tx Control Example ***\n");

#if 1
  Serial.listen();
  askModem();
#else
  modem = 0;
  txPower = 14;
  cr = Radio::CR_4_8;
  sf = Radio::SF12;
  bw = Radio::BW_125kHz;
  iq = true;
  syncword = 0x12;
  appStart();
#endif
}
