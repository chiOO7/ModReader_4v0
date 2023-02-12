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

bool responseIsNotValid(int bytesCount) {
    unsigned int u16MsgCRC = ((rxBuffer[bytesCount - 2] << 8) | rxBuffer[bytesCount - 1]);
    unsigned int calcedCRC = modbusCalcCRC(bytesCount - 2, rxBuffer);
    if (calcedCRC == u16MsgCRC) {
        return false;
    }
    return true;
}

String process01And02functions(int bytesCount) {
	String str = "";
	
	int regsCount = (val % 8) > 0 ? (val / 8) + 1 : (val / 8);
	
	int bitCount = 0;
	
	if (bytesCount == (regsCount + 5)) {
		for (int b = 3; b < bytesCount - 2; b++) {
			for (int bit = 0; bit < 8; bit++) {
				str += bitRead(rxBuffer[b], bit);
				str += ",";
				bitCount++;
				if (bitCount == val) break;
			}
		}
		str = str.substring(0, str.length() - 1);
	} else {
		return BAD_RX;
	}
	
	return str;
}

String process03And04functions(int bytesCount) {
	String str = "";
	if (bytesCount == (val * 2 + 5)) {
		for (int b = 3; b < bytesCount - 2; b += 2) {
			str += ((rxBuffer[b] << 8) + rxBuffer[b + 1]);
			str += ",";
		}
		str = str.substring(0, str.length() - 1);
	} else {
		return BAD_RX;
	}
	
	return str;
}

String process05And06functions() {
	if (word(0, rxBuffer[1]) == command &&
		word(rxBuffer[2], rxBuffer[3]) == reg &&
		word(rxBuffer[4], rxBuffer[5]) == val) {
		return SET_OK;	
	}	else {
		return BAD_RX;
	}
}

//String printToWeb(String str) {
//	return "[" + str + "]";
//}

String processingModbusResponse() {
	String printToWeb = "[";
	int timesCount = 0;
	while (Serial.available() <= 0) {
		delay(WAIT_SERIAL_DELAY);
		timesCount++;
		if (timesCount > WAIT_COUNT_DELAY) {
			return "[" + SERIAL_TIMEOUT + "]";
		}
	}
	
  	if (Serial.available() > 0) {
        int bytesCount = modbusGetRxBuffer();
		
		if (bytesCount == -3) return "[" + BUFFER_OVERFLOW + "]";
		
		if (responseIsNotValid(bytesCount)) return "[" + BAD_RX + "]";
    
		if (word(0, rxBuffer[0]) != id) return "[" + BAD_RX + "]";
			
		if (word(0, rxBuffer[1]) != command) return "[" + BAD_RX + "]";
		
		switch (rxBuffer[1]) {
		    case 1 :
				  //printToWeb += "1, 123";
			    printToWeb = process01And02functions(bytesCount);
		    break;
		    case 2 :
				  //printToWeb += "2, 234";
          printToWeb = process01And02functions(bytesCount);
		    break;
		    case 3 :
			    printToWeb += process03And04functions(bytesCount);
		    break;
		    case 4 :
			    printToWeb += process03And04functions(bytesCount);
		    break;
		    case 5 :
			    printToWeb = process05And06functions();
		    break;
		    case 6 :
			    printToWeb += process05And06functions();
		    break;
		}
	} 
    
    return printToWeb + "]";
}

String processingModbusRequest() {
	processingPostReqestData();
	byte bytesToPort[6] = {
		lowByte(id), 
		lowByte(command), 
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
    delay(RESPONSE_DELAY);
	  
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
	digitalWrite(LED_RX_PIN, HIGH);
	//for (int i = 0; i < 8; i++) {
	//	rxBuffer[i] = 0;
	//}
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
	if (bufferOverflow) {
		return -3;
	}
	digitalWrite(LED_RX_PIN, LOW);
	
	return readBytesCount;
}

void ApReconnect() {
    WiFi.softAPConfig(ESPControllerWifiAP_ip, ESPControllerWifiAP_gateway, ESPControllerWifiAP_subnet);
    WiFi.softAP(ESPControllerWifiAP_SSID, ESPControllerWifiAP_password);
}

void modbusSendTxBuffer(byte buff[], int len) {
    digitalWrite(LED_TX_PIN, HIGH);
    int crc = modbusCalcCRC(len, buff);
    buff[len] = highByte(crc);
    buff[len + 1] = lowByte(crc);
    digitalWrite(RS485_PIN, HIGH);
    delay(WAIT_SERIAL_DELAY / 2);
    Serial.write(buff, len);
    delay(WAIT_SERIAL_DELAY);
    digitalWrite(RS485_PIN, LOW);
    Serial.flush();
    digitalWrite(LED_TX_PIN, LOW);
}

unsigned int modbusCalcCRC(byte length, byte bufferArray[]) {
    unsigned int temp, temp2, flag;
    temp = 0xFFFF;
    for (unsigned char i = 0; i < length; i++) {
        temp = temp ^ bufferArray[i];
        for (unsigned char j = 1; j <= 8; j++) {
            flag = temp & 0x0001;
            temp >>= 1;
            if (flag) temp ^= 0xA001;
        }
    }
    temp2 = temp >> 8;
    temp = (temp << 8) | temp2;
    temp &= 0xFFFF;
    
	return temp;
}
