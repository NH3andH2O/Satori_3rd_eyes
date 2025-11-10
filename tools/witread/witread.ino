/*
本程式碼用於獲取陀螺儀的原始資訊
上傳程式碼後，即可使用上位機檢視陀螺儀的資料
*/

#define serial2
/*
如果需要獲得眼睛陀螺儀（pcd上面的陀螺儀）的原始資訊，請刪除 #define serial2
如果需要獲得頭部陀螺儀（透過杜邦線連線的陀螺儀）的原始資訊，請保留 #define serial2
*/

#define serialPort 115200
/*
波特率相關
*/

void setup() {
	Serial.begin(serialPort);
	#ifdef serial2
	Serial1.begin(serialPort, SERIAL_8N1, 39, 38);
	#else
	Serial1.begin(serialPort, SERIAL_8N1, 18, 8);
	#endif
}

void loop() {
	if(Serial.available()){
		Serial1.write(Serial.read());
	}
		if(Serial1.available()){
		Serial.write(Serial1.read());
	}
}
