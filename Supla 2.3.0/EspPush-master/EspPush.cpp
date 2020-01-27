#include "EspPush.h"

EspPush::EspPush(String token, String user) {
	_token = token;
	_user = user;
}
void EspPush::setMessage(String message) {
	_message = message;
}
void EspPush::setDevice(String device) {
	_device = device;
}
void EspPush::setTitle(String title) {
	_title = title;
}
void EspPush::setUrl(String url) {
	_url = url;
}
void EspPush::setUrlTitle(String url_title) {
	_url_title = url_title;
}
void EspPush::setPriority(int8_t priority) {
	_priority = priority;
}
void EspPush::setRetry(uint16_t retry) {
	_retry = retry;
}
void EspPush::setExpire(uint16_t expire) {
	_expire = expire;
}
void EspPush::setTimestamp(uint32_t timestamp) {
	_timestamp = timestamp;
}
void EspPush::setSound(String sound) {
	_sound = sound;
}
void EspPush::setTimeout(uint16_t timeout) {
	_timeout = timeout;
}
void EspPush::setHTML(boolean html) {
	_html = html;
}
boolean EspPush::send(void) {
	WiFiClientSecure Pusclient;
	Pusclient.setInsecure();
	if (!Pusclient.connect("api.pushover.net", 443)) {
		return false;
	}
	String post = String("token=")+_token+"&user="+_user+"&title="+_title+"&message="+_message+"&device="+_device+"&url="+_url+"&url_title="+_url_title+"&priority="+_priority+"&retry="+_retry+"&expire="+_expire+"&sound="+_sound;
	if (_timestamp != 0) post += String("&timestamp=")+_timestamp;
	if (_html == true) post += String("&html=1");
	String http = String("POST /1/messages.json HTTP/1.1\r\nHost: api.pushover.net\r\nConnection: close\r\nAccept: Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nContent-Length: ")+post.length()+"\r\n\r\n"+post;
	Pusclient.print(http);
	int timeout_at = millis() + _timeout;
	while (!Pusclient.available() && timeout_at - millis() < 0) {
		Pusclient.stop();
		return false;
	}
	String line;
	while (Pusclient.connected()) {
		if (Pusclient.read() == '{') break;
	}
	line = Pusclient.readStringUntil('\n');
	if((line.indexOf("\"status\":1") != -1) || (line.indexOf("200 OK") != -1)) {
		return false;
	} else {
		return true;
	}
}
