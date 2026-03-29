#include <Servo.h>
const int touch_pin = 2;
const int servo_pin = 4;
const int level_pin = 7;
const bool touch_active_high = true;
Servo lock_servo;
const unsigned long touch_debounce_ms = 180;
const unsigned long restart_gap_ms = 1800;
unsigned long last_touch_time = 0;
unsigned long last_event_time = 0;
int last_touch_state = LOW;
int last_level_state = HIGH;
unsigned long intervals[3];
int interval_count = 0;
int tilt_value_from_level_state(int level_state) {
	return level_state == LOW ? 1 : 0;
}
void emit_startup_state() {
	int tilt = tilt_value_from_level_state(last_level_state);
	Serial.print("STATUS:LOCKED,TILT:");
	Serial.println(tilt);
}
void emit_tilt_change(int level_state) {
	int tilt = tilt_value_from_level_state(level_state);
	Serial.print("TILT:");
	Serial.println(tilt);
}
void emit_touch_detected() {
	Serial.println("KNOCK:DETECTED");
}
void emit_pattern_ok(int level_state) {
	int tilt = tilt_value_from_level_state(level_state);
	Serial.print("STATUS:ACCESS_GRANTED,KNOCK:PATTERN_OK,TILT:");
	Serial.println(tilt);
}
void emit_pattern_bad(int level_state) {
	int tilt = tilt_value_from_level_state(level_state);
	Serial.print("STATUS:ACCESS_DENIED,KNOCK:PATTERN_BAD,TILT:");
	Serial.println(tilt);
}
void emit_locked(int level_state) {
	int tilt = tilt_value_from_level_state(level_state);
	Serial.print("STATUS:LOCKED,TILT:");
	Serial.println(tilt);
}
void reset_pattern() {
	interval_count = 0;
}
void push_interval(unsigned long gap) {
	if (interval_count < 3) {
		intervals[interval_count] = gap;
		interval_count++;
	} else {
		intervals[0] = intervals[1];
		intervals[1] = intervals[2];
		intervals[2] = gap;
	}
}
bool matches_pattern(unsigned long g1, unsigned long g2, unsigned long g3) {
	bool middle_shortest = g2 < g1 && g2 < g3;
	bool strong_difference = g1 > g2 + 80 && g3 > g2 + 80;
	bool similar_outer = abs((long)g1 - (long)g3) < 220;
	return middle_shortest && strong_difference && similar_outer;
}
void unlock_box() {
	lock_servo.write(90);
	delay(3000);
	lock_servo.write(0);
	emit_locked(digitalRead(level_pin));
}
bool touch_detected(int previous_state, int current_state) {
	if (touch_active_high) {
		return previous_state == LOW && current_state == HIGH;
	}
	return previous_state == HIGH && current_state == LOW;
}
void setup() {
	Serial.begin(9600);
	pinMode(touch_pin, INPUT);
	pinMode(level_pin, INPUT);
	lock_servo.attach(servo_pin);
	lock_servo.write(0);
	last_touch_state = digitalRead(touch_pin);
	last_level_state = digitalRead(level_pin);
	emit_startup_state();
}
void loop() {
	unsigned long now = millis();
	int current_touch_state = digitalRead(touch_pin);
	int current_level_state = digitalRead(level_pin);
	if (current_level_state != last_level_state) {
		emit_tilt_change(current_level_state);
		last_level_state = current_level_state;
	}
	if (touch_detected(last_touch_state, current_touch_state)) {
		if (last_event_time == 0 || now - last_event_time >= touch_debounce_ms) {
			last_event_time = now;
			emit_touch_detected();
			if (last_touch_time != 0) {
				unsigned long gap = now - last_touch_time;
				Serial.print("INTERVAL:");
				Serial.println(gap);
				if (gap > restart_gap_ms) {
					reset_pattern();
				} else {
					push_interval(gap);
					if (interval_count == 3) {
						if (matches_pattern(intervals[0], intervals[1], intervals[2])) {
							emit_pattern_ok(current_level_state);
							unlock_box();
						} else {
							emit_pattern_bad(current_level_state);
						}
						reset_pattern();
					}
				}
			}
			last_touch_time = now;
		}
	}
	last_touch_state = current_touch_state;
}
