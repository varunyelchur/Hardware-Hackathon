import time
import random
from collections import deque
from datetime import datetime
import streamlit as st

try:
    import serial
    from serial.tools import list_ports
except Exception:
    serial = None
    list_ports = None

st.set_page_config(
    page_title="VaultBox Security Dashboard",
    page_icon="🔐",
    layout="wide",
    initial_sidebar_state="expanded",
)

st.markdown("""
<style>
@import url('https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&family=IBM+Plex+Mono:wght@400;500&display=swap');

html, body, [data-testid="stAppViewContainer"] {
    background: #f8f9fb !important;
    font-family: 'Inter', sans-serif !important;
    color: #1a1d23 !important;
}
[data-testid="stSidebar"] {
    background: #ffffff !important;
    border-right: 1px solid #e5e7eb !important;
}
[data-testid="stSidebar"] * { color: #1a1d23 !important; }
h1, h2, h3, h4 {
    font-family: 'Inter', sans-serif !important;
    font-weight: 600 !important;
    color: #1a1d23 !important;
    letter-spacing: -0.02em !important;
}
[data-testid="metric-container"] {
    background: #ffffff !important;
    border: 1px solid #e5e7eb !important;
    border-radius: 10px !important;
    padding: 18px 20px !important;
}
[data-testid="stMetricLabel"] {
    font-family: 'Inter', sans-serif !important;
    font-size: 0.72rem !important;
    font-weight: 500 !important;
    color: #6b7280 !important;
    text-transform: uppercase !important;
    letter-spacing: 0.06em !important;
}
[data-testid="stMetricValue"] {
    font-family: 'Inter', sans-serif !important;
    font-size: 1.5rem !important;
    font-weight: 700 !important;
    color: #1a1d23 !important;
}
.stButton > button {
    background: #1a1d23 !important;
    border: none !important;
    color: #ffffff !important;
    font-family: 'Inter', sans-serif !important;
    font-size: 0.8rem !important;
    font-weight: 500 !important;
    border-radius: 8px !important;
    padding: 8px 16px !important;
    transition: background 0.15s !important;
}
.stButton > button:hover { background: #374151 !important; }
.stSelectbox > div > div {
    background: #ffffff !important;
    border: 1px solid #e5e7eb !important;
    border-radius: 8px !important;
    font-family: 'Inter', sans-serif !important;
    font-size: 0.85rem !important;
}
.stRadio label {
    font-family: 'Inter', sans-serif !important;
    font-size: 0.85rem !important;
    font-weight: 500 !important;
}
.stCode, code {
    background: #f3f4f6 !important;
    border: 1px solid #e5e7eb !important;
    font-family: 'IBM Plex Mono', monospace !important;
    color: #374151 !important;
    border-radius: 6px !important;
}
.stCaption {
    font-family: 'Inter', sans-serif !important;
    color: #9ca3af !important;
    font-size: 0.75rem !important;
}
hr { border-color: #e5e7eb !important; }
::-webkit-scrollbar { width: 4px; }
::-webkit-scrollbar-track { background: #f8f9fb; }
::-webkit-scrollbar-thumb { background: #d1d5db; border-radius: 2px; }
</style>
""", unsafe_allow_html=True)

# ─── HELPERS ───────────────────────────────────────────────────────────────────
def now_str():
    return datetime.now().strftime("%H:%M:%S")

def add_log(logs, level, message):
    logs.appendleft({"time": now_str(), "level": level, "message": message})

def parse_serial_line(line: str):
    data = {}
    for part in [p.strip() for p in line.split(",") if p.strip()]:
        if ":" in part:
            k, v = part.split(":", 1)
            data[k.strip().upper()] = v.strip()
    return data

def ai_interpretation(status, tilt, sound):
    if status == "ACCESS_GRANTED":
        return "Knock pattern authenticated. Servo actuated, box unlocked successfully."
    if status == "ACCESS_DENIED":
        return "Knock sequence received but timing deltas exceeded the stored tolerance threshold. Access rejected."
    if status == "INTRUSION":
        return "Physical tamper event detected via tilt sensor while box remained locked. Flagged as intrusion."
    if status == "SUSPICIOUS":
        return "Partial disturbance detected. System is holding state and monitoring additional sensor inputs."
    if status == "UNLOCKED":
        return "Box is in authorized unlocked state. Auto-relock timer is active."
    return f"All channels nominal. No anomalies detected. Tilt={tilt} | Sound={sound} dB"

def threat_level(status, tilt, sound):
    if status == "INTRUSION":
        return 95
    if status == "ACCESS_DENIED":
        return 60
    if status == "SUSPICIOUS" or tilt:
        return 40
    if sound > 70:
        return 25
    if status in {"ACCESS_GRANTED", "UNLOCKED"}:
        return 5
    return 10

def status_pill(status):
    cfg = {
        "ACCESS_GRANTED": ("#065f46", "#d1fae5", "#10b981"),
        "UNLOCKED":       ("#065f46", "#d1fae5", "#10b981"),
        "ACCESS_DENIED":  ("#92400e", "#fef3c7", "#f59e0b"),
        "SUSPICIOUS":     ("#92400e", "#fef3c7", "#f59e0b"),
        "INTRUSION":      ("#7f1d1d", "#fee2e2", "#ef4444"),
        "LOCKED":         ("#1e3a5f", "#dbeafe", "#3b82f6"),
    }
    tc, bg, bd = cfg.get(status, ("#1e3a5f", "#dbeafe", "#3b82f6"))
    return f"""<div style="display:inline-flex;align-items:center;gap:8px;
        background:{bg};border:1px solid {bd};color:{tc};
        font-family:'Inter',sans-serif;font-weight:600;font-size:0.85rem;
        letter-spacing:0.04em;padding:8px 18px;border-radius:8px;text-transform:uppercase;">
        <span style="width:7px;height:7px;border-radius:50%;background:{bd};"></span>
        {status.replace('_',' ')}
    </div>"""

def threat_bar(level):
    color = "#ef4444" if level >= 70 else "#f59e0b" if level >= 40 else "#10b981"
    label = "CRITICAL" if level >= 70 else "ELEVATED" if level >= 40 else "LOW"
    return f"""<div style="margin:14px 0 8px;">
        <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:6px;">
            <span style="font-family:'Inter',sans-serif;font-size:0.72rem;font-weight:500;
                         color:#6b7280;text-transform:uppercase;letter-spacing:0.06em;">Threat Level</span>
            <span style="font-family:'IBM Plex Mono',monospace;font-size:0.72rem;color:{color};font-weight:500;">
                {level}% &mdash; {label}
            </span>
        </div>
        <div style="background:#e5e7eb;border-radius:4px;height:5px;overflow:hidden;">
            <div style="width:{level}%;height:100%;background:{color};border-radius:4px;"></div>
        </div>
    </div>"""

def sparkline_svg(data, color):
    if not data:
        return ""
    mn, mx = min(data), max(data)
    rng = mx - mn or 1
    w, h = 100, 32
    pts = []
    for i, v in enumerate(data):
        x = i * w / max(len(data)-1, 1)
        y = h - ((v - mn) / rng) * (h - 4) - 2
        pts.append(f"{x:.1f},{y:.1f}")
    last = pts[-1].split(",")
    return f'<svg width="{w}" height="{h}" style="display:block"><polyline points="{" ".join(pts)}" fill="none" stroke="{color}" stroke-width="1.5" stroke-linejoin="round" stroke-linecap="round"/><circle cx="{last[0]}" cy="{last[1]}" r="2.5" fill="{color}"/></svg>'

def log_row(entry):
    level = entry["level"]
    dot = {"crit":"#ef4444","error":"#ef4444","warn":"#f59e0b","serial":"#3b82f6"}.get(level,"#10b981")
    tag = {"crit":"ERR","error":"ERR","warn":"WRN","serial":"SER"}.get(level,"INF")
    return f"""<div style="display:flex;gap:12px;align-items:flex-start;padding:7px 0;
        border-bottom:1px solid #f3f4f6;font-family:'IBM Plex Mono',monospace;font-size:0.72rem;">
        <span style="color:{dot};flex-shrink:0;font-weight:500;">{tag}</span>
        <span style="color:#9ca3af;flex-shrink:0;">{entry['time']}</span>
        <span style="color:#374151;">{entry['message']}</span>
    </div>"""

# ─── SESSION STATE ─────────────────────────────────────────────────────────────
defaults = {
    "status":"LOCKED",
    "tilt":0,
    "sound":20,
    "last_knock":"NONE",
    "knock_count":0,
    "access_count":0,
    "deny_count":0,
    "intrusion_count":0,
    "logs":None,
    "sound_history":deque([20]*30, maxlen=30),
    "serial_connected":False,
    "ser":None,
    "uptime_start":time.time(),
}

for k, v in defaults.items():
    if k not in st.session_state:
        st.session_state[k] = v

if st.session_state.logs is None:
    st.session_state.logs = deque(maxlen=50)
    add_log(st.session_state.logs, "info", "Dashboard initialized")
    add_log(st.session_state.logs, "info", "All sensors nominal")

# ─── SIDEBAR ───────────────────────────────────────────────────────────────────
with st.sidebar:
    st.markdown("""
    <div style="padding:4px 0 20px;">
        <div style="font-family:'Inter',sans-serif;font-size:1rem;font-weight:700;
                    color:#1a1d23;letter-spacing:-0.01em;">VaultBox</div>
        <div style="font-family:'IBM Plex Mono',monospace;font-size:0.65rem;
                    color:#9ca3af;margin-top:2px;">Security Dashboard v2.0</div>
    </div>
    """, unsafe_allow_html=True)

    mode = st.radio("Input Mode", ["Mock Mode", "Serial Mode"], index=0)
    st.divider()

    if mode == "Serial Mode":
        if serial is None:
            st.error("pyserial not installed.\n`pip install pyserial`")
        else:
            ports = [p.device for p in list_ports.comports()] if list_ports else []
            port = st.selectbox("Port", ports or ["No ports found"])
            baud = st.selectbox("Baud Rate", [9600, 115200])
            ca, cb = st.columns(2)

            with ca:
                if st.button("Connect", use_container_width=True):
                    try:
                        if st.session_state.ser:
                            st.session_state.ser.close()
                        st.session_state.ser = serial.Serial(port, baud, timeout=0.2)
                        st.session_state.serial_connected = True
                        add_log(st.session_state.logs, "info", f"Connected to {port} @ {baud}")
                    except Exception as e:
                        st.session_state.serial_connected = False
                        st.error(str(e))

            with cb:
                if st.button("Disconnect", use_container_width=True):
                    try:
                        if st.session_state.ser:
                            st.session_state.ser.close()
                    except:
                        pass
                    st.session_state.ser = None
                    st.session_state.serial_connected = False
                    add_log(st.session_state.logs, "info", "Serial disconnected")

            sc = "#10b981" if st.session_state.serial_connected else "#ef4444"
            st.markdown(
                f'<span style="font-size:0.75rem;color:{sc};font-family:monospace;">{"Connected" if st.session_state.serial_connected else "Offline"}</span>',
                unsafe_allow_html=True
            )
    else:
        st.markdown('<span style="font-size:0.72rem;font-weight:500;color:#6b7280;text-transform:uppercase;letter-spacing:0.06em;">Mock Controls</span>', unsafe_allow_html=True)

        mock_status = st.selectbox("Status", ["LOCKED","ACCESS_GRANTED","ACCESS_DENIED","SUSPICIOUS","INTRUSION","UNLOCKED"])
        mock_tilt = st.selectbox("Tilt", [0, 1])
        mock_sound = st.slider("Sound (dB)", 0, 100, 20)
        mock_knock = st.selectbox("Knock Event", ["NONE","DETECTED","PATTERN_OK","PATTERN_BAD"])

        if st.button("Apply State", use_container_width=True):
            st.session_state.update({
                "status": mock_status,
                "tilt": mock_tilt,
                "sound": mock_sound,
                "last_knock": mock_knock,
            })
            st.session_state.sound_history.append(mock_sound)

            if mock_status == "ACCESS_GRANTED":
                st.session_state.access_count += 1
            elif mock_status == "ACCESS_DENIED":
                st.session_state.deny_count += 1
            elif mock_status == "INTRUSION":
                st.session_state.intrusion_count += 1

            if mock_knock != "NONE":
                st.session_state.knock_count += 1

            add_log(st.session_state.logs, "info", f"State set: {mock_status} | {mock_sound}dB")

        c1, c2 = st.columns(2)

        with c1:
            if st.button("Randomize", use_container_width=True):
                st.session_state.status = random.choice(["LOCKED","ACCESS_GRANTED","ACCESS_DENIED","SUSPICIOUS","INTRUSION","UNLOCKED"])
                st.session_state.sound = random.randint(0, 100)
                st.session_state.tilt = random.randint(0, 1)
                st.session_state.sound_history.append(st.session_state.sound)
                add_log(st.session_state.logs, "info", f"Randomized: {st.session_state.status}")

        with c2:
            if st.button("Reset", use_container_width=True):
                for k in ["status","tilt","sound","last_knock","knock_count","access_count","deny_count","intrusion_count"]:
                    st.session_state[k] = defaults[k]
                st.session_state.logs = deque(maxlen=50)
                st.session_state.sound_history = deque([20]*30, maxlen=30)
                add_log(st.session_state.logs, "info", "System reset to defaults")

    st.divider()

    uptime_sec = int(time.time() - st.session_state.uptime_start)
    h2, rem = divmod(uptime_sec, 3600)
    m2, s2 = divmod(rem, 60)

    st.markdown(f"""
    <div style="font-family:'IBM Plex Mono',monospace;font-size:0.72rem;color:#6b7280;line-height:2.2;">
        Uptime &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{h2:02d}:{m2:02d}:{s2:02d}<br>
        Knocks &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{st.session_state.knock_count}<br>
        Grants &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{st.session_state.access_count}<br>
        Denials &nbsp;&nbsp;&nbsp;&nbsp;{st.session_state.deny_count}<br>
        Intrusions &nbsp;{st.session_state.intrusion_count}
    </div>
    """, unsafe_allow_html=True)

# ─── SERIAL POLLING ────────────────────────────────────────────────────────────
if mode == "Serial Mode" and st.session_state.serial_connected and st.session_state.ser:
    try:
        raw = st.session_state.ser.readline().decode(errors="ignore").strip()
        if raw:
            p = parse_serial_line(raw)

            if "STATUS" in p:
                ns = p["STATUS"].upper()
                if ns == "INTRUSION":
                    st.session_state.intrusion_count += 1
                elif ns == "ACCESS_GRANTED":
                    st.session_state.access_count += 1
                elif ns == "ACCESS_DENIED":
                    st.session_state.deny_count += 1
                st.session_state.status = ns

            for key in ["TILT", "SOUND"]:
                if key in p:
                    val = int(float(p[key]))
                    st.session_state[key.lower()] = val
                    if key == "SOUND":
                        st.session_state.sound_history.append(val)

            if "KNOCK" in p:
                st.session_state.last_knock = p["KNOCK"].upper()
                st.session_state.knock_count += 1

            add_log(st.session_state.logs, "serial", raw)
    except Exception as e:
        add_log(st.session_state.logs, "error", f"Serial error: {e}")

# ─── MAIN UI ───────────────────────────────────────────────────────────────────
status = st.session_state.status
tlevel = threat_level(status, st.session_state.tilt, st.session_state.sound)

if status == "INTRUSION":
    st.markdown("""
    <div style="background:#fef2f2;border:1px solid #fecaca;border-left:4px solid #ef4444;
                border-radius:8px;padding:12px 18px;margin-bottom:16px;
                font-family:'Inter',sans-serif;font-size:0.85rem;font-weight:500;color:#7f1d1d;">
        Intrusion Alert &mdash; Physical tamper detected while box is locked.
    </div>
    """, unsafe_allow_html=True)

st.markdown("""
<div style="margin-bottom:20px;">
    <div style="font-family:'Inter',sans-serif;font-size:1.3rem;font-weight:700;
                color:#1a1d23;letter-spacing:-0.02em;">VaultBox Security Dashboard</div>
    <div style="font-family:'IBM Plex Mono',monospace;font-size:0.7rem;color:#9ca3af;margin-top:3px;">
        Knock-pattern authentication &nbsp;&middot;&nbsp; Edge node &nbsp;&middot;&nbsp; Real-time monitoring
    </div>
</div>
""", unsafe_allow_html=True)

col_l, col_r = st.columns([1, 1.8])

with col_l:
    st.markdown(status_pill(status), unsafe_allow_html=True)
    st.markdown(threat_bar(tlevel), unsafe_allow_html=True)
    st.markdown(f"""
    <div style="font-family:'IBM Plex Mono',monospace;font-size:0.72rem;color:#9ca3af;margin-top:10px;">
        Last knock: <span style="color:#374151;font-weight:500;">{st.session_state.last_knock}</span>
    </div>
    """, unsafe_allow_html=True)

with col_r:
    st.markdown(f"""
    <div style="background:#fff;border:1px solid #e5e7eb;border-radius:10px;padding:16px 20px;">
        <div style="font-family:'Inter',sans-serif;font-size:0.7rem;font-weight:500;
                    color:#6b7280;text-transform:uppercase;letter-spacing:0.06em;margin-bottom:8px;">
            System Interpretation
        </div>
        <div style="font-family:'Inter',sans-serif;font-size:0.9rem;color:#374151;line-height:1.6;">
            {ai_interpretation(status, st.session_state.tilt, st.session_state.sound)}
        </div>
    </div>
    """, unsafe_allow_html=True)

st.divider()

st.markdown('<span style="font-family:Inter,sans-serif;font-size:0.72rem;font-weight:500;color:#6b7280;text-transform:uppercase;letter-spacing:0.06em;">Sensor Telemetry</span>', unsafe_allow_html=True)
m1, m2, m3, m4 = st.columns(4)
m1.metric("Tilt", "Triggered" if st.session_state.tilt else "Stable")
m2.metric("Sound", f"{st.session_state.sound} dB")
m3.metric("Threat Index", f"{tlevel}%")
m4.metric("Status", status.replace("_", " "))

st.divider()

col_charts, col_log = st.columns([1, 1.2])

with col_charts:
    st.markdown('<span style="font-family:Inter,sans-serif;font-size:0.72rem;font-weight:500;color:#6b7280;text-transform:uppercase;letter-spacing:0.06em;">Signal History</span>', unsafe_allow_html=True)
    sh = list(st.session_state.sound_history)

    st.markdown(f"""
    <div style="background:#fff;border:1px solid #e5e7eb;border-radius:10px;padding:16px 18px;margin-top:8px;margin-bottom:10px;">
        <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:10px;">
            <span style="font-family:'Inter',sans-serif;font-size:0.78rem;font-weight:500;color:#374151;">Sound Level (dB)</span>
            <span style="font-family:'IBM Plex Mono',monospace;font-size:0.7rem;color:#6b7280;">
                cur {sh[-1]} &nbsp;&middot;&nbsp; min {min(sh)} &nbsp;&middot;&nbsp; max {max(sh)}
            </span>
        </div>
        {sparkline_svg(sh, "#3b82f6")}
    </div>
    """, unsafe_allow_html=True)

    st.divider()
    st.markdown('<span style="font-family:Inter,sans-serif;font-size:0.72rem;font-weight:500;color:#6b7280;text-transform:uppercase;letter-spacing:0.06em;">Session Summary</span>', unsafe_allow_html=True)
    s1, s2, s3 = st.columns(3)
    s1.metric("Grants", st.session_state.access_count)
    s2.metric("Denials", st.session_state.deny_count)
    s3.metric("Intrusions", st.session_state.intrusion_count)

with col_log:
    st.markdown('<span style="font-family:Inter,sans-serif;font-size:0.72rem;font-weight:500;color:#6b7280;text-transform:uppercase;letter-spacing:0.06em;">Event Log</span>', unsafe_allow_html=True)
    rows = "".join(log_row(e) for e in list(st.session_state.logs)[:25])
    st.markdown(f"""
    <div style="background:#fff;border:1px solid #e5e7eb;border-radius:10px;
                padding:12px 16px;height:360px;overflow-y:auto;margin-top:8px;">
        {rows}
    </div>
    """, unsafe_allow_html=True)
    if st.button("Clear Log", use_container_width=True):
        st.session_state.logs = deque(maxlen=50)
        add_log(st.session_state.logs, "info", "Log cleared")

st.divider()

with st.expander("Arduino Serial Protocol Reference"):
    st.code(
        "STATUS:ACCESS_GRANTED\nSTATUS:ACCESS_DENIED\nSTATUS:INTRUSION\n"
        "STATUS:SUSPICIOUS\nKNOCK:DETECTED\nTILT:1\nSOUND:42\n"
        "STATUS:LOCKED,TILT:0,SOUND:20,KNOCK:NONE",
        language="text",
    )
    st.caption("CSV combined format supported. Keys are case-insensitive.")

if mode == "Serial Mode" and st.session_state.serial_connected:
    time.sleep(0.3)
    st.rerun()

