#include <HijelHID_BLEKeyboard.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==================== Pin OLED ====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ==================== Pin RFID ====================
#define SS_PIN  5
#define RST_PIN 15
MFRC522 rfid(SS_PIN, RST_PIN);

// ==================== Keyboard Bluetooth ====================
HijelHID_BLEKeyboard keyboard;

// ==================== Password Laptop ====================
const char* passwordLaptop = "passwordanda"; 

// ==================== UID Kartu ====================
byte kartuKerja[] = {0x1, 0x4C, 0x26, 0x64};
byte kartuSantai[] = {0x80, 0xE4, 0xB5, 0x54};
byte kartuPanic[] = {0x5, 0x88, 0x79, 0x87, 0xFE, 0x24, 0x0}; 

// ==================== Link & URI ====================
const char* spotifyUri = "musikapasaja";
const char* linkYouTube = "linkyoutubebebas"; 
const char* link = "linkapasaja";

bool sedangProses = false;

// ==================== FUNGSI OLED ====================
void tampilLayar(String baris1, String baris2 = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 15); // Posisi agak ke tengah
  display.println(baris1);
  
  if (baris2 != "") {
    display.setCursor(0, 35);
    display.println(baris2);
  }
  display.display();
}

// ==================== FUNGSI BANTUAN ====================
bool cekUID(byte *uid, byte *kartuSah, byte panjang) {
  if (rfid.uid.size != panjang) return false;
  for (byte i = 0; i < panjang; i++) {
    if (uid[i] != kartuSah[i]) return false;
  }
  return true;
}

void ketikPassword(const char* pass) {
  for (int i = 0; i < strlen(pass); i++) {
    keyboard.print(pass[i]);
    delay(100); 
  }
}

void bukaAplikasi(String perintah) {
  keyboard.tap(KEY_R, KEY_MOD_LGUI); 
  delay(800); 
  keyboard.print(perintah);
  delay(200);
  keyboard.tap(KEY_RETURN); 
  delay(2500); 
}

void bukaLink(String url) {
  keyboard.tap(KEY_R, KEY_MOD_LGUI); 
  delay(800); 
  keyboard.print(url);
  delay(200);
  keyboard.tap(KEY_RETURN); 
  delay(3000); 
}

void bukaYouTubeFullscreen(String url) {
  keyboard.tap(KEY_R, KEY_MOD_LGUI); 
  delay(800); 
  keyboard.print(url);
  delay(200);
  keyboard.tap(KEY_RETURN); 
  delay(8000); 
  keyboard.tap(KEY_F); 
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  
  // Inisialisasi OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED gagal terdeteksi"));
    for(;;); // Hentikan program jika OLED tidak ada
  }
  
  tampilLayar("Sistem Siap", "Silakan Tap Kartu");

  SPI.begin();
  rfid.PCD_Init();
  keyboard.begin();
}

// ==================== LOOP ====================
void loop() {
  if (sedangProses) return;

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  sedangProses = true;

  if (!keyboard.isPaired()) {
    tampilLayar("Bluetooth", "Belum Terhubung!");
    delay(2000);
    tampilLayar("Sistem Siap", "Silakan Tap Kartu");
    sedangProses = false;
    rfid.PICC_HaltA();
    return;
  }

  // ==================== LOGIKA KARTU ====================
  
  // 1. KARTU KERJA
  if (cekUID(rfid.uid.uidByte, kartuKerja, sizeof(kartuKerja))) {
    tampilLayar("Mode KERJA", "Membuka Aplikasi...");
    
    delay(1000); 
    ketikPassword(passwordLaptop);
    delay(300);
    keyboard.tap(KEY_RETURN);
    delay(3000); 
    
    bukaAplikasi("cmd");
    bukaAplikasi("code"); 
    bukaAplikasi("chrome");
    bukaLink(link);
    bukaLink(spotifyUri); 
  }
  
  // 2. KARTU SANTAI
  else if (cekUID(rfid.uid.uidByte, kartuSantai, sizeof(kartuSantai))) {
    tampilLayar("Mode SANTAI", "YouTube Fullscreen");
    
    delay(1000); 
    ketikPassword(passwordLaptop);
    delay(300);
    keyboard.tap(KEY_RETURN);
    delay(3000);
    
    bukaYouTubeFullscreen(linkYouTube); 
  }

  // 3. KARTU PANIC
  else if (cekUID(rfid.uid.uidByte, kartuPanic, sizeof(kartuPanic))) {
    tampilLayar("PANIC MODE", "Mengunci Layar...");
    keyboard.tap(KEY_L, KEY_MOD_LGUI);
  }

  // 4. KARTU TIDAK DIKENAL
  else {
    tampilLayar("Akses Ditolak", "Kartu Asing!");
  }

  delay(3000); // Tahan tampilan sebentar
  
  // Kembalikan ke layar standby
  tampilLayar("Sistem Siap", "Silakan Tap Kartu");
  sedangProses = false;
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}