#include <Servo.h>
const int shock_pin = 2;
const int servo_pin = 4;
const int hw_505_pin = A0;
Servo lock_servo;
const unsigned long merge_gap_ms = 110;
const unsigned long restart_gap_ms = 1200;
const unsigned long short_gap_min = 140;
const unsigned long short_gap_max = 380;
const unsigned long long_gap_min = 300;
const unsigned long long_gap_max = 900;
const unsigned long tilt_report_interval_ms = 3000;
const int level_min = 470;
const int level_max = 550;
unsigned long last_knock_time = 0;
unsigned long last_trigger_time = 0;
unsigned long last_tilt_report_time = 0;
int last_state = HIGH;
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
bool is_level() {
	int tilt_value = analogRead(hw_505_pin);
	return tilt_value >= level_min && tilt_value <= level_max;
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
void unlock_box() {
	lock_servo.write(90);
	delay(3000);
	lock_servo.write(0);
}
void report_tilt_if_needed(unsigned long now) {
	if (now - last_tilt_report_time >= tilt_report_interval_ms) {
		last_tilt_report_time = now;
		bool level = is_level();
		Serial.print("LEVEL STATE: ");
		Serial.println(level ? "LEVEL" : "NOT LEVEL");
	}
}
void setup() {
	Serial.begin(9600);
	pinMode(shock_pin, INPUT);
	lock_servo.attach(servo_pin);
	lock_servo.write(0);
}
void loop() {
	unsigned long now = millis();
	report_tilt_if_needed(now);
	int current_state = digitalRead(shock_pin);
	if (last_state == HIGH && current_state == LOW) {
		if (last_trigger_time == 0 || now - last_trigger_time >= merge_gap_ms) {
			last_trigger_time = now;
			if (last_knock_time != 0) {
				unsigned long gap = now - last_knock_time;
				if (gap > restart_gap_ms) {
					reset_pattern();
				} else {
					push_interval(gap);
					if (interval_count == 3) {
						if (matches_pattern(intervals[0], intervals[1], intervals[2])) {
							Serial.println("PATTERN DETECTED");
							unlock_box();
							reset_pattern();
						}
					}
				}
			}
			last_knock_time = now;
		}
	}
	last_state = current_state;
}
