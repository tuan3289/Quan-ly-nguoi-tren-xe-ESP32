#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// Khai báo LCD I2C với địa chỉ 0x27 và kích thước 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Chân nút bấm
const int buttonPin = 15;  //cảm biến hồng ngoai 
const int modeButtonPin = 27;  // Nút chuyển đổi chế độ tăng/giảm

// Chân điều khiển chế độ
const int modePin = 2;  // Chân 33 điều khiển khi ở chế độ tăng
const int modePin1 =4;  // Chân 32 điều khiển khi ở chế độ giảm

// Biến đếm số học sinh
int studentCount = 0;
bool buttonState = false;  // Trạng thái nút tăng
bool lastButtonState = false; // Trạng thái nút lần trước
bool modeButtonState = false; // Trạng thái nút chuyển chế độ
bool lastModeButtonState = false; // Trạng thái nút chuyển chế độ lần trước
bool increaseMode = true;  // Biến xác định chế độ (true = tăng, false = giảm)

unsigned long lastDebounceTime = 0;  // Thời gian để xử lý debouncing
unsigned long debounceDelay = 50;    // Thời gian debounce (ms)

// Thông tin Wi-Fi
const char* ssid = "P1";     // Thay YOUR_SSID bằng tên mạng Wi-Fi của bạn
const char* password = "999999999"; // Thay YOUR_PASSWORD bằng mật khẩu Wi-Fi của bạn

// Khởi tạo server
AsyncWebServer server(80);

void setup() {
  // Khởi động giao tiếp I2C
  Wire.begin(13, 14);  // SDA = 13, SCL = 14 (chỉnh tùy theo ESP32 của bạn)

  // Khởi động LCD
  lcd.begin();  // Sử dụng begin() thay cho init()
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("SO HS : 0");
  lcd.setCursor(0, 1);
  lcd.print("BY DANG TRUNG-62");
  
  // Cấu hình chân nút bấm với pull-up nội bộ
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(modeButtonPin, INPUT_PULLUP);
  pinMode(modePin, OUTPUT);  // Chân điều khiển chế độ
  pinMode(modePin1, OUTPUT); // Chân D32 điều khiển chế độ giảm

  // Cấu hình chân D35 để xuất mức LOW hoặc HIGH
  pinMode(35, OUTPUT);

  // Bắt đầu serial (nếu cần debug)
  Serial.begin(115200);

  // Kết nối Wi-Fi
  WiFi.begin(ssid, password);
  lcd.setCursor(0, 1);
  lcd.print("Connecting WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  // Khi kết nối thành công, hiển thị địa chỉ IP
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP: ");
  lcd.print(WiFi.localIP());
  delay(2000); // Hiển thị địa chỉ IP trong 2 giây

  // Định nghĩa route cho web server
 server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  String html = "<html><head>";
  html += "<meta charset='UTF-8'>"; // Thêm mã hóa UTF-8
  html += "<title>Quản lý số học sinh</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f9f9f9; }";
  html += ".header { display: flex; align-items: center; justify-content: flex-start; background-color: #003865; color: white; padding: 10px; }"; // Thay đổi ở đây
  html += ".header img { height: 50px; margin-right: 15px; }";
  html += "h1 { color: #003865; }";
  html += "button { padding: 10px 20px; margin: 10px; font-size: 16px; cursor: pointer; }";
  html += ".profile { margin-top: 20px; padding: 10px; background-color: #e0f7fa; border-radius: 8px; }";
  html += ".profile img { border-radius: 50%; height: 100px; width: 100px; }";
  html += ".profile h3 { color: #003865; }";
  html += ".profile p { color: #555; }";
  html += "</style>";
  html += "</head><body>";

  // Header với logo và thông tin trường
  html += "<div class='header'>";
  html += "<img src='https://sis.utc.edu.vn/logo.png' alt='Logo'>";
  html += "<h2>HỆ THỐNG GIÁM SÁT HỌC SINH TRÊN XE </h2>";
  html += "</div>";

  // Thông tin cá nhân
  html += "<div class='profile'>";
  html += "<img src='https://scontent.fsgn19-1.fna.fbcdn.net/v/t39.30808-1/272885157_657572332154949_7395247709639282217_n.jpg?stp=dst-jpg_s200x200_tt6&_nc_cat=105&ccb=1-7&_nc_sid=e99d92&_nc_eui2=AeHdCMn-uT1lWuZc7_yRGNx0gsnDHyJyoROCycMfInKhE_CLF6poevIuZvbhWbeI8kGay_n1lZpIg3GL5O49PnIg&_nc_ohc=MkjZwS0VQS8Q7kNvgFXFOEG&_nc_oc=AdheRf-wBX9FpGPclwZGS8u-GqdQ51__sXpOYGFPjnaiykiNoa1AxgKYR2Bw0dFZGO4&_nc_zt=24&_nc_ht=scontent.fsgn19-1.fna&_nc_gid=ATPYXeNwMUG_H08j2ehX7z_&oh=00_AYCiI6NcumcSlhGiXW8JgguzVTVplxP1Gc30bCZuZYBTDw&oe=676A2A43' alt='Avatar'>"; // Thay 'http://example.com/your-avatar.jpg' bằng đường dẫn tới ảnh của bạn
  html += "<h3>Đặng Thành Trung</h3>";
  html += "<p>Chuyên ngành: Kỹ sư viễn thông</p>";
  html += "<p>Mssv: 62510200 </p>";
  html += "</div>";

  // Nội dung chính
  html += "<h1>Quản lý số học sinh</h1>";
  html += "<h2>Số học sinh còn lại: <span id='studentCount'>" + String(studentCount) + "</span></h2>";
  html += "<h2>Trạng thái: <span id='modeStatus'>" + String(increaseMode ? "Lên xe" : "Xuống xe") + "</span></h2>";
  html += "<button onclick=\"location.href='/increase'\">Tăng</button>";
  html += "<button onclick=\"location.href='/decrease'\">Giảm</button>";
  html += "<button onclick=\"location.href='/toggle'\">Đổi chế độ</button>";

  // AJAX tự động cập nhật
  html += "<script>";
  html += "setInterval(function() {";
  html += "  fetch('/studentCount').then(response => response.text()).then(data => {";
  html += "    document.getElementById('studentCount').innerText = data;";
  html += "  });";
  html += "  fetch('/modeStatus').then(response => response.text()).then(data => {";
  html += "    document.getElementById('modeStatus').innerText = data;";
  html += "  });";
  html += "}, 1000);";
  html += "</script>";

  html += "</body></html>";
  request->send(200, "text/html", html);
});

server.on("/studentCount", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(200, "text/plain", String(studentCount));
});


server.on("/studentCount", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(200, "text/plain", String(studentCount));
});

server.on("/modeStatus", HTTP_GET, [](AsyncWebServerRequest *request){
  String modeStatus = increaseMode ? "Len xe" : "Xuong xe";
  request->send(200, "text/plain", modeStatus);
});

// Các routes điều khiển tăng giảm số học sinh và chế độ
server.on("/increase", HTTP_GET, [](AsyncWebServerRequest *request){
    if (increaseMode) studentCount++;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SO HS : ");
    lcd.print(studentCount);
    lcd.setCursor(0, 1);
   lcd.print("DANG THANH TRUNG");
    request->redirect("/");
  });

server.on("/decrease", HTTP_GET, [](AsyncWebServerRequest *request){
    if (studentCount > 0 && !increaseMode) studentCount--;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SO HS : ");
    lcd.print(studentCount);
    lcd.setCursor(0, 1);
    lcd.print("DANG THANH TRUNG");
    request->redirect("/");
  });

server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
    increaseMode = !increaseMode;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SO HS : ");
    lcd.print(studentCount);
    lcd.setCursor(0, 1);
    lcd.print("DANG THANH TRUNG");

    // Xuất tín hiệu ra GPIO
    digitalWrite(modePin, increaseMode ? LOW : HIGH);  // LOW khi chế độ tăng
    digitalWrite(modePin1, increaseMode ? HIGH : LOW); // HIGH khi chế độ tăng

    request->redirect("/");
  });

  // Bắt đầu server
  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  // Đọc trạng thái nút bấm
  bool reading = digitalRead(buttonPin);

  // Kiểm tra thay đổi trạng thái nút
  if (reading != lastButtonState) {
    lastDebounceTime = currentMillis; // Ghi nhận thời gian thay đổi
  }

  // Chỉ xử lý nếu thay đổi trạng thái ổn định sau thời gian debounce
  if ((currentMillis - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      // Nếu nút được nhấn (LOW)
      if (buttonState == LOW) {
        if (increaseMode) {
          studentCount++;
        } else if (studentCount > 0) {
          studentCount--;
        }

        // Hiển thị số học sinh lên LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SO HS : ");
        lcd.print(studentCount);
        lcd.setCursor(0, 1);
        lcd.print("DANG THANH TRUNG");
      }
    }
  }

  // Lưu trạng thái nút hiện tại
  lastButtonState = reading;

  // Xử lý nút chuyển chế độ tương tự
  bool modeReading = digitalRead(modeButtonPin);
  if (modeReading != lastModeButtonState) {
    lastDebounceTime = currentMillis;
  }
  if ((currentMillis - lastDebounceTime) > debounceDelay) {
    if (modeReading != modeButtonState) {
      modeButtonState = modeReading;
      if (modeButtonState == LOW) {
        increaseMode = !increaseMode;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Che Do: ");
        lcd.print("DANG THANH TRUNG");
      }
    }
  }
  lastModeButtonState = modeReading;
   if (increaseMode) {
    digitalWrite(modePin, LOW);  // Chân 33 LOW khi chế độ tăng
    digitalWrite(modePin1, HIGH); // Chân D32 HIGH khi chế độ tăng
  } else {
    digitalWrite(modePin, HIGH); // Chân 33 HIGH khi chế độ giảm
    digitalWrite(modePin1, LOW); // Chân D32 LOW khi chế độ giảm
  }
}