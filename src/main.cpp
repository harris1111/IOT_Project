/*
 * Typical pin layout used:
 * -------------------------------------
 *             MFRC522      Arduino    
 *             Reader/PCD   Uno/101      
 * Signal      Pin          Pin           
 * -------------------------------------
 * RST/Reset   RST          9             
 * SPI SS      SDA(SS)      10            
 * IRQ         ?            ?             
 * SPI MOSI    MOSI         11 / ICSP-4   
 * SPI MISO    MISO         12 / ICSP-1  
 * SPI SCK     SCK          13 / ICSP-3   
*/


#include <Arduino.h>



#include <SPI.h>
#include <MFRC522.h>
// #include <EEPROM.h>
#include <Eeprom24Cxx.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);


#define RST_PIN 9
#define SS_PIN 10

unsigned long UID[4];
unsigned long i;

MFRC522 mfrc522(SS_PIN, RST_PIN);

int numOfMemory = 0;
int count = 0;
int count_total = 0;
int count_menu = 0;
int led = 8;
int id_new[4];
int enter = 7;
int enterVal = 0;
int up = 5;
int upVal = 0;
int down = 6;
int downVal = 0;

int total = 0;
int defaultVal = 1;
int valZero;
int n = 4;
int addrZero = 0;
int addr_del = 0;


void LCD_begin() {
  lcd.setCursor(5, 0);
  lcd.print("HELLO");
  lcd.setCursor(0, 1);
  lcd.print("MOI NHAP THE...");
}

void screenSave() {
  lcd.clear();
  lcd.print("DANG LUU...");
  delay(3000);
}

void screenSetupMaster() {
  lcd.setCursor(2, 0);
  lcd.print("SETUP MASTER");
  lcd.setCursor(0, 1);
  lcd.print("MOI NHAP THE...");
}

void menu() {
  if (count_total == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(">TRO VE");
    lcd.setCursor(0, 1);
    lcd.print(" THEM THE");
  } else if (count_total == 1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" TRO VE");
    lcd.setCursor(0, 1);
    lcd.print(">THEM THE");
  } else if (count_total == 2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(">XOA THE BAT KY");
    lcd.setCursor(0, 1);
    lcd.print(" XOA TAT CA THE");
  } else if (count_total == 3) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" XOA THE BAT KY");
    lcd.setCursor(0, 1);
    lcd.print(">XOA TAT CA THE");
  }
}

// Thao tác trên menu
void selectMenu() {
  if (count_total == 1) {  //nhập thẻ mới
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("THEM THE MOI");
    lcd.setCursor(0, 1);
    lcd.print("MOI NHAP THE....");
  } else if (count_total == 2) {  //Xóa thẻ bất kỳ
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("XOA THE BAT KY ");
    lcd.setCursor(0, 1);
    lcd.print("MOI NHAP THE....");
  } else if (count_total == 3) {  //Xóa tất cả thẻ
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("XOA TAT CA THE");
  }
}

bool ss_epprom(int arr[]) {
  int item = 0;
  int tempVal[4];

  for (int i = 5; i < numOfMemory; i++) {
    tempVal[item] = EEPROM.read(i);
    if (item == 3) {
      if (arr[0] == tempVal[0] && arr[1] == tempVal[1] && arr[2] == tempVal[2] && arr[3] == tempVal[3]) {
        //Thẻ tồn tại
        item = 0;
        addr_del = i;
        return true;
      } else {
        //Thẻ không tồn tại
        item = -1;
      }
    }
    item++;
  }
  return false;
}

bool checkMasterExist() {
  int masVal = 0;
  for (int i = 1; i < 4; i++) {
    masVal = EEPROM.read(i) + masVal;
  }
  Serial.println(masVal);
  if (masVal == 0) {
    return false;
  } else {
    return true;
  }
}

bool checkIsMaster(int arr[]) {
  if (arr[0] == EEPROM.read(1) && arr[1] == EEPROM.read(2) && arr[2] == EEPROM.read(3) && arr[3] == EEPROM.read(4)) {
    return true;
  } else {
    return false;
  }
}

void setupMaster() {
  int addr = 1;
  screenSetupMaster();
  if (!mfrc522.PICC_IsNewCardPresent()) { return; }
  if (!mfrc522.PICC_ReadCardSerial()) { return; }
  for (byte i = 0; i < 4; i++) {
    UID[i] = mfrc522.uid.uidByte[i];
    EEPROM.write(addr, UID[i]);
    addr = addr + 1;
  }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(50);
  screenSave();
  lcd.clear();
}

void setupNewCard(int addr, int arr[]) {
  for (byte i = 0; i < 4; i++) {
    EEPROM.write(addr, arr[i]);
    addr = addr + 1;
    numOfMemory = addr;
  }
  EEPROM.write(0, addr);  // Sau khi lưu 1 thẻ mới vào thì cập nhật số ô nhớ đã sử dụng vào ô 0
  screenSave();
  lcd.clear();
  selectMenu();
}

void findEmptyMemory() {
  int n = 0;
  while (n < numOfMemory) {
    if (EEPROM.read(n) == 0) {
      addrZero = n;
      break;
    }
    n++;
  }
  if (addrZero == 0) {
    addrZero = numOfMemory;
  }
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  pinMode(led, OUTPUT);
  mfrc522.PCD_Init();
  lcd.init();
  lcd.backlight();
  pinMode(enter, INPUT_PULLUP);
  pinMode(up, INPUT_PULLUP);
  pinMode(down, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  // Đoạn code reset bộ nhớ Eeprom
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0);
  }
}



void loop() {
  enterVal = digitalRead(enter);
  upVal = digitalRead(up);
  downVal = digitalRead(down);


  if (checkMasterExist() == false) {  // Kiểm tra có thẻ master chưa
    setupMaster();                    //Chưa -> setupMaster
  } else {                            // Có -> màn hình chính
    total = 1;
    if (total == 1 && count == 0) {
      LCD_begin();
      if (!mfrc522.PICC_IsNewCardPresent()) { return; }
      if (!mfrc522.PICC_ReadCardSerial()) { return; }
      for (byte i = 0; i < 4; i++) {
        UID[i] = mfrc522.uid.uidByte[i];
        id_new[i] = UID[i];
      }
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      if (checkIsMaster(id_new) == true) {  //kiểm tra thẻ đưa vào có phải master k
        count = 1;
        if (total == 1 && count == 1) {  // Thẻ master -> vào màn hình cài đặt
          menu();
        }
      } else {
        if (ss_epprom(id_new) == true) {
          //Thẻ tồn tại
          digitalWrite(led, HIGH);
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("   MO DEN....   ");
          delay(5000);
          digitalWrite(led, LOW);
          lcd.clear();
        } else if (ss_epprom(id_new) == false) {
          //Thẻ không tồn tại
          digitalWrite(led, LOW);
          lcd.clear();
          lcd.setCursor(0, 1);
          lcd.print("    SAI THE!    ");
          delay(3000);
          lcd.clear();
        }
      }
    }
  }

  if (count_menu == 1 && count_total == 1 && count == 1) {
    numOfMemory = EEPROM.read(0);
    if (numOfMemory == 0) {  // không có thẻ con
      int addrTemp = 5;
      if (!mfrc522.PICC_IsNewCardPresent()) {
        return;
      }
      if (!mfrc522.PICC_ReadCardSerial()) {
        return;
      }
      for (byte i = 0; i < 4; i++)  // Quét thẻ mới
      {
        UID[i] = mfrc522.uid.uidByte[i];
        id_new[i] = UID[i];
      }
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      if (checkIsMaster(id_new) == true) {
        lcd.clear();
        menu();
        count_menu = 0;
      } else {
        setupNewCard(addrTemp, id_new);
      }
    } else if (numOfMemory != 0) {  //có thẻ con
      if (!mfrc522.PICC_IsNewCardPresent()) {
        return;
      }
      if (!mfrc522.PICC_ReadCardSerial()) {
        return;
      }
      for (byte i = 0; i < 4; i++)  // Quét thẻ mới
      {
        UID[i] = mfrc522.uid.uidByte[i];
        id_new[i] = UID[i];
      }
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      if (ss_epprom(id_new) == true) {  //kiểm tra có bị trùng id thẻ hay không
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("THE DA CO !");
        delay(1000);
        lcd.clear();
        selectMenu();
      } else if (checkIsMaster(id_new) == true) {  // kiểm tra có phải thẻ master k
        lcd.clear();
        LCD_begin();
        count_menu = 0;
        count = 0;
        count_total = 0;
      } else if (ss_epprom(id_new) == false) {
        findEmptyMemory();
        if (addrZero == numOfMemory) { //Chưa có thẻ phụ
          setupNewCard(addrZero, id_new);
        } else if (addrZero != numOfMemory) { // đã có thẻ phụ
          for (byte i = 0; i < 4; i++) {
            EEPROM.write(addrZero, id_new[i]);
            addrZero = addrZero + 1;
          }
          addrZero = numOfMemory;
          screenSave();
          lcd.clear();
          selectMenu();
        }
      }
    }
  } else if (count_menu == 1 && count_total == 2 && count == 1) {  //Xóa thẻ bất kì
    numOfMemory = EEPROM.read(0);
    if (numOfMemory == 0) {  //Không có thẻ phụ
      lcd.setCursor(0, 1);
      lcd.print(" CHUA CO THE... ");
      delay(1000);
      lcd.clear();
      menu();
      count_menu = 0;
    } else {  //Có thẻ phụ
      if (!mfrc522.PICC_IsNewCardPresent()) {
        return;
      }
      if (!mfrc522.PICC_ReadCardSerial()) {
        return;
      }
      for (byte i = 0; i < 4; i++)  // Quét thẻ mới
      {
        UID[i] = mfrc522.uid.uidByte[i];
        id_new[i] = UID[i];
      }
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      if (checkIsMaster(id_new) == true) {
        count_menu = 0;
        count_total = 0;
        count = 0;  
        lcd.clear();
        LCD_begin();
      } else if (ss_epprom(id_new) == false) {
        lcd.setCursor(0, 1);
        lcd.print(" THE CHUA CO... ");
        delay(1000);
        lcd.clear();
        selectMenu();
      }
      else if (ss_epprom(id_new) == true) {
        addr_del = addr_del - 3;
        for (int i = 0; i < 4; i++) {
          EEPROM.write(addr_del, 0);
          addr_del = addr_del + 1;
        }
        lcd.setCursor(0, 1);
        lcd.print(" DA XOA THE.... ");
        delay(1000);
        lcd.clear();
        selectMenu();
      } 
    }
  } else if (count_menu == 1 && count_total == 3 && count == 1) { //Xóa tất cả thẻ
    numOfMemory = EEPROM.read(0);
    if (numOfMemory == 0) {
      lcd.setCursor(0, 1);
      lcd.print(" CHUA CO THE... ");
      count_menu = 0;
      delay(2000);
      lcd.clear();
      menu();
    } else if (numOfMemory > 0) {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("  DANG XOA....  ");
      for (int i = 5; i < numOfMemory; i++)  // Bắt đầu từ ô 5 đến ô đang sử dụng
      {
        EEPROM.write(i, 0);
      }
      EEPROM.write(0, 0);
      delay(3000);
      lcd.clear();
      LCD_begin();
      count_menu = 0;
      count = 0;
      count_total = 0;
    }
    addrZero = 0;
    numOfMemory = 0;
  }



  // Button lên
  if (upVal != defaultVal) {
    if (upVal == 0) {
      if (count == 1) {
        if (count_total <= 0) {
          count_total = 3;
        } else {
          count_total--;
        }
        menu();
      }
      delay(100);
    }
    defaultVal = upVal;
  }
  // Button xuống
  if (downVal != defaultVal) {
    if (downVal == 0) {
      if (count == 1) {
        if (count_total >= 3) {
          count_total = 0;
        } else {
          count_total++;
        }
        menu();
      }
      delay(100);
    }
    defaultVal = downVal;
  }
  // Button confirm
  if (enterVal != defaultVal) {
    if (enterVal == 0) {
      if (count == 1 && count_total != 0) {
        selectMenu();
        count_menu = 1;
      } else if (count == 1 && count_total == 0) {
        lcd.clear();
        LCD_begin();
        count = 0;
      }
      delay(100);
    }
    defaultVal = enterVal;
  }
}
