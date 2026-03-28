<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ASN-001 // Autonomous Safety Node</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=Orbitron:wght@400;700;900&display=swap" rel="stylesheet">
<style>
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

:root {
  --bg: #080c0f;
  --bg2: #0d1317;
  --bg3: #111820;
  --panel: #0a1018;
  --mono: 'Share Tech Mono', monospace;
  --display: 'Orbitron', monospace;
  --amber: #f5a623;
  --amber-dim: #7a4f0a;
  --red: #e8453c;
  --red-dim: #6b1a17;
  --green: #39d98a;
  --green-dim: #0d4a28;
  --cyan: #2dd4f7;
  --cyan-dim: #0a3d47;
  --text: #c8d8e8;
  --text2: #6a8090;
  --text3: #3a5060;
  --border: rgba(45, 212, 247, 0.12);
  --border2: rgba(45, 212, 247, 0.25);
  --scanline: repeating-linear-gradient(
    0deg,
    transparent,
    transparent 2px,
    rgba(0,0,0,0.07) 2px,
    rgba(0,0,0,0.07) 4px
  );
}

html, body {
  height: 100%;
  background: var(--bg);
  color: var(--text);
  font-family: var(--mono);
  overflow-x: hidden;
}

/* Scanline overlay */
body::before {
  content: '';
  position: fixed;
  inset: 0;
  background: var(--scanline);
  pointer-events: none;
  z-index: 9999;
  opacity: 0.6;
}

/* Corner bracket decoration */
.bracket {
  position: absolute;
  width: 14px;
  height: 14px;
  border-color: var(--cyan);
  border-style: solid;
  opacity: 0.5;
}
.bracket.tl { top: -1px; left: -1px; border-width: 2px 0 0 2px; }
.bracket.tr { top: -1px; right: -1px; border-width: 2px 2px 0 0; }
.bracket.bl { bottom: -1px; left: -1px; border-width: 0 0 2px 2px; }
.bracket.br { bottom: -1px; right: -1px; border-width: 0 2px 2px 0; }

/* Layout */
.app {
  min-height: 100vh;
  display: grid;
  grid-template-rows: auto 1fr auto;
  max-width: 960px;
  margin: 0 auto;
  padding: 1.5rem;
  gap: 1.25rem;
}

/* Header */
header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  border-bottom: 1px solid var(--border);
  padding-bottom: 1rem;
  animation: fadeIn 0.6s ease forwards;
}
.header-left { display: flex; flex-direction: column; gap: 2px; }
.header-title {
  font-family: var(--display);
  font-size: 18px;
  font-weight: 900;
  letter-spacing: 0.12em;
  color: var(--cyan);
  text-shadow: 0 0 20px rgba(45,212,247,0.4);
}
.header-sub {
  font-size: 10px;
  letter-spacing: 0.2em;
  color: var(--text2);
  text-transform: uppercase;
}
.header-right {
  display: flex;
  align-items: center;
  gap: 1.5rem;
}
.live-indicator {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 10px;
  letter-spacing: 0.15em;
  color: var(--green);
}
.live-dot {
  width: 6px; height: 6px;
  border-radius: 50%;
  background: var(--green);
  box-shadow: 0 0 8px var(--green);
  animation: blink 1.4s infinite;
}
@keyframes blink { 0%,100%{opacity:1} 50%{opacity:0.2} }

.clock {
  font-family: var(--display);
  font-size: 13px;
  font-weight: 700;
  color: var(--text2);
  letter-spacing: 0.1em;
  min-width: 80px;
  text-align: right;
}

/* Main grid */
.main-grid {
  display: grid;
  grid-template-columns: 1fr 340px;
  gap: 1.25rem;
  align-items: start;
}

/* Panel base */
.panel {
  background: var(--panel);
  border: 1px solid var(--border);
  border-radius: 4px;
  position: relative;
  padding: 1.25rem;
  animation: slideUp 0.5s ease forwards;
  opacity: 0;
}
.panel:nth-child(1) { animation-delay: 0.1s; }
.panel:nth-child(2) { animation-delay: 0.2s; }
.panel:nth-child(3) { animation-delay: 0.3s; }
.panel:nth-child(4) { animation-delay: 0.4s; }

@keyframes slideUp {
  from { opacity: 0; transform: translateY(10px); }
  to { opacity: 1; transform: translateY(0); }
}
@keyframes fadeIn {
  from { opacity: 0; } to { opacity: 1; }
}

.panel-label {
  font-size: 9px;
  letter-spacing: 0.22em;
  color: var(--text3);
  text-transform: uppercase;
  margin-bottom: 1rem;
  display: flex;
  align-items: center;
  gap: 8px;
}
.panel-label::after {
  content: '';
  flex: 1;
  height: 1px;
  background: var(--border);
}

/* Sensor rows */
.sensors-grid { display: flex; flex-direction: column; gap: 1rem; }

.sensor-row {
  display: grid;
  grid-template-columns: 100px 1fr 60px;
  align-items: center;
  gap: 12px;
}
.sensor-name {
  font-size: 10px;
  letter-spacing: 0.15em;
  color: var(--text2);
  text-transform: uppercase;
}
.sensor-val-wrap {
  display: flex;
  flex-direction: column;
  gap: 4px;
}

/* Custom range slider */
input[type=range] {
  -webkit-appearance: none;
  appearance: none;
  width: 100%;
  height: 2px;
  background: var(--bg3);
  outline: none;
  cursor: pointer;
  border-radius: 1px;
}
input[type=range]::-webkit-slider-thumb {
  -webkit-appearance: none;
  width: 12px; height: 12px;
  border-radius: 50%;
  background: var(--cyan);
  box-shadow: 0 0 8px rgba(45,212,247,0.6);
  cursor: pointer;
  transition: transform 0.1s;
}
input[type=range]:hover::-webkit-slider-thumb { transform: scale(1.3); }
input[type=range].warn::-webkit-slider-thumb { background: var(--amber); box-shadow: 0 0 8px rgba(245,166,35,0.6); }
input[type=range].crit::-webkit-slider-thumb { background: var(--red); box-shadow: 0 0 8px rgba(232,69,60,0.6); }

.track-bg {
  width: 100%;
  height: 2px;
  background: var(--bg3);
  border-radius: 1px;
  position: relative;
  overflow: hidden;
}
.track-fill {
  height: 100%;
  background: var(--cyan);
  border-radius: 1px;
  transition: width 0.15s, background 0.3s;
}

.sensor-number {
  font-family: var(--display);
  font-size: 14px;
  font-weight: 700;
  color: var(--cyan);
  text-align: right;
  letter-spacing: 0.05em;
  transition: color 0.3s;
  min-width: 55px;
}
.sensor-number .unit {
  font-family: var(--mono);
  font-size: 9px;
  color: var(--text3);
  margin-left: 2px;
}

/* Binary sensors */
.binary-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-top: 0.5rem; }
.binary-btn {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 6px;
  padding: 14px 10px;
  background: var(--bg3);
  border: 1px solid var(--border);
  border-radius: 4px;
  cursor: pointer;
  transition: all 0.2s;
  position: relative;
  overflow: hidden;
}
.binary-btn::before {
  content: '';
  position: absolute;
  inset: 0;
  opacity: 0;
  transition: opacity 0.2s;
}
.binary-btn.off::before { background: radial-gradient(ellipse at center, rgba(57,217,138,0.06) 0%, transparent 70%); opacity: 1; }
.binary-btn.on::before { background: radial-gradient(ellipse at center, rgba(232,69,60,0.15) 0%, transparent 70%); opacity: 1; }
.binary-btn.off { border-color: rgba(57,217,138,0.25); }
.binary-btn.on { border-color: rgba(232,69,60,0.5); animation: alertPulse 1s infinite; }
@keyframes alertPulse { 0%,100%{box-shadow:0 0 0 0 rgba(232,69,60,0)} 50%{box-shadow:0 0 12px 2px rgba(232,69,60,0.3)} }

.binary-icon { font-size: 22px; line-height: 1; filter: grayscale(1) brightness(0.4); transition: filter 0.2s; }
.binary-btn.on .binary-icon { filter: grayscale(0) brightness(1); }
.binary-label { font-size: 9px; letter-spacing: 0.18em; color: var(--text3); text-transform: uppercase; }
.binary-state { font-family: var(--display); font-size: 10px; font-weight: 700; letter-spacing: 0.1em; }
.binary-btn.off .binary-state { color: var(--green); }
.binary-btn.on .binary-state { color: var(--red); }

/* Status panel — right column */
.status-column { display: flex; flex-direction: column; gap: 1.25rem; }

/* Ring gauge */
.ring-wrap {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 1rem;
}
.ring-svg { position: relative; }
.ring-svg svg { display: block; }
.ring-label-center {
  position: absolute;
  inset: 0;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 2px;
}
.ring-status-text {
  font-family: var(--display);
  font-size: 16px;
  font-weight: 900;
  letter-spacing: 0.15em;
  transition: color 0.4s;
}
.ring-status-sub { font-size: 9px; letter-spacing: 0.2em; color: var(--text3); text-transform: uppercase; }

/* Threat level bar */
.threat-bar-wrap { width: 100%; }
.threat-track {
  width: 100%;
  height: 6px;
  background: var(--bg3);
  border-radius: 3px;
  overflow: hidden;
  margin: 8px 0 4px;
}
.threat-fill {
  height: 100%;
  border-radius: 3px;
  transition: width 0.4s cubic-bezier(0.4,0,0.2,1), background 0.4s;
}
.threat-labels {
  display: flex;
  justify-content: space-between;
  font-size: 8px;
  letter-spacing: 0.12em;
  color: var(--text3);
  text-transform: uppercase;
}

/* Interpretation box */
.interp-box {
  background: var(--bg3);
  border: 1px solid var(--border);
  border-left: 3px solid var(--cyan);
  border-radius: 0 4px 4px 0;
  padding: 12px 14px;
  transition: border-left-color 0.4s;
}
.interp-box p {
  font-size: 11px;
  line-height: 1.8;
  color: var(--text2);
  letter-spacing: 0.02em;
}

/* Event log */
.log-list {
  display: flex;
  flex-direction: column;
  gap: 4px;
  max-height: 130px;
  overflow-y: auto;
}
.log-list::-webkit-scrollbar { width: 3px; }
.log-list::-webkit-scrollbar-track { background: transparent; }
.log-list::-webkit-scrollbar-thumb { background: var(--border2); border-radius: 2px; }
.log-entry {
  display: flex;
  align-items: baseline;
  gap: 10px;
  font-size: 10px;
  padding: 3px 0;
  border-bottom: 1px solid rgba(255,255,255,0.03);
  animation: logFade 0.3s ease;
}
@keyframes logFade { from { opacity: 0; transform: translateX(-6px); } to { opacity: 1; transform: none; } }
.log-time { color: var(--text3); min-width: 62px; flex-shrink: 0; }
.log-msg { color: var(--text2); }
.log-entry.safe .log-msg { color: var(--green); }
.log-entry.warn .log-msg { color: var(--amber); }
.log-entry.crit .log-msg { color: var(--red); }

/* Footer */
footer {
  border-top: 1px solid var(--border);
  padding-top: 0.75rem;
  display: flex;
  justify-content: space-between;
  font-size: 9px;
  letter-spacing: 0.15em;
  color: var(--text3);
  text-transform: uppercase;
  animation: fadeIn 1s ease 0.5s both;
}

/* Responsive */
@media (max-width: 680px) {
  .main-grid { grid-template-columns: 1fr; }
  .status-column { order: -1; }
  .ring-wrap { flex-direction: row; align-items: center; }
}
</style>
</head>
<body>
<div class="app">

  <!-- Header -->
  <header>
    <div class="header-left">
      <div class="header-title">ASN-001</div>
      <div class="header-sub">Autonomous Safety Node // Environmental Hazard Monitor</div>
    </div>
    <div class="header-right">
      <div class="live-indicator">
        <div class="live-dot"></div>
        <span>LIVE</span>
      </div>
      <div class="clock" id="clock">00:00:00</div>
    </div>
  </header>

  <!-- Main -->
  <div class="main-grid">

    <!-- Left: sensors -->
    <div style="display:flex;flex-direction:column;gap:1.25rem;">

      <!-- Analog sensors panel -->
      <div class="panel">
        <div class="bracket tl"></div><div class="bracket tr"></div>
        <div class="bracket bl"></div><div class="bracket br"></div>
        <div class="panel-label">Analog Sensors</div>
        <div class="sensors-grid">

          <div class="sensor-row">
            <div class="sensor-name">Temp</div>
            <div class="sensor-val-wrap">
              <input type="range" min="0" max="100" value="25" step="1" id="s_temp" oninput="update()">
              <div class="track-bg"><div class="track-fill" id="f_temp" style="width:25%;background:var(--cyan)"></div></div>
            </div>
            <div class="sensor-number" id="v_temp">25<span class="unit">°C</span></div>
          </div>

          <div class="sensor-row">
            <div class="sensor-name">Light</div>
            <div class="sensor-val-wrap">
              <input type="range" min="0" max="100" value="50" step="1" id="s_light" oninput="update()">
              <div class="track-bg"><div class="track-fill" id="f_light" style="width:50%;background:var(--cyan)"></div></div>
            </div>
            <div class="sensor-number" id="v_light">50<span class="unit">lux</span></div>
          </div>

          <div class="sensor-row">
            <div class="sensor-name">Sound</div>
            <div class="sensor-val-wrap">
              <input type="range" min="0" max="100" value="20" step="1" id="s_sound" oninput="update()">
              <div class="track-bg"><div class="track-fill" id="f_sound" style="width:20%;background:var(--cyan)"></div></div>
            </div>
            <div class="sensor-number" id="v_sound">20<span class="unit">dB</span></div>
          </div>

        </div>
      </div>

      <!-- Binary sensors panel -->
      <div class="panel">
        <div class="bracket tl"></div><div class="bracket tr"></div>
        <div class="bracket bl"></div><div class="bracket br"></div>
        <div class="panel-label">Binary Sensors</div>
        <div class="binary-grid">
          <button class="binary-btn off" id="btn_tilt" onclick="toggleSensor('tilt')">
            <span class="binary-icon">⚖️</span>
            <span class="binary-label">Tilt</span>
            <span class="binary-state" id="st_tilt">NOMINAL</span>
          </button>
          <button class="binary-btn off" id="btn_flame" onclick="toggleSensor('flame')">
            <span class="binary-icon">🔥</span>
            <span class="binary-label">Flame</span>
            <span class="binary-state" id="st_flame">NOMINAL</span>
          </button>
        </div>
      </div>

      <!-- Event log -->
      <div class="panel">
        <div class="bracket tl"></div><div class="bracket tr"></div>
        <div class="bracket bl"></div><div class="bracket br"></div>
        <div class="panel-label">Event Log</div>
        <div class="log-list" id="log"></div>
      </div>

    </div>

    <!-- Right: status -->
    <div class="status-column">

      <!-- Ring gauge panel -->
      <div class="panel">
        <div class="bracket tl"></div><div class="bracket tr"></div>
        <div class="bracket bl"></div><div class="bracket br"></div>
        <div class="panel-label">Threat Level</div>
        <div class="ring-wrap">
          <div class="ring-svg" style="width:160px;height:160px;">
            <svg width="160" height="160" viewBox="0 0 160 160">
              <!-- bg ring -->
              <circle cx="80" cy="80" r="60" fill="none" stroke="#111820" stroke-width="10"/>
              <!-- tick marks -->
              <g id="ticks"></g>
              <!-- active ring -->
              <circle id="ring_arc" cx="80" cy="80" r="60"
                fill="none" stroke="var(--green)" stroke-width="10"
                stroke-dasharray="0 377"
                stroke-dashoffset="94"
                stroke-linecap="round"
                style="transition: stroke-dasharray 0.6s cubic-bezier(0.4,0,0.2,1), stroke 0.4s;"
                transform="rotate(-90 80 80)"/>
              <!-- glow ring -->
              <circle id="ring_glow" cx="80" cy="80" r="60"
                fill="none" stroke="var(--green)" stroke-width="18"
                stroke-dasharray="0 377"
                stroke-dashoffset="94"
                stroke-linecap="round"
                style="transition: stroke-dasharray 0.6s cubic-bezier(0.4,0,0.2,1), stroke 0.4s; opacity:0.12; filter:blur(4px);"
                transform="rotate(-90 80 80)"/>
            </svg>
            <div class="ring-label-center">
              <div class="ring-status-text" id="ring_text" style="color:var(--green)">SAFE</div>
              <div class="ring-status-sub">system status</div>
            </div>
          </div>

          <div class="threat-bar-wrap" style="width:100%;">
            <div style="display:flex;justify-content:space-between;font-size:9px;color:var(--text3);letter-spacing:0.12em;">
              <span>THREAT SCORE</span>
              <span id="threat_pct" style="font-family:var(--display);font-size:11px;font-weight:700;color:var(--green);">0%</span>
            </div>
            <div class="threat-track">
              <div class="threat-fill" id="threat_bar" style="width:0%;background:var(--green);"></div>
            </div>
            <div class="threat-labels">
              <span>Nominal</span>
              <span>Warning</span>
              <span>Critical</span>
            </div>
          </div>
        </div>
      </div>

      <!-- Interpretation panel -->
      <div class="panel">
        <div class="bracket tl"></div><div class="bracket tr"></div>
        <div class="bracket bl"></div><div class="bracket br"></div>
        <div class="panel-label">AI Interpretation</div>
        <div class="interp-box" id="interp_box">
          <p id="interp_text">All sensors within nominal operating parameters. No hazards detected. System operating normally.</p>
        </div>
      </div>

    </div>
  </div>

  <!-- Footer -->
  <footer>
    <span>Node ID: ASN-001-FL-GAV</span>
    <span id="uptime">UPTIME: 00:00:00</span>
    <span>FW: v2.4.1</span>
  </footer>

</div>

<script>
const state = { tilt: 0, flame: 0 };
const startTime = Date.now();
let lastStatus = null;
const logEntries = [];

// Draw tick marks on ring
(function drawTicks() {
  const g = document.getElementById('ticks');
  for (let i = 0; i < 36; i++) {
    const angle = (i / 36) * 360 - 90;
    const rad = angle * Math.PI / 180;
    const r1 = 72, r2 = i % 3 === 0 ? 67 : 70;
    const x1 = 80 + r1 * Math.cos(rad);
    const y1 = 80 + r1 * Math.sin(rad);
    const x2 = 80 + r2 * Math.cos(rad);
    const y2 = 80 + r2 * Math.sin(rad);
    const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
    line.setAttribute('x1', x1); line.setAttribute('y1', y1);
    line.setAttribute('x2', x2); line.setAttribute('y2', y2);
    line.setAttribute('stroke', i % 3 === 0 ? '#1a2a35' : '#131e26');
    line.setAttribute('stroke-width', i % 3 === 0 ? '1.5' : '1');
    g.appendChild(line);
  }
})();

function classify(temp, light, sound, tilt, flame) {
  if (flame) return { status: 'CRITICAL', score: 100, label: 'Flame Detected', msg: 'CRITICAL: Active flame sensor trigger. Immediate evacuation required. Suppress ignition source. All clear protocol initiated.' };
  if (tilt && temp > 70) return { status: 'CRITICAL', score: 90, label: 'Compound Failure', msg: 'CRITICAL: Device tilt combined with thermal anomaly detected. Possible structural + heat failure. Inspect immediately.' };
  if (temp > 80) return { status: 'CRITICAL', score: 85, label: 'Thermal Limit', msg: 'CRITICAL: Temperature exceeds safe threshold. Risk of fire or hardware damage. Engage thermal protocols.' };
  if (tilt) return { status: 'WARNING', score: 60, label: 'Tilt Detected', msg: 'WARNING: Orientation sensor triggered. Device mounting may be compromised. Verify physical stability.' };
  if (temp > 60) return { status: 'WARNING', score: 55, label: 'High Temperature', msg: 'WARNING: Temperature elevated beyond normal range. Monitor thermal trend. Check ventilation.' };
  if (sound > 75) return { status: 'WARNING', score: 45, label: 'Acoustic Anomaly', msg: 'WARNING: Sound level exceeds threshold. Possible mechanical fault, intrusion event, or equipment failure nearby.' };
  if (light > 85) return { status: 'WARNING', score: 35, label: 'Light Overexposure', msg: 'WARNING: Photosensor reading unusually high. Check for direct light exposure or sensor obstruction.' };

  // Compute baseline score from sensor values
  const base = Math.round((temp * 0.5 + sound * 0.3 + light * 0.2) * 0.25);
  return { status: 'SAFE', score: Math.min(base, 28), label: 'Nominal', msg: 'All systems nominal. Sensors within operating range. No hazards or anomalies detected.' };
}

function setColor(status) {
  return status === 'SAFE' ? 'var(--green)' : status === 'WARNING' ? 'var(--amber)' : 'var(--red)';
}

function addLog(status, label) {
  const now = new Date();
  const t = now.toLocaleTimeString('en-US', { hour12: false });
  logEntries.unshift({ t, label, cls: status === 'SAFE' ? 'safe' : status === 'WARNING' ? 'warn' : 'crit' });
  if (logEntries.length > 20) logEntries.pop();
  const el = document.getElementById('log');
  el.innerHTML = logEntries.map(e =>
    `<div class="log-entry ${e.cls}"><span class="log-time">${e.t}</span><span class="log-msg">${e.label}</span></div>`
  ).join('');
}

function update() {
  const temp = +document.getElementById('s_temp').value;
  const light = +document.getElementById('s_light').value;
  const sound = +document.getElementById('s_sound').value;

  // Update readouts
  document.getElementById('v_temp').innerHTML = temp + '<span class="unit">°C</span>';
  document.getElementById('v_light').innerHTML = light + '<span class="unit">lux</span>';
  document.getElementById('v_sound').innerHTML = sound + '<span class="unit">dB</span>';

  // Track fills & slider colors
  const sensors = [
    { id: 'temp', val: temp, warn: 60, crit: 80 },
    { id: 'light', val: light, warn: 85, crit: 95 },
    { id: 'sound', val: sound, warn: 75, crit: 90 }
  ];
  sensors.forEach(({ id, val, warn, crit }) => {
    const fill = document.getElementById('f_' + id);
    const slider = document.getElementById('s_' + id);
    const color = val >= crit ? 'var(--red)' : val >= warn ? 'var(--amber)' : 'var(--cyan)';
    fill.style.width = val + '%';
    fill.style.background = color;
    document.getElementById('v_' + id).style.color = color;
    slider.className = val >= crit ? 'crit' : val >= warn ? 'warn' : '';
  });

  const result = classify(temp, light, sound, state.tilt, state.flame);
  const color = setColor(result.status);
  const circumference = 2 * Math.PI * 60; // ~377
  const dashLen = (result.score / 100) * circumference;

  document.getElementById('ring_arc').style.stroke = color;
  document.getElementById('ring_arc').setAttribute('stroke-dasharray', `${dashLen} ${circumference - dashLen}`);
  document.getElementById('ring_glow').style.stroke = color;
  document.getElementById('ring_glow').setAttribute('stroke-dasharray', `${dashLen} ${circumference - dashLen}`);
  document.getElementById('ring_text').textContent = result.status;
  document.getElementById('ring_text').style.color = color;

  document.getElementById('threat_bar').style.width = result.score + '%';
  document.getElementById('threat_bar').style.background = color;
  document.getElementById('threat_pct').textContent = result.score + '%';
  document.getElementById('threat_pct').style.color = color;

  document.getElementById('interp_text').textContent = result.msg;
  document.getElementById('interp_box').style.borderLeftColor = color;

  if (result.status !== lastStatus) {
    addLog(result.status, result.label);
    lastStatus = result.status;
  }
}

function toggleSensor(key) {
  state[key] = state[key] === 0 ? 1 : 0;
  const btn = document.getElementById('btn_' + key);
  const st = document.getElementById('st_' + key);
  btn.className = 'binary-btn ' + (state[key] ? 'on' : 'off');
  st.textContent = state[key] ? 'TRIGGERED' : 'NOMINAL';
  update();
}

// Clock & uptime
function tick() {
  const now = new Date();
  document.getElementById('clock').textContent = now.toLocaleTimeString('en-US', { hour12: false });
  const elapsed = Math.floor((Date.now() - startTime) / 1000);
  const h = String(Math.floor(elapsed / 3600)).padStart(2, '0');
  const m = String(Math.floor((elapsed % 3600) / 60)).padStart(2, '0');
  const s = String(elapsed % 60).padStart(2, '0');
  document.getElementById('uptime').textContent = `UPTIME: ${h}:${m}:${s}`;
}

addLog('SAFE', 'System boot — nominal');
tick();
setInterval(tick, 1000);
update();
</script>
</body>
</html>