#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h> 

// =======================
// === PENGATURAN OLED I2C ===
// =======================
// Sesuaikan dengan konstruktor dan alamat I2C Anda (default 0x3C)
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 0x3C);

// ===============================
// === PENGATURAN PIN & WAKTU ===
// ===============================
const int LED_PIN = 8;          // LED Output untuk transmisi Morse
const int BUTTON_PIN = 2;       // Tombol Input untuk membuat kode Morse

// Konstanta Waktu untuk Kode Morse (dalam milidetik)
const int DEBOUNCE_DELAY = 50;      
const int DIT_DAH_THRESHOLD = 300;  // Durasi untuk membedakan Dit dan Dah

// Waktu untuk Dekoding dan Spasi
const int LETTER_SPACE_THRESHOLD = 1500; // Spasi antar huruf (memicu dekode)
const int WORD_SPACE_THRESHOLD = 3000;   // Spasi antar kata

// Variabel Status dan Logika
unsigned long buttonDownTime = 0;   
unsigned long buttonUpTime = 0;     
bool isButtonDown = false;          
String currentMorseCode = "";       // Menyimpan kode Morse yang belum di-decode
String decodedText = "";            // Menyimpan teks yang sudah di-decode

// =====================
// === FUNGSI SETUP ===
// =====================
void setup() {
    Serial.begin(9600);
    
    // --- Inisialisasi Pin ---
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP); 

    // --- INISIALISASI OLED (U8G2) ---
    u8g2.begin(); 
    
    // Tampilkan Layar Konfigurasi saat startup
    displayConfigScreen();
    
    // Tampilan Transmiter Morse
    u8g2.clearBuffer(); 
    u8g2.setFont(u8g2_font_5x7_tr); 
    u8g2.drawStr(0, 8, "MORSE TRANSMITTER/DECODE");
    u8g2.drawStr(0, 18, "Ready to Transmit...");
    u8g2.sendBuffer(); 
}

// ====================
// === FUNGSI LOOP ===
// ====================
void loop() {
    int buttonState = digitalRead(BUTTON_PIN);
    unsigned long currentTime = millis();

    // ------------------------------------
    // 1. DETEKSI TOMBOL (TRANSMISI)
    // ------------------------------------
    if (buttonState == LOW && !isButtonDown) { // Tombol baru saja ditekan
        buttonDownTime = currentTime;
        isButtonDown = true;
        digitalWrite(LED_PIN, HIGH); 
    }
    else if (buttonState == HIGH && isButtonDown) { // Tombol baru saja dilepas
        buttonUpTime = currentTime;
        isButtonDown = false;
        digitalWrite(LED_PIN, LOW); 
        
        unsigned long duration = buttonUpTime - buttonDownTime;
        handleTransmission(duration); // Simpan Dit/Dah
    }
    
    // ------------------------------------
    // 2. LOGIKA DEKODE (SPASI ANTAR HURUF/KATA)
    // ------------------------------------
    if (!isButtonDown && currentMorseCode.length() > 0) {
        unsigned long timeSinceLastRelease = currentTime - buttonUpTime;
        
        if (timeSinceLastRelease > LETTER_SPACE_THRESHOLD) {
            // SPASI ANTAR HURUF: Waktunya untuk dekode kode yang sudah terkumpul
            char decodedChar = morseToChar(currentMorseCode);
            
            if (decodedChar != '?') { 
                decodedText += decodedChar;
            } else {
                // Tambahkan karakter yang belum dikenal jika decoding gagal
                // decodedText += '?'; 
            }

            // Cek apakah jeda cukup panjang untuk SPASI ANTAR KATA
            if (timeSinceLastRelease > WORD_SPACE_THRESHOLD) { 
                if (decodedText.length() == 0 || decodedText[decodedText.length() - 1] != ' ') {
                    decodedText += " ";
                }
            }
            
            currentMorseCode = ""; // Reset kode untuk huruf berikutnya
            buttonUpTime = currentTime; // Reset waktu jeda
        }
    }
    
    // ------------------------------------
    // 3. TAMPILAN
    // ------------------------------------
    updateDisplay(currentMorseCode, decodedText); 
}

// =========================
// === FUNGSI PEMBANTU ===
// =========================

void handleTransmission(unsigned long duration) {
    if (duration > DEBOUNCE_DELAY) {
        if (duration < DIT_DAH_THRESHOLD) {
            currentMorseCode += "."; 
        } else {
            currentMorseCode += "-"; 
        }
    }
}

// --- FUNGSI DECODING MORSE LENGKAP (A-Z, 0-9) ---
char morseToChar(String morse) {
    // Alphabet A-Z
    if (morse == ".-") return 'A';
    if (morse == "-...") return 'B';
    if (morse == "-.-.") return 'C';
    if (morse == "-..") return 'D';
    if (morse == ".") return 'E';
    if (morse == "..-.") return 'F';
    if (morse == "--.") return 'G';
    if (morse == "....") return 'H';
    if (morse == "..") return 'I';
    if (morse == ".---") return 'J';
    if (morse == "-.-") return 'K';
    if (morse == ".-..") return 'L';
    if (morse == "--") return 'M';
    if (morse == "-.") return 'N';
    if (morse == "---") return 'O';
    if (morse == ".--.") return 'P';
    if (morse == "--.-") return 'Q';
    if (morse == ".-.") return 'R';
    if (morse == "...") return 'S';
    if (morse == "-") return 'T';
    if (morse == "..-") return 'U';
    if (morse == "...-") return 'V';
    if (morse == ".--") return 'W';
    if (morse == "-..-") return 'X';
    if (morse == "-.--") return 'Y';
    if (morse == "--..") return 'Z';
    
    // Angka 0-9
    if (morse == ".----") return '1';
    if (morse == "..---") return '2';
    if (morse == "...--") return '3';
    if (morse == "....-") return '4';
    if (morse == ".....") return '5';
    if (morse == "-....") return '6';
    if (morse == "--...") return '7';
    if (morse == "---..") return '8';
    if (morse == "----.") return '9';
    if (morse == "-----") return '0';

    // Karakter Lain (Opsional, umum digunakan)
    // if (morse == ".-.-.-") return '.'; // Titik
    // if (morse == "--..--") return ','; // Koma
    // if (morse == "---...") return ':'; // Titik dua
    
    return '?'; // Mengembalikan '?' jika kode tidak ditemukan
}

// --- FUNGSI MENAMPILKAN KONFIGURASI PADA OLED ---
void displayConfigScreen() {
    u8g2.clearBuffer(); 
    u8g2.setFont(u8g2_font_5x7_tr);
    
    // Baris 1: Judul
    u8g2.drawStr(0, 7, "** KONFIGURASI PIN & TIME **");
    
    // Baris 2: Pin LED dan Tombol
    String pinInfo = "LED: D" + String(LED_PIN) + " | BTN: D" + String(BUTTON_PIN);
    u8g2.drawStr(0, 18, pinInfo.c_str());
    
    // Baris 3: Threshold Waktu
    String timeInfo = "Dit Thr: " + String(DIT_DAH_THRESHOLD) + "ms";
    u8g2.drawStr(0, 28, timeInfo.c_str());
    
    u8g2.sendBuffer(); 
    delay(3000); 
}


// --- FUNGSI UNTUK MEMPERBARUI TAMPILAN OLED 128x32 ---
void updateDisplay(String morse, String decoded) {
    u8g2.clearBuffer(); 
    
    // Baris 1: Kode Morse Saat Ini
    u8g2.setFont(u8g2_font_7x13_tr); 
    u8g2.drawStr(0, 12, "Kode:"); 
    u8g2.drawStr(35, 12, morse.c_str()); 
    
    // Baris 2: Hasil Decode
    u8g2.setFont(u8g2_font_9x15_t_symbols); 
    
    // Batasi panjang teks decoded agar muat di layar
    if (decoded.length() > 16) {
        decoded = decoded.substring(decoded.length() - 16); 
    }
    u8g2.drawStr(0, 32, decoded.c_str()); 
    
    u8g2.sendBuffer(); 
}