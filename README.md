# Hardware-Hackathon
VaultBox — Hardware Hackathon Project

VaultBox is a behavior-based smart lock system that combines Arduino hardware, sensor-based authentication, and a real-time monitoring dashboard.

It uses a knock-pattern authentication system along with tilt-based intrusion detection, and visualizes everything through a live Streamlit dashboard.

⸻

Features
	•	Knock-pattern authentication (behavior-based access)
	•	Tilt-based intrusion detection
	•	Real-time serial communication (Arduino → Dashboard)
	•	Live monitoring dashboard (Streamlit)
	•	Servo-based physical locking mechanism

⸻

System Architecture
Arduino (Sensors + Logic)
        ↓ Serial (USB)
Python (Streamlit Dashboard)
        ↓
Live UI + Logs + Metrics

Main Files
	•	app.py → Streamlit dashboard (frontend + serial parser)
	•	knocklock.ino (or sketch_mar28a.ino) → Arduino logic (sensors + decision engine)

⸻

Hardware Setup

Components
	•	Arduino Uno
	•	Servo motor
	•	Shock/vibration sensor
	•	Tilt sensor (HW-505)
	•	Breadboard + wires

  Wiring (Servo)
	•	Red → 5V
	•	Brown/Black → GND
	•	Yellow/Orange → Pin 4

  Software Setup

1. Install dependencies
pip install streamlit pyserial

2. Upload Arduino Code
	1.	Open Arduino IDE
	2.	Open knocklock.ino (or main .ino file)
	3.	Select:
	•	Board: Arduino Uno
	•	Port: /dev/cu.usbmodemXXX (Mac) or COM port (Windows)
	4.	Click Upload
 5.	Important:
	•	Close Serial Monitor after uploading
	•	Only one program can use the port at a time

⸻

3. Run the Dashboard

In terminal:
streamlit run app.py
http://localhost:8501



  
