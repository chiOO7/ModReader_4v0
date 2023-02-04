void processingPostReqestData() {
    if(HTTP.args() == 0) {
        return;
    }
    if (HTTP.hasArg("id")) {
        id = HTTP.arg("id").toInt();
    }
    if (HTTP.hasArg("command")) {
        command = HTTP.arg("command").toInt();
    }
    if (HTTP.hasArg("reg")) {
        reg = HTTP.arg("reg").toInt();
    }
    if (HTTP.hasArg("val")) {
        val = HTTP.arg("val").toInt();
    }
}

String processingModbusResponse() {
  	if (Serial.available() > 0) {
        int bytesCount = modbusGetRxBuffer();
	  	if (bytesCount == (val * 2 + 5)) {
            printToWeb = "";
            printToWeb += "[";
		    for (int b = 3; b < bytesCount - 2; b += 2) {
		        printToWeb += ((rxBuffer[b] << 8) + rxBuffer[b + 1]);
		        printToWeb += ",";
		    }
		    printToWeb = printToWeb.substring(0, printToWeb.length() - 1);
		    printToWeb += "]";
		    } else {
			      printToWeb = "[\"ERROR!\"]";
		    }
	} else {
		    printToWeb = "";
	}
  
    return printToWeb;
}

String processingModbusRequest() {
	processingPostReqestData();
	byte bytesToPort[6] = {
		id, 
		command, 
		highByte(reg), lowByte(reg), 
		highByte(val), lowByte(val)
	};
  
	byte telegramm[8] = {
		bytesToPort[0], 
		bytesToPort[1], 
		bytesToPort[2], 
		bytesToPort[3], 
		bytesToPort[4], 
		bytesToPort[5], 
		highByte(modbusCalcCRC(6, bytesToPort)), 
		lowByte(modbusCalcCRC(6, bytesToPort))
	};
	modbusSendTxBuffer(telegramm, 8);
    delay(500);
	  
    return processingModbusResponse();
}

void serverInit() {
  	WiFi.mode(WIFI_AP);
	ApReconnect();
	HTTP.enableCORS(true);  
	HTTP.begin();
	HTTP.on("/data", HTTP_POST,[](){
		HTTP.send(200, "text/plain", processingModbusRequest());
	});
}

int modbusGetRxBuffer() {
	digitalWrite(LED_PIN_4, HIGH);
	for (int i = 0; i < 8; i++) {
		rxBuffer[i] = 0;
	}
	boolean bufferOverflow = false;
	digitalWrite(RS485_PIN, LOW);
	int readBytesCount = 0;
	while (Serial.available()) {
		rxBuffer[readBytesCount] = Serial.read();
		readBytesCount++;
		if (readBytesCount >= 64) {
			bufferOverflow = true;
		}
	}
	//digitalWrite(RS485_PIN, HIGH);
	if (bufferOverflow) {
		return -3;
	}
	digitalWrite(LED_PIN_4, LOW);
	return readBytesCount;
}

void ApReconnect() {
    WiFi.softAPConfig(ESPControllerWifiAP_ip, ESPControllerWifiAP_gateway, ESPControllerWifiAP_subnet);
    WiFi.softAP(ESPControllerWifiAP_SSID, ESPControllerWifiAP_password);
}

void modbusSendTxBuffer(byte buff[], int len) {
    digitalWrite(LED_PIN_3, HIGH);
    int crc = modbusCalcCRC(len, buff);
    buff[len] = crc;
    digitalWrite(RS485_PIN, HIGH);
    delay(5);
    Serial.write(buff, len);
    delay(10);
    digitalWrite(RS485_PIN, LOW);
    Serial.flush();
    digitalWrite(LED_PIN_3, LOW);
}

int modbusCalcCRC(byte length, byte bufferArray[]) {
    unsigned int temp, temp2, flag;
    temp = 0xFFFF;
    for (unsigned char i = 0; i < length; i++) {
        temp = temp ^ bufferArray[i];
        for (unsigned char j = 1; j <= 8; j++) {
            flag = temp & 0x0001;
            temp >>= 1;
            if (flag)   temp ^= 0xA001;
        }
    }
    temp2 = temp >> 8;
    temp = (temp << 8) | temp2;
    temp &= 0xFFFF;
    
	return temp;
}
