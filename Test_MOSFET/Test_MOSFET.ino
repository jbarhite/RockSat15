#define PWM 3

void setup() {

}

void loop() {
	for (int i=0; i<=255; i++) {
		analogWrite(PWM, i);
		delay(10);
	}
	delay(2000);
	for (int i=255; i>=0; i--) {
		analogWrite(PWM, i);
		delay(10);
	}
	delay(2000);
}
