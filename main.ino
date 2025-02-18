#include <Adafruit_PN532.h>

// This project modify from SigmaDolphin/Adafruit-PN532 fork from adafruit/Adafruit-PN532.

#include <SPI.h>
#include <string.h> // memcpy memcmp

#define GREEN_LED          (2)
#define BUZZER             (3)
#define BLUE_LED           (4)
#define RED_LED            (5)
#define SWITCH             (6)
#define BUTTON             (8)

#define PN532_SCK          (13)
#define PN532_MOSI         (11)
#define PN532_SS           (10)
#define PN532_MISO         (12)
#define PN532_RSTO         (9)

#define BUZZER_MAX_SOUND   (150)
// #define PN532_MAX_RETRY    (0xFE) // 0x00-0xFF

bool success; // 全域變數
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; // 全域變數
uint8_t prev_uid[] = { 0, 0, 0, 0, 0, 0, 0 }; // 全域變數
uint8_t uidLength; // 全域變數

// const uint8_t REPLACE_PART[8] = {0x67, 0x72, 0x65, 0x65, 0x6E, 0x39, 0x32, 0x35};
const uint8_t KEY_DEFAULT_KEY[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t block0buffer[16] = {0x00};

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

void gpio_begin(void){
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(PN532_RSTO, OUTPUT);
  pinMode(SWITCH, INPUT);
  pinMode(BUTTON, INPUT); // BUTTON 設INPUT+HIGH就是上拉電阻
  // testing light
  green_light_on();
  red_light_on();
  blue_light_on();
  beep_normal();
  delay(1000);
  green_light_off();
  red_light_off();
  blue_light_off();
  digitalWrite(BUTTON, HIGH); // BUTTON 設INPUT+HIGH就是上拉電阻
  digitalWrite(SWITCH, HIGH); // BUTTON 設INPUT+HIGH就是上拉電阻
}

void beep_warning(void){
  for (int i = 0 ; i < 3 ; i++){
    analogWrite(BUZZER, BUZZER_MAX_SOUND);
    delay(100);
    digitalWrite(BUZZER, LOW);
    delay(100);
  }
}

void beep_normal(void){
  for (int i = 0 ; i < 2 ; i++){
    analogWrite(BUZZER, BUZZER_MAX_SOUND);
    delay(100);
    digitalWrite(BUZZER, LOW);
    delay(100);
  }
}

void beep_good(void){
  analogWrite(BUZZER, BUZZER_MAX_SOUND);
  delay(100);
  digitalWrite(BUZZER, LOW);
  delay(100);
}

void red_light_on(void){
  analogWrite(RED_LED, 100);
}

void red_light_off(void){
  digitalWrite(RED_LED, LOW);
}

void green_light_on(void){
  digitalWrite(GREEN_LED, HIGH); // 不支援PWM
}

void green_light_off(void){
  digitalWrite(GREEN_LED, LOW);
}

void blue_light_on(void){
  digitalWrite(BLUE_LED, HIGH); // 不支援PWM
}

void blue_light_off(void){
  digitalWrite(BLUE_LED, LOW);
}

void clear_all_light(void){
  green_light_off();
  red_light_off();
  blue_light_off();
}

bool get_button_status(void){
  int status = digitalRead(BUTTON);
  if (status == 1) {
    return false;
  }
  return true;
}

bool get_switch_status(void){
  int status = digitalRead(SWITCH);
  if (status == 1) {
    return false;
  }
  return true;
}

void pn532_soft_reboot(void){
  if (!nfc.reboot()){
    Serial.println("reboot failed");
  }
}


void pn532_begin(void){
  uint32_t versiondata = nfc.getFirmwareVersion();
  while (! versiondata) {
    // retry every 1 sec if it doesn't connect
    Serial.print("Didn't find PN53x board");
    red_light_on();
    beep_warning();
    delay(1000);
  }
  /*
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  */
  nfc.SAMConfig(); // important. start listening
  // nfc.setPassiveActivationRetries(0xFE); // Set the max number of retry attempts to read from a card
}

void pn532_soft_begin(void){
  uint32_t versiondata = nfc.getFirmwareVersion();
  while (! versiondata) {
    // retry every 1 sec if it doesn't connect
    Serial.print("Didn't find PN53x board");
    red_light_on();
    beep_warning();
    delay(1000);
  }
  nfc.SAMConfig(); // important. start listening
  // nfc.setPassiveActivationRetries(PN532_MAX_RETRY); // Set the max number of retry attempts to read from a card
}

bool read_id(void){
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength, 100);
  if (success) {
    Serial.println("Found a card!");
    Serial.print("UID Length: ");
    Serial.print(uidLength, DEC); // 十進位
    Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i=0; i < uidLength; i++)
    {
      Serial.print(uid[i], HEX);
    }
    Serial.println("");
    return true;
  }
  else{
    Serial.println("Read card timeout.");
  }
  return false;
}

bool check_gen1_card(void){
  Serial.println("Checking gen1...");
  if (nfc.UnlockBackdoor()){
    Serial.println("Theres backdoor. It is magic gen1 card (aka UID card)");
    return true;
  }
  else{
    Serial.println("Theres no backdoor. Maybe it is Normal card or magic gen2 card (aka CUID card).");
  }
  return false;
}

bool check_gen2_card(void){
  Serial.println("Checking gen2...");
  success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 0, 0, KEY_DEFAULT_KEY);
  if (success){
    Serial.println("Authenticate successfully");
    success = nfc.mifareclassic_ReadDataBlock(0, block0buffer);
    if(success){ // read success
    success = nfc.mifareclassic_WriteDataBlock(0, block0buffer);
      if(success){
        Serial.println("Write successfully, This is a magic gen2 card (aka CUID card).");
        return true;
      }
      else{
        Serial.println("Write failed, This is a Normal card or magic gen1 card (aka UID card).");
      }
    }
    else{ // read fail
      Serial.println("Read failed");
    }
  }
  else{
    Serial.println("Authenticate failed");
  }
  return false;
}

void setup(void) {
  gpio_begin();
  Serial.begin(115200);
  nfc.begin();
  pn532_begin();
  clear_all_light();
}

bool is_uid_the_same(uint8_t uid[], uint8_t prev_uid[], size_t length) {
  // true為相同 false為不同
  return memcmp(uid, prev_uid, length) == 0; // 0為相同 非0為不同
}

void copy_uid() {
  memcpy(prev_uid, uid, sizeof(uid));
}

void clear_prev_uid() {
  uint8_t clear_uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  memcpy(prev_uid, clear_uid, sizeof(uid));
}

void case_continue(void) {
  /*
     ####    #####   ##   ##  ######    ####    ##   ##  ##   ##  #######
    ##  ##  ##   ##  ###  ##  # ## #     ##     ###  ##  ##   ##   ##   #
   ##       ##   ##  #### ##    ##       ##     #### ##  ##   ##   ## #
   ##       ##   ##  ## ####    ##       ##     ## ####  ##   ##   ####
   ##       ##   ##  ##  ###    ##       ##     ##  ###  ##   ##   ## #
    ##  ##  ##   ##  ##   ##    ##       ##     ##   ##  ##   ##   ##   #
     ####    #####   ##   ##   ####     ####    ##   ##   #####   #######
  */
  bool is_gen1 = false;
  bool is_gen2 = false;
  bool read_card = false;

  if (read_id()) {
    if (is_uid_the_same(uid, prev_uid, sizeof(uid))){
      delay(100);
      return;
    }
    is_gen2 = check_gen2_card();
    if(!is_gen2) {
      read_id(); // needs to select again
      is_gen1 = check_gen1_card();
    }
    pn532_begin();
    delay(200);
    copy_uid();
  }

  if (is_gen1){
    clear_all_light();
    green_light_on();
  }
  else if (is_gen2){
    clear_all_light();
    blue_light_on();
  }
  else{ // timeout or Normal card or NOT mifare card
    clear_all_light();
    red_light_on();
    clear_prev_uid(); // timeout表示前面的卡已經離開感應區

  }
  delay(200);
}

void case_single(void){
  /*
    #####    ####    ##   ##    ####   ####     #######
   ##   ##    ##     ###  ##   ##  ##   ##       ##   #
   #          ##     #### ##  ##        ##       ## #
    #####     ##     ## ####  ##        ##       ####
        ##    ##     ##  ###  ##  ###   ##   #   ## #
   ##   ##    ##     ##   ##   ##  ##   ##  ##   ##   #
    #####    ####    ##   ##    #####  #######  #######
  */
  bool is_gen1 = false;
  bool is_gen2 = false;
  bool read_card = false;

  if (read_id()) {
    is_gen2 = check_gen2_card();
    if(!is_gen2) {
      read_id(); // needs to select again
      is_gen1 = check_gen1_card();
    }
    pn532_begin();
    delay(200);
  }
  else{ // read card failed (no card)
    clear_all_light();
    red_light_on();
    beep_normal();
    return;
  }

  if (is_gen1){
    clear_all_light();
    green_light_on();
    beep_good();
  }
  else if (is_gen2){
    clear_all_light();
    blue_light_on();
    beep_good();
  }
  else{ // timeout or Normal card or NOT mifare card
    clear_all_light();
    red_light_on();
    beep_good();
  }
  delay(200);
}

void loop(void){

  if (get_switch_status() && get_button_status()){  // case_single
    Serial.println("case_single");
    case_single();
  }
  else if (!get_switch_status()){  // case_continue
    case_continue();
  }
  delay(1);
}

