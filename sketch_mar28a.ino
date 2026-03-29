#include <Servo.h>
const int shock_pin = 2;
const int servo_pin = 4;
const int level_pin = 7;
Servo lock_servo;
const unsigned long merge_gap_ms = 110;
const unsigned long restart_gap_ms = 1200;
const unsigned long short_gap_min = 140;
const unsigned long short_gap_max = 380;
const unsigned long long_gap_min = 300;
const unsigned long long_gap_max = 900;
unsigned long last_knock_time = 0;
unsigned long last_trigger_time = 0;
int last_shock_state = HIGH;
int last_level_state = HIGH;
unsigned long intervals[3];
int interval_count = 0;
bool is_short_gap(unsigned long gap) {
	return gap >= short_gap_min && gap <= short_gap_max;
}
bool is_long_gap(unsigned long gap) {
	return gap >= long_gap_min && gap <= long_gap_max;
}
bool matches_pattern(unsigned long g1, unsigned long g2, unsigned long g3) {
	bool in_range = is_long_gap(g1) && is_short_gap(g2) && is_long_gap(g3);
	bool relative_shape = g2 < g1 && g2 < g3;
	bool separation = g1 > g2 + 80 && g3 > g2 + 80;
	return in_range && relative_shape && separation;
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
void emit_knock_detected() {
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
void unlock_box() {
	lock_servo.write(90);
	delay(3000);
	lock_servo.write(0);
	emit_locked(digitalRead(level_pin));
}
void setup() {
	Serial.begin(9600);
	pinMode(shock_pin, INPUT);
	pinMode(level_pin, INPUT);
	lock_servo.attach(servo_pin);
	lock_servo.write(0);
	last_level_state = digitalRead(level_pin);
	emit_startup_state();
}
void loop() {
	unsigned long now = millis();
	int current_shock_state = digitalRead(shock_pin);
	int current_level_state = digitalRead(level_pin);
	if (current_level_state != last_level_state) {
		emit_tilt_change(current_level_state);
		last_level_state = current_level_state;
	}
	if (last_shock_state == HIGH && current_shock_state == LOW) {
		if (last_trigger_time == 0 || now - last_trigger_time >= merge_gap_ms) {
			last_trigger_time = now;
			emit_knock_detected();
			if (last_knock_time != 0) {
				unsigned long gap = now - last_knock_time;
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
			last_knock_time = now;
		}
	}
	last_shock_state = current_shock_state;
}