#pragma once

// =============================================================================
//  §5. Embedded Responsive Web Dashboard
// =============================================================================

static const char k_indexHtml[] = R"rawindex(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>GTA SA — System Core Remote</title>
<style>
  :root {
    --bg: #090d16;
    --card: #131b2e;
    --card-border: #1e293b;
    --card-hover: #1c2742;
    --primary: #38bdf8;
    --success: #22c55e;
    --success-bg: rgba(34, 197, 94, 0.15);
    --warning: #f59e0b;
    --text: #f8fafc;
    --text-muted: #94a3b8;
  }
  * { box-sizing: border-box; margin: 0; padding: 0; touch-action: manipulation; }
  body {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    background: var(--bg);
    color: var(--text);
    display: flex;
    justify-content: center;
    align-items: center;
    min-height: 100vh;
    padding: 16px;
  }
  .dashboard {
    width: 100%;
    max-width: 520px;
    background: var(--card);
    border: 1px solid var(--card-border);
    border-radius: 18px;
    padding: 22px;
    box-shadow: 0 25px 35px -5px rgba(0, 0, 0, 0.6);
  }
  .header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 18px;
    border-bottom: 1px solid var(--card-border);
    padding-bottom: 14px;
  }
  .title { font-size: 1.2rem; font-weight: 800; color: var(--primary); letter-spacing: 0.02em; }
  .subtitle { font-size: 0.72rem; color: var(--text-muted); text-transform: uppercase; }
  .badge {
    display: flex;
    align-items: center;
    gap: 6px;
    background: var(--success-bg);
    color: var(--success);
    padding: 4px 10px;
    border-radius: 9999px;
    font-size: 0.75rem;
    font-weight: 700;
  }
  .badge-dot {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: var(--success);
    box-shadow: 0 0 8px var(--success);
  }
  .btn-gps {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 10px;
    width: 100%;
    margin-bottom: 16px;
    padding: 14px 18px;
    background: linear-gradient(135deg, rgba(56, 189, 248, 0.25) 0%, rgba(14, 165, 233, 0.45) 100%);
    border: 1px solid rgba(56, 189, 248, 0.6);
    border-radius: 12px;
    color: #f0f9ff;
    font-size: 1rem;
    font-weight: 800;
    text-decoration: none;
    letter-spacing: 0.03em;
    box-shadow: 0 8px 24px rgba(56, 189, 248, 0.25);
    transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
  }
  .btn-gps:hover {
    background: linear-gradient(135deg, rgba(56, 189, 248, 0.4) 0%, rgba(14, 165, 233, 0.65) 100%);
    border-color: #38bdf8;
    box-shadow: 0 10px 28px rgba(56, 189, 248, 0.4);
    transform: translateY(-2px);
  }
  .btn-gps:active {
    transform: scale(0.98);
  }
  .stats-grid {
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    gap: 8px;
    margin-bottom: 14px;
  }
  .stat-card {
    background: rgba(15, 23, 42, 0.7);
    border: 1px solid var(--card-border);
    border-radius: 10px;
    padding: 8px 6px;
    text-align: center;
  }
  .stat-label { font-size: 0.65rem; color: var(--text-muted); text-transform: uppercase; margin-bottom: 3px; }
  .stat-value { font-size: 0.95rem; font-weight: 700; font-family: monospace; color: var(--primary); }
  
  .gps-box {
    background: rgba(56, 189, 248, 0.08);
    border: 1px solid rgba(56, 189, 248, 0.25);
    border-radius: 12px;
    padding: 12px 14px;
    margin-bottom: 16px;
    display: flex;
    justify-content: space-between;
    align-items: center;
  }
  .gps-title { font-size: 0.72rem; text-transform: uppercase; color: var(--text-muted); }
  .gps-coords { font-family: monospace; font-size: 0.9rem; font-weight: 700; color: #38bdf8; }

  .section-title {
    font-size: 0.75rem;
    text-transform: uppercase;
    letter-spacing: 0.06em;
    color: var(--text-muted);
    margin: 14px 0 8px 0;
  }
  .grid {
    display: grid;
    grid-template-columns: repeat(2, 1fr);
    gap: 8px;
  }
  .grid-3 {
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    gap: 8px;
  }
  .btn {
    background: #1e293b;
    color: var(--text);
    border: 1px solid var(--card-border);
    border-radius: 10px;
    padding: 12px 8px;
    font-size: 0.88rem;
    font-weight: 600;
    cursor: pointer;
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 6px;
    user-select: none;
    -webkit-tap-highlight-color: transparent;
    transition: background 0.15s ease, border-color 0.15s ease;
  }
  .btn:hover { background: #27354f; }
  .btn:active { transform: scale(0.96); border-color: var(--primary); }
  .btn-vitals {
    width: 100%;
    margin-top: 4px;
    background: var(--success-bg);
    border-color: rgba(34, 197, 94, 0.4);
    color: #4ade80;
    padding: 14px;
    font-size: 0.95rem;
    font-weight: 700;
  }
  .btn-vitals:hover { background: rgba(34, 197, 94, 0.25); }
  .console {
    margin-top: 16px;
    padding: 10px;
    background: #060911;
    border: 1px solid #1a2234;
    border-radius: 8px;
    font-family: monospace;
    font-size: 0.75rem;
    color: var(--text-muted);
    min-height: 38px;
    display: flex;
    align-items: center;
  }
</style>
</head>
<body>
<div class="dashboard">
  <div class="header">
    <div>
      <div class="title">GTA SYSTEM CORE</div>
      <div class="subtitle">Multi-Threaded SPSC Engine</div>
    </div>
    <div class="badge">
      <div class="badge-dot"></div>
      <span id="sys-status">PORT 8080</span>
    </div>
  </div>

  <div style="display: flex; gap: 8px; margin-bottom: 14px;">
    <a href="/map" class="btn-gps" style="flex: 1; margin-bottom: 0;">
      <span>🗺️</span>
      <span>Live GPS Radar</span>
    </a>
    <a href="/logistics" class="btn-gps" style="flex: 1; margin-bottom: 0; background: linear-gradient(135deg, rgba(56,189,248,0.2), rgba(15,23,42,0.8)); border-color: #38bdf8;">
      <span>🏢</span>
      <span>Logistics Tycoon</span>
    </a>
    <a href="/municipal" class="btn-gps" style="flex: 1; margin-bottom: 0; background: linear-gradient(135deg, rgba(251,191,36,0.2), rgba(15,23,42,0.8)); border-color: #fbbf24; color: #fef08a;">
      <span>🏛️</span>
      <span>City Hall</span>
    </a>
  </div>

  <div class="stats-grid">
    <div class="stat-card">
      <div class="stat-label">Web Cmds</div>
      <div class="stat-value" id="q-web">0</div>
    </div>
    <div class="stat-card">
      <div class="stat-label">AI Telemetry</div>
      <div class="stat-value" id="q-tel">0</div>
    </div>
    <div class="stat-card">
      <div class="stat-label">AI Directives</div>
      <div class="stat-value" id="q-dir">0</div>
    </div>
  </div>

  <div class="gps-box">
    <div>
      <div class="gps-title">Live Position (X, Y, Z)</div>
      <div class="gps-coords" id="gps-pos">Waiting for data...</div>
    </div>
    <div style="text-align: right;">
      <div class="gps-title">Speed / Heading</div>
      <div class="gps-coords" id="gps-spd">0 km/h · 0°</div>
    </div>
  </div>

  <div class="section-title">Environment & Weather</div>
  <div class="grid">
    <button class="btn" onclick="sendCmd('/api/weather?id=1')">☀️ Sunny</button>
    <button class="btn" onclick="sendCmd('/api/weather?id=0')">🌤️ Extra Sunny</button>
    <button class="btn" onclick="sendCmd('/api/weather?id=4')">☁️ Cloudy</button>
    <button class="btn" onclick="sendCmd('/api/weather?id=8')">🌧️ Rainy</button>
    <button class="btn" onclick="sendCmd('/api/weather?id=9')">🌫️ Foggy</button>
    <button class="btn" onclick="sendCmd('/api/weather?id=19')">🏜️ Sandstorm</button>
  </div>

  <div class="section-title">Player Vitals</div>
  <button class="btn btn-vitals" onclick="sendCmd('/api/vitals')">🛡️ Restore Health & Armor</button>

  <div class="section-title">Tactical HUD Notices</div>
  <div class="grid-3">
    <button class="btn" onclick="sendCmd('/api/notice?id=0')">💬 Mobile</button>
    <button class="btn" onclick="sendCmd('/api/notice?id=1')">⚠️ Alert</button>
    <button class="btn" onclick="sendCmd('/api/notice?id=2')">ℹ️ Nominal</button>
  </div>

  <div class="console" id="console">System ready. Polling active.</div>
</div>

<script>
async function updateStats() {
  try {
    const res = await fetch('/api/status');
    if (res.ok) {
      const data = await res.json();
      const stats = data.stats || {};
      document.getElementById('q-web').textContent = stats.web ?? data.webCommandsTotal ?? 0;
      document.getElementById('q-tel').textContent = stats.telemetry ?? data.telemetryTotal ?? 0;
      document.getElementById('q-dir').textContent = stats.directives ?? data.directivesTotal ?? 0;
      
      if (data.pos) {
        document.getElementById('gps-pos').textContent = 
          `${data.pos.x.toFixed(1)}, ${data.pos.y.toFixed(1)}, ${data.pos.z.toFixed(1)}`;
      }
      const heading = data.heading != null ? Math.round(data.heading) : 0;
      const speedKmh = data.speed != null ? (data.speed * 180).toFixed(0) : 0;
      document.getElementById('gps-spd').textContent = `${speedKmh} km/h · ${heading}°`;

      const statusEl = document.getElementById('sys-status');
      if (statusEl) {
        statusEl.textContent = 'ONLINE';
        statusEl.style.color = 'var(--success)';
      }
    }
  } catch (e) {
    const statusEl = document.getElementById('sys-status');
    if (statusEl) {
      statusEl.textContent = 'OFFLINE';
      statusEl.style.color = '#ef4444';
    }
  }
}
setInterval(updateStats, 400);
updateStats();

async function sendCmd(url) {
  const con = document.getElementById('console');
  try {
    const res = await fetch(url);
    if (res.ok) {
      const data = await res.json();
      con.textContent = `[OK] Dispatched: ${data.type || 'Action'}`;
      updateStats();
    }
  } catch (err) {
    con.textContent = `[Error] ${err.message}`;
  }
}
</script>
</body>
</html>)rawindex";

// =============================================================================
//  §6. Embedded Interactive Live GPS Companion Map (Leaflet.js CRS.Simple)
// =============================================================================

static const char k_mapHtml[] = R"rawmap(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>GTA SA — Live GPS Radar</title>
<link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" />
<script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"></script>
<style>
  :root {
    --bg: #090d16;
    --card: #131b2e;
    --card-border: #1e293b;
    --primary: #38bdf8;
    --primary-glow: rgba(56, 189, 248, 0.4);
    --target: #f59e0b;
    --target-glow: rgba(245, 158, 11, 0.4);
    --success: #22c55e;
    --text: #f8fafc;
    --text-muted: #94a3b8;
  }
  * { box-sizing: border-box; margin: 0; padding: 0; }
  html, body {
    width: 100%;
    height: 100%;
    overflow: hidden;
    background: var(--bg);
    color: var(--text);
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
  }
  #map {
    width: 100%;
    height: 100%;
    background: #060911;
  }
  .leaflet-container {
    background: #060911 !important;
  }

  /* Dedicated Strategic POI Orange Highlight Markers & Tactical Popups */
  .strategic-hub-icon-wrap {
    background: transparent !important;
    border: none !important;
  }
  .strategic-hub-icon {
    position: relative;
    width: 18px;
    height: 18px;
    cursor: pointer;
  }
  .strategic-hub-icon .pulse-ring {
    position: absolute;
    top: -9px;
    left: -9px;
    width: 36px;
    height: 36px;
    border-radius: 50%;
    border: 2px solid #f59e0b;
    box-shadow: 0 0 10px #f59e0b;
    animation: hubPulseAnimation 1.8s cubic-bezier(0.24, 0, 0.38, 1) infinite;
    pointer-events: none;
  }
  .strategic-hub-icon .hub-dot {
    position: absolute;
    top: 0;
    left: 0;
    width: 18px;
    height: 18px;
    border-radius: 50%;
    background: #f59e0b;
    border: 2px solid #ffffff;
    box-shadow: 0 0 12px rgba(245, 158, 11, 0.95);
    box-sizing: border-box;
    transition: transform 0.2s ease, box-shadow 0.2s ease;
  }
  .strategic-hub-icon:hover .hub-dot {
    transform: scale(1.25);
    box-shadow: 0 0 18px #f59e0b;
  }
  @keyframes hubPulseAnimation {
    0% {
      transform: scale(0.5);
      opacity: 1;
    }
    70% {
      transform: scale(1.6);
      opacity: 0.15;
    }
    100% {
      transform: scale(1.9);
      opacity: 0;
    }
  }

  .tactical-popup-wrap .leaflet-popup-content-wrapper {
    background: #0b1120 !important;
    border: 1.5px solid #f59e0b !important;
    border-radius: 8px !important;
    box-shadow: 0 10px 25px rgba(0,0,0,0.8), 0 0 15px rgba(245,158,11,0.25) !important;
    color: #f1f5f9 !important;
    padding: 0 !important;
  }
  .tactical-popup-wrap .leaflet-popup-tip {
    background: #0b1120 !important;
    border: 1px solid #f59e0b !important;
  }
  .tactical-popup-wrap .leaflet-popup-content {
    margin: 12px 14px !important;
    line-height: 1.4 !important;
  }
  .tactical-popup {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    color: #f1f5f9;
    min-width: 260px;
  }
  .tactical-popup .hub-badge {
    display: inline-block;
    font-size: 9px;
    font-weight: 800;
    letter-spacing: 0.08em;
    text-transform: uppercase;
    background: rgba(245, 158, 11, 0.18);
    color: #f59e0b;
    border: 1px solid rgba(245, 158, 11, 0.4);
    padding: 2px 6px;
    border-radius: 4px;
    margin-bottom: 6px;
  }
  .tactical-popup .hub-title {
    font-size: 14px;
    font-weight: 700;
    color: #ffffff;
    display: flex;
    align-items: center;
    gap: 6px;
  }
  .tactical-popup .hub-sub {
    font-size: 11px;
    color: #94a3b8;
    margin-bottom: 8px;
    border-bottom: 1px solid rgba(255, 255, 255, 0.1);
    padding-bottom: 6px;
  }
  .tactical-popup .hub-metric-row {
    display: flex;
    justify-content: space-between;
    font-size: 11px;
    margin-bottom: 4px;
  }
  .tactical-popup .hub-metric-row .lbl {
    color: #94a3b8;
  }
  .tactical-popup .hub-metric-row .val {
    font-weight: 600;
    color: #f1f5f9;
  }
  .tactical-popup .hub-metric-row .val.food { color: #22c55e; }
  .tactical-popup .hub-metric-row .val.fuel { color: #eab308; }
  .tactical-popup .hub-metric-row .val.queue { color: #38bdf8; }
  .tactical-popup .hub-metric-row .val.pass { color: #10b981; }
  .tactical-popup .hub-role {
    font-size: 10px;
    color: #94a3b8;
    background: rgba(255, 255, 255, 0.05);
    border-radius: 4px;
    padding: 6px;
    margin-top: 8px;
    line-height: 1.35;
  }


  /* HUD Overlay: Glassmorphism Top-Left Telemetry */
  .hud-telemetry {
    position: absolute;
    top: 14px;
    left: 14px;
    z-index: 1000;
    width: 320px;
    background: rgba(19, 27, 46, 0.88);
    backdrop-filter: blur(14px);
    -webkit-backdrop-filter: blur(14px);
    border: 1px solid rgba(56, 189, 248, 0.35);
    border-radius: 14px;
    padding: 14px 16px;
    box-shadow: 0 16px 32px rgba(0, 0, 0, 0.6), 0 0 15px rgba(56, 189, 248, 0.15);
    pointer-events: auto;
  }
  .hud-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    border-bottom: 1px solid var(--card-border);
    padding-bottom: 8px;
    margin-bottom: 10px;
  }
  .hud-title {
    font-size: 0.78rem;
    font-weight: 800;
    color: var(--primary);
    text-transform: uppercase;
    letter-spacing: 0.08em;
    display: flex;
    align-items: center;
    gap: 6px;
  }
  .hud-status {
    display: flex;
    align-items: center;
    gap: 6px;
    font-size: 0.72rem;
    font-weight: 800;
    color: var(--success);
    background: rgba(34, 197, 94, 0.15);
    padding: 3px 10px;
    border-radius: 9999px;
    border: 1px solid rgba(34, 197, 94, 0.3);
    transition: all 0.25s ease;
  }
  .status-dot {
    width: 7px;
    height: 7px;
    border-radius: 50%;
    background: var(--success);
    box-shadow: 0 0 8px var(--success);
    animation: pulse 1.5s infinite;
  }
  @keyframes pulse {
    0%, 100% { opacity: 1; transform: scale(1); }
    50% { opacity: 0.4; transform: scale(0.85); }
  }

  .hud-stats-row {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 8px;
    margin-bottom: 8px;
  }
  .hud-stat-box {
    background: rgba(15, 23, 42, 0.75);
    border: 1px solid var(--card-border);
    border-radius: 8px;
    padding: 6px 10px;
  }
  .hud-label {
    font-size: 0.62rem;
    text-transform: uppercase;
    color: var(--text-muted);
    letter-spacing: 0.04em;
  }
  .hud-val-lg {
    font-family: monospace;
    font-size: 1.15rem;
    font-weight: 800;
    color: #ffffff;
  }
  .hud-val-lg span {
    font-size: 0.7rem;
    color: var(--primary);
    font-weight: 600;
    margin-left: 2px;
  }

  .hud-coords-box {
    background: rgba(15, 23, 42, 0.75);
    border: 1px solid var(--card-border);
    border-radius: 8px;
    padding: 6px 10px;
    margin-bottom: 8px;
  }
  .hud-coords-val {
    font-family: monospace;
    font-size: 0.82rem;
    font-weight: 700;
    color: var(--primary);
    display: flex;
    justify-content: space-between;
  }

  .hud-fleet-box {
    background: rgba(56, 189, 248, 0.08);
    border: 1px solid rgba(56, 189, 248, 0.25);
    border-radius: 8px;
    padding: 6px 10px;
    margin-bottom: 8px;
    display: flex;
    justify-content: space-between;
    align-items: center;
    font-size: 0.72rem;
  }
  .hud-fleet-title { color: var(--text-muted); text-transform: uppercase; font-weight: 600; }
  .hud-fleet-val { font-family: monospace; font-weight: 800; color: #38bdf8; }

  .hud-balance-box {
    background: rgba(34, 197, 94, 0.1);
    border: 1px solid rgba(34, 197, 94, 0.3);
    border-radius: 8px;
    padding: 6px 10px;
    margin-bottom: 8px;
    display: flex;
    justify-content: space-between;
    align-items: center;
    font-size: 0.72rem;
  }
  .hud-balance-title { color: #86efac; text-transform: uppercase; font-weight: 600; }
  .hud-balance-val { font-family: monospace; font-weight: 800; color: #22c55e; }

  /* Tactical Dark Theme Leaflet Popup */
  .leaflet-popup-content-wrapper {
    background: rgba(15, 23, 42, 0.95) !important;
    color: #f8fafc !important;
    border: 1px solid rgba(56, 189, 248, 0.4) !important;
    border-radius: 10px !important;
    box-shadow: 0 12px 30px rgba(0, 0, 0, 0.6) !important;
    backdrop-filter: blur(12px) !important;
  }
  .leaflet-popup-tip {
    background: rgba(15, 23, 42, 0.95) !important;
    border: 1px solid rgba(56, 189, 248, 0.4) !important;
  }
  .leaflet-popup-close-button {
    color: #94a3b8 !important;
  }

  .hud-directive-box {
    background: rgba(245, 158, 11, 0.1);
    border: 1px solid rgba(245, 158, 11, 0.35);
    border-radius: 8px;
    padding: 6px 10px;
    font-size: 0.7rem;
    color: #fef08a;
    display: flex;
    justify-content: space-between;
    align-items: center;
  }

  /* HUD Top-Right Controls */
  .hud-controls {
    position: absolute;
    top: 14px;
    right: 14px;
    z-index: 1000;
    display: flex;
    flex-direction: column;
    gap: 8px;
    pointer-events: auto;
  }
  .ctrl-btn {
    background: rgba(19, 27, 46, 0.9);
    backdrop-filter: blur(10px);
    -webkit-backdrop-filter: blur(10px);
    border: 1px solid var(--card-border);
    border-radius: 10px;
    padding: 9px 12px;
    color: var(--text);
    font-size: 0.78rem;
    font-weight: 700;
    cursor: pointer;
    display: flex;
    align-items: center;
    gap: 8px;
    box-shadow: 0 8px 18px rgba(0, 0, 0, 0.4);
    transition: all 0.18s ease;
    user-select: none;
    text-decoration: none;
  }
  .ctrl-btn:hover {
    background: #1e293b;
    border-color: var(--primary);
    color: var(--primary);
  }
  .ctrl-btn.active {
    background: rgba(56, 189, 248, 0.2);
    border-color: var(--primary);
    color: var(--primary);
  }

  /* Waypoint Markers & Badges */
  .wp-tooltip {
    background: rgba(15, 23, 42, 0.9) !important;
    border: 1px solid rgba(6, 182, 212, 0.7) !important;
    color: #38bdf8 !important;
    font-size: 10px !important;
    font-weight: 700 !important;
    padding: 2px 5px !important;
    border-radius: 4px !important;
    box-shadow: 0 2px 8px rgba(0, 0, 0, 0.6) !important;
    white-space: nowrap !important;
  }
  .wp-tooltip-facility {
    background: rgba(30, 27, 75, 0.95) !important;
    border: 1.5px solid #f59e0b !important;
    color: #fbbf24 !important;
    font-size: 11px !important;
    font-weight: 800 !important;
    padding: 3px 7px !important;
    border-radius: 5px !important;
    box-shadow: 0 0 10px rgba(245, 158, 11, 0.4) !important;
    white-space: nowrap !important;
  }

  /* District Zone Bounding Overlays & Labels */
  .zone-label {
    background: rgba(15, 23, 42, 0.88) !important;
    backdrop-filter: blur(4px) !important;
    -webkit-backdrop-filter: blur(4px) !important;
    border-radius: 6px !important;
    font-size: 11px !important;
    font-weight: 900 !important;
    letter-spacing: 0.12em !important;
    padding: 3px 9px !important;
    box-shadow: 0 4px 14px rgba(0, 0, 0, 0.6) !important;
    text-transform: uppercase !important;
    pointer-events: none !important;
    user-select: none !important;
  }
  .zone-label-0 {
    border: 1.5px solid rgba(244, 63, 94, 0.85) !important;
    color: #fda4af !important;
    box-shadow: 0 0 14px rgba(244, 63, 94, 0.4) !important;
  }
  .zone-label-1 {
    border: 1.5px solid rgba(56, 189, 248, 0.85) !important;
    color: #bae6fd !important;
    box-shadow: 0 0 14px rgba(56, 189, 248, 0.4) !important;
  }
  .zone-label-2 {
    border: 1.5px solid rgba(234, 179, 8, 0.85) !important;
    color: #fef08a !important;
    box-shadow: 0 0 14px rgba(234, 179, 8, 0.4) !important;
  }

  /* Retail Store Tooltips */
  .store-tooltip {
    background: rgba(15, 23, 42, 0.95) !important;
    border: 1px solid rgba(255, 255, 255, 0.15) !important;
    border-radius: 6px !important;
    font-size: 11px !important;
    font-weight: 700 !important;
    padding: 4px 8px !important;
    color: #f1f5f9 !important;
    box-shadow: 0 4px 14px rgba(0, 0, 0, 0.6) !important;
    white-space: nowrap !important;
  }

  /* Roadblock Crisis Alerts */
  .alert-box {
    padding: 8px 12px;
    border-radius: 8px;
    margin-bottom: 8px;
    font-size: 0.78rem;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
  }
  .alert-danger {
    background: rgba(239, 68, 68, 0.15);
    border: 1px solid rgba(239, 68, 68, 0.5);
    color: #fca5a5;
  }
  .alert-success {
    background: rgba(34, 197, 94, 0.15);
    border: 1px solid rgba(34, 197, 94, 0.5);
    color: #86efac;
  }
  .alert-title {
    font-weight: 800;
    margin-bottom: 2px;
    letter-spacing: 0.02em;
  }
  .alert-desc {
    font-size: 0.72rem;
    opacity: 0.9;
  }

  @keyframes pulse-bankrupt {
    0%, 100% { opacity: 1; box-shadow: 0 0 4px rgba(239, 68, 68, 0.4); }
    50% { opacity: 0.6; box-shadow: 0 0 16px rgba(239, 68, 68, 0.9); }
  }

  /* Player Marker */
  .player-marker-node {
    position: relative;
    width: 44px;
    height: 44px;
  }
  .player-pulse-ring {
    position: absolute;
    top: 2px;
    left: 2px;
    width: 40px;
    height: 40px;
    border-radius: 50%;
    border: 2px solid var(--primary);
    background: rgba(56, 189, 248, 0.2);
    animation: player-pulse 1.8s cubic-bezier(0.2, 0.6, 0.3, 1) infinite;
    pointer-events: none;
  }
  @keyframes player-pulse {
    0% { transform: scale(0.5); opacity: 0.9; }
    80% { transform: scale(1.7); opacity: 0.1; }
    100% { transform: scale(1.9); opacity: 0; }
  }
  .player-arrow-rotator {
    position: absolute;
    top: 0;
    left: 0;
    width: 44px;
    height: 44px;
    display: flex;
    align-items: center;
    justify-content: center;
    transform-origin: 50% 50%;
    transition: transform 0.22s linear;
  }
  .player-arrow-svg {
    width: 28px;
    height: 28px;
    filter: drop-shadow(0 0 6px rgba(56, 189, 248, 0.9));
  }
  .player-dot-center {
    position: absolute;
    top: 18px;
    left: 18px;
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: #ffffff;
    box-shadow: 0 0 10px var(--primary);
  }

  /* Target Marker */
  .target-marker-node {
    position: relative;
    width: 36px;
    height: 36px;
  }
  .target-pulse {
    position: absolute;
    top: 0;
    left: 0;
    width: 36px;
    height: 36px;
    border-radius: 50%;
    border: 2px solid var(--target);
    background: rgba(245, 158, 11, 0.25);
    animation: target-anim 1.5s ease-out infinite;
  }
  @keyframes target-anim {
    0% { transform: scale(0.6); opacity: 1; }
    100% { transform: scale(1.6); opacity: 0; }
  }
  .target-reticle-svg {
    position: absolute;
    top: 2px;
    left: 2px;
    width: 32px;
    height: 32px;
    filter: drop-shadow(0 0 6px rgba(245, 158, 11, 0.8));
    animation: target-spin 12s linear infinite;
  }
  @keyframes target-spin {
    from { transform: rotate(0deg); }
    to { transform: rotate(360deg); }
  }

  /* SOS Breakdown Marker */
  .sos-marker-node {
    position: relative;
    width: 32px;
    height: 32px;
    display: flex;
    align-items: center;
    justify-content: center;
  }
  .sos-pulse-ring {
    position: absolute;
    top: 0;
    left: 0;
    width: 32px;
    height: 32px;
    border-radius: 50%;
    border: 2px solid #ef4444;
    background: rgba(239, 68, 68, 0.35);
    animation: sos-anim 1.2s ease-out infinite;
  }
  @keyframes sos-anim {
    0% { transform: scale(0.6); opacity: 1; }
    100% { transform: scale(2.0); opacity: 0; }
  }
  .sos-core-dot {
    position: relative;
    width: 18px;
    height: 18px;
    border-radius: 50%;
    background: #ef4444;
    border: 2px solid #ffffff;
    box-shadow: 0 0 10px #ef4444;
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 10px;
    line-height: 1;
    font-weight: 900;
    color: #ffffff;
  }

  /* Compact Bottom Strip */
  .hud-bottom {
    position: absolute;
    bottom: 14px;
    left: 50%;
    transform: translateX(-50%);
    z-index: 1000;
    background: rgba(19, 27, 46, 0.85);
    backdrop-filter: blur(10px);
    -webkit-backdrop-filter: blur(10px);
    border: 1px solid var(--card-border);
    border-radius: 9999px;
    padding: 6px 18px;
    display: flex;
    gap: 16px;
    font-size: 0.72rem;
    color: var(--text-muted);
    font-family: monospace;
    pointer-events: auto;
    box-shadow: 0 8px 20px rgba(0, 0, 0, 0.5);
  }
  .hud-bottom span { color: var(--text); font-weight: 700; }

  @media (max-width: 600px) {
    .hud-telemetry { width: calc(100vw - 28px); top: 10px; left: 14px; padding: 10px 12px; }
    .hud-controls { top: auto; bottom: 50px; right: 14px; }
    .hud-bottom { bottom: 10px; width: calc(100vw - 28px); justify-content: space-around; gap: 8px; border-radius: 10px; }
  }
</style>
</head>
<body>

<div id="map"></div>

<!-- Telemetry HUD Overlay -->
<div class="hud-telemetry">
  <div class="hud-header">
    <div class="hud-title">
      <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5">
        <circle cx="12" cy="12" r="10"></circle>
        <polygon points="16.24 7.76 14.12 14.12 7.76 16.24 9.88 9.88 16.24 7.76"></polygon>
      </svg>
      GTA SA GPS Radar
    </div>
    <div class="hud-status" id="status-badge">
      <div class="status-dot" id="status-dot"></div>
      <span id="sys-status">ONLINE</span>
    </div>
  </div>

  <div class="hud-stats-row">
    <div class="hud-stat-box">
      <div class="hud-label">Ground Speed</div>
      <div class="hud-val-lg" id="hud-speed">0 <span>km/h</span></div>
    </div>
    <div class="hud-stat-box">
      <div class="hud-label">Altitude & Heading</div>
      <div class="hud-val-lg" id="hud-head">0° <span>N</span></div>
    </div>
  </div>

  <div class="hud-coords-box">
    <div class="hud-label">World Coordinates (X, Y, Z)</div>
    <div class="hud-coords-val">
      <span id="hud-pos-x">X: 0.0</span>
      <span id="hud-pos-y">Y: 0.0</span>
      <span id="hud-pos-z">Z: 0.0</span>
    </div>
  </div>

  <div class="hud-fleet-box">
    <span class="hud-fleet-title">🚚 Logistics Fleet</span>
    <span class="hud-fleet-val" id="hud-fleet">15 Total · 0 in 3D</span>
  </div>

  <div class="hud-balance-box">
    <span class="hud-balance-title">💼 Balance</span>
    <span class="hud-balance-val" id="hud-balance">$250,000</span>
  </div>

  <div class="hud-bank-box" style="margin-top: 6px; padding: 7px 10px; background: rgba(16, 185, 129, 0.08); border: 1px solid rgba(16, 185, 129, 0.3); border-radius: 8px; font-size: 0.72rem; display: flex; justify-content: space-between; align-items: center;">
    <span style="color: #6ee7b7; font-weight: 800;">🏦 Fleeca Rate: <span id="hud-fleeca-rate">5.0%</span></span>
    <span style="color: #94a3b8; font-weight: 700;">Reserves: <span id="hud-fleeca-reserves" style="color: #f8fafc; font-weight: 800;">$2.50M</span></span>
  </div>

  <div class="hud-directive-box" id="hud-directive-box" style="display: none;">
    <span id="hud-dir-title">🎯 AI Intercept #0</span>
    <span id="hud-dir-dist" style="font-weight: 700;">0 m</span>
  </div>

  <div class="hud-retail-box" style="margin-top: 10px; padding: 10px; background: rgba(0, 0, 0, 0.4); border-radius: 8px; border: 1px solid rgba(255, 255, 255, 0.08);">
    <div style="font-size: 0.72rem; font-weight: 800; color: #38bdf8; margin-bottom: 6px; text-transform: uppercase;">🏪 Retail Nodes & Market</div>
    <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 6px; font-size: 0.72rem;">
      <div><b>Food:</b> <span id="store-stock-0">0</span> (<span id="store-sold-0">0</span> sold / <span id="store-rev-0">$0</span>)</div>
      <div><b>Gas:</b> <span id="store-stock-1">0</span> (<span id="store-sold-1">0</span> sold / <span id="store-rev-1">$0</span>)</div>
      <div><b>Lumber:</b> <span id="store-stock-2">0</span> (<span id="store-sold-2">0</span> sold / <span id="store-rev-2">$0</span>)</div>
      <div><b>Tech:</b> <span id="store-stock-3">0</span> (<span id="store-sold-3">0</span> sold / <span id="store-rev-3">$0</span>)</div>
    </div>
  </div>

  <div id="roadblocks-container" style="margin-top: 10px;"></div>
</div>

<!-- Floating Controls -->
<div class="hud-controls">
  <button class="ctrl-btn active" id="btn-follow" onclick="toggleFollow()">
    <span>🎯</span> <span id="txt-follow">Follow: ON</span>
  </button>
  <button class="ctrl-btn" onclick="recenterPlayer()">
    <span>⚡</span> <span>Recenter</span>
  </button>
  <button class="ctrl-btn active" id="btn-zones" onclick="toggleZones()">
    <span>🗺️</span> <span id="txt-zones">Zones: ON</span>
  </button>
  <button class="ctrl-btn active" id="btn-stores" onclick="toggleStores()">
    <span>🏪</span> <span id="txt-stores">Stores: ON</span>
  </button>
  <button class="ctrl-btn active" id="btn-master-highway" onclick="toggleMasterHighway()">
    <span>🛣️</span> <span id="txt-master-highway">Master Highway: ON</span>
  </button>
  <button class="ctrl-btn active" id="btn-local-feeders" onclick="toggleLocalFeeders()">
    <span>🚛</span> <span id="txt-local-feeders">Local Feeders: ON</span>
  </button>
  <button class="ctrl-btn" id="btn-record" onclick="toggleRecordMode()"><span>📍</span> <span id="txt-record">Plotter: OFF</span></button>
  <button class="ctrl-btn" id="btn-export" onclick="exportRoute()" style="display:none;"><span>💾</span> <span>Copy C++ Loop</span></button>
  <button class="ctrl-btn" id="btn-waypoints" onclick="toggleWaypoints()"><span>🛣️</span> <span id="txt-waypoints">Waypoints: OFF</span></button>
  <button class="ctrl-btn" id="btn-layer" onclick="cycleMapLayer()">
    <span>🗺️</span> <span id="txt-layer">Style: Satellite</span>
  </button>
  <a href="/municipal" class="ctrl-btn">
    <span>🏛️</span> <span>City Hall</span>
  </a>
  <a href="/logistics" class="ctrl-btn">
    <span>🏢</span> <span>Logistics</span>
  </a>
  <a href="/" class="ctrl-btn">
    <span>📊</span> <span>Dashboard</span>
  </a>
</div>

<!-- Bottom Status Bar -->
<div class="hud-bottom">
  <div>FLEET: <span id="bot-fleet">24</span></div>
  <div>TEL: <span id="bot-tel">0</span></div>
  <div>DIR: <span id="bot-dir">0</span></div>
  <div>PING: <span id="bot-ping">280ms</span></div>
</div>

<script>
// =============================================================================
//  1. Coordinate Transformation (Game World to Leaflet CRS.Simple)
//  Calibrated GTA SA radar boundaries (6000x6000m with zero-center at 3000, 3000)
// =============================================================================
const WORLD_BOUNDS = [[0, 0], [6000, 6000]];

function gameToMap(x, y) {
  return [y + 3000.0, x + 3000.0];
}

function mapToGame(lat, lng) {
  return [lng - 3000.0, lat - 3000.0];
}

const map = L.map('map', {
  crs: L.CRS.Simple,
  minZoom: -2,
  maxZoom: 3,
  zoomSnap: 0.25,
  zoomDelta: 0.5,
  maxBounds: [[-500, -500], [6500, 6500]],
  maxBoundsViscosity: 0.9,
  attributionControl: false
});

map.setView([3000, 3000], -1);

const SATELLITE_MAP_URLS = [
  'https://raw.githubusercontent.com/interactive-game-maps/grand_theft_auto_san_andreas/master/gtasa-satellite.jpg',
  'https://raw.githubusercontent.com/gptrk0/mtasa-map-images/master/default/map_default_4096.jpg',
  'https://raw.githubusercontent.com/pawn-lang/compiler/gh-pages/images/sa-map.jpg',
  'https://upload.wikimedia.org/wikipedia/commons/4/4b/San_andreas_map.jpg',
  'https://i.imgur.com/3N4oZ9p.jpeg'
];

const NIGHT_MAP_URLS = [
  'https://i.imgur.com/jE2bwoA.jpg',
  'https://raw.githubusercontent.com/gptrk0/mtasa-map-images/master/night/map_night_2048.jpg'
];

function createFallbackImageOverlay(urls, bounds, options, fallbackFn) {
  let idx = 0;
  let lastFailedIdx = -1;
  const opts = Object.assign({ interactive: false }, options || {});
  const overlay = L.imageOverlay(urls[0], bounds, opts);

  function tryNext() {
    if (idx === lastFailedIdx) return;
    lastFailedIdx = idx;
    idx++;
    if (idx < urls.length) {
      overlay.setUrl(urls[idx]);
    } else if (fallbackFn) {
      const fallbackUrl = fallbackFn();
      if (fallbackUrl) overlay.setUrl(fallbackUrl);
    }
  }

  overlay.on('error', tryNext);
  overlay.on('add', function() {
    const el = overlay.getElement();
    if (el) el.onerror = tryNext;
  });

  return overlay;
}

const vectorCanvasUrl = createVectorRadarCanvas();
const layerVector = L.imageOverlay(vectorCanvasUrl, WORLD_BOUNDS, { pane: 'tilePane', opacity: 0.95 });
const layerSat = createFallbackImageOverlay(SATELLITE_MAP_URLS, WORLD_BOUNDS, { pane: 'tilePane', opacity: 1.0 }, createVectorRadarCanvas);
const layerNight = createFallbackImageOverlay(NIGHT_MAP_URLS, WORLD_BOUNDS, { pane: 'tilePane', opacity: 0.95 }, createVectorRadarCanvas);

layerSat.addTo(map);

// =============================================================================
//  1b. Highway Backbone Polyline & Waypoint Markers
// =============================================================================
const highwayRaw = [
    [2312.2,-2252.0], [2347.9,-2222.3], [2419.2,-2172.3], [2697.5,-2169.9], [2766.5,-2146.1], [2828.3,-2089.1], [2830.7,-1996.3], [2833.1,-1897.6],
    [2840.2,-1838.1], [2850.9,-1733.5], [2883.0,-1599.1], [2909.2,-1525.4], [2916.3,-1411.2], [2909.2,-1337.5], [2886.6,-1279.2], [2883.0,-1203.1],
    [2884.4,-1147.2], [2883.2,-1077.1], [2882.1,-1021.2], [2886.8,-932.0], [2887.8,-761.9], [2890.2,-689.4], [2881.8,-556.2], [2840.2,-500.3],
    [2776.0,-407.5], [2716.4,-354.5], [2698.8,-289.8], [2743.3,-167.9], [2763.5,-109.0], [2770.2,4.5], [2777.0,51.6], [2772.8,90.3],
    [2777.0,155.9], [2770.2,224.8], [2721.5,293.8], [2679.4,316.5], [2613.0,319.9], [2548.2,307.2], [2493.6,310.6], [2441.4,320.7],
    [2390.1,327.4], [2329.6,324.9], [2255.6,324.9], [2169.8,323.2], [2083.2,324.1], [2000.8,315.7], [1908.3,302.2], [1833.5,282.0],
    [1771.2,277.0], [1732.6,290.4], [1710.7,310.6], [1700.6,345.9], [1698.9,385.4], [1715.7,419.9], [1736.8,482.2], [1757.8,543.5],
    [1780.5,606.6], [1784.7,655.4], [1791.4,697.4], [1801.5,765.5], [1803.2,800.9], [1806.8,893.5], [1809.7,950.5], [1808.6,1014.2],
    [1804.4,1040.9], [1799.6,1109.9], [1810.3,1195.5], [1803.2,1309.7], [1810.3,1389.4], [1799.6,1501.1], [1803.2,1577.3], [1820.0,1593.0],
    [1835.5,1630.5], [1858.0,1649.5], [1871.5,1678.0], [1872.2,1714.9], [1846.1,1720.0], [1804.9,1714.1], [1746.9,1710.7], [1686.3,1711.5],
    [1635.9,1709.0], [1580.4,1707.3], [1566.9,1723.3], [1567.7,1769.6], [1569.4,1825.9], [1565.2,1869.6], [1521.4,1872.2], [1490.4,1880.5],
    [1496.4,1934.0], [1488.1,1953.0], [1495.2,1969.7], [1572.5,1976.8], [1567.7,2025.6], [1566.6,2051.8], [1464.3,2042.2], [1404.8,2052.9],
    [1320.4,2042.2], [1269.3,2054.1], [1196.7,2045.8], [1139.6,2044.6], [1067.1,2052.9], [1004.1,2047.0], [1010.0,2003.0], [1011.2,1951.9],
    [1007.6,1898.3], [1008.8,1841.3], [1061.1,1812.7], [1112.3,1813.9], [1140.8,1812.7], [1287.1,1808.0], [1296.6,1825.8], [1265.7,1892.4],
    [1228.8,1953.0], [1228.8,2089.8], [1227.6,2154.0], [1251.4,2202.8], [1290.7,2245.6], [1347.7,2325.3], [1348.9,2370.5], [1308.5,2406.1],
    [1258.6,2424.0], [1193.1,2439.4], [1126.5,2473.9], [1076.6,2504.8], [1013.6,2550.0], [941.0,2594.0], [856.6,2633.3], [760.3,2655.9],
    [622.3,2657.1], [454.6,2657.1], [416.6,2696.3], [338.1,2714.1], [228.7,2747.4], [165.7,2753.4], [118.1,2720.1], [97.9,2699.9],
    [51.5,2657.1], [-62.7,2640.4], [-150.7,2634.5], [-245.8,2634.5], [-336.2,2635.7], [-383.7,2690.4], [-452.7,2721.3], [-521.7,2717.7],
    [-574.0,2740.3], [-619.2,2759.3], [-698.9,2739.1], [-840.4,2726.0], [-991.4,2717.7], [-1138.9,2699.9], [-1237.6,2679.7], [-1323.2,2647.5],
    [-1347.6,2637.4], [-1379.2,2686.8], [-1435.1,2721.3], [-1516.0,2729.6], [-1618.3,2733.2], [-1694.4,2724.8], [-1752.6,2727.2], [-1793.1,2696.3],
    [-1809.3,2679.8], [-1822.8,2686.1], [-1853.5,2677.3], [-1860.6,2658.8], [-1878.3,2638.2], [-1901.0,2626.4], [-1931.3,2612.6], [-1957.8,2607.5],
    [-2009.0,2626.0], [-2053.0,2637.0], [-2100.0,2659.0], [-2234.0,2673.0], [-2287.0,2677.0], [-2390.0,2671.0], [-2515.0,2669.0], [-2607.0,2668.0],
    [-2680.0,2660.0], [-2741.0,2611.0], [-2761.0,2565.0], [-2772.0,2513.0], [-2778.0,2461.0], [-2780.0,2402.0], [-2771.0,2354.0], [-2751.0,2297.0],
    [-2724.0,2245.0], [-2700.0,2208.0], [-2687.0,2163.0], [-2688.0,2113.0], [-2683.0,2040.0], [-2690.0,2026.0], [-2688.0,1985.0], [-2690.0,1936.0],
    [-2693.0,1889.0], [-2689.0,1799.0], [-2689.0,1700.0], [-2687.0,1599.0], [-2686.0,1534.0], [-2684.0,1516.0], [-2684.0,1444.0], [-2692.0,1362.0],
    [-2687.0,1285.0], [-2671.0,1196.0], [-2629.0,1148.0], [-2607.0,1128.0], [-2537.0,1110.0], [-2474.0,1101.0], [-2449.4,1093.3], [-2379.8,1078.4],
    [-2316.2,1067.1], [-2286.5,1067.1], [-2197.9,1065.9], [-2095.6,1065.3], [-2042.7,1067.7], [-1993.0,1072.0], [-1899.0,1059.0], [-1887.0,1043.0],
    [-1894.0,990.0], [-1896.4,943.4], [-1790.6,924.4], [-1691.9,922.0], [-1607.4,920.8], [-1536.1,922.0], [-1524.2,917.2], [-1527.8,873.2],
    [-1536.1,844.7], [-1545.6,791.2], [-1555.1,751.9], [-1553.9,700.8], [-1561.1,599.7], [-1563.4,508.2], [-1589.6,459.4], [-1626.5,424.9],
    [-1677.6,370.2], [-1722.8,331.0], [-1764.4,290.5], [-1801.3,251.3], [-1807.8,214.5], [-1809.2,179.2], [-1778.8,184.1], [-1762.6,177.0],
    [-1765.4,141.7], [-1759.7,110.6], [-1758.3,54.7], [-1761.9,-16.7], [-1761.1,-100.2], [-1761.9,-117.1], [-1788.2,-116.2], [-1797.2,-136.9],
    [-1795.8,-194.9], [-1798.6,-233.8], [-1802.2,-326.4], [-1813.2,-390.9], [-1826.3,-468.2], [-1825.1,-528.8], [-1820.3,-558.6], [-1814.4,-607.3],
    [-1820.3,-666.8], [-1808.4,-725.0], [-1807.2,-795.2], [-1807.2,-849.9], [-1809.6,-929.6], [-1795.3,-1040.2], [-1766.8,-1123.4], [-1737.1,-1184.1],
    [-1690.7,-1266.1], [-1663.3,-1316.1], [-1620.5,-1382.7], [-1567.0,-1438.6], [-1530.1,-1442.1], [-1526.6,-1391.0], [-1537.3,-1291.1], [-1617.0,-1194.8],
    [-1671.7,-1154.4], [-1709.7,-1106.8], [-1737.1,-1027.1], [-1746.6,-958.1], [-1752.5,-904.6], [-1750.1,-861.8], [-1695.4,-796.4], [-1658.6,-791.6],
    [-1628.8,-826.1], [-1626.5,-879.6], [-1633.6,-949.8], [-1638.4,-1014.0], [-1608.6,-1123.4], [-1574.1,-1166.2], [-1534.9,-1209.1], [-1480.2,-1268.5],
    [-1445.7,-1328.0], [-1421.9,-1408.8], [-1404.1,-1416.0], [-1358.9,-1396.9], [-1266.1,-1368.4], [-1217.4,-1352.9], [-1171.0,-1345.8], [-1129.4,-1342.2],
    [-1077.1,-1351.8], [-1005.7,-1382.7], [-964.1,-1398.1], [-924.8,-1393.4], [-911.8,-1363.7], [-898.7,-1319.7], [-890.3,-1186.5], [-877.3,-1084.2],
    [-817.8,-1020.0], [-759.5,-1003.3], [-679.9,-999.8], [-620.6,-979.8], [-583.1,-953.6], [-538.6,-925.3], [-457.2,-844.0], [-362.5,-837.0],
    [-314.4,-865.2], [-246.5,-892.1], [-197.7,-934.5], [-156.0,-957.9], [-105.8,-998.9], [-88.1,-1034.2], [-90.0,-1092.5], [-99.5,-1132.9],
    [-119.7,-1180.5], [-140.0,-1235.2], [-149.5,-1297.1], [-156.6,-1356.5], [-144.7,-1421.9], [-100.7,-1498.0], [-17.5,-1518.2], [65.8,-1526.6],
    [137.1,-1555.1], [182.3,-1608.6], [256.0,-1687.1], [322.0,-1708.0], [427.0,-1711.0], [591.0,-1726.0], [663.0,-1748.0], [861.0,-1784.0],
    [972.0,-1780.0], [1060.0,-1847.0], [1060.0,-1943.0], [1051.0,-2009.0], [1046.0,-2057.0], [1045.0,-2266.0], [1138.0,-2401.0], [1197.0,-2429.0],
    [1282.0,-2460.0], [1343.0,-2467.0], [1341.6,-2507.9], [1340.2,-2594.1], [1386.9,-2666.2], [1504.3,-2684.6], [1593.4,-2681.8], [1750.3,-2687.5],
    [1884.7,-2677.6], [2023.3,-2690.3], [2125.1,-2654.9], [2167.5,-2616.7], [2171.8,-2537.6], [2170.4,-2481.0], [2173.2,-2413.1], [2211.4,-2355.1],
    [2309.0,-2256.1]
  ];

const highwayCoords = highwayRaw.map(p => gameToMap(p[0], p[1]));
if (highwayCoords.length > 0) {
  highwayCoords.push(highwayCoords[0]); // Ensure closed freeway circuit
}

// Persistent Highway Polyline (active on map load)
const highwayPolyline = L.polyline(highwayCoords, {
  color: '#06b6d4',
  weight: 4,
  opacity: 0.95,
  dashArray: '6, 6'
}).addTo(map);

const waypointLayer = L.layerGroup();
let showWaypoints = false;

highwayRaw.forEach((pt, idx) => {
  const isFacility = (idx === 0 || idx === 85 || idx === 135 || idx === 240);
  if (idx % 5 === 0 || isFacility) {
    const pos = gameToMap(pt[0], pt[1]);
    let label = `#${idx}`;
    if (idx === 0) label = `#0 (Ocean Docks · Loading Hub)`;
    else if (idx === 85) label = `#85 (Weigh Station · East LV)`;
    else if (idx === 135) label = `#135 (Bone County · Rest Stop)`;
    else if (idx === 240) label = `#240 (SF Depot · Unloading Terminal)`;

    const marker = L.circleMarker(pos, {
      radius: isFacility ? 7 : 3.5,
      color: isFacility ? '#ffffff' : '#06b6d4',
      weight: isFacility ? 2 : 1,
      fillColor: isFacility ? (idx === 0 ? '#38bdf8' : (idx === 135 ? '#f59e0b' : (idx === 85 ? '#eab308' : '#a855f7'))) : '#06b6d4',
      fillOpacity: isFacility ? 1.0 : 0.6
    });

    marker.bindTooltip(label, { direction: 'top', offset: [0, -6] });
    marker.bindPopup(`
      <div style="font-family:Inter,sans-serif;min-width:180px;">
        <div style="font-weight:700;font-size:13px;margin-bottom:4px;color:#38bdf8;">Waypoint #${idx}</div>
        <div style="color:#94a3b8;font-size:11px;margin-bottom:6px;">Game Coords: X ${pt[0]}, Y ${pt[1]}</div>
        <div style="background:rgba(255,255,255,0.06);border-radius:4px;padding:4px 6px;font-size:11px;font-weight:600;">
          ${isFacility ? (idx === 0 ? '🏭 Ocean Docks (Freight Loading)' : (idx === 85 ? '⚖️ Weigh Station (Safety & Axle Inspection)' : (idx === 135 ? '⛽ Bone County (Rest / Refuel Area)' : '📦 SF Depot (Freight Unloading Terminal)'))) : '🛣️ Master Highway Backbone Node'}
        </div>
      </div>
    `);

    waypointLayer.addLayer(marker);
  }
});

const companyRoutesGroup = L.layerGroup().addTo(map);

// Hardcoded 63 Roadblock coordinates table
const k_roadblocksData = [
    { id: 0,  x: 1369.0, y: -1400.0, z: 12.0 }, { id: 1,  x: 1211.0, y: -1573.0, z: 12.0 },
    { id: 2,  x: 1198.0, y: -1711.0, z: 12.0 }, { id: 3,  x: 1047.0, y: -2087.0, z: 12.0 },
    { id: 4,  x: 1031.0, y: -2087.0, z: 12.0 }, { id: 5,  x: 1032.0, y: -2222.0, z: 12.0 },
    { id: 6,  x: 1032.0, y: -2172.0, z: 12.0 }, { id: 7,  x: 1023.0, y: -2120.0, z: 12.0 },
    { id: 8,  x: 1330.0, y: -2448.0, z: 7.0  }, { id: 9,  x: 1329.0, y: -2464.0, z: 6.0  },
    { id: 10, x: 1367.0, y: -2447.0, z: 7.0  }, { id: 11, x: 1369.0, y: -2465.0, z: 6.0  },
    { id: 12, x: 1345.0, y: -2407.0, z: 12.0 }, { id: 13, x: 1332.0, y: -2414.0, z: 12.0 },
    { id: 14, x: 1481.0, y: -2686.0, z: 10.0 }, { id: 15, x: 1467.0, y: -2669.0, z: 11.0 },
    { id: 16, x: 1849.0, y: -1271.0, z: 12.0 }, { id: 17, x: 1820.0, y: -1260.0, z: 12.0 },
    { id: 18, x: 1746.0, y: -1161.0, z: 23.0 }, { id: 19, x: 1701.0, y: -1301.0, z: 12.0 },
    { id: 20, x: 1714.0, y: -1317.0, z: 12.0 }, { id: 21, x: 1715.0, y: -1278.0, z: 12.0 },
    { id: 22, x: 1453.0, y: -1463.0, z: 12.0 }, { id: 23, x: 1438.0, y: -1525.0, z: 12.0 },
    { id: 24, x: 1196.0, y: -1418.0, z: 12.0 }, { id: 25, x: 1164.0, y: -1281.0, z: 12.0 },
    { id: 26, x: 1215.0, y: -1258.0, z: 13.0 }, { id: 27, x: 1200.0, y: -1334.0, z: 12.0 },
    { id: 28, x: 1187.0, y: -1331.0, z: 13.0 }, { id: 29, x: 1214.0, y: -1333.0, z: 12.0 },
    { id: 30, x: 1257.0, y: -1313.0, z: 12.0 }, { id: 31, x: 1092.0, y: -1399.0, z: 12.0 },
    { id: 32, x: 1055.0, y: -1393.0, z: 12.0 }, { id: 33, x: 1061.0, y: -1438.0, z: 12.0 },
    { id: 34, x: 919.0,  y: -1409.0, z: 12.0 }, { id: 35, x: 917.0,  y: -1385.0, z: 12.0 },
    { id: 36, x: 798.0,  y: -1414.0, z: 12.0 }, { id: 37, x: 761.0,  y: -1400.0, z: 12.0 },
    { id: 38, x: 797.0,  y: -1370.0, z: 12.0 }, { id: 39, x: 663.0,  y: -1315.0, z: 12.0 },
    { id: 40, x: 668.0,  y: -1234.0, z: 14.0 }, { id: 41, x: 792.0,  y: -1150.0, z: 23.0 },
    { id: 42, x: 962.0,  y: -1127.0, z: 23.0 }, { id: 43, x: 967.0,  y: -1038.0, z: 29.0 },
    { id: 44, x: 963.0,  y: -978.0,  z: 38.0 }, { id: 45, x: 794.0,  y: -1041.0, z: 24.0 },
    { id: 46, x: 1155.0, y: -792.0,  z: 55.0 }, { id: 47, x: 1369.0, y: -925.0,  z: 33.0 },
    { id: 48, x: 1379.0, y: -927.0,  z: 33.0 }, { id: 49, x: 1490.0, y: -938.0,  z: 36.0 },
    { id: 50, x: 1479.0, y: -975.0,  z: 36.0 }, { id: 51, x: 1659.0, y: -811.0,  z: 56.0 },
    { id: 52, x: 1679.0, y: -807.0,  z: 55.0 }, { id: 53, x: 1703.0, y: -784.0,  z: 53.0 },
    { id: 54, x: 2451.0, y: -1249.0, z: 23.0 }, { id: 55, x: 2431.0, y: -1636.0, z: 26.0 },
    { id: 56, x: 2029.0, y: -1752.0, z: 12.0 }, { id: 57, x: 1960.0, y: -1994.0, z: 12.0 },
    { id: 58, x: 2264.0, y: -2233.0, z: 12.0 }, { id: 59, x: 2756.0, y: -2150.0, z: 10.0 },
    { id: 60, x: 2762.0, y: -2162.0, z: 10.0 }, { id: 61, x: 2849.0, y: -1656.0, z: 10.0 },
    { id: 62, x: 2858.0, y: -1137.0, z: 10.0 }
];

let roadblockLayer = L.layerGroup().addTo(map);
let staticRoadblockElements = [];

// Initialize markers once
function initRoadblocksOnce() {
    if (staticRoadblockElements.length > 0) return;
    
    k_roadblocksData.forEach(pt => {
        const pos = typeof gameToMap === 'function' ? gameToMap(pt.x, pt.y) : [pt.y, pt.x];
        const zone = L.circle(pos, {
            radius: 35.0,
            color: '#b91c1c',
            weight: 1.5,
            fillColor: '#ef4444',
            fillOpacity: 0.25,
            interactive: false
        });

        const marker = L.circleMarker(pos, {
            radius: 5,
            color: '#7f1d1d',
            weight: 1.5,
            fillColor: '#dc2626',
            fillOpacity: 1.0
        }).bindTooltip(`<b>ROADBLOCK #${pt.id}</b><br>CORDON ACTIVE<br>Pos: ${pt.x}, ${pt.y}`);

        staticRoadblockElements.push({ id: pt.id, zone: zone, marker: marker });
    });
}

// Statically toggle visibility without recreating DOM elements
function renderAllRoadblocks(roadblocks, macro) {
    initRoadblocksOnce();
    const isCrisis = macro && (macro.unrest >= 60.0 || macro.curfew);

    if (!isCrisis) {
        if (map.hasLayer(roadblockLayer)) {
            map.removeLayer(roadblockLayer);
        }
        return;
    }

    if (!map.hasLayer(roadblockLayer)) {
        map.addLayer(roadblockLayer);
    }

    if (roadblockLayer.getLayers().length === 0) {
        staticRoadblockElements.forEach(item => {
            roadblockLayer.addLayer(item.zone);
            roadblockLayer.addLayer(item.marker);
        });
    }
}

function updateRoadblocksAlertUI(d) {
    const rbCont = document.getElementById('roadblocks-container');
    if (rbCont) {
        if (d && d.roadblocks && d.roadblocks.length > 0) {
            rbCont.innerHTML = d.roadblocks.map(r => `
                <div class="alert-box alert-danger">
                    <div class="alert-title">CRISIS ROADBLOCK: ${r.location || 'UNKNOWN'}</div>
                    <div class="alert-desc">Rioters: ${r.riotersAlive} | Status: ${r.provoked ? 'HOSTILE ENGAGED' : 'BARRICADED'}</div>
                </div>
            `).join('');
        } else {
            rbCont.innerHTML = '<div class="alert-box alert-success" style="opacity: 0.7;">NO ACTIVE ROADBLOCKS DETECTED - CORRIDORS CLEAR</div>';
        }
    }
}

let showMasterHighway = true;
function toggleMasterHighway() {
  showMasterHighway = !showMasterHighway;
  const btn = document.getElementById('btn-master-highway');
  const txt = document.getElementById('txt-master-highway');
  if (btn) btn.classList.toggle('active', showMasterHighway);
  if (txt) txt.textContent = showMasterHighway ? 'Master Highway: ON' : 'Master Highway: OFF';
  if (showMasterHighway) {
    if (!map.hasLayer(highwayPolyline)) highwayPolyline.addTo(map);
  } else {
    if (map.hasLayer(highwayPolyline)) map.removeLayer(highwayPolyline);
  }
}

let showLocalFeeders = true;
function toggleLocalFeeders() {
  showLocalFeeders = !showLocalFeeders;
  const btn = document.getElementById('btn-local-feeders');
  const txt = document.getElementById('txt-local-feeders');
  if (btn) btn.classList.toggle('active', showLocalFeeders);
  if (txt) txt.textContent = showLocalFeeders ? 'Local Feeders: ON' : 'Local Feeders: OFF';
  if (showLocalFeeders) {
    if (!map.hasLayer(companyRoutesGroup)) companyRoutesGroup.addTo(map);
  } else {
    if (map.hasLayer(companyRoutesGroup)) map.removeLayer(companyRoutesGroup);
  }
}


const STRATEGIC_HUBS = [
  {
    id: 'ocean_docks',
    name: 'Ocean Docks Mega-Terminal',
    subtitle: 'Central Transfer Hub & Weigh Station',
    icon: '🏭',
    x: 2312.2,
    y: -2252.0,
    getDetails: (data) => {
      const food = data?.port_supplies?.food ?? 120;
      const fuel = data?.port_supplies?.fuel ?? 120;
      const q = data?.trucks?.filter(t => t.current_node === 0 || Math.hypot((t.pos ? t.pos.x : (t.x || 0)) - 2312.2, (t.pos ? t.pos.y : (t.y || 0)) - (-2252.0)) < 350.0).length ?? 0;
      return `
        <div class="hub-metric-row"><span class="lbl">Port Food Stock:</span><span class="val food">${food} / 500 u</span></div>
        <div class="hub-metric-row"><span class="lbl">Port Fuel Stock:</span><span class="val fuel">${fuel} / 500 u</span></div>
        <div class="hub-metric-row"><span class="lbl">Loading Bay Queue:</span><span class="val queue">${q} Trucks Active</span></div>
        <div class="hub-role">Central state interchange: Receives local feeder shuttles from Flint Farm & Bone Wells; loads 24 Master Interstate Haulers for San Fierro delivery.</div>
      `;
    }
  },
  {
    id: 'flint_farm',
    name: 'Flint County Farm Depot',
    subtitle: 'Agro Loading & Harvest Hub',
    icon: '🌾',
    x: -74.8,
    y: -1142.2,
    getDetails: (data) => {
      const shuttles = data?.trucks?.filter(t => t.id >= 24 && t.id <= 26) ?? [];
      const totalCargo = shuttles.reduce((acc, t) => acc + (t.cargo_units || 0), 0);
      return `
        <div class="hub-metric-row"><span class="lbl">Commodity:</span><span class="val food">Fresh Agro Produce</span></div>
        <div class="hub-metric-row"><span class="lbl">Dedicated Feeders:</span><span class="val">3 Shuttles (#24..#26)</span></div>
        <div class="hub-metric-row"><span class="lbl">In-Transit Freight:</span><span class="val food">${totalCargo} Units Agro</span></div>
        <div class="hub-role">Local agricultural production supplying Ocean Docks export terminal via custom snapped supply line.</div>
      `;
    }
  },
  {
    id: 'bone_wells',
    name: 'Bone County Oil Field',
    subtitle: 'Fuel Extraction & Refining Well',
    icon: '⛽',
    x: 342.1,
    y: 1422.3,
    getDetails: (data) => {
      const shuttles = data?.trucks?.filter(t => t.id >= 27 && t.id <= 29) ?? [];
      const totalCargo = shuttles.reduce((acc, t) => acc + (t.cargo_units || 0), 0);
      return `
        <div class="hub-metric-row"><span class="lbl">Commodity:</span><span class="val fuel">Industrial Fuel / Octane</span></div>
        <div class="hub-metric-row"><span class="lbl">Dedicated Feeders:</span><span class="val">3 Tankers (#27..#29)</span></div>
        <div class="hub-metric-row"><span class="lbl">In-Transit Freight:</span><span class="val fuel">${totalCargo} Units Fuel</span></div>
        <div class="hub-role">Continuous petrochemical extraction shuttles directly restocking Ocean Docks fuel atomics.</div>
      `;
    }
  },
  {
    id: 'weigh_station',
    name: 'Interstate Weigh Station',
    subtitle: 'Safety & Axle Inspection Facility',
    icon: '⚖️',
    x: 1565.2,
    y: 1869.6,
    getDetails: (data) => {
      const nearby = data?.trucks?.filter(t => Math.hypot((t.pos ? t.pos.x : (t.x || 0)) - 1565.2, (t.pos ? t.pos.y : (t.y || 0)) - 1869.6) < 400.0).length ?? 0;
      return `
        <div class="hub-metric-row"><span class="lbl">Corridor Node:</span><span class="val">Waypoint #85 (East LV Freeway)</span></div>
        <div class="hub-metric-row"><span class="lbl">Inspection Scales:</span><span class="val pass">OPERATIONAL (100% Flow)</span></div>
        <div class="hub-metric-row"><span class="lbl">Haulers In-Transit:</span><span class="val queue">${nearby} Trucks in Sector</span></div>
        <div class="hub-role">Automated dynamic weigh-in-motion scales ensuring compliance across LV-Bone County freeway corridor.</div>
      `;
    }
  },
  {
    id: 'sf_terminal',
    name: 'San Fierro Freight Terminal',
    subtitle: 'Interstate Drop-off & Metro Hub',
    icon: '📦',
    x: -1765.4,
    y: 141.7,
    getDetails: (data) => {
      const food = data?.sf_supplies?.food ?? 80;
      const fuel = data?.sf_supplies?.fuel ?? 80;
      const nearby = data?.trucks?.filter(t => Math.hypot((t.pos ? t.pos.x : (t.x || 0)) - (-1765.4), (t.pos ? t.pos.y : (t.y || 0)) - 141.7) < 400.0).length ?? 0;
      return `
        <div class="hub-metric-row"><span class="lbl">SF Food Depot:</span><span class="val food">${food} / 500 u</span></div>
        <div class="hub-metric-row"><span class="lbl">SF Fuel Depot:</span><span class="val fuel">${fuel} / 500 u</span></div>
        <div class="hub-metric-row"><span class="lbl">Unloading Bay Queue:</span><span class="val queue">${nearby} Trucks in Area</span></div>
        <div class="hub-role">Final drop-off hub for Interstate haulers; feeds San Fierro metropolitan commercial supply chains.</div>
      `;
    }
  }
];

const strategicHubLayer = L.layerGroup().addTo(map);
const g_strategicMarkers = [];
let g_latestMapTelemetry = null;

function initStrategicHubs(targetMap, targetGroup) {
  STRATEGIC_HUBS.forEach(hub => {
    const pos = gameToMap(hub.x, hub.y);
    const icon = L.divIcon({
      className: 'strategic-hub-icon-wrap',
      html: `
        <div class="strategic-hub-icon" title="${hub.name}">
          <div class="pulse-ring"></div>
          <div class="hub-dot"></div>
        </div>
      `,
      iconSize: [18, 18],
      iconAnchor: [9, 9],
      popupAnchor: [0, -12]
    });

    const marker = L.marker(pos, { icon: icon, zIndexOffset: 2000 });

    function renderPopup() {
      return `
        <div class="tactical-popup">
          <div class="hub-badge">KEY LOGISTICS HUB</div>
          <div class="hub-title"><span>${hub.icon}</span> <span>${hub.name}</span></div>
          <div class="hub-sub">${hub.subtitle}</div>
          <div class="hub-content">${hub.getDetails(g_latestMapTelemetry)}</div>
          <div style="margin-top:8px;font-size:10px;color:#94a3b8;">Coords: X: ${hub.x.toFixed(1)}, Y: ${hub.y.toFixed(1)}</div>
        </div>
      `;
    }

    marker.bindPopup(renderPopup, {
      className: 'tactical-popup-wrap',
      maxWidth: 320,
      autoPan: true
    });

    marker.on('click', () => {
      marker.setPopupContent(renderPopup());
      marker.openPopup();
    });

    targetGroup.addLayer(marker);
    g_strategicMarkers.push({ marker, hub, renderPopup });
  });
}
initStrategicHubs(map, strategicHubLayer);

async function loadMapCompanyRoutes() {
  try {
    const res = await fetch('/api/routes/load');
    if (res.ok) {
      const data = await res.json();
      if (data && data.routes && Array.isArray(data.routes)) {
        companyRoutesGroup.clearLayers();
        const compNames = ["Ocean Docks Logistics", "Octane Springs Energy", "San Andreas Agro Food"];
        data.routes.forEach(r => {
          if (r && r.nodes && r.nodes.length >= 2) {
            const latlngs = r.nodes.map(n => gameToMap(n.x, n.y));
            // Local Feeders: ON renders active company corridors at full opacity (1.0)
            L.polyline(latlngs, {
              color: r.color || '#38bdf8',
              weight: 3.5,
              opacity: 1.0,
              dashArray: '8, 4'
            }).addTo(companyRoutesGroup);

            const compName = compNames[r.companyId] || `Company #${r.companyId}`;
            const startPt = gameToMap(r.nodes[0].x, r.nodes[0].y);
            const endPt = gameToMap(r.nodes[r.nodes.length - 1].x, r.nodes[r.nodes.length - 1].y);

            // Origin Base (Node 0)
            L.circleMarker(startPt, {
              radius: 8,
              fillColor: '#22c55e',
              color: '#ffffff',
              weight: 2.5,
              fillOpacity: 1.0
            }).bindTooltip(`🏭 ORIGIN BASE: ${compName} (Loading Zone)`, { direction: 'top', offset: [0, -6] }).addTo(companyRoutesGroup);

            // Cargo Unloading Terminal (Node N-1)
            L.circleMarker(endPt, {
              radius: 8,
              fillColor: '#f59e0b',
              color: '#ffffff',
              weight: 2.5,
              fillOpacity: 1.0
            }).bindTooltip(`📦 CARGO UNLOADING TERMINAL (Depot Drop-off)`, { direction: 'top', offset: [0, -6] }).addTo(companyRoutesGroup);
          }
        });
      }
    }
  } catch (err) {}
}
loadMapCompanyRoutes();

function toggleWaypoints() {
  showWaypoints = !showWaypoints;
  const btn = document.getElementById('btn-waypoints');
  const txt = document.getElementById('txt-waypoints');
  if (btn) btn.classList.toggle('active', showWaypoints);
  if (txt) txt.textContent = showWaypoints ? 'Waypoints: ON' : 'Waypoints: OFF';
  if (showWaypoints) {
    waypointLayer.addTo(map);
  } else {
    map.removeLayer(waypointLayer);
  }
}

// =============================================================================
//  1c. Interactive Route Plotter
// =============================================================================
let recordMode = false;
const recordedPoints = [];
const recordPolyline = L.polyline([], { color: '#22c55e', weight: 4 }).addTo(map);

function toggleRecordMode() {
  recordMode = !recordMode;
  document.getElementById('btn-record').classList.toggle('active', recordMode);
  document.getElementById('txt-record').textContent = recordMode ? 'Plotter: ON (Click roads)' : 'Plotter: OFF';
  document.getElementById('btn-export').style.display = recordMode ? 'flex' : 'none';
  if (recordMode) {
    if (!showWaypoints) toggleWaypoints();
  } else {
    if (showWaypoints) toggleWaypoints();
  }
}

map.on('click', function(e) {
  if (!recordMode) return;
  const gameX = e.latlng.lng - 3000;
  const gameY = e.latlng.lat - 3000;
  recordedPoints.push({ x: Number(gameX.toFixed(1)), y: Number(gameY.toFixed(1)), z: 15.0 });
  recordPolyline.addLatLng(e.latlng);
  L.circleMarker(e.latlng, { radius: 4, color: '#22c55e' }).addTo(map);
});

function exportRoute() {
  let code = "static constexpr HighwayWaypoint s_highwayLoop[] = {\n";
  recordedPoints.forEach(p => {
    code += `    { ${p.x.toFixed(1)}f, ${p.y.toFixed(1)}f, 15.0f },\n`;
  });
  code += "};\n";
  navigator.clipboard.writeText(code);
  alert(`Copied ${recordedPoints.length} exact road waypoints to clipboard!`);
}

// =============================================================================
//  2. Procedural Vector Radar Canvas Grid (Guaranteed Offline Fallback)
// =============================================================================
function createVectorRadarCanvas() {
  const c = document.createElement('canvas');
  c.width = 2048;
  c.height = 2048;
  const ctx = c.getContext('2d');

  ctx.fillStyle = '#060a14';
  ctx.fillRect(0, 0, 2048, 2048);

  // Tactical Grid Lines
  ctx.lineWidth = 1;
  ctx.strokeStyle = 'rgba(30, 41, 59, 0.7)';
  const step = 2048 / 12;
  for (let i = 0; i <= 2048; i += step) {
    ctx.beginPath();
    ctx.moveTo(i, 0); ctx.lineTo(i, 2048);
    ctx.moveTo(0, i); ctx.lineTo(2048, i);
    ctx.stroke();
  }

  // Major Coordinate Axis Crosshair
  ctx.lineWidth = 2;
  ctx.strokeStyle = 'rgba(56, 189, 248, 0.35)';
  ctx.beginPath();
  ctx.moveTo(1024, 0); ctx.lineTo(1024, 2048);
  ctx.moveTo(0, 1024); ctx.lineTo(2048, 1024);
  ctx.stroke();

  // Radar Concentric Circles
  ctx.strokeStyle = 'rgba(56, 189, 248, 0.2)';
  const radii = [1024 * (1000/6000) * 2, 1024 * (2000/6000) * 2, 1024 * (3000/6000) * 2];
  radii.forEach(r => {
    ctx.beginPath();
    ctx.arc(1024, 1024, r, 0, Math.PI * 2);
    ctx.stroke();
  });

  // Accurate Highway Waypoint Loop Circuit Visualization (15 Nodes)
  const hwWaypoints = [
    [2680, -1450], [2250, -800], [1500, -600], [1280, 250], [1500, 900],
    [2400, 1300], [1800, 2700], [100, 2400], [-2200, 2200], [-2670, 1500],
    [-2100, 500], [-1000, -700], [-200, -1500], [900, -1700], [2000, -1900]
  ];
  ctx.lineWidth = 2;
  ctx.strokeStyle = 'rgba(56, 189, 248, 0.3)';
  ctx.setLineDash([8, 8]);
  ctx.beginPath();
  hwWaypoints.forEach((wp, idx) => {
    const cx = ((wp[0] + 3000) / 6000) * 2048;
    const cy = 2048 - (((wp[1] + 3000) / 6000) * 2048);
    if (idx === 0) ctx.moveTo(cx, cy); else ctx.lineTo(cx, cy);
  });
  ctx.closePath();
  ctx.stroke();
  ctx.setLineDash([]);

  // Major San Andreas Districts
  const districts = [
    { name: "LOS SANTOS", x: 1800, y: -1600 },
    { name: "SAN FIERRO", x: -2000, y: 200 },
    { name: "LAS VENTURAS", x: 1800, y: 1800 },
    { name: "MOUNT CHILIAD", x: -2300, y: -1600 },
    { name: "RED COUNTY", x: 800, y: -400 },
    { name: "FLINT COUNTY", x: -800, y: -1200 },
    { name: "BONE COUNTY", x: 200, y: 1500 },
    { name: "TIERRA ROBADA", x: -1500, y: 1900 }
  ];

  ctx.font = 'bold 24px -apple-system, sans-serif';
  ctx.textAlign = 'center';
  ctx.textBaseline = 'middle';
  ctx.fillStyle = 'rgba(56, 189, 248, 0.45)';

  districts.forEach(d => {
    const cx = ((d.x + 3000) / 6000) * 2048;
    const cy = 2048 - (((d.y + 3000) / 6000) * 2048);
    ctx.fillText(d.name, cx, cy);
  });

  return c.toDataURL('image/png');
}

let currentLayerMode = 0;
function cycleMapLayer() {
  currentLayerMode = (currentLayerMode + 1) % 3;
  map.removeLayer(layerSat);
  map.removeLayer(layerNight);
  map.removeLayer(layerVector);

  const txt = document.getElementById('txt-layer');
  if (currentLayerMode === 0) {
    layerSat.addTo(map);
    txt.textContent = 'Style: Satellite';
  } else if (currentLayerMode === 1) {
    layerNight.addTo(map);
    txt.textContent = 'Style: Night';
  } else {
    layerVector.addTo(map);
    txt.textContent = 'Style: Vector';
  }
}

// =============================================================================
//  3. Dynamic Markers & Tactical Overlays
// =============================================================================

// Player Marker
const playerIcon = L.divIcon({
  className: '',
  iconSize: [44, 44],
  iconAnchor: [22, 22],
  html: `
    <div class="player-marker-node">
      <div class="player-pulse-ring"></div>
      <div class="player-arrow-rotator" id="player-arrow">
        <svg class="player-arrow-svg" viewBox="0 0 40 40">
          <polygon points="20,2 34,36 20,28 6,36" fill="#38bdf8" stroke="#ffffff" stroke-width="2" />
        </svg>
      </div>
      <div class="player-dot-center"></div>
    </div>
  `
});

const playerMarker = L.marker([3000, 3000], { icon: playerIcon, zIndexOffset: 1000 }).addTo(map);
playerMarker.bindPopup("<b>CJ (Player)</b><br><span id='pop-player-info'>Locating...</span>");

// Target Marker (AI Directive)
const targetIcon = L.divIcon({
  className: '',
  iconSize: [36, 36],
  iconAnchor: [18, 18],
  html: `
    <div class="target-marker-node">
      <div class="target-pulse"></div>
      <svg class="target-reticle-svg" viewBox="0 0 40 40">
        <circle cx="20" cy="20" r="14" fill="none" stroke="#f59e0b" stroke-width="2.5" stroke-dasharray="8 4"/>
        <circle cx="20" cy="20" r="4" fill="#f59e0b"/>
      </svg>
    </div>
  `
});

let targetMarker = null;
let interceptPolyline = null;
let playerTrail = L.polyline([], { color: '#38bdf8', weight: 2.5, opacity: 0.45, dashArray: '4, 4' }).addTo(map);
const trailCoords = [];


// =============================================================================
//  4. Auto-Pan & Control Logic
// =============================================================================
let autoFollow = true;
let lastKnownPos = [3000, 3000];

function toggleFollow() {
  autoFollow = !autoFollow;
  const btn = document.getElementById('btn-follow');
  const txt = document.getElementById('txt-follow');
  if (autoFollow) {
    btn.classList.add('active');
    txt.textContent = 'Follow: ON';
    map.panTo(lastKnownPos, { animate: true, duration: 0.25 });
  } else {
    btn.classList.remove('active');
    txt.textContent = 'Follow: OFF';
  }
}

function recenterPlayer() {
  autoFollow = true;
  document.getElementById('btn-follow').classList.add('active');
  document.getElementById('txt-follow').textContent = 'Follow: ON';
  map.panTo(lastKnownPos, { animate: true, duration: 0.25 });
}

map.on('dragstart', () => {
  if (autoFollow) {
    autoFollow = false;
    document.getElementById('btn-follow').classList.remove('active');
    document.getElementById('txt-follow').textContent = 'Follow: OFF';
  }
});

function getCardinal(deg) {
  const dirs = ['N', 'NE', 'E', 'SE', 'S', 'SW', 'W', 'NW'];
  const idx = Math.round(deg / 45) % 8;
  return dirs[idx];
}

// =============================================================================
//  3b. District AABB Zoning Overlays (Los Santos Master Zoning)
// =============================================================================
const zoneLayer = L.layerGroup().addTo(map);
let showZones = true;

const DISTRICT_SPECIALIZATIONS = {
  0: "Primary Demand: Food & Ammunition",
  1: "Primary Demand: High-Octane Fuel & Tech",
  2: "Primary Demand: Industrial Lumber & Heavy Fuel",
  3: "Primary Demand: Transit Fuel & Rural Timber"
};

const DISTRICT_ZONES = [
  {
    id: 0,
    name: "South Central",
    spec: "Primary Demand: Food & Ammunition",
    label: "SOUTH CENTRAL",
    minX: 1800.0, maxX: 2900.0,
    minY: -1850.0, maxY: -900.0,
    fillColor: 'rgba(244, 63, 94, 0.18)',
    borderColor: 'rgba(244, 63, 94, 0.6)',
    color: '#f43f5e',
    labelClass: 'zone-label-0'
  },
  {
    id: 1,
    name: "Downtown / West LS",
    spec: "Primary Demand: High-Octane Fuel & Tech",
    label: "DOWNTOWN",
    minX: 400.0, maxX: 1800.0,
    minY: -1850.0, maxY: -900.0,
    fillColor: 'rgba(56, 189, 248, 0.18)',
    borderColor: 'rgba(56, 189, 248, 0.6)',
    color: '#38bdf8',
    labelClass: 'zone-label-1'
  },
  {
    id: 2,
    name: "Industrial Port & Docks",
    spec: "Primary Demand: Industrial Lumber & Heavy Fuel",
    label: "PORT & DOCKS",
    minX: 1000.0, maxX: 2900.0,
    minY: -2800.0, maxY: -1850.0,
    fillColor: 'rgba(234, 179, 8, 0.18)',
    borderColor: 'rgba(234, 179, 8, 0.6)',
    color: '#eab308',
    labelClass: 'zone-label-2'
  }
];

DISTRICT_ZONES.forEach(z => {
  const sw = gameToMap(z.minX, z.minY);
  const ne = gameToMap(z.maxX, z.maxY);
  const rect = L.rectangle([sw, ne], {
    color: z.borderColor,
    weight: 2,
    fillColor: z.fillColor,
    fillOpacity: 1.0,
    dashArray: '5, 5'
  });

  rect.bindTooltip(z.label, {
    permanent: true,
    direction: 'center',
    className: `zone-label ${z.labelClass}`
  });

  rect.bindPopup(`
    <div style="font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;min-width:190px;color:#f1f5f9;padding:2px 0;">
      <div style="font-weight:800;font-size:13px;color:${z.color};margin-bottom:2px;border-bottom:1px solid rgba(255,255,255,0.12);padding-bottom:4px;">
        🏛️ District #${z.id}: ${z.name}
      </div>
      <div style="font-size:10.5px;font-weight:700;color:#fbbf24;margin-bottom:6px;">
        ${z.spec || DISTRICT_SPECIALIZATIONS[z.id] || ''}
      </div>
      <div style="font-size:11px;color:#94a3b8;margin-bottom:3px;">
        AABB X: [${z.minX.toFixed(0)}, ${z.maxX.toFixed(0)}]
      </div>
      <div style="font-size:11px;color:#94a3b8;">
        AABB Y: [${z.minY.toFixed(0)}, ${z.maxY.toFixed(0)}]
      </div>
    </div>
  `);

  zoneLayer.addLayer(rect);
});

function toggleZones() {
  showZones = !showZones;
  const btn = document.getElementById('btn-zones');
  const txt = document.getElementById('txt-zones');
  if (btn) btn.classList.toggle('active', showZones);
  if (txt) txt.textContent = showZones ? 'Zones: ON' : 'Zones: OFF';
  if (showZones) {
    zoneLayer.addTo(map);
  } else {
    map.removeLayer(zoneLayer);
  }
}

// =============================================================================
//  3c. Retail Stores Network (20 Stores across 4 Districts)
// =============================================================================
const storeLayer = L.layerGroup().addTo(map);
let showStores = true;

const STORE_CATS = {
  0: { name: 'Food', color: '#10b981', icon: '🛒' },
  1: { name: 'Fuel', color: '#f59e0b', icon: '⛽' },
  2: { name: 'Lumber', color: '#a855f7', icon: '🪵' },
  3: { name: 'Tech/Ammo', color: '#38bdf8', icon: '⚡' }
};

const INITIAL_RETAIL_STORES = [
  { id: 0, cat: 0, dist: 0, x: 1930.0, y: -1770.0, name: "Idlewood 24-7", cap: 120, stock: 79, rate: 14.5 },
  { id: 1, cat: 0, dist: 0, x: 2245.0, y: -1660.0, name: "Ganton Bodega", cap: 120, stock: 92, rate: 11.2 },
  { id: 2, cat: 1, dist: 0, x: 1944.5, y: -1771.2, name: "Idlewood Gas", cap: 100, stock: 48, rate: 16.8 },
  { id: 3, cat: 2, dist: 0, x: 2320.0, y: -1645.0, name: "Willowfield Timber", cap: 80, stock: 54, rate: 8.5 },
  { id: 4, cat: 3, dist: 0, x: 2400.0, y: -1250.0, name: "East LS AmmuTech", cap: 60, stock: 35, rate: 10.0 },
  { id: 5, cat: 0, dist: 1, x: 569.0, y: -1335.0, name: "Rodeo Deli", cap: 120, stock: 85, rate: 12.0 },
  { id: 6, cat: 0, dist: 1, x: 1315.0, y: -905.0, name: "Mulholland Market", cap: 120, stock: 78, rate: 9.4 },
  { id: 7, cat: 1, dist: 1, x: 1585.0, y: -1670.0, name: "Downtown Petrol", cap: 100, stock: 62, rate: 18.2 },
  { id: 8, cat: 2, dist: 1, x: 1050.0, y: -1200.0, name: "Market Hardware", cap: 80, stock: 45, rate: 7.6 },
  { id: 9, cat: 3, dist: 1, x: 1365.2, y: -1279.8, name: "Downtown Tech", cap: 60, stock: 40, rate: 13.1 },
  { id: 10, cat: 0, dist: 2, x: 2255.0, y: -2387.0, name: "Ocean Docks Diner", cap: 120, stock: 62, rate: 6.4 },
  { id: 11, cat: 1, dist: 2, x: 2640.0, y: -2115.0, name: "Terminal Fuel", cap: 100, stock: 71, rate: 8.9 },
  { id: 12, cat: 1, dist: 2, x: 1980.0, y: -2490.0, name: "LSX Aviation Fuel", cap: 100, stock: 60, rate: 11.5 },
  { id: 13, cat: 2, dist: 2, x: 2750.0, y: -2400.0, name: "Ocean Docks Timber", cap: 80, stock: 48, rate: 5.8 },
  { id: 14, cat: 3, dist: 2, x: 2445.0, y: -2547.0, name: "Port Radio Depot", cap: 60, stock: 34, rate: 7.2 },
  { id: 15, cat: 0, dist: 3, x: 1260.0, y: 250.0, name: "Montgomery Grocery", cap: 120, stock: 58, rate: 15.0 },
  { id: 16, cat: 1, dist: 3, x: -91.3, y: -1170.5, name: "Flint County Gas", cap: 100, stock: 49, rate: 12.3 },
  { id: 17, cat: 1, dist: 3, x: 660.0, y: -560.0, name: "Dillimore Diesel", cap: 100, stock: 55, rate: 9.8 },
  { id: 18, cat: 2, dist: 3, x: 850.0, y: -200.0, name: "Red County Timber", cap: 80, stock: 42, rate: 14.1 },
  { id: 19, cat: 3, dist: 3, x: 200.0, y: -240.0, name: "Blueberry Depot", cap: 60, stock: 30, rate: 10.5 }
];

const storeMarkers = {};

function formatStoreTooltip(store) {
  const cat = STORE_CATS[store.cat] || { name: 'Store', color: '#94a3b8', icon: '🏪' };
  const cap = store.cap || 100;
  const stock = store.stock ?? 0;
  const isCrit = (stock < cap * 0.25);
  const statusStr = isCrit ? '⚠️ Critical' : 'Normal';
  return `<b>${cat.icon} ${store.name}</b><br><span style="color:${cat.color};font-weight:700;">${cat.name}</span> · Stock: <b>${stock}/${cap}</b> (${statusStr})`;
}

function formatStorePopup(store) {
  const cat = STORE_CATS[store.cat] || { name: 'General', color: '#94a3b8', icon: '🏪' };
  const cap = store.cap || 100;
  const stock = store.stock ?? 0;
  const isCritical = (stock < cap * 0.25);
  const statusBadge = isCritical
    ? `<span style="color:#ef4444;font-weight:800;background:rgba(239,68,68,0.18);border:1px solid rgba(239,68,68,0.4);padding:2px 7px;border-radius:4px;font-size:11px;">⚠️ CRITICAL</span>`
    : `<span style="color:#22c55e;font-weight:800;background:rgba(34,197,94,0.18);border:1px solid rgba(34,197,94,0.4);padding:2px 7px;border-radius:4px;font-size:11px;">✅ NORMAL</span>`;
  const pct = Math.min(100, Math.max(0, Math.round((stock / cap) * 100)));
  const barColor = isCritical ? '#ef4444' : cat.color;
  const rate = (store.rate != null) ? Number(store.rate).toFixed(1) : '0.0';

  const score = store.score != null ? Number(store.score) : 80;
  let scoreColor = '#22c55e'; // Green (>70)
  if (score < 35) scoreColor = '#ef4444'; // Red (<35)
  else if (score <= 70) scoreColor = '#f59e0b'; // Amber (35-70)

  const debt = store.debt != null ? Number(store.debt) : 0;
  const isBankrupt = Boolean(store.bankrupt === true || store.bankrupt === 'true');

  const distNames = ["South Central", "Downtown", "Industrial Port", "Country / Hwy"];
  const distName = distNames[store.dist] || ("District #" + store.dist);
  const distSpec = DISTRICT_SPECIALIZATIONS[store.dist] || "Primary Demand: General Balanced";

  return `
    <div style="font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;min-width:220px;color:#f1f5f9;padding:2px 0;">
      ${isBankrupt ? `
        <div style="background:rgba(239,68,68,0.25);border:1.5px solid #ef4444;border-radius:6px;padding:6px 8px;margin-bottom:8px;text-align:center;font-weight:900;font-size:11px;color:#fca5a5;animation:pulse-bankrupt 1.5s infinite;">
          ⚠️ DECLARED BANKRUPT (NO RESTOCK)
        </div>
      ` : ''}
      <div style="display:flex;justify-content:space-between;align-items:center;border-bottom:1px solid rgba(255,255,255,0.12);padding-bottom:6px;margin-bottom:8px;">
        <span style="font-size:13px;font-weight:800;color:${cat.color};">${cat.icon} ${store.name}</span>
        ${statusBadge}
      </div>
      <div style="display:flex;justify-content:space-between;margin-bottom:2px;font-size:12px;">
        <span style="color:#94a3b8;">District:</span>
        <span style="font-weight:700;color:#38bdf8;">${distName}</span>
      </div>
      <div style="font-size:10px;font-weight:700;color:#fbbf24;margin-bottom:6px;">
        ${distSpec}
      </div>
      <div style="display:flex;justify-content:space-between;margin-bottom:6px;font-size:12px;">
        <span style="color:#94a3b8;">Category:</span>
        <span style="font-weight:700;color:${cat.color};">${cat.name} (Cat ${store.cat})</span>
      </div>
      <div style="display:flex;justify-content:space-between;margin-bottom:6px;font-size:12px;">
        <span style="color:#94a3b8;">Credit Rating:</span>
        <span style="font-weight:800;color:${scoreColor};">${score}/100</span>
      </div>
      <div style="display:flex;justify-content:space-between;margin-bottom:6px;font-size:12px;">
        <span style="color:#94a3b8;">Loan Debt:</span>
        <span style="font-weight:700;color:${debt > 0 ? '#f87171' : '#4ade80'};">$${debt.toLocaleString()}</span>
      </div>
      <div style="margin-bottom:8px;">
        <div style="display:flex;justify-content:space-between;font-size:11px;margin-bottom:3px;">
          <span style="color:#94a3b8;">Stock Level:</span>
          <span style="font-weight:700;color:#f8fafc;">${stock} / ${cap} (${pct}%)</span>
        </div>
        <div style="background:rgba(255,255,255,0.1);border-radius:4px;height:6px;overflow:hidden;">
          <div style="background:${barColor};width:${pct}%;height:100%;transition:width 0.3s;"></div>
        </div>
      </div>
      <div style="display:flex;justify-content:space-between;align-items:center;font-size:12px;background:rgba(255,255,255,0.05);padding:5px 8px;border-radius:6px;">
        <span style="color:#94a3b8;">Sales Velocity:</span>
        <span style="font-weight:700;color:#38bdf8;">${rate} u/m</span>
      </div>
    </div>
  `;
}

INITIAL_RETAIL_STORES.forEach(s => {
  const pos = gameToMap(s.x, s.y);
  const cat = STORE_CATS[s.cat] || { color: '#ffffff' };
  const marker = L.circleMarker(pos, {
    radius: 7,
    fillColor: cat.color,
    color: '#ffffff',
    weight: 2,
    opacity: 0.95,
    fillOpacity: 0.85
  });

  marker.bindTooltip(formatStoreTooltip(s), {
    direction: 'top',
    offset: [0, -7],
    className: 'store-tooltip'
  });
  marker.bindPopup(formatStorePopup(s));

  storeLayer.addLayer(marker);
  storeMarkers[s.id] = { marker, data: { ...s } };
});

function toggleStores() {
  showStores = !showStores;
  const btn = document.getElementById('btn-stores');
  const txt = document.getElementById('txt-stores');
  if (btn) btn.classList.toggle('active', showStores);
  if (txt) txt.textContent = showStores ? 'Stores: ON' : 'Stores: OFF';
  if (showStores) {
    storeLayer.addTo(map);
  } else {
    map.removeLayer(storeLayer);
  }
}

const truckLayer = L.layerGroup().addTo(map);
const truckMarkers = {};

async function updateRadar() {
  try {
    const res = await fetch('/api/status');
    if (!res.ok) return;
    const data = await res.json();

    g_latestMapTelemetry = data;
    g_strategicMarkers.forEach(item => {
      if (item.marker && item.marker.isPopupOpen()) {
        item.marker.setPopupContent(item.renderPopup());
      }
    });

    if (typeof renderAllRoadblocks === 'function') {
      renderAllRoadblocks(data.roadblocks, data.macro || g_latestMapTelemetry?.macro);
    }
    updateRoadblocksAlertUI(data);

    // 1. Status badge
    const st = document.getElementById('sys-status');
    if (st) {
      st.textContent = 'ONLINE';
      st.style.color = '#22c55e';
    }

    // Company Balance display
    if (data.company_balance != null) {
      const balEl = document.getElementById('hud-balance');
      if (balEl) {
        balEl.textContent = `$${Number(data.company_balance).toLocaleString()}`;
      }
    }

    // 2. Player marker position & telemetry HUD
    if (data.pos) {
      const pPos = gameToMap(data.pos.x, data.pos.y);
      const distJump = Math.hypot(pPos[0] - lastKnownPos[0], pPos[1] - lastKnownPos[1]);
      lastKnownPos = pPos;
      if (!map.hasLayer(playerMarker)) {
        playerMarker.addTo(map);
      }
      playerMarker.setLatLng(pPos);
      if (autoFollow) {
        if (distJump > 200) {
          // Teleport detected: instantly jump camera without broken animation interpolation
          map.setView(pPos, map.getZoom(), { animate: false });
        } else {
          map.panTo(pPos, { animate: true, duration: 0.25 });
        }
      }
      const pElement = playerMarker.getElement();
      const arrowEl = pElement ? pElement.querySelector('#player-arrow') : document.getElementById('player-arrow');
      if (arrowEl && data.heading != null) {
        const compassDeg = (360 - (data.heading % 360)) % 360;
        arrowEl.style.transform = `rotate(${compassDeg}deg)`;
      }

      document.getElementById('hud-pos-x').textContent = `X: ${data.pos.x.toFixed(1)}`;
      document.getElementById('hud-pos-y').textContent = `Y: ${data.pos.y.toFixed(1)}`;
      document.getElementById('hud-pos-z').textContent = `Z: ${data.pos.z.toFixed(1)}`;
      document.getElementById('hud-speed').innerHTML = `${(data.speed * 180).toFixed(0)} <span>km/h</span>`;
      document.getElementById('hud-head').innerHTML = `${Math.round(data.heading)}° <span>${getCardinal(data.heading)}</span>`;
    }

    // 3. Render and sync trucks
    if (data.trucks && Array.isArray(data.trucks)) {
      const activeIds = new Set();
      data.trucks.forEach(t => {
        activeIds.add(t.id);
        const tPos = gameToMap(t.x, t.y);
        
        const isDestroyed = (t.state === 'DESTROYED' || t.destroyed);
        const isBroken = (t.state === 'BROKEN_DOWN');
        let truckColor;
        if (isDestroyed) truckColor = '#ef4444';
        else if (isBroken) truckColor = '#ef4444';
        else if (t.state === 'RESTING') truckColor = '#3b82f6';
        else if (t.state === 'LOADING') truckColor = '#f97316';
        else if (t.state === 'UNLOADING') truckColor = '#a855f7';
        else truckColor = t.materialized ? '#22c55e' : (t.company_color || '#eab308');

        const tooltipContent = isDestroyed
          ? `💥 Truck #${t.id} DESTROYED (Fatal Wreck) — Insurance Claim Processing`
          : (isBroken
            ? `⚠️ Truck #${t.id} BREAKDOWN (Engine Fault)`
            : `Truck #${t.id} [${t.materialized ? '3D' : 'Sim'}] — ${t.state}<br>🏢 ${t.company_name || 'Haulage'} · 👤 ${t.driver_name || 'Driver'}<br>⛽ Fuel: ${t.fuel}% | 💤 Fatigue: ${t.fatigue}%<br>📦 ${t.cargo} (${t.cargo_tons}t)`);

        const popupContent = `
          <div style="font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif; min-width:210px; color:#f1f5f9; padding:2px 0;">
            <div style="display:flex; justify-content:space-between; align-items:center; border-bottom:1px solid rgba(255,255,255,0.1); padding-bottom:6px; margin-bottom:8px;">
              <span style="font-size:14px; font-weight:800; color:${t.company_color || '#38bdf8'};">Truck #${t.id} · ${t.driver_name || 'Driver'}</span>
              <span style="font-size:11px; font-weight:700; background:rgba(255,255,255,0.1); padding:2px 6px; border-radius:4px; color:${t.materialized ? '#4ade80' : '#facc15'};">${t.materialized ? '3D Active' : 'Virtual Sim'}</span>
            </div>
            <div style="display:flex; justify-content:space-between; margin-bottom:6px; font-size:12px;">
              <span style="color:#94a3b8;">Company:</span>
              <span style="font-weight:700; color:${t.company_color || '#38bdf8'};">${t.company_name || 'Haulage'}</span>
            </div>
            <div style="display:flex; justify-content:space-between; margin-bottom:8px; font-size:12px;">
              <span style="color:#94a3b8;">Status:</span>
              <span style="font-weight:700; color:${truckColor};">${isDestroyed ? '💥 DESTROYED (Fatal Wreck)' : (isBroken ? '⚠️ BROKEN DOWN (Engine Fault)' : t.state)}</span>
            </div>
            <div style="margin-bottom:8px;">
              <div style="display:flex; justify-content:space-between; font-size:11px; margin-bottom:3px;">
                <span style="color:#94a3b8;">⛽ Fuel</span>
                <span style="font-weight:700; color:#e2e8f0;">${t.fuel}%</span>
              </div>
              <div style="background:rgba(255,255,255,0.1); border-radius:4px; height:6px; overflow:hidden;">
                <div style="background:${t.fuel < 20 ? '#ef4444' : '#22c55e'}; width:${Math.min(100, Math.max(0, t.fuel))}%; height:100%; transition:width 0.3s;"></div>
              </div>
            </div>
            <div style="margin-bottom:8px;">
              <div style="display:flex; justify-content:space-between; font-size:11px; margin-bottom:3px;">
                <span style="color:#94a3b8;">💤 Fatigue</span>
                <span style="font-weight:700; color:#e2e8f0;">${t.fatigue}%</span>
              </div>
              <div style="background:rgba(255,255,255,0.1); border-radius:4px; height:6px; overflow:hidden;">
                <div style="background:${t.fatigue > 75 ? '#ef4444' : (t.fatigue > 40 ? '#f59e0b' : '#38bdf8')}; width:${Math.min(100, Math.max(0, t.fatigue))}%; height:100%; transition:width 0.3s;"></div>
              </div>
            </div>
            <div style="background:rgba(255,255,255,0.04); border:1px solid rgba(255,255,255,0.08); border-radius:6px; padding:6px 8px; font-size:11px;">
              <div style="display:flex; justify-content:space-between; margin-bottom:4px;">
                <span style="color:#94a3b8;">📦 Cargo:</span>
                <span style="font-weight:700; color:#f8fafc;">${t.cargo} (${t.cargo_tons}t)</span>
              </div>
              <div style="display:flex; justify-content:space-between; margin-bottom:4px;">
                <span style="color:#94a3b8;">🏁 Deliveries:</span>
                <span style="font-weight:700; color:#38bdf8;">${t.deliveries} completed</span>
              </div>
              <div style="display:flex; justify-content:space-between;">
                <span style="color:#94a3b8;">💵 Driver Wallet:</span>
                <span style="font-weight:700; color:#4ade80;">$${Number(t.driver_wallet || 0).toLocaleString()}</span>
              </div>
            </div>
          </div>
        `;

        let markerType = 'normal';
        if (isDestroyed) markerType = 'destroyed';
        else if (isBroken) markerType = 'sos';

        let marker = truckMarkers[t.id];
        if (marker && marker._markerType !== markerType) {
          truckLayer.removeLayer(marker);
          delete truckMarkers[t.id];
          marker = null;
        }

        if (!marker) {
          if (isDestroyed) {
            const skullIcon = L.divIcon({
              className: '',
              iconSize: [32, 32],
              iconAnchor: [16, 16],
              html: `<div style="font-size:22px; filter: drop-shadow(0 0 6px #ef4444); text-align:center;">💀</div>`
            });
            marker = L.marker(tPos, { icon: skullIcon, zIndexOffset: 600 }).addTo(truckLayer);
            marker._markerType = 'destroyed';
          } else if (isBroken) {
            const sosIcon = L.divIcon({
              className: '',
              iconSize: [32, 32],
              iconAnchor: [16, 16],
              html: `
                <div class="sos-marker-node">
                  <div class="sos-pulse-ring"></div>
                  <div class="sos-core-dot">!</div>
                </div>
              `
            });
            marker = L.marker(tPos, { icon: sosIcon, zIndexOffset: 500 }).addTo(truckLayer);
            marker._markerType = 'sos';
          } else {
            marker = L.circleMarker(tPos, {
              radius: t.materialized ? 9 : 6,
              fillColor: truckColor,
              color: '#ffffff',
              weight: 2,
              opacity: 1,
              fillOpacity: 0.9
            }).addTo(truckLayer);
            marker._markerType = 'normal';
          }
          marker.bindTooltip(tooltipContent, { permanent: false });
          marker.bindPopup(popupContent);
          truckMarkers[t.id] = marker;
        } else {
          marker.setLatLng(tPos);
          if (markerType === 'normal') {
            marker.setStyle({
              radius: t.materialized ? 9 : 6,
              fillColor: truckColor
            });
          }
          marker.setTooltipContent(tooltipContent);
          marker.setPopupContent(popupContent);
        }
      });

      for (const id in truckMarkers) {
        if (!activeIds.has(Number(id))) {
          truckLayer.removeLayer(truckMarkers[id]);
          delete truckMarkers[id];
        }
      }
    }

    // Update #hud-fleet & #hud-balance
    const matCount = data.trucks ? data.trucks.filter(t => t.materialized).length : 0;
    const brokenCount = data.trucks ? data.trucks.filter(t => t.state === 'BROKEN_DOWN').length : 0;
    const hudFleet = document.getElementById('hud-fleet');
    if (hudFleet) {
      hudFleet.textContent = `${data.trucks ? data.trucks.length : 0} Total · ${matCount} in 3D${brokenCount > 0 ? ` · ${brokenCount} SOS` : ''}`;
    }
    if (data.companies && data.companies.length > 0) {
      const hudBal = document.getElementById('hud-balance');
      if (hudBal) {
        const totalBal = data.companies.reduce((sum, c) => sum + (c.balance || 0), 0);
        hudBal.textContent = `$${totalBal.toLocaleString()}`;
      }
    }

    // Bottom stats
    const stats = data.stats || {};
    const botTel = document.getElementById('bot-tel');
    if (botTel) botTel.textContent = stats.telemetry ?? data.telemetryTotal ?? 0;
    const botDir = document.getElementById('bot-dir');
    if (botDir) botDir.textContent = stats.directives ?? data.directivesTotal ?? 0;
    const botFleet = document.getElementById('bot-fleet');
    if (botFleet) botFleet.textContent = `${data.trucks ? data.trucks.length : 0} (${matCount} 3D)`;

  } catch (e) {
    const st = document.getElementById('sys-status');
    if (st) {
      st.textContent = 'OFFLINE';
      st.style.color = '#ef4444';
    }
  }
}

setInterval(updateRadar, 400);
updateRadar();
updateRoadblocksAlertUI({ roadblocks: [] });
if (typeof renderAllRoadblocks === 'function') renderAllRoadblocks([], null);

setInterval(async () => {
    try {
        const res = await fetch('/telemetry');
        const data = await res.json();
        if (typeof renderAllRoadblocks === 'function') {
            renderAllRoadblocks(data.roadblocks, data.macro);
        }
        updateRoadblocksAlertUI(data);
        if (data.stores) {
            data.stores.forEach(s => {
                const stockEl = document.getElementById(`store-stock-${s.id}`);
                const soldEl = document.getElementById(`store-sold-${s.id}`);
                const revEl = document.getElementById(`store-rev-${s.id}`);
                if (stockEl) stockEl.innerText = s.stock;
                if (soldEl) soldEl.innerText = s.sold;
                if (revEl) revEl.innerText = `$${s.rev}`;

                const sm = storeMarkers[s.id];
                if (sm) {
                    Object.assign(sm.data, s);
                    if (s.x != null && s.y != null) {
                        sm.marker.setLatLng(gameToMap(s.x, s.y));
                    }
                    sm.marker.setTooltipContent(formatStoreTooltip(sm.data));
                    sm.marker.setPopupContent(formatStorePopup(sm.data));
                }
            });
        }
        if (data.bank) {
            const b = data.bank;
            const rateEl = document.getElementById('hud-fleeca-rate');
            const resEl = document.getElementById('hud-fleeca-reserves');
            if (rateEl) rateEl.textContent = `${(b.rate * 100).toFixed(1)}%`;
            if (resEl) resEl.textContent = `$${(b.reserves / 1000000).toFixed(2)}M`;
        }
    } catch(e) {}
}, 2000);
</script>
</body>
</html>)rawmap";

static const char k_logisticsHtml[] = R"rawlogistics(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>GTA SA — Logistics Tycoon & Route Editor</title>
<link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" />
<script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"></script>
<style>
  :root {
    --bg: #090d16;
    --card: #131b2e;
    --card-border: #1e293b;
    --card-hover: #1c2742;
    --primary: #38bdf8;
    --amber: #f59e0b;
    --purple: #a855f7;
    --danger: #ef4444;
    --success: #22c55e;
    --text: #f8fafc;
    --text-muted: #94a3b8;
  }

  /* Route Editor & Leaflet Map Styles */
  .route-editor-card {
    background: var(--card);
    border: 1px solid var(--card-border);
    border-radius: 14px;
    padding: 20px;
    margin-bottom: 28px;
    box-shadow: 0 10px 30px rgba(0, 0, 0, 0.4);
  }
  .editor-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    flex-wrap: wrap;
    gap: 12px;
    margin-bottom: 16px;
    padding-bottom: 12px;
    border-bottom: 1px solid var(--card-border);
  }
  .snap-badge {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    padding: 5px 12px;
    background: rgba(34, 197, 94, 0.15);
    border: 1px solid rgba(34, 197, 94, 0.4);
    color: #4ade80;
    font-size: 0.75rem;
    font-weight: 800;
    border-radius: 9999px;
  }
  .editor-toolbar {
    display: flex;
    justify-content: space-between;
    align-items: center;
    flex-wrap: wrap;
    gap: 12px;
    margin-bottom: 12px;
  }
  .toolbar-group {
    display: flex;
    gap: 8px;
    flex-wrap: wrap;
    align-items: center;
  }
  .comp-btn {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    padding: 8px 14px;
    background: rgba(255, 255, 255, 0.04);
    border: 1px solid var(--card-border);
    border-radius: 8px;
    color: var(--text-muted);
    font-size: 0.8rem;
    font-weight: 700;
    cursor: pointer;
    transition: all 0.2s;
  }
  .comp-btn:hover {
    background: rgba(255, 255, 255, 0.08);
    color: var(--text);
  }
  .comp-btn.active {
    background: rgba(56, 189, 248, 0.15);
    border-color: var(--primary);
    color: #f8fafc;
    box-shadow: 0 0 12px rgba(56, 189, 248, 0.25);
  }
  .comp-dot {
    width: 9px;
    height: 9px;
    border-radius: 50%;
  }
  .tool-btn {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    padding: 8px 14px;
    background: rgba(255, 255, 255, 0.05);
    border: 1px solid var(--card-border);
    border-radius: 8px;
    color: var(--text);
    font-size: 0.8rem;
    font-weight: 700;
    cursor: pointer;
    transition: all 0.2s;
  }
  .tool-btn:hover {
    background: rgba(255, 255, 255, 0.1);
  }
  .tool-btn.active {
    background: rgba(245, 158, 11, 0.2);
    border-color: #f59e0b;
    color: #fbbf24;
    box-shadow: 0 0 12px rgba(245, 158, 11, 0.3);
  }
  .tool-btn.danger:hover {
    background: rgba(239, 68, 68, 0.2);
    border-color: #ef4444;
    color: #f87171;
  }
  .tool-btn.deploy {
    background: linear-gradient(135deg, rgba(34, 197, 94, 0.25) 0%, rgba(16, 185, 129, 0.45) 100%);
    border: 1px solid rgba(34, 197, 94, 0.6);
    color: #f0fdf4;
    font-weight: 800;
    box-shadow: 0 4px 14px rgba(34, 197, 94, 0.25);
  }
  .tool-btn.deploy:hover {
    background: linear-gradient(135deg, rgba(34, 197, 94, 0.4) 0%, rgba(16, 185, 129, 0.65) 100%);
    border-color: #22c55e;
    box-shadow: 0 6px 18px rgba(34, 197, 94, 0.4);
    transform: translateY(-1px);
  }

  /* Dedicated Strategic POI Orange Highlight Markers & Tactical Popups */
  .strategic-hub-icon-wrap {
    background: transparent !important;
    border: none !important;
  }
  .strategic-hub-icon {
    position: relative;
    width: 18px;
    height: 18px;
    cursor: pointer;
  }
  .strategic-hub-icon .pulse-ring {
    position: absolute;
    top: -9px;
    left: -9px;
    width: 36px;
    height: 36px;
    border-radius: 50%;
    border: 2px solid #f59e0b;
    box-shadow: 0 0 10px #f59e0b;
    animation: hubPulseAnimation 1.8s cubic-bezier(0.24, 0, 0.38, 1) infinite;
    pointer-events: none;
  }
  .strategic-hub-icon .hub-dot {
    position: absolute;
    top: 0;
    left: 0;
    width: 18px;
    height: 18px;
    border-radius: 50%;
    background: #f59e0b;
    border: 2px solid #ffffff;
    box-shadow: 0 0 12px rgba(245, 158, 11, 0.95);
    box-sizing: border-box;
    transition: transform 0.2s ease, box-shadow 0.2s ease;
  }
  .strategic-hub-icon:hover .hub-dot {
    transform: scale(1.25);
    box-shadow: 0 0 18px #f59e0b;
  }
  @keyframes hubPulseAnimation {
    0% {
      transform: scale(0.5);
      opacity: 1;
    }
    70% {
      transform: scale(1.6);
      opacity: 0.15;
    }
    100% {
      transform: scale(1.9);
      opacity: 0;
    }
  }

  .tactical-popup-wrap .leaflet-popup-content-wrapper {
    background: #0b1120 !important;
    border: 1.5px solid #f59e0b !important;
    border-radius: 8px !important;
    box-shadow: 0 10px 25px rgba(0,0,0,0.8), 0 0 15px rgba(245,158,11,0.25) !important;
    color: #f1f5f9 !important;
    padding: 0 !important;
  }
  .tactical-popup-wrap .leaflet-popup-tip {
    background: #0b1120 !important;
    border: 1px solid #f59e0b !important;
  }
  .tactical-popup-wrap .leaflet-popup-content {
    margin: 12px 14px !important;
    line-height: 1.4 !important;
  }
  .tactical-popup {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    color: #f1f5f9;
    min-width: 260px;
  }
  .tactical-popup .hub-badge {
    display: inline-block;
    font-size: 9px;
    font-weight: 800;
    letter-spacing: 0.08em;
    text-transform: uppercase;
    background: rgba(245, 158, 11, 0.18);
    color: #f59e0b;
    border: 1px solid rgba(245, 158, 11, 0.4);
    padding: 2px 6px;
    border-radius: 4px;
    margin-bottom: 6px;
  }
  .tactical-popup .hub-title {
    font-size: 14px;
    font-weight: 700;
    color: #ffffff;
    display: flex;
    align-items: center;
    gap: 6px;
  }
  .tactical-popup .hub-sub {
    font-size: 11px;
    color: #94a3b8;
    margin-bottom: 8px;
    border-bottom: 1px solid rgba(255, 255, 255, 0.1);
    padding-bottom: 6px;
  }
  .tactical-popup .hub-metric-row {
    display: flex;
    justify-content: space-between;
    font-size: 11px;
    margin-bottom: 4px;
  }
  .tactical-popup .hub-metric-row .lbl {
    color: #94a3b8;
  }
  .tactical-popup .hub-metric-row .val {
    font-weight: 600;
    color: #f1f5f9;
  }
  .tactical-popup .hub-metric-row .val.food { color: #22c55e; }
  .tactical-popup .hub-metric-row .val.fuel { color: #eab308; }
  .tactical-popup .hub-metric-row .val.queue { color: #38bdf8; }
  .tactical-popup .hub-metric-row .val.pass { color: #10b981; }
  .tactical-popup .hub-role {
    font-size: 10px;
    color: #94a3b8;
    background: rgba(255, 255, 255, 0.05);
    border-radius: 4px;
    padding: 6px;
    margin-top: 8px;
    line-height: 1.35;
  }

  #route-editor-map {
    cursor: default;
  }
  #route-editor-map.edit-active,
  #route-editor-map.edit-active .leaflet-container {
    cursor: crosshair;
  }

  .route-meta-bar {
    display: flex;
    align-items: center;
    flex-wrap: wrap;
    gap: 16px;
    padding: 10px 14px;
    background: rgba(15, 23, 42, 0.7);
    border: 1px solid var(--card-border);
    border-radius: 8px;
    font-size: 0.78rem;
    color: var(--text-muted);
  }
  .meta-item b {
    color: #f8fafc;
    font-family: monospace;
    font-size: 0.85rem;
  }
  .editor-toast {
    margin-left: auto;
    font-size: 0.75rem;
    font-weight: 700;
    color: #38bdf8;
    background: rgba(56, 189, 248, 0.1);
    border: 1px solid rgba(56, 189, 248, 0.25);
    padding: 4px 10px;
    border-radius: 6px;
    transition: all 0.3s;
  }
  .chevron-marker {
    pointer-events: none;
  }
  .chevron-svg {
    filter: drop-shadow(0 0 4px rgba(0,0,0,0.8));
  }
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    background: var(--bg);
    color: var(--text);
    padding: 24px;
    min-height: 100vh;
  }
  .container { max-width: 1280px; margin: 0 auto; }
  header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding-bottom: 20px;
    border-bottom: 1px solid var(--card-border);
    margin-bottom: 24px;
    flex-wrap: wrap;
    gap: 16px;
  }
  .brand { display: flex; align-items: center; gap: 12px; }
  .brand h1 { font-size: 1.5rem; font-weight: 800; letter-spacing: -0.02em; }
  .brand span { color: var(--primary); }
  .nav-btns { display: flex; gap: 10px; align-items: center; }
  .nav-btn {
    background: var(--card);
    border: 1px solid var(--card-border);
    color: var(--text);
    padding: 8px 14px;
    border-radius: 8px;
    text-decoration: none;
    font-size: 0.82rem;
    font-weight: 700;
    display: flex;
    align-items: center;
    gap: 6px;
    transition: all 0.2s;
  }
  .nav-btn:hover { background: #1e293b; border-color: var(--primary); color: var(--primary); }
  .nav-btn.active { background: rgba(56, 189, 248, 0.15); border-color: var(--primary); color: var(--primary); }
  .status-pill {
    background: rgba(34, 197, 94, 0.15);
    border: 1px solid rgba(34, 197, 94, 0.3);
    color: #4ade80;
    padding: 4px 10px;
    border-radius: 9999px;
    font-size: 0.75rem;
    font-weight: 800;
    display: flex;
    align-items: center;
    gap: 6px;
  }
  .pulse-dot { width: 7px; height: 7px; border-radius: 50%; background: #22c55e; animation: pulse 1.5s infinite; }
  @keyframes pulse { 0% { opacity: 1; transform: scale(1); } 50% { opacity: 0.4; transform: scale(1.3); } 100% { opacity: 1; transform: scale(1); } }

  .section-title {
    font-size: 1.15rem;
    font-weight: 800;
    margin-bottom: 14px;
    display: flex;
    align-items: center;
    gap: 8px;
  }

  /* Companies Grid */
  .companies-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(320px, 1fr));
    gap: 16px;
    margin-bottom: 28px;
  }
  .company-card {
    background: var(--card);
    border: 1px solid var(--card-border);
    border-radius: 12px;
    padding: 20px;
    position: relative;
    overflow: hidden;
    transition: transform 0.2s, border-color 0.2s;
  }
  .company-card:hover { transform: translateY(-2px); }
  .card-top-bar { position: absolute; top: 0; left: 0; right: 0; height: 4px; }
  .company-header { display: flex; justify-content: space-between; align-items: flex-start; margin-bottom: 12px; }
  .company-name { font-size: 1.15rem; font-weight: 800; }
  .company-hub { font-size: 0.75rem; color: var(--text-muted); margin-top: 2px; }
  .company-balance { font-size: 1.85rem; font-weight: 900; margin: 12px 0 6px; }
  .company-stats {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 10px;
    background: rgba(255, 255, 255, 0.03);
    border: 1px solid rgba(255, 255, 255, 0.05);
    border-radius: 8px;
    padding: 10px 12px;
    margin-top: 14px;
  }
  .stat-label { font-size: 0.72rem; color: var(--text-muted); text-transform: uppercase; font-weight: 700; }
  .stat-val { font-size: 0.95rem; font-weight: 800; margin-top: 2px; }
  .perk-badge {
    display: inline-flex;
    align-items: center;
    gap: 4px;
    font-size: 0.7rem;
    font-weight: 800;
    padding: 3px 8px;
    border-radius: 6px;
    background: rgba(245, 158, 11, 0.15);
    border: 1px solid rgba(245, 158, 11, 0.3);
    color: #fbbf24;
    margin-top: 10px;
  }

  /* Assets Grid */
  .assets-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
    gap: 16px;
    margin-bottom: 28px;
  }
  .asset-card {
    background: var(--card);
    border: 1px solid var(--card-border);
    border-radius: 12px;
    padding: 16px 18px;
    display: flex;
    flex-direction: column;
    justify-content: space-between;
  }
  .asset-head { display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px; }
  .asset-title { font-size: 1rem; font-weight: 800; }
  .asset-cost { font-size: 0.85rem; font-weight: 700; color: #94a3b8; }
  .asset-owner-status {
    margin-top: 10px;
    padding: 8px 12px;
    border-radius: 8px;
    font-size: 0.8rem;
    font-weight: 700;
    display: flex;
    align-items: center;
    justify-content: space-between;
  }

  /* San Fierro Port Supply Depot */
  .sf-depot-card {
    background: var(--card);
    border: 1px solid var(--card-border);
    border-radius: 12px;
    padding: 18px 20px;
    margin-bottom: 28px;
  }
  .sf-depot-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    flex-wrap: wrap;
    gap: 10px;
    margin-bottom: 14px;
  }
  .sf-bonus-pill {
    font-size: 0.75rem;
    font-weight: 800;
    padding: 4px 10px;
    border-radius: 9999px;
    display: inline-flex;
    align-items: center;
    gap: 6px;
  }
  .sf-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(230px, 1fr));
    gap: 14px;
  }
  .sf-item {
    background: rgba(255, 255, 255, 0.02);
    border: 1px solid rgba(255, 255, 255, 0.06);
    border-radius: 10px;
    padding: 12px 14px;
  }
  .sf-item-head {
    display: flex;
    justify-content: space-between;
    align-items: center;
    font-size: 0.85rem;
    font-weight: 800;
    margin-bottom: 6px;
  }
  .sf-stock-val {
    font-family: monospace;
    font-size: 0.8rem;
    color: var(--primary);
  }

  /* Table Card */
  .table-card {
    background: var(--card);
    border: 1px solid var(--card-border);
    border-radius: 12px;
    overflow: hidden;
  }
  .table-responsive { overflow-x: auto; }
  table { width: 100%; border-collapse: collapse; text-align: left; font-size: 0.82rem; }
  th {
    background: rgba(255, 255, 255, 0.02);
    border-bottom: 1px solid var(--card-border);
    padding: 12px 16px;
    color: var(--text-muted);
    font-weight: 700;
    text-transform: uppercase;
    font-size: 0.7rem;
    letter-spacing: 0.05em;
  }
  td { padding: 12px 16px; border-bottom: 1px solid rgba(255, 255, 255, 0.04); vertical-align: middle; }
  tr:last-child td { border-bottom: none; }
  tr:hover { background: rgba(255, 255, 255, 0.02); }

  .driver-cell { display: flex; align-items: center; gap: 10px; font-weight: 800; }
  .driver-avatar {
    width: 28px;
    height: 28px;
    border-radius: 50%;
    background: #1e293b;
    border: 1px solid rgba(255, 255, 255, 0.1);
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 0.75rem;
    font-weight: 900;
    color: var(--primary);
  }
  .company-pill {
    font-size: 0.7rem;
    font-weight: 800;
    padding: 2px 8px;
    border-radius: 6px;
    display: inline-block;
  }
  .state-badge {
    font-size: 0.7rem;
    font-weight: 800;
    padding: 3px 8px;
    border-radius: 6px;
    display: inline-flex;
    align-items: center;
    gap: 4px;
  }
  .rank-badge {
    font-size: 0.65rem;
    font-weight: 800;
    padding: 2px 6px;
    border-radius: 4px;
    display: inline-flex;
    align-items: center;
    gap: 3px;
    letter-spacing: 0.02em;
  }
  .state-broken {
    background: rgba(239, 68, 68, 0.2);
    border: 1px solid rgba(239, 68, 68, 0.5);
    color: #f87171;
    animation: pulse-sos 1s infinite;
  }
  @keyframes pulse-sos { 0%, 100% { opacity: 1; } 50% { opacity: 0.5; } }

  .progress-wrap { display: flex; align-items: center; gap: 8px; }
  .bar-bg { flex: 1; height: 6px; background: rgba(255, 255, 255, 0.08); border-radius: 3px; overflow: hidden; min-width: 60px; }
  .bar-fill { height: 100%; border-radius: 3px; }

  /* 20-Store Retail Network Catalog */
  .retail-tabs {
    display: flex;
    gap: 8px;
    margin-bottom: 14px;
    flex-wrap: wrap;
  }
  .retail-tab-btn {
    background: rgba(255, 255, 255, 0.04);
    border: 1px solid var(--card-border);
    color: var(--text-muted);
    padding: 6px 14px;
    border-radius: 8px;
    font-size: 0.78rem;
    font-weight: 700;
    cursor: pointer;
    transition: all 0.2s;
  }
  .retail-tab-btn:hover { color: var(--text); background: rgba(255, 255, 255, 0.08); }
  .retail-tab-btn.active { background: rgba(56, 189, 248, 0.2); border-color: var(--primary); color: var(--primary); }

  .retail-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
    gap: 14px;
    align-items: start;
  }
  .retail-card, .store-card {
    background: rgba(255, 255, 255, 0.02);
    border: 1px solid rgba(255, 255, 255, 0.06);
    border-radius: 10px;
    padding: 14px;
    display: flex;
    flex-direction: column;
    gap: 8px;
    transition: transform 0.2s, border-color 0.2s;
  }
  .retail-card:hover, .store-card:hover { transform: translateY(-2px); border-color: rgba(56, 189, 248, 0.4); }
  .retail-card-top { display: flex; justify-content: space-between; align-items: flex-start; gap: 8px; }
  .retail-card-title { font-size: 0.88rem; font-weight: 800; color: #f8fafc; }
  .retail-cat-badge { font-size: 0.68rem; font-weight: 800; padding: 2px 7px; border-radius: 4px; text-transform: uppercase; }
  .retail-bal-row { display: flex; justify-content: space-between; align-items: baseline; font-size: 0.75rem; }
  .retail-bal-val { font-size: 0.95rem; font-weight: 900; color: #38bdf8; }
  .retail-metrics { display: flex; justify-content: space-between; font-size: 0.72rem; color: #94a3b8; }

  /* Detail Toggle Button & Progressive Disclosure Drawer */
  .btn-store-detail {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    gap: 4px;
    background: rgba(56, 189, 248, 0.1);
    color: #38bdf8;
    border: 1px solid rgba(56, 189, 248, 0.25);
    border-radius: 6px;
    padding: 3px 8px;
    font-size: 0.68rem;
    font-weight: 700;
    cursor: pointer;
    transition: all 0.2s ease;
    user-select: none;
    white-space: nowrap;
  }
  .btn-store-detail:hover {
    background: rgba(56, 189, 248, 0.22);
    border-color: #38bdf8;
    transform: translateY(-1px);
  }
  .btn-store-detail:active {
    transform: translateY(0);
  }

  .store-detail-drawer.collapsed {
    display: none;
  }
  .store-detail-drawer.expanded {
    display: flex;
    flex-direction: column;
    gap: 8px;
    background: rgba(15, 23, 42, 0.95);
    border: 1px solid #334155;
    border-radius: 8px;
    padding: 10px 12px;
    margin-top: 6px;
    box-shadow: 0 8px 20px -4px rgba(0, 0, 0, 0.6);
    animation: drawerFadeIn 0.2s ease-out;
  }
  @keyframes drawerFadeIn {
    from { opacity: 0; transform: translateY(-4px); }
    to { opacity: 1; transform: translateY(0); }
  }

  .drawer-title-row {
    display: flex;
    justify-content: space-between;
    align-items: center;
    border-bottom: 1px solid #334155;
    padding-bottom: 6px;
  }
  .drawer-title {
    font-size: 0.74rem;
    font-weight: 800;
    color: #f8fafc;
    letter-spacing: 0.02em;
  }
  .btn-drawer-collapse {
    background: rgba(239, 68, 68, 0.12);
    color: #f87171;
    border: 1px solid rgba(239, 68, 68, 0.3);
    border-radius: 4px;
    padding: 2px 7px;
    font-size: 0.65rem;
    font-weight: 700;
    cursor: pointer;
    transition: all 0.15s ease;
    white-space: nowrap;
  }
  .btn-drawer-collapse:hover {
    background: rgba(239, 68, 68, 0.22);
    border-color: #f87171;
  }

  .drawer-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 8px 12px;
    font-size: 0.68rem;
  }
  .drawer-item {
    display: flex;
    flex-direction: column;
    gap: 2px;
  }
  .drawer-label {
    color: #94a3b8;
    font-size: 0.60rem;
    text-transform: uppercase;
    font-weight: 700;
    letter-spacing: 0.03em;
  }
  .drawer-value {
    color: #f1f5f9;
    font-weight: 700;
    font-size: 0.72rem;
    line-height: 1.3;
  }

  /* Universal Import Route Modal */
  .modal-overlay {
    position: fixed;
    top: 0;
    left: 0;
    width: 100vw;
    height: 100vh;
    background: rgba(3, 7, 18, 0.85);
    backdrop-filter: blur(8px);
    -webkit-backdrop-filter: blur(8px);
    display: flex;
    align-items: center;
    justify-content: center;
    z-index: 99999;
    opacity: 0;
    pointer-events: none;
    transition: opacity 0.25s ease;
  }
  .modal-overlay.active {
    opacity: 1;
    pointer-events: auto;
  }
  .modal-card {
    background: #0f172a;
    border: 1px solid rgba(56, 189, 248, 0.35);
    box-shadow: 0 20px 40px -10px rgba(0, 0, 0, 0.8), 0 0 25px rgba(56, 189, 248, 0.15);
    border-radius: 14px;
    width: 90%;
    max-width: 660px;
    display: flex;
    flex-direction: column;
    overflow: hidden;
    transform: scale(0.95);
    transition: transform 0.25s ease;
  }
  .modal-overlay.active .modal-card {
    transform: scale(1);
  }
  .modal-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 16px 20px;
    background: rgba(255, 255, 255, 0.03);
    border-bottom: 1px solid rgba(255, 255, 255, 0.08);
  }
  .modal-title {
    font-size: 1.05rem;
    font-weight: 800;
    color: #f8fafc;
    display: flex;
    align-items: center;
    gap: 8px;
  }
  .modal-close {
    background: transparent;
    border: none;
    color: #94a3b8;
    font-size: 1.4rem;
    cursor: pointer;
    line-height: 1;
    padding: 4px 8px;
    border-radius: 6px;
    transition: all 0.15s;
  }
  .modal-close:hover {
    color: #f8fafc;
    background: rgba(255, 255, 255, 0.1);
  }
  .modal-body {
    padding: 20px;
    display: flex;
    flex-direction: column;
    gap: 12px;
  }
  .modal-hint {
    font-size: 0.82rem;
    color: #94a3b8;
    line-height: 1.45;
  }
  .modal-hint code {
    background: rgba(255, 255, 255, 0.08);
    padding: 2px 6px;
    border-radius: 4px;
    color: #38bdf8;
    font-family: monospace;
    font-size: 0.78rem;
  }
  .import-textarea {
    width: 100%;
    height: 220px;
    background: #090d16;
    border: 1px solid rgba(255, 255, 255, 0.12);
    border-radius: 8px;
    color: #e2e8f0;
    font-family: 'Fira Code', Consolas, Monaco, monospace;
    font-size: 0.82rem;
    line-height: 1.45;
    padding: 12px;
    box-sizing: border-box;
    resize: vertical;
    outline: none;
    transition: border-color 0.2s, box-shadow 0.2s;
  }
  .import-textarea:focus {
    border-color: #38bdf8;
    box-shadow: 0 0 0 2px rgba(56, 189, 248, 0.25);
  }
  .modal-footer {
    display: flex;
    justify-content: flex-end;
    align-items: center;
    gap: 10px;
    padding: 14px 20px;
    background: rgba(255, 255, 255, 0.02);
    border-top: 1px solid rgba(255, 255, 255, 0.06);
  }

  /* Dedicated Glowing Factory Hub (Node 0) Marker & Tooltip */
  .factory-hub-marker-wrap {
    background: transparent !important;
    border: none !important;
  }
  .factory-hub-glow {
    position: relative;
    width: 34px;
    height: 34px;
    display: flex;
    align-items: center;
    justify-content: center;
    background: linear-gradient(135deg, rgba(16, 185, 129, 0.95), rgba(5, 150, 105, 0.98));
    border: 2px solid #ffffff;
    border-radius: 8px;
    box-shadow: 0 0 16px rgba(16, 185, 129, 0.85), 0 0 4px #34d399, inset 0 0 6px rgba(255, 255, 255, 0.3);
    font-size: 17px;
    cursor: pointer;
    transition: transform 0.2s ease, box-shadow 0.2s ease;
    animation: factoryPulse 2.4s infinite ease-in-out;
  }
  @keyframes factoryPulse {
    0%, 100% {
      box-shadow: 0 0 12px rgba(16, 185, 129, 0.8), 0 0 4px #10b981;
      transform: scale(1);
    }
    50% {
      box-shadow: 0 0 24px rgba(52, 211, 153, 1), 0 0 8px #34d399;
      transform: scale(1.08);
    }
  }
  .factory-hub-glow:hover {
    transform: scale(1.15);
    box-shadow: 0 0 28px rgba(52, 211, 153, 1), 0 0 10px #ffffff;
  }
  .factory-hub-tooltip {
    background: rgba(15, 23, 42, 0.96) !important;
    border: 1.5px solid #10b981 !important;
    color: #6ee7b7 !important;
    font-size: 11px !important;
    font-weight: 800 !important;
    letter-spacing: 0.3px !important;
    padding: 4px 9px !important;
    border-radius: 6px !important;
    box-shadow: 0 4px 14px rgba(0, 0, 0, 0.8), 0 0 8px rgba(16, 185, 129, 0.4) !important;
    white-space: nowrap !important;
  }
</style>
</head>
<body>
<div class="container">
  <header>
    <div class="brand">
      <h1>🚛 San Andreas <span>Logistics Tycoon</span></h1>
    </div>
    <div class="nav-btns">
      <a href="/map" class="nav-btn"><span>🗺️</span> Live GPS Radar</a>
      <a href="/" class="nav-btn"><span>🎮</span> Vehicle Remote</a>
      <a href="/logistics" class="nav-btn active"><span>🏢</span> Tycoon</a>
      <a href="/municipal" class="nav-btn"><span>🏛️</span> City Hall</a>
      <div class="status-pill"><div class="pulse-dot"></div> ONLINE</div>
    </div>
  </header>

  <!-- Section A: Companies -->
  <div class="section-title"><span>🏢</span> Corporate Standings</div>
  <div class="companies-grid" id="companies-container">
    <!-- Populated via JS -->
  </div>

  <!-- Section: Interactive Route Editor & Logistics Corridors -->
  <div class="section-title"><span>🛣️</span> Autonomous Corridor & Route Editor</div>
  <div class="route-editor-card">
    <div class="editor-header">
      <div>
        <div style="font-size: 0.95rem; font-weight: 800; color: #f8fafc;">Interactive Multi-Company Route Dispatcher</div>
        <div style="font-size: 0.75rem; color: var(--text-muted); margin-top: 3px;">
          Click map to plot waypoints. Snaps to depots & nodes within 15.0m. Autonomous trucks bind strictly to company corridors, stage at pockets, and reverse.
        </div>
      </div>
      <div id="route-snap-badge" class="snap-badge">
        <span>🧲</span> Snapping Engine: 15.0m ON
      </div>
    </div>

    <!-- Company Selector & Editor Controls -->
    <div class="editor-toolbar">
      <div class="toolbar-group">
        <button class="comp-btn active" id="btn-comp-0" onclick="selectCompanyRoute(0)">
          <span class="comp-dot" style="background:#38bdf8;"></span> Ocean Docks (Blue)
        </button>
        <button class="comp-btn" id="btn-comp-1" onclick="selectCompanyRoute(1)">
          <span class="comp-dot" style="background:#f59e0b;"></span> Bone County (Orange)
        </button>
        <button class="comp-btn" id="btn-comp-2" onclick="selectCompanyRoute(2)">
          <span class="comp-dot" style="background:#22c55e;"></span> Agro Food (Green)
        </button>
      </div>

      <div class="toolbar-group">
        <button class="tool-btn" id="btn-route-layer" onclick="cycleRouteMapLayer()">
          <span>🗺️</span> <span id="txt-route-layer">Style: Satellite</span>
        </button>
        <button class="tool-btn active" id="btn-master-highway" onclick="toggleLogisticsHighway()">
          <span>🛣️</span> <span id="txt-master-highway">Master Highway: ON</span>
        </button>
        <button class="tool-btn active" id="btn-local-feeders" onclick="toggleLogisticsFeeders()">
          <span>🚛</span> <span id="txt-local-feeders">Local Feeders: ON</span>
        </button>
        <div class="toolbar-group" style="display:inline-flex; align-items:center; gap:6px;">
          <button class="tool-btn" id="btn-mode-facility" onclick="setEditorSubMode('facility')">
            <span>🏢</span> 1. Set Factory Hub
          </button>
          <button class="tool-btn" id="btn-mode-draw" onclick="setEditorSubMode('draw')">
            <span>✏️</span> 2. Plot Route
          </button>
          
          <select id="select-preset-facility" class="tool-btn" style="background:#131b2e; color:#f8fafc; cursor:pointer;" onchange="applyPresetFacility(this.value)">
            <option value="">-- Choose Factory Preset --</option>
            <option value="bone_oil">⛽ Bone County Refinery (-1038.1, -607.4)</option>
            <option value="flint_farm">🌾 Flint County Agro Silo (-133.5, -350.1)</option>
            <option value="ocean_docks">📦 Ocean Docks Freight Yard (2758.3, -2447.0)</option>
            <option value="custom_click">📍 Pick Custom Location on Map</option>
          </select>
        </div>
        <button class="tool-btn" onclick="undoLastWaypoint()">
          <span>↩️</span> Undo
        </button>
        <button class="tool-btn danger" onclick="clearCurrentRoute()">
          <span>🗑️</span> Clear
        </button>
        <button class="tool-btn" id="btn-copy-cpp" onclick="exportCompanyRouteCpp()">
          <span>📋</span> Copy C++ Array
        </button>
        <button class="tool-btn" id="btn-import-route" onclick="openImportModal()">📥 Import from Notepad</button>
        <button class="tool-btn deploy" onclick="saveAndDeployRoute()">
          <span>💾</span> Save & Deploy Route
        </button>
      </div>
    </div>

    <!-- Route Stats & Live Toast Info -->
    <div class="route-meta-bar">
      <div class="meta-item">Selected: <b id="meta-company-name" style="color:#38bdf8;">Ocean Docks Logistics</b></div>
      <div class="meta-item">Waypoints: <b id="meta-node-count">0</b></div>
      <div class="meta-item">Route Length: <b id="meta-route-dist">0.00 km</b></div>
      <div class="meta-item">Bound Fleet: <b id="meta-truck-count">8 Trucks</b></div>
      <div id="editor-toast" class="editor-toast">Ready. Click 'Edit Route' to plot waypoints.</div>
    </div>

    <!-- Leaflet Route Editor Map Canvas -->
    <div id="route-editor-map" style="height: 520px; width: 100%; border-radius: 10px; background: #060a14; border: 1px solid var(--card-border); margin-top: 12px; position: relative;"></div>
  </div>


  <!-- Section B: Real-Estate Assets -->
  <div class="section-title"><span>🏛️</span> Real-Estate Asset Portfolio</div>
  <div class="assets-grid" id="assets-container">
    <!-- Populated via JS -->
  </div>

  <!-- Section: San Fierro Port Supply Depot -->
  <div class="section-title"><span>⚓</span> San Fierro Port Warehouse Inventory</div>
  <div class="sf-depot-card">
    <div class="sf-depot-header">
      <div style="font-size: 0.85rem; color: var(--text-muted);">
        Regional commodity depot at San Fierro Terminal (Waypoint 240). Consumed over time by local city commerce.
      </div>
      <div id="sf-bonus-badge">
        <!-- Bonus indicator -->
      </div>
    </div>
    <div class="sf-grid" id="sf-supplies-container">
      <!-- Populated via JS -->
    </div>
  </div>

  <!-- Section: Retail Market & Commercial Network (20 Stores) -->
  <div class="section-title"><span>🏪</span> Retail Outlets & Commercial Network (20 Locations)</div>
  <div class="sf-depot-card">
    <div style="font-size: 0.82rem; color: var(--text-muted); margin-bottom: 12px;">
      Autonomous commercial stores with independent capital reserves. Stores fund their own wholesale restocks from San Fierro central depots and remit sales tax directly to the City Treasury.
    </div>

    <!-- District Telemetry & Supply Status Summary -->
    <div id="district-summary-container" style="display:grid; grid-template-columns:repeat(auto-fit, minmax(230px, 1fr)); gap:10px; margin-bottom:14px;">
      <!-- Populated dynamically via JS -->
    </div>

    <div class="retail-tabs">
      <button class="retail-tab-btn active" onclick="filterRetailStores('all', this)">All Stores (20)</button>
      <button class="retail-tab-btn" onclick="filterRetailStores(0, this)">🥗 Food & Grocery (5)</button>
      <button class="retail-tab-btn" onclick="filterRetailStores(1, this)">⛽ Fuel Stations (5)</button>
      <button class="retail-tab-btn" onclick="filterRetailStores(2, this)">🪵 Lumber & Timber (5)</button>
      <button class="retail-tab-btn" onclick="filterRetailStores(3, this)">💻 Tech & Defense (5)</button>
    </div>
    <div class="retail-tabs" style="margin-top:6px; margin-bottom:12px;">
      <button class="retail-dist-btn active" onclick="filterRetailDist('all', this)" style="background:rgba(255,255,255,0.05); border:1px solid rgba(255,255,255,0.1); color:#94a3b8; border-radius:6px; padding:5px 10px; font-size:0.72rem; cursor:pointer; font-weight:700;">All Districts</button>
      <button class="retail-dist-btn" onclick="filterRetailDist(0, this)" style="background:rgba(255,255,255,0.05); border:1px solid rgba(255,255,255,0.1); color:#94a3b8; border-radius:6px; padding:5px 10px; font-size:0.72rem; cursor:pointer; font-weight:700;">📍 South Central</button>
      <button class="retail-dist-btn" onclick="filterRetailDist(1, this)" style="background:rgba(255,255,255,0.05); border:1px solid rgba(255,255,255,0.1); color:#94a3b8; border-radius:6px; padding:5px 10px; font-size:0.72rem; cursor:pointer; font-weight:700;">📍 Downtown</button>
      <button class="retail-dist-btn" onclick="filterRetailDist(2, this)" style="background:rgba(255,255,255,0.05); border:1px solid rgba(255,255,255,0.1); color:#94a3b8; border-radius:6px; padding:5px 10px; font-size:0.72rem; cursor:pointer; font-weight:700;">📍 Industrial Port</button>
      <button class="retail-dist-btn" onclick="filterRetailDist(3, this)" style="background:rgba(255,255,255,0.05); border:1px solid rgba(255,255,255,0.1); color:#94a3b8; border-radius:6px; padding:5px 10px; font-size:0.72rem; cursor:pointer; font-weight:700;">📍 Country / Hwy</button>
    </div>
    <div class="retail-grid" id="retail-stores-container">
      <!-- Populated dynamically via JS -->
    </div>
  </div>

  <!-- Section C: Driver Roster Table -->
  <div class="section-title"><span>🧑‍✈️</span> Fleet Telemetry & Driver Roster (24 Trucks)</div>
  <div class="table-card">
    <div class="table-responsive">
      <table>
        <thead>
          <tr>
            <th>Driver</th>
            <th>Rank & XP</th>
            <th>Company</th>
            <th>Truck ID</th>
            <th>Status</th>
            <th>Fuel</th>
            <th>Fatigue</th>
            <th>Cargo</th>
            <th>Deliveries</th>
            <th>Driver Wallet</th>
          </tr>
        </thead>
        <tbody id="drivers-tbody">
          <!-- Populated via JS -->
        </tbody>
      </table>
    </div>
  </div>
</div>

<!-- Universal Import Route Modal -->
<div id="import-route-modal" class="modal-overlay" onclick="handleModalBackdropClick(event)">
  <div class="modal-card" onclick="event.stopPropagation()">
    <div class="modal-header">
      <div class="modal-title"><span>📥</span> Import Route from Notepad</div>
      <button class="modal-close" onclick="closeImportModal()">&times;</button>
    </div>
    <div class="modal-body">
      <div class="modal-hint">
        Paste coordinates directly from Notepad. Supports both <b>C++ coordinate arrays</b> (e.g. <code>{ 123.4f, -567.8f, 15.0f }</code>) and <b>JSON arrays</b> (e.g. <code>[{"x":123.4,"y":-567.8}]</code>).
      </div>
      <textarea id="import-route-textarea" class="import-textarea" placeholder="Paste C++ array or JSON here...&#10;&#10;Examples:&#10;{ 2490.5f, -2120.4f, 13.5f },&#10;{ 2510.0f, -2100.2f, 13.5f }&#10;&#10;or&#10;[{&quot;x&quot;: 2490.5, &quot;y&quot;: -2120.4, &quot;z&quot;: 13.5 }]"></textarea>
    </div>
    <div class="modal-footer">
      <button class="tool-btn" onclick="closeImportModal()">Cancel</button>
      <button class="tool-btn deploy" onclick="submitImportedRoute()"><span>📥</span> Apply to Route</button>
    </div>
  </div>
</div>

<script>
async function refreshDashboard() {
  try {
    const res = await fetch('/api/status');
    if (!res.ok) return;
    const data = await res.json();

    g_latestLogisticsData = data;
    if (g_logisticsStrategicMarkers && g_logisticsStrategicMarkers.length > 0) {
      g_logisticsStrategicMarkers.forEach(item => {
        if (item.marker && item.marker.isPopupOpen()) {
          item.marker.setPopupContent(item.renderPopup());
        }
      });
    }

    // 1. Render Companies
    if (data.companies && Array.isArray(data.companies)) {
      const compContainer = document.getElementById('companies-container');
      compContainer.innerHTML = data.companies.map(c => {
        const hasPerk = c.ownedAssets > 0;
        return `
          <div class="company-card" style="border-color:${c.color}33;">
            <div class="card-top-bar" style="background:${c.color};"></div>
            <div class="company-header">
              <div>
                <div class="company-name" style="color:${c.color};">${c.name}</div>
                <div class="company-hub">📍 Base Hub: ${c.baseHub}</div>
              </div>
              <span class="company-pill" style="background:${c.color}22; color:${c.color}; border:1px solid ${c.color}44;">ID #${c.id}</span>
            </div>
            <div class="company-balance">$${Number(c.balance).toLocaleString()}</div>
            <div class="company-stats">
              <div>
                <div class="stat-label">Active Fleet</div>
                <div class="stat-val">🚚 ${c.truckCount} Trucks</div>
              </div>
              <div>
                <div class="stat-label">Real Estate</div>
                <div class="stat-val">🏛️ ${c.ownedAssets} Owned</div>
              </div>
            </div>
            ${hasPerk ? `<div class="perk-badge">⚡ +25% Delivery Payout Bonus Active</div>` : `<div style="font-size:0.7rem; color:#64748b; margin-top:10px;">No facilities acquired yet</div>`}
          </div>
        `;
      }).join('');
    }

    // 2. Render Real-Estate Assets
    if (data.assets && Array.isArray(data.assets)) {
      const assetContainer = document.getElementById('assets-container');
      assetContainer.innerHTML = data.assets.map(a => {
        const isOwned = a.ownerId >= 0;
        const ownerComp = (data.companies && isOwned) ? data.companies[a.ownerId] : null;
        return `
          <div class="asset-card">
            <div class="asset-head">
              <div class="asset-title">${a.name}</div>
              <div class="asset-cost">$${Number(a.cost).toLocaleString()}</div>
            </div>
            <div style="font-size:0.75rem; color:#94a3b8; margin:6px 0;">⚡ Strategic Perk: Grants +25% bonus on all completed freight deliveries.</div>
            <div class="asset-owner-status" style="${isOwned && ownerComp ? `background:${ownerComp.color}22; border:1px solid ${ownerComp.color}44; color:${ownerComp.color};` : `background:rgba(255,255,255,0.03); border:1px solid rgba(255,255,255,0.08); color:#64748b;`}">
              <span>${isOwned && ownerComp ? `🏢 Owned by ${ownerComp.name}` : `🏛️ Available for Acquisition`}</span>
              <span>${isOwned ? '✅ Active' : 'Market Open'}</span>
            </div>
          </div>
        `;
      }).join('');
    }

    // 3. Render San Fierro Port Warehouse Inventory
    if (data.sf_supplies) {
      const s = data.sf_supplies;
      const maxCap = 250;
      const commodities = [
        { name: 'Timber / Wood', icon: '🌲', val: s.timber || 0, color: '#10b981' },
        { name: 'Fuel / Oil', icon: '⛽', val: s.fuel || 0, color: '#f59e0b' },
        { name: 'Electronics', icon: '💡', val: s.electronics || 0, color: '#38bdf8' },
        { name: 'Food & Groceries', icon: '🍔', val: s.food || 0, color: '#ec4899' }
      ];

      const isWellStocked = (s.food > 200 || s.electronics > 200);
      const bonusBadge = document.getElementById('sf-bonus-badge');
      if (bonusBadge) {
        bonusBadge.innerHTML = isWellStocked
          ? `<span class="sf-bonus-pill" style="background:rgba(34,197,94,0.15); border:1px solid rgba(34,197,94,0.4); color:#4ade80;">🌟 Well-Stocked City (+ $100 Driver Tip)</span>`
          : `<span class="sf-bonus-pill" style="background:rgba(255,255,255,0.04); border:1px solid rgba(255,255,255,0.08); color:#94a3b8;">Depot Operational</span>`;
      }

      const sfContainer = document.getElementById('sf-supplies-container');
      if (sfContainer) {
        sfContainer.innerHTML = commodities.map(c => {
          const pct = Math.min(100, Math.round((c.val / maxCap) * 100));
          return `
            <div class="sf-item">
              <div class="sf-item-head">
                <span>${c.icon} ${c.name}</span>
                <span class="sf-stock-val">${c.val}t <span style="color:#64748b; font-size:0.7rem;">(${pct}%)</span></span>
              </div>
              <div class="progress-wrap" style="margin-top:6px;">
                <div class="bar-bg" style="height:7px;">
                  <div class="bar-fill" style="background:${c.color}; width:${pct}%;"></div>
                </div>
              </div>
            </div>
          `;
        }).join('');
      }
    }

    // 4. Render Drivers Roster
    if (data.trucks && Array.isArray(data.trucks)) {
      const tbody = document.getElementById('drivers-tbody');
      tbody.innerHTML = data.trucks.map(t => {
        const isDestroyed = (t.state === 'DESTROYED' || t.destroyed);
        let stateHtml;
        if (isDestroyed) {
          stateHtml = `<span class="state-badge" style="background:#ef444433; color:#f87171; border:1px solid #ef444466;">💥 WRECKED</span>`;
        } else if (t.state === 'BROKEN_DOWN') {
          stateHtml = `<span class="state-badge state-broken">⚠️ BROKEN DOWN <span style="color:#f59e0b; font-size:0.85rem; margin-left:2px;">🔧</span></span>`;
        } else if (t.state === 'RESTING') {
          stateHtml = `<span class="state-badge" style="background:#3b82f622; color:#60a5fa; border:1px solid #3b82f644;">💤 RESTING</span>`;
        } else if (t.state === 'LOADING') {
          stateHtml = `<span class="state-badge" style="background:#f9731622; color:#fb923c; border:1px solid #f9731644;">🏭 LOADING</span>`;
        } else if (t.state === 'UNLOADING') {
          stateHtml = `<span class="state-badge" style="background:#a855f722; color:#c084fc; border:1px solid #a855f744;">📦 UNLOADING</span>`;
        } else {
          stateHtml = `<span class="state-badge" style="background:#22c55e22; color:#4ade80; border:1px solid #22c55e44;">🚛 EN ROUTE</span>`;
        }

        const fuelColor = t.fuel < 20 ? '#ef4444' : '#22c55e';
        const fatigueColor = t.fatigue > 75 ? '#ef4444' : (t.fatigue > 40 ? '#f59e0b' : '#38bdf8');

        const lvl = t.level || 1;
        const xp = t.xp || 0;
        let rankName = "Rookie";
        let rankColor = "#94a3b8";
        let rankIcon = "🔰";
        let xpNext = 300;
        let xpBase = 0;
        if (lvl === 2) {
          rankName = "Veteran";
          rankColor = "#38bdf8";
          rankIcon = "🎖️";
          xpBase = 300;
          xpNext = 800;
        } else if (lvl >= 3) {
          rankName = "Master";
          rankColor = "#f59e0b";
          rankIcon = "⭐";
          xpBase = 800;
          xpNext = 800;
        }
        let xpPct = 100;
        if (lvl < 3) {
          xpPct = Math.min(100, Math.max(0, Math.round(((xp - xpBase) / (xpNext - xpBase)) * 100)));
        }

        return `
          <tr style="${isDestroyed ? 'background: rgba(239, 68, 68, 0.15); border-left: 3px solid #ef4444;' : ''}">
            <td>
              <div class="driver-cell">
                <div class="driver-avatar">${(t.driver_name || 'D')[0]}</div>
                <div>${t.driver_name || 'Driver'}</div>
              </div>
            </td>
            <td>
              <div style="display:flex; flex-direction:column; gap:4px; min-width:115px;">
                <div style="display:flex; justify-content:space-between; align-items:center;">
                  <span class="rank-badge" style="background:${rankColor}22; color:${rankColor}; border:1px solid ${rankColor}44;">
                    ${rankIcon} ${rankName}
                  </span>
                  <span style="font-size:0.7rem; color:#94a3b8; font-weight:700;">${xp} XP</span>
                </div>
                <div class="bar-bg" style="height:5px;">
                  <div class="bar-fill" style="background:${rankColor}; width:${xpPct}%;"></div>
                </div>
              </div>
            </td>
            <td>
              <span class="company-pill" style="background:${t.company_color}22; color:${t.company_color}; border:1px solid ${t.company_color}44;">
                ${t.company_name || 'Company'}
              </span>
            </td>
            <td><b style="color:#f1f5f9;">#${t.id}</b> <span style="color:#64748b; font-size:0.7rem;">[${t.materialized ? '3D' : 'Sim'}]</span></td>
            <td>${stateHtml}</td>
            <td>
              <div class="progress-wrap">
                <div class="bar-bg"><div class="bar-fill" style="background:${fuelColor}; width:${Math.min(100, Math.max(0, t.fuel))}%;"></div></div>
                <span style="font-size:0.75rem; font-weight:700;">${t.fuel}%</span>
              </div>
            </td>
            <td>
              <div class="progress-wrap">
                <div class="bar-bg"><div class="bar-fill" style="background:${fatigueColor}; width:${Math.min(100, Math.max(0, t.fatigue))}%;"></div></div>
                <span style="font-size:0.75rem; font-weight:700;">${t.fatigue}%</span>
              </div>
            </td>
            <td><b style="color:#e2e8f0;">${t.cargo}</b> <span style="color:#94a3b8; font-size:0.75rem;">(${t.cargo_tons}t)</span></td>
            <td><b>${t.deliveries}</b></td>
            <td><b style="color:#38bdf8;">$${Number(t.driver_wallet || 0).toLocaleString()}</b></td>
          </tr>
        `;
      }).join('');
      if (typeof updateTruckMapOverlays === 'function') {
        updateTruckMapOverlays(data.trucks);
      }
    }
  } catch (err) {
    console.error('Error polling dashboard:', err);
  }
}

setInterval(refreshDashboard, 500);
refreshDashboard();

let g_currentRetailCat = 'all';
let g_currentRetailDist = 'all';
let g_lastStoresData = [];
let g_lastMacroData = {};
let g_lastDistrictsData = [];
const g_expandedStores = new Set();

function toggleStoreDetail(storeId, ev) {
  if (ev) {
    ev.stopPropagation();
    ev.preventDefault();
  }
  const idNum = Number(storeId);
  if (g_expandedStores.has(idNum)) {
    g_expandedStores.delete(idNum);
  } else {
    g_expandedStores.add(idNum);
  }
  const drawer = document.getElementById('store-detail-' + idNum);
  const btn = document.getElementById('store-btn-' + idNum);
  if (drawer) {
    if (g_expandedStores.has(idNum)) {
      drawer.classList.remove('collapsed');
      drawer.classList.add('expanded');
      if (btn) btn.innerHTML = '▲ Свернуть';
    } else {
      drawer.classList.remove('expanded');
      drawer.classList.add('collapsed');
      if (btn) btn.innerHTML = 'ℹ️ Детали';
    }
  }
}

function filterRetailStores(cat, btn) {
  g_currentRetailCat = cat;
  document.querySelectorAll('.retail-tab-btn').forEach(b => b.classList.remove('active'));
  if (btn) btn.classList.add('active');
  renderRetailStores();
}

function filterRetailDist(dist, btn) {
  g_currentRetailDist = dist;
  document.querySelectorAll('.retail-dist-btn').forEach(b => {
    b.style.borderColor = 'rgba(255,255,255,0.1)';
    b.style.color = '#94a3b8';
    b.style.background = 'rgba(255,255,255,0.05)';
  });
  if (btn) {
    btn.style.borderColor = '#38bdf8';
    btn.style.color = '#38bdf8';
    btn.style.background = 'rgba(56,189,248,0.15)';
  }
  renderRetailStores();
}

const distMeta = [
  { name: "South Central", spec: "Primary Demand: Food & Ammunition", col: "#f43f5e", bg: "rgba(244,63,94,0.15)", border: "rgba(244,63,94,0.35)" },
  { name: "Downtown", spec: "Primary Demand: High-Octane Fuel & Tech", col: "#38bdf8", bg: "rgba(56,189,248,0.15)", border: "rgba(56,189,248,0.35)" },
  { name: "Industrial Port", spec: "Primary Demand: Industrial Lumber & Heavy Fuel", col: "#eab308", bg: "rgba(234,179,8,0.15)", border: "rgba(234,179,8,0.35)" },
  { name: "Country / Hwy", spec: "Primary Demand: Transit Fuel & Rural Timber", col: "#10b981", bg: "rgba(16,185,129,0.15)", border: "rgba(16,185,129,0.35)" }
];

function renderDistrictSummaries() {
  const container = document.getElementById('district-summary-container');
  if (!container || !g_lastStoresData || g_lastStoresData.length === 0) return;

  const crime = (g_lastMacroData && typeof g_lastMacroData.crime === 'number') ? g_lastMacroData.crime : 25.0;
  const unrest = (g_lastMacroData && typeof g_lastMacroData.unrest === 'number') ? g_lastMacroData.unrest : 10.0;
  const curfew = (g_lastMacroData && g_lastMacroData.curfew) ? true : false;
  const trafficImpactPct = curfew ? 30 : Math.round(Math.max(35, Math.min(120, (1.0 - (crime * 0.4 + unrest * 0.4) / 100.0) * 100)));

  const dStats = [0, 1, 2, 3].map(dId => {
    const stores = g_lastStoresData.filter(s => s.dist === dId);
    const essStores = stores.filter(s => s.cat === 0 || s.cat === 1);
    const totalEssStock = essStores.reduce((acc, s) => acc + (s.stock || 0), 0);
    const totalEssCap = essStores.reduce((acc, s) => acc + (s.cap || 120), 0);
    const essPct = totalEssCap > 0 ? Math.round((totalEssStock / totalEssCap) * 100) : 100;
    const totalRate = stores.reduce((acc, s) => acc + (s.rate || 0), 0);
    return { dId, totalEssStock, totalEssCap, essPct, totalRate };
  });

  container.innerHTML = dStats.map(st => {
    const dm = distMeta[st.dId] || distMeta[0];
    const isCritical = st.essPct < 25;
    const isSurplus = st.essPct > 70;
    const statusText = isCritical ? '⚠️ STARVATION RISK' : (isSurplus ? '🛡️ ABUNDANT' : '⚖️ BALANCED');
    const statusCol = isCritical ? '#ef4444' : (isSurplus ? '#22c55e' : '#38bdf8');
    const statusBg = isCritical ? 'rgba(239,68,68,0.15)' : (isSurplus ? 'rgba(34,197,94,0.15)' : 'rgba(56,189,248,0.15)');

    const isPlayerPresent = (g_lastMacroData && typeof g_lastMacroData.playerDist === 'number' && st.dId === g_lastMacroData.playerDist);
    const cjBadge = isPlayerPresent
      ? `<span style="font-size:0.65rem; font-weight:800; padding:2px 6px; border-radius:4px; background:rgba(6,182,212,0.2); color:#06b6d4; border:1px solid #06b6d4; box-shadow:0 0 8px rgba(6,182,212,0.6);">👤 CJ Present</span>`
      : '';

    return `
      <div style="background:rgba(15,23,42,0.65); border:1px solid ${dm.border}; border-radius:10px; padding:10px 12px; display:flex; flex-direction:column; gap:6px;">
        <div style="display:flex; justify-content:space-between; align-items:flex-start;">
          <div style="display:flex; flex-direction:column; gap:2px;">
            <span style="font-size:0.8rem; font-weight:800; color:${dm.col};">📍 ${dm.name}</span>
            <span style="font-size:0.65rem; font-weight:700; color:#fbbf24;">${dm.spec}</span>
          </div>
          <div style="display:flex; align-items:center; gap:4px;">
            ${cjBadge}
            <span style="font-size:0.65rem; font-weight:800; padding:2px 6px; border-radius:4px; background:${statusBg}; color:${statusCol}; border:1px solid ${statusCol}40;">${statusText}</span>
          </div>
        </div>
        <div style="display:grid; grid-template-columns:1fr 1fr; gap:6px; font-size:0.7rem; color:#94a3b8;">
          <div>Supply: <b style="color:${statusCol};">${st.essPct}%</b> <span style="font-size:0.62rem;">(${st.totalEssStock}/${st.totalEssCap})</span></div>
          <div>Traffic: <b style="color:${trafficImpactPct < 70 ? '#f59e0b' : '#38bdf8'};">${trafficImpactPct}%</b></div>
          <div>Velocity: <b style="color:#f8fafc;">${st.totalRate.toFixed(1)} u/m</b></div>
          <div>Crime Impact: <b style="color:${crime > 50 ? '#ef4444' : '#22c55e'};">${crime > 50 ? 'Panic Buy' : 'Normal'}</b></div>
        </div>
      </div>
    `;
  }).join('');
}

function renderRetailStores() {
  const container = document.getElementById('retail-stores-container');
  if (!container || !g_lastStoresData || g_lastStoresData.length === 0) return;

  renderDistrictSummaries();

  const catMeta = [
    { tag: "Grocery", col: "#10b981", bg: "rgba(16,185,129,0.15)", border: "rgba(16,185,129,0.3)" },
    { tag: "Fuel Station", col: "#f59e0b", bg: "rgba(245,158,11,0.15)", border: "rgba(245,158,11,0.3)" },
    { tag: "Lumber Yard", col: "#a855f7", bg: "rgba(168,85,247,0.15)", border: "rgba(168,85,247,0.3)" },
    { tag: "Tech Depot", col: "#38bdf8", bg: "rgba(56,189,248,0.15)", border: "rgba(56,189,248,0.3)" }
  ];

  const taxSalesGen = (g_lastMacroData && typeof g_lastMacroData.taxSalesGen === 'number') ? g_lastMacroData.taxSalesGen : 0.12;
  const taxSalesLux = (g_lastMacroData && typeof g_lastMacroData.taxSalesLux === 'number') ? g_lastMacroData.taxSalesLux : 0.25;

  const filtered = g_lastStoresData.filter(s => {
    const catMatch = g_currentRetailCat === 'all' || s.cat === Number(g_currentRetailCat);
    const distMatch = g_currentRetailDist === 'all' || s.dist === Number(g_currentRetailDist);
    return catMatch && distMatch;
  });

  const raidBannerHtml = (g_lastTelemetryData && g_lastTelemetryData.isUnderRaid) ? `
    <div style="grid-column: 1 / -1; background:rgba(239,68,68,0.22); border:2px solid #ef4444; border-radius:8px; padding:12px 16px; margin-bottom:8px; display:flex; align-items:center; justify-content:space-between; animation:pulse 1s infinite;">
      <div style="display:flex; align-items:center; gap:12px;">
        <span style="font-size:1.6rem;">🚨</span>
        <div>
          <div style="font-weight:800; color:#fca5a5; font-size:0.95rem; text-transform:uppercase;">CRITICAL INCIDENT: Store Raid in Progress!</div>
          <div style="font-size:0.78rem; color:#fecaca; margin-top:2px;">Target: <b style="color:#fff;">${g_lastTelemetryData.raidedStoreName}</b> (Store #${g_lastTelemetryData.raidedStoreId}) | Defend local stock: <b>${g_lastTelemetryData.localStock} units remaining</b></div>
        </div>
      </div>
      <span class="badge" style="background:#ef4444; color:#fff; font-weight:800; padding:5px 12px; font-size:0.75rem; letter-spacing:0.5px;">ACTIVE LOOTERS</span>
    </div>
  ` : '';

  container.innerHTML = raidBannerHtml + filtered.map(s => {
    const meta = catMeta[s.cat] || catMeta[0];
    const dm = distMeta[s.dist] || distMeta[0];
    const cap = s.cap || 120;
    const pct = Math.min(100, Math.max(0, Math.round((s.stock / cap) * 100)));
    const barColor = pct < 25 ? '#ef4444' : (pct < 45 ? '#f59e0b' : '#22c55e');
    const rateVal = (typeof s.rate === 'number') ? s.rate.toFixed(1) : '0.0';

    const isLuxury = (s.cat === 3);
    const sphereName = isLuxury ? "Surplus / Tech & Arms" : "General Retail";
    const taxRateVal = isLuxury ? taxSalesLux : taxSalesGen;
    const taxPct = (taxRateVal * 100).toFixed(0);
    const taxType = isLuxury ? "Luxury Surtax" : "General VAT";

    const avgUnitPrice = (s.sold > 0 && s.rev > 0) ? (s.rev / s.sold) : (isLuxury ? 120 : (s.cat === 1 ? 42 : (s.cat === 2 ? 65 : 35)));
    const rollingTurnover = Math.round((s.win || 0) * avgUnitPrice);

    const score = Number(s.score || 0);
    const scoreColor = score >= 700 ? '#22c55e' : (score >= 500 ? '#f59e0b' : '#ef4444');
    const scoreRating = score >= 700 ? 'Prime' : (score >= 500 ? 'Fair' : 'Subprime');

    const isExpanded = g_expandedStores.has(s.id);

    return `
      <div class="retail-card store-card" id="store-card-${s.id}">
        ${s.isUnderRaid ? `
          <div style="background:rgba(239,68,68,0.25); border:1px solid #ef4444; border-radius:6px; padding:4px 8px; color:#fca5a5; font-size:0.72rem; font-weight:800; text-align:center; margin-bottom:8px; animation:pulse 1s infinite;">
            🚨 RAID IN PROGRESS — LOOTERS ATTACKING STORE
          </div>` : (s.isRansacked ? `
          <div style="background:rgba(245,158,11,0.2); border:1px solid #f59e0b; border-radius:6px; padding:4px 8px; color:#fcd34d; font-size:0.72rem; font-weight:800; text-align:center; margin-bottom:8px;">
            ⚠️ STORE RANSACKED — TRADING SUSPENDED
          </div>` : '')}
        <div class="retail-card-top">
          <div style="display:flex; flex-direction:column; gap:2px;">
            <span class="retail-card-title">${s.name}</span>
            <div style="display:flex; gap:4px; align-items:center; flex-wrap:wrap;">
              <span class="district-badge" style="display:inline-block; width:fit-content; font-size:0.65rem; font-weight:700; color:${dm.col}; background:${dm.bg}; border:1px solid ${dm.border}; padding:1px 6px; border-radius:4px;">📍 ${dm.name}</span>
              <span class="sphere-badge" style="display:inline-block; font-size:0.63rem; font-weight:700; color:${isLuxury ? '#c084fc' : '#38bdf8'}; background:${isLuxury ? 'rgba(192,132,252,0.12)' : 'rgba(56,189,248,0.12)'}; border:1px solid ${isLuxury ? 'rgba(192,132,252,0.3)' : 'rgba(56,189,248,0.3)'}; padding:1px 6px; border-radius:4px;">🏛️ ${sphereName}</span>
            </div>
            <span style="font-size:0.62rem; font-weight:700; color:#fbbf24; margin-top:1px;">${dm.spec}</span>
          </div>
          <div style="display:flex; flex-direction:column; align-items:flex-end; gap:6px;">
            <span class="retail-cat-badge" style="background:${meta.bg}; color:${meta.col}; border:1px solid ${meta.border};">${meta.tag}</span>
            <button class="btn-store-detail" id="store-btn-${s.id}" onclick="toggleStoreDetail(${s.id}, event)" title="Показать детальные финансовые и регуляторные метрики">${isExpanded ? '▲ Свернуть' : 'ℹ️ Детали'}</button>
          </div>
        </div>
        <div class="retail-bal-row">
          <div style="display:flex; align-items:center; gap:8px;">
            <span style="color:#94a3b8; font-weight:700; text-transform:uppercase; font-size:0.68rem;">Capital Balance</span>
            <span class="badge" style="padding:2px 8px; font-size:0.7rem; background:rgba(56,189,248,0.15); color:#38bdf8; border:1px solid rgba(56,189,248,0.3);">Rate: ${rateVal}/m</span>
          </div>
          <span class="retail-bal-val">$${Number(s.bal || 0).toLocaleString()}</span>
        </div>
        <div class="progress-wrap">
          <div class="bar-bg"><div class="bar-fill" style="background:${barColor}; width:${pct}%;"></div></div>
          <span style="font-size:0.75rem; font-weight:700; color:${barColor};">${s.stock}/${cap} (${pct}%)</span>
        </div>
        <div class="retail-metrics">
          <span>Sold: <b>${Number(s.sold || 0).toLocaleString()}</b></span>
          <span>Rev: <b style="color:#22c55e;">$${Number(s.rev || 0).toLocaleString()}</b></span>
          <span>Tax: <b style="color:#f59e0b;">$${Number(s.tax || 0).toLocaleString()} (${taxPct}%)</b></span>
        </div>
        <div class="store-detail-drawer ${isExpanded ? 'expanded' : 'collapsed'}" id="store-detail-${s.id}">
          <div class="drawer-title-row">
            <div style="display:flex; align-items:center; gap:6px;">
              <span style="font-size:0.8rem;">📊</span>
              <span class="drawer-title">${s.name} — Regulatory & Fleeca Details</span>
            </div>
            <button class="btn-drawer-collapse" onclick="toggleStoreDetail(${s.id}, event)">▲ Свернуть</button>
          </div>
          <div class="drawer-grid">
            <div class="drawer-item">
              <span class="drawer-label">Sphere / Category</span>
              <span class="drawer-value" style="color:${isLuxury ? '#c084fc' : '#38bdf8'};">${sphereName} • ${meta.tag} (ID: ${s.cat})</span>
            </div>
            <div class="drawer-item">
              <span class="drawer-label">Effective Tax & Type</span>
              <span class="drawer-value" style="color:#f59e0b;">${taxPct}% <span style="font-size:0.65rem; color:#94a3b8; font-weight:normal;">(${taxType})</span></span>
            </div>
            <div class="drawer-item">
              <span class="drawer-label">Rolling 60s Base</span>
              <span class="drawer-value">~$${rollingTurnover.toLocaleString()} <span style="font-size:0.65rem; color:#94a3b8; font-weight:normal;">(${Number(s.win || 0)} units)</span></span>
            </div>
            <div class="drawer-item">
              <span class="drawer-label">Fleeca Loan Debt</span>
              <span class="drawer-value" style="color:${Number(s.debt || 0) > 0 ? '#ef4444' : '#22c55e'};">$${Number(s.debt || 0).toLocaleString()}</span>
            </div>
            <div class="drawer-item">
              <span class="drawer-label">Credit Score</span>
              <span class="drawer-value" style="color:${scoreColor};">${Number(s.score || 0)} / 1000 <span style="font-size:0.65rem; font-weight:normal;">(${scoreRating})</span></span>
            </div>
            <div class="drawer-item">
              <span class="drawer-label">Restock / Batch Size</span>
              <span class="drawer-value">Threshold: <b>${Number(s.thresh || 90)}</b> | Batch: <b>${Number(s.batch || 100)}</b></span>
            </div>
          </div>
          ${s.bankrupt ? '<div style="background:rgba(239,68,68,0.15); border:1px solid rgba(239,68,68,0.4); border-radius:6px; padding:4px 8px; color:#f87171; font-size:0.65rem; font-weight:800; text-align:center;">⚠️ STORE BANKRUPT — FLEECA RECEIVERSHIP ACTIVE</div>' : ''}
        </div>
      </div>
    `;
  }).join('');
}

let g_lastTelemetryData = null;

async function fetchTelemetry() {
  try {
    const res = await fetch('/telemetry');
    if (!res.ok) return;
    const data = await res.json();
    g_lastTelemetryData = data;
    if (data.macro) g_lastMacroData = data.macro;
    if (data.districts) g_lastDistrictsData = data.districts;
    if (data.stores && Array.isArray(data.stores)) {
      g_lastStoresData = data.stores;
      renderRetailStores();
    }
    if (typeof renderAllRoadblocks === 'function') {
      renderAllRoadblocks(data.roadblocks, data.macro);
    }
  } catch (e) {}
}

setInterval(fetchTelemetry, 2000);
fetchTelemetry();

// =============================================================================
//  Interactive Leaflet Route Editor & Corridor Planner Engine
// =============================================================================

const WORLD_BOUNDS = [[0, 0], [6000, 6000]];

function gameToMap(x, y) {
  return [y + 3000.0, x + 3000.0];
}

function mapToGame(lat, lng) {
  return [lng - 3000.0, lat - 3000.0];
}

const STRATEGIC_HUBS = [
  {
    id: 'ocean_docks',
    name: 'Ocean Docks Mega-Terminal',
    subtitle: 'Central Transfer Hub & Weigh Station',
    icon: '🏭',
    x: 2312.2,
    y: -2252.0,
    getDetails: (data) => {
      const food = data?.port_supplies?.food ?? 120;
      const fuel = data?.port_supplies?.fuel ?? 120;
      const q = data?.trucks?.filter(t => t.current_node === 0 || Math.hypot((t.pos ? t.pos.x : (t.x || 0)) - 2312.2, (t.pos ? t.pos.y : (t.y || 0)) - (-2252.0)) < 350.0).length ?? 0;
      return `
        <div class="hub-metric-row"><span class="lbl">Port Food Stock:</span><span class="val food">${food} / 500 u</span></div>
        <div class="hub-metric-row"><span class="lbl">Port Fuel Stock:</span><span class="val fuel">${fuel} / 500 u</span></div>
        <div class="hub-metric-row"><span class="lbl">Loading Bay Queue:</span><span class="val queue">${q} Trucks Active</span></div>
        <div class="hub-role">Central state interchange: Receives local feeder shuttles from Flint Farm & Bone Wells; loads 24 Master Interstate Haulers for San Fierro delivery.</div>
      `;
    }
  },
  {
    id: 'flint_farm',
    name: 'Flint County Farm Depot',
    subtitle: 'Agro Loading & Harvest Hub',
    icon: '🌾',
    x: -74.8,
    y: -1142.2,
    getDetails: (data) => {
      const shuttles = data?.trucks?.filter(t => t.id >= 24 && t.id <= 26) ?? [];
      const totalCargo = shuttles.reduce((acc, t) => acc + (t.cargo_units || 0), 0);
      return `
        <div class="hub-metric-row"><span class="lbl">Commodity:</span><span class="val food">Fresh Agro Produce</span></div>
        <div class="hub-metric-row"><span class="lbl">Dedicated Feeders:</span><span class="val">3 Shuttles (#24..#26)</span></div>
        <div class="hub-metric-row"><span class="lbl">In-Transit Freight:</span><span class="val food">${totalCargo} Units Agro</span></div>
        <div class="hub-role">Local agricultural production supplying Ocean Docks export terminal via custom snapped supply line.</div>
      `;
    }
  },
  {
    id: 'bone_wells',
    name: 'Bone County Oil Field',
    subtitle: 'Fuel Extraction & Refining Well',
    icon: '⛽',
    x: 342.1,
    y: 1422.3,
    getDetails: (data) => {
      const shuttles = data?.trucks?.filter(t => t.id >= 27 && t.id <= 29) ?? [];
      const totalCargo = shuttles.reduce((acc, t) => acc + (t.cargo_units || 0), 0);
      return `
        <div class="hub-metric-row"><span class="lbl">Commodity:</span><span class="val fuel">Industrial Fuel / Octane</span></div>
        <div class="hub-metric-row"><span class="lbl">Dedicated Feeders:</span><span class="val">3 Tankers (#27..#29)</span></div>
        <div class="hub-metric-row"><span class="lbl">In-Transit Freight:</span><span class="val fuel">${totalCargo} Units Fuel</span></div>
        <div class="hub-role">Continuous petrochemical extraction shuttles directly restocking Ocean Docks fuel atomics.</div>
      `;
    }
  },
  {
    id: 'weigh_station',
    name: 'Interstate Weigh Station',
    subtitle: 'Safety & Axle Inspection Facility',
    icon: '⚖️',
    x: 1565.2,
    y: 1869.6,
    getDetails: (data) => {
      const nearby = data?.trucks?.filter(t => Math.hypot((t.pos ? t.pos.x : (t.x || 0)) - 1565.2, (t.pos ? t.pos.y : (t.y || 0)) - 1869.6) < 400.0).length ?? 0;
      return `
        <div class="hub-metric-row"><span class="lbl">Corridor Node:</span><span class="val">Waypoint #85 (East LV Freeway)</span></div>
        <div class="hub-metric-row"><span class="lbl">Inspection Scales:</span><span class="val pass">OPERATIONAL (100% Flow)</span></div>
        <div class="hub-metric-row"><span class="lbl">Haulers In-Transit:</span><span class="val queue">${nearby} Trucks in Sector</span></div>
        <div class="hub-role">Automated dynamic weigh-in-motion scales ensuring compliance across LV-Bone County freeway corridor.</div>
      `;
    }
  },
  {
    id: 'sf_terminal',
    name: 'San Fierro Freight Terminal',
    subtitle: 'Interstate Drop-off & Metro Hub',
    icon: '📦',
    x: -1765.4,
    y: 141.7,
    getDetails: (data) => {
      const food = data?.sf_supplies?.food ?? 80;
      const fuel = data?.sf_supplies?.fuel ?? 80;
      const nearby = data?.trucks?.filter(t => Math.hypot((t.pos ? t.pos.x : (t.x || 0)) - (-1765.4), (t.pos ? t.pos.y : (t.y || 0)) - 141.7) < 400.0).length ?? 0;
      return `
        <div class="hub-metric-row"><span class="lbl">SF Food Depot:</span><span class="val food">${food} / 500 u</span></div>
        <div class="hub-metric-row"><span class="lbl">SF Fuel Depot:</span><span class="val fuel">${fuel} / 500 u</span></div>
        <div class="hub-metric-row"><span class="lbl">Unloading Bay Queue:</span><span class="val queue">${nearby} Trucks in Area</span></div>
        <div class="hub-role">Final drop-off hub for Interstate haulers; feeds San Fierro metropolitan commercial supply chains.</div>
      `;
    }
  }
];

const COMPANY_META = {
  0: { name: "Ocean Docks Logistics", color: "#38bdf8", hub: "Ocean Docks" },
  1: { name: "Bone County Petroleum", color: "#f59e0b", hub: "Bone County" },
  2: { name: "San Andreas Agro Food", color: "#22c55e", hub: "Flint County Farms" }
};

let g_companyRoutes = {
  0: {
    companyId: 0,
    color: "#38bdf8",
    active: false,
    nodes: []
  },
  1: {
    companyId: 1,
    color: "#f59e0b",
    active: false,
    nodes: []
  },
  2: {
    companyId: 2,
    color: "#22c55e",
    active: false,
    nodes: []
  }
};

let g_selectedCompanyId = 0;
let g_isEditMode = false;
let g_editorSubMode = 'idle'; // 'idle', 'facility', 'draw'
let g_routeMap = null;
let g_routeLayerGroup = null;
let g_truckLayerGroup = null;
let g_chevronLayerGroup = null;

const SATELLITE_MAP_URLS = [
  'https://raw.githubusercontent.com/interactive-game-maps/grand_theft_auto_san_andreas/master/gtasa-satellite.jpg',
  'https://raw.githubusercontent.com/gptrk0/mtasa-map-images/master/default/map_default_4096.jpg',
  'https://raw.githubusercontent.com/pawn-lang/compiler/gh-pages/images/sa-map.jpg',
  'https://upload.wikimedia.org/wikipedia/commons/4/4b/San_andreas_map.jpg',
  'https://i.imgur.com/3N4oZ9p.jpeg'
];

const NIGHT_MAP_URLS = [
  'https://i.imgur.com/jE2bwoA.jpg',
  'https://raw.githubusercontent.com/gptrk0/mtasa-map-images/master/night/map_night_2048.jpg'
];

let g_routeLayerSat = null;
let g_routeLayerVector = null;
let g_routeLayerNight = null;
let g_routeLayerMode = 0; // 0: Satellite, 1: Clean Radar, 2: Dark

function createFallbackImageOverlay(urls, bounds, options, fallbackFn) {
  let idx = 0;
  let lastFailedIdx = -1;
  const overlay = L.imageOverlay(urls[0], bounds, options || {});

  function tryNext() {
    if (idx === lastFailedIdx) return;
    lastFailedIdx = idx;
    idx++;
    if (idx < urls.length) {
      overlay.setUrl(urls[idx]);
    } else if (fallbackFn) {
      const fallbackUrl = fallbackFn();
      if (fallbackUrl) overlay.setUrl(fallbackUrl);
    }
  }

  overlay.on('error', tryNext);
  overlay.on('add', function() {
    const el = overlay.getElement();
    if (el) el.onerror = tryNext;
  });

  return overlay;
}

function cycleRouteMapLayer() {
  if (!g_routeMap) return;
  g_routeLayerMode = (g_routeLayerMode + 1) % 3;
  if (g_routeLayerSat && g_routeMap.hasLayer(g_routeLayerSat)) g_routeMap.removeLayer(g_routeLayerSat);
  if (g_routeLayerVector && g_routeMap.hasLayer(g_routeLayerVector)) g_routeMap.removeLayer(g_routeLayerVector);
  if (g_routeLayerNight && g_routeMap.hasLayer(g_routeLayerNight)) g_routeMap.removeLayer(g_routeLayerNight);

  const txt = document.getElementById('txt-route-layer');
  if (g_routeLayerMode === 0) {
    g_routeLayerSat.addTo(g_routeMap);
    if (txt) txt.textContent = 'Style: Satellite';
  } else if (g_routeLayerMode === 1) {
    g_routeLayerVector.addTo(g_routeMap);
    if (txt) txt.textContent = 'Style: Clean Radar';
  } else {
    g_routeLayerNight.addTo(g_routeMap);
    if (txt) txt.textContent = 'Style: Dark';
  }
}

function createProceduralRadarCanvas() {
  const c = document.createElement('canvas');
  c.width = 2048;
  c.height = 2048;
  const ctx = c.getContext('2d');

  ctx.fillStyle = '#060a14';
  ctx.fillRect(0, 0, 2048, 2048);

  // Tactical Grid Lines
  ctx.lineWidth = 1;
  ctx.strokeStyle = 'rgba(30, 41, 59, 0.7)';
  const step = 2048 / 12;
  for (let i = 0; i <= 2048; i += step) {
    ctx.beginPath();
    ctx.moveTo(i, 0); ctx.lineTo(i, 2048);
    ctx.moveTo(0, i); ctx.lineTo(2048, i);
    ctx.stroke();
  }

  // Major Axis
  ctx.lineWidth = 2;
  ctx.strokeStyle = 'rgba(56, 189, 248, 0.35)';
  ctx.beginPath();
  ctx.moveTo(1024, 0); ctx.lineTo(1024, 2048);
  ctx.moveTo(0, 1024); ctx.lineTo(2048, 1024);
  ctx.stroke();

  // Radar Circles
  ctx.strokeStyle = 'rgba(56, 189, 248, 0.2)';
  [512, 1024, 1536].forEach(r => {
    ctx.beginPath();
    ctx.arc(1024, 1024, r, 0, Math.PI * 2);
    ctx.stroke();
  });

  // Districts
  const districts = [
    { name: "LOS SANTOS", x: 1800, y: -1600 },
    { name: "SAN FIERRO", x: -2000, y: 200 },
    { name: "LAS VENTURAS", x: 1800, y: 1800 },
    { name: "FLINT COUNTY", x: -800, y: -1200 },
    { name: "RED COUNTY", x: 800, y: -400 }
  ];
  ctx.font = 'bold 24px -apple-system, sans-serif';
  ctx.textAlign = 'center';
  ctx.textBaseline = 'middle';
  ctx.fillStyle = 'rgba(56, 189, 248, 0.4)';
  districts.forEach(d => {
    const cx = ((d.x + 3000) / 6000) * 2048;
    const cy = 2048 - (((d.y + 3000) / 6000) * 2048);
    ctx.fillText(d.name, cx, cy);
  });

  return c.toDataURL('image/png');
}

const highwayRaw = [
    [2312.2,-2252.0], [2347.9,-2222.3], [2419.2,-2172.3], [2697.5,-2169.9], [2766.5,-2146.1], [2828.3,-2089.1], [2830.7,-1996.3], [2833.1,-1897.6],
    [2840.2,-1838.1], [2850.9,-1733.5], [2883.0,-1599.1], [2909.2,-1525.4], [2916.3,-1411.2], [2909.2,-1337.5], [2886.6,-1279.2], [2883.0,-1203.1],
    [2884.4,-1147.2], [2883.2,-1077.1], [2882.1,-1021.2], [2886.8,-932.0], [2887.8,-761.9], [2890.2,-689.4], [2881.8,-556.2], [2840.2,-500.3],
    [2776.0,-407.5], [2716.4,-354.5], [2698.8,-289.8], [2743.3,-167.9], [2763.5,-109.0], [2770.2,4.5], [2777.0,51.6], [2772.8,90.3],
    [2777.0,155.9], [2770.2,224.8], [2721.5,293.8], [2679.4,316.5], [2613.0,319.9], [2548.2,307.2], [2493.6,310.6], [2441.4,320.7],
    [2390.1,327.4], [2329.6,324.9], [2255.6,324.9], [2169.8,323.2], [2083.2,324.1], [2000.8,315.7], [1908.3,302.2], [1833.5,282.0],
    [1771.2,277.0], [1732.6,290.4], [1710.7,310.6], [1700.6,345.9], [1698.9,385.4], [1715.7,419.9], [1736.8,482.2], [1757.8,543.5],
    [1780.5,606.6], [1784.7,655.4], [1791.4,697.4], [1801.5,765.5], [1803.2,800.9], [1806.8,893.5], [1809.7,950.5], [1808.6,1014.2],
    [1804.4,1040.9], [1799.6,1109.9], [1810.3,1195.5], [1803.2,1309.7], [1810.3,1389.4], [1799.6,1501.1], [1803.2,1577.3], [1820.0,1593.0],
    [1835.5,1630.5], [1858.0,1649.5], [1871.5,1678.0], [1872.2,1714.9], [1846.1,1720.0], [1804.9,1714.1], [1746.9,1710.7], [1686.3,1711.5],
    [1635.9,1709.0], [1580.4,1707.3], [1566.9,1723.3], [1567.7,1769.6], [1569.4,1825.9], [1565.2,1869.6], [1521.4,1872.2], [1490.4,1880.5],
    [1496.4,1934.0], [1488.1,1953.0], [1495.2,1969.7], [1572.5,1976.8], [1567.7,2025.6], [1566.6,2051.8], [1464.3,2042.2], [1404.8,2052.9],
    [1320.4,2042.2], [1269.3,2054.1], [1196.7,2045.8], [1139.6,2044.6], [1067.1,2052.9], [1004.1,2047.0], [1010.0,2003.0], [1011.2,1951.9],
    [1007.6,1898.3], [1008.8,1841.3], [1061.1,1812.7], [1112.3,1813.9], [1140.8,1812.7], [1287.1,1808.0], [1296.6,1825.8], [1265.7,1892.4],
    [1228.8,1953.0], [1228.8,2089.8], [1227.6,2154.0], [1251.4,2202.8], [1290.7,2245.6], [1347.7,2325.3], [1348.9,2370.5], [1308.5,2406.1],
    [1258.6,2424.0], [1193.1,2439.4], [1126.5,2473.9], [1076.6,2504.8], [1013.6,2550.0], [941.0,2594.0], [856.6,2633.3], [760.3,2655.9],
    [622.3,2657.1], [454.6,2657.1], [416.6,2696.3], [338.1,2714.1], [228.7,2747.4], [165.7,2753.4], [118.1,2720.1], [97.9,2699.9],
    [51.5,2657.1], [-62.7,2640.4], [-150.7,2634.5], [-245.8,2634.5], [-336.2,2635.7], [-383.7,2690.4], [-452.7,2721.3], [-521.7,2717.7],
    [-574.0,2740.3], [-619.2,2759.3], [-698.9,2739.1], [-840.4,2726.0], [-991.4,2717.7], [-1138.9,2699.9], [-1237.6,2679.7], [-1323.2,2647.5],
    [-1347.6,2637.4], [-1379.2,2686.8], [-1435.1,2721.3], [-1516.0,2729.6], [-1618.3,2733.2], [-1694.4,2724.8], [-1752.6,2727.2], [-1793.1,2696.3],
    [-1809.3,2679.8], [-1822.8,2686.1], [-1853.5,2677.3], [-1860.6,2658.8], [-1878.3,2638.2], [-1901.0,2626.4], [-1931.3,2612.6], [-1957.8,2607.5],
    [-2009.0,2626.0], [-2053.0,2637.0], [-2100.0,2659.0], [-2234.0,2673.0], [-2287.0,2677.0], [-2390.0,2671.0], [-2515.0,2669.0], [-2607.0,2668.0],
    [-2680.0,2660.0], [-2741.0,2611.0], [-2761.0,2565.0], [-2772.0,2513.0], [-2778.0,2461.0], [-2780.0,2402.0], [-2771.0,2354.0], [-2751.0,2297.0],
    [-2724.0,2245.0], [-2700.0,2208.0], [-2687.0,2163.0], [-2688.0,2113.0], [-2683.0,2040.0], [-2690.0,2026.0], [-2688.0,1985.0], [-2690.0,1936.0],
    [-2693.0,1889.0], [-2689.0,1799.0], [-2689.0,1700.0], [-2687.0,1599.0], [-2686.0,1534.0], [-2684.0,1516.0], [-2684.0,1444.0], [-2692.0,1362.0],
    [-2687.0,1285.0], [-2671.0,1196.0], [-2629.0,1148.0], [-2607.0,1128.0], [-2537.0,1110.0], [-2474.0,1101.0], [-2449.4,1093.3], [-2379.8,1078.4],
    [-2316.2,1067.1], [-2286.5,1067.1], [-2197.9,1065.9], [-2095.6,1065.3], [-2042.7,1067.7], [-1993.0,1072.0], [-1899.0,1059.0], [-1887.0,1043.0],
    [-1894.0,990.0], [-1896.4,943.4], [-1790.6,924.4], [-1691.9,922.0], [-1607.4,920.8], [-1536.1,922.0], [-1524.2,917.2], [-1527.8,873.2],
    [-1536.1,844.7], [-1545.6,791.2], [-1555.1,751.9], [-1553.9,700.8], [-1561.1,599.7], [-1563.4,508.2], [-1589.6,459.4], [-1626.5,424.9],
    [-1677.6,370.2], [-1722.8,331.0], [-1764.4,290.5], [-1801.3,251.3], [-1807.8,214.5], [-1809.2,179.2], [-1778.8,184.1], [-1762.6,177.0],
    [-1765.4,141.7], [-1759.7,110.6], [-1758.3,54.7], [-1761.9,-16.7], [-1761.1,-100.2], [-1761.9,-117.1], [-1788.2,-116.2], [-1797.2,-136.9],
    [-1795.8,-194.9], [-1798.6,-233.8], [-1802.2,-326.4], [-1813.2,-390.9], [-1826.3,-468.2], [-1825.1,-528.8], [-1820.3,-558.6], [-1814.4,-607.3],
    [-1820.3,-666.8], [-1808.4,-725.0], [-1807.2,-795.2], [-1807.2,-849.9], [-1809.6,-929.6], [-1795.3,-1040.2], [-1766.8,-1123.4], [-1737.1,-1184.1],
    [-1690.7,-1266.1], [-1663.3,-1316.1], [-1620.5,-1382.7], [-1567.0,-1438.6], [-1530.1,-1442.1], [-1526.6,-1391.0], [-1537.3,-1291.1], [-1617.0,-1194.8],
    [-1671.7,-1154.4], [-1709.7,-1106.8], [-1737.1,-1027.1], [-1746.6,-958.1], [-1752.5,-904.6], [-1750.1,-861.8], [-1695.4,-796.4], [-1658.6,-791.6],
    [-1628.8,-826.1], [-1626.5,-879.6], [-1633.6,-949.8], [-1638.4,-1014.0], [-1608.6,-1123.4], [-1574.1,-1166.2], [-1534.9,-1209.1], [-1480.2,-1268.5],
    [-1445.7,-1328.0], [-1421.9,-1408.8], [-1404.1,-1416.0], [-1358.9,-1396.9], [-1266.1,-1368.4], [-1217.4,-1352.9], [-1171.0,-1345.8], [-1129.4,-1342.2],
    [-1077.1,-1351.8], [-1005.7,-1382.7], [-964.1,-1398.1], [-924.8,-1393.4], [-911.8,-1363.7], [-898.7,-1319.7], [-890.3,-1186.5], [-877.3,-1084.2],
    [-817.8,-1020.0], [-759.5,-1003.3], [-679.9,-999.8], [-620.6,-979.8], [-583.1,-953.6], [-538.6,-925.3], [-457.2,-844.0], [-362.5,-837.0],
    [-314.4,-865.2], [-246.5,-892.1], [-197.7,-934.5], [-156.0,-957.9], [-105.8,-998.9], [-88.1,-1034.2], [-90.0,-1092.5], [-99.5,-1132.9],
    [-119.7,-1180.5], [-140.0,-1235.2], [-149.5,-1297.1], [-156.6,-1356.5], [-144.7,-1421.9], [-100.7,-1498.0], [-17.5,-1518.2], [65.8,-1526.6],
    [137.1,-1555.1], [182.3,-1608.6], [256.0,-1687.1], [322.0,-1708.0], [427.0,-1711.0], [591.0,-1726.0], [663.0,-1748.0], [861.0,-1784.0],
    [972.0,-1780.0], [1060.0,-1847.0], [1060.0,-1943.0], [1051.0,-2009.0], [1046.0,-2057.0], [1045.0,-2266.0], [1138.0,-2401.0], [1197.0,-2429.0],
    [1282.0,-2460.0], [1343.0,-2467.0], [1341.6,-2507.9], [1340.2,-2594.1], [1386.9,-2666.2], [1504.3,-2684.6], [1593.4,-2681.8], [1750.3,-2687.5],
    [1884.7,-2677.6], [2023.3,-2690.3], [2125.1,-2654.9], [2167.5,-2616.7], [2171.8,-2537.6], [2170.4,-2481.0], [2173.2,-2413.1], [2211.4,-2355.1],
    [2309.0,-2256.1]
  ];

// Hardcoded 63 Roadblock coordinates table
const k_roadblocksData = [
    { id: 0,  x: 1369.0, y: -1400.0, z: 12.0 }, { id: 1,  x: 1211.0, y: -1573.0, z: 12.0 },
    { id: 2,  x: 1198.0, y: -1711.0, z: 12.0 }, { id: 3,  x: 1047.0, y: -2087.0, z: 12.0 },
    { id: 4,  x: 1031.0, y: -2087.0, z: 12.0 }, { id: 5,  x: 1032.0, y: -2222.0, z: 12.0 },
    { id: 6,  x: 1032.0, y: -2172.0, z: 12.0 }, { id: 7,  x: 1023.0, y: -2120.0, z: 12.0 },
    { id: 8,  x: 1330.0, y: -2448.0, z: 7.0  }, { id: 9,  x: 1329.0, y: -2464.0, z: 6.0  },
    { id: 10, x: 1367.0, y: -2447.0, z: 7.0  }, { id: 11, x: 1369.0, y: -2465.0, z: 6.0  },
    { id: 12, x: 1345.0, y: -2407.0, z: 12.0 }, { id: 13, x: 1332.0, y: -2414.0, z: 12.0 },
    { id: 14, x: 1481.0, y: -2686.0, z: 10.0 }, { id: 15, x: 1467.0, y: -2669.0, z: 11.0 },
    { id: 16, x: 1849.0, y: -1271.0, z: 12.0 }, { id: 17, x: 1820.0, y: -1260.0, z: 12.0 },
    { id: 18, x: 1746.0, y: -1161.0, z: 23.0 }, { id: 19, x: 1701.0, y: -1301.0, z: 12.0 },
    { id: 20, x: 1714.0, y: -1317.0, z: 12.0 }, { id: 21, x: 1715.0, y: -1278.0, z: 12.0 },
    { id: 22, x: 1453.0, y: -1463.0, z: 12.0 }, { id: 23, x: 1438.0, y: -1525.0, z: 12.0 },
    { id: 24, x: 1196.0, y: -1418.0, z: 12.0 }, { id: 25, x: 1164.0, y: -1281.0, z: 12.0 },
    { id: 26, x: 1215.0, y: -1258.0, z: 13.0 }, { id: 27, x: 1200.0, y: -1334.0, z: 12.0 },
    { id: 28, x: 1187.0, y: -1331.0, z: 13.0 }, { id: 29, x: 1214.0, y: -1333.0, z: 12.0 },
    { id: 30, x: 1257.0, y: -1313.0, z: 12.0 }, { id: 31, x: 1092.0, y: -1399.0, z: 12.0 },
    { id: 32, x: 1055.0, y: -1393.0, z: 12.0 }, { id: 33, x: 1061.0, y: -1438.0, z: 12.0 },
    { id: 34, x: 919.0,  y: -1409.0, z: 12.0 }, { id: 35, x: 917.0,  y: -1385.0, z: 12.0 },
    { id: 36, x: 798.0,  y: -1414.0, z: 12.0 }, { id: 37, x: 761.0,  y: -1400.0, z: 12.0 },
    { id: 38, x: 797.0,  y: -1370.0, z: 12.0 }, { id: 39, x: 663.0,  y: -1315.0, z: 12.0 },
    { id: 40, x: 668.0,  y: -1234.0, z: 14.0 }, { id: 41, x: 792.0,  y: -1150.0, z: 23.0 },
    { id: 42, x: 962.0,  y: -1127.0, z: 23.0 }, { id: 43, x: 967.0,  y: -1038.0, z: 29.0 },
    { id: 44, x: 963.0,  y: -978.0,  z: 38.0 }, { id: 45, x: 794.0,  y: -1041.0, z: 24.0 },
    { id: 46, x: 1155.0, y: -792.0,  z: 55.0 }, { id: 47, x: 1369.0, y: -925.0,  z: 33.0 },
    { id: 48, x: 1379.0, y: -927.0,  z: 33.0 }, { id: 49, x: 1490.0, y: -938.0,  z: 36.0 },
    { id: 50, x: 1479.0, y: -975.0,  z: 36.0 }, { id: 51, x: 1659.0, y: -811.0,  z: 56.0 },
    { id: 52, x: 1679.0, y: -807.0,  z: 55.0 }, { id: 53, x: 1703.0, y: -784.0,  z: 53.0 },
    { id: 54, x: 2451.0, y: -1249.0, z: 23.0 }, { id: 55, x: 2431.0, y: -1636.0, z: 26.0 },
    { id: 56, x: 2029.0, y: -1752.0, z: 12.0 }, { id: 57, x: 1960.0, y: -1994.0, z: 12.0 },
    { id: 58, x: 2264.0, y: -2233.0, z: 12.0 }, { id: 59, x: 2756.0, y: -2150.0, z: 10.0 },
    { id: 60, x: 2762.0, y: -2162.0, z: 10.0 }, { id: 61, x: 2849.0, y: -1656.0, z: 10.0 },
    { id: 62, x: 2858.0, y: -1137.0, z: 10.0 }
];

let roadblockLayer = null;
let staticRoadblockElements = [];

// Initialize markers once
function initRoadblocksOnce() {
    if (staticRoadblockElements.length > 0) return;
    
    k_roadblocksData.forEach(pt => {
        const pos = typeof gameToMap === 'function' ? gameToMap(pt.x, pt.y) : [pt.y, pt.x];
        const zone = L.circle(pos, {
            radius: 35.0,
            color: '#b91c1c',
            weight: 1.5,
            fillColor: '#ef4444',
            fillOpacity: 0.25,
            interactive: false
        });

        const marker = L.circleMarker(pos, {
            radius: 5,
            color: '#7f1d1d',
            weight: 1.5,
            fillColor: '#dc2626',
            fillOpacity: 1.0
        }).bindTooltip(`<b>ROADBLOCK #${pt.id}</b><br>CORDON ACTIVE<br>Pos: ${pt.x}, ${pt.y}`);

        staticRoadblockElements.push({ id: pt.id, zone: zone, marker: marker });
    });
}

// Statically toggle visibility without recreating DOM elements
function renderAllRoadblocks(roadblocks, macro) {
    const targetMap = (typeof map !== 'undefined' && map) ? map : (typeof g_routeMap !== 'undefined' ? g_routeMap : null);
    if (!targetMap) return;

    if (!roadblockLayer) {
        roadblockLayer = L.layerGroup().addTo(targetMap);
    }

    initRoadblocksOnce();
    const isCrisis = macro && (macro.unrest >= 60.0 || macro.curfew);

    if (!isCrisis) {
        if (targetMap.hasLayer(roadblockLayer)) {
            targetMap.removeLayer(roadblockLayer);
        }
        return;
    }

    if (!targetMap.hasLayer(roadblockLayer)) {
        targetMap.addLayer(roadblockLayer);
    }

    if (roadblockLayer.getLayers().length === 0) {
        staticRoadblockElements.forEach(item => {
            roadblockLayer.addLayer(item.zone);
            roadblockLayer.addLayer(item.marker);
        });
    }
}

function initRouteEditorMap() {
  if (g_routeMap) return;
  const container = document.getElementById('route-editor-map');
  if (!container) return;

  g_routeMap = L.map('route-editor-map', {
    crs: L.CRS.Simple,
    minZoom: -2,
    maxZoom: 3,
    zoomSnap: 0.25,
    zoomDelta: 0.5,
    maxBounds: [[-500, -500], [6500, 6500]],
    maxBoundsViscosity: 0.9,
    attributionControl: false
  });

  g_routeMap.setView([3000, 3000], -1);

  g_routeLayerSat = createFallbackImageOverlay(SATELLITE_MAP_URLS, WORLD_BOUNDS, { pane: 'tilePane', opacity: 1.0, interactive: false }, createProceduralRadarCanvas);
  g_routeLayerVector = L.imageOverlay(createProceduralRadarCanvas(), WORLD_BOUNDS, { pane: 'tilePane', opacity: 0.95, interactive: false });
  g_routeLayerNight = createFallbackImageOverlay(NIGHT_MAP_URLS, WORLD_BOUNDS, { pane: 'tilePane', opacity: 0.95, interactive: false }, createProceduralRadarCanvas);

  g_routeLayerSat.addTo(g_routeMap);
  // Persistent Master Highway Circuit on Route Editor:
  const hwCoords = highwayRaw.map(p => gameToMap(p[0], p[1]));
  if (hwCoords.length > 0) hwCoords.push(hwCoords[0]);
  g_highwayPolyline = L.polyline(hwCoords, {
    color: '#06b6d4',
    weight: 4,
    opacity: 0.95,
    dashArray: '6, 6',
    interactive: false
  }).addTo(g_routeMap);

  g_routeLayerGroup = L.layerGroup().addTo(g_routeMap);
  g_chevronLayerGroup = L.layerGroup().addTo(g_routeMap);
  g_truckLayerGroup = L.layerGroup().addTo(g_routeMap);
  g_strategicHubLayer = L.layerGroup().addTo(g_routeMap);
  if (!roadblockLayer && g_routeMap) {
    roadblockLayer = L.layerGroup().addTo(g_routeMap);
  }
  if (typeof renderAllRoadblocks === 'function') {
    renderAllRoadblocks(g_lastTelemetryData?.roadblocks, g_lastTelemetryData?.macro);
  }

  try {
    initLogisticsStrategicHubs();
  } catch (err) {
    console.error('[initLogisticsStrategicHubs Error]', err);
  }

  g_routeMap.on('click', function(e) {
    if (g_editorSubMode === 'idle' || !e.latlng) return;
    let targetX = Number((e.latlng.lng - 3000.0).toFixed(1));
    let targetY = Number((e.latlng.lat - 3000.0).toFixed(1));
    let snappedToHub = false;
    let hubSnapName = "";

    if (typeof STRATEGIC_HUBS !== 'undefined' && Array.isArray(STRATEGIC_HUBS)) {
      for (const hub of STRATEGIC_HUBS) {
        const dist = Math.hypot(hub.x - targetX, hub.y - targetY);
        // 35m magnetic radius for terminals and loading hubs
        if (dist < 35.0) {
          targetX = Number(hub.x.toFixed(1));
          targetY = Number(hub.y.toFixed(1));
          snappedToHub = true;
          hubSnapName = hub.name;
          break;
        }
      }
    }

    if (!g_companyRoutes[g_selectedCompanyId]) {
      g_companyRoutes[g_selectedCompanyId] = {
        companyId: g_selectedCompanyId,
        color: COMPANY_META[g_selectedCompanyId].color,
        nodes: []
      };
    }

    const route = g_companyRoutes[g_selectedCompanyId];

    if (g_editorSubMode === 'facility') {
      const facilityNode = { x: targetX, y: targetY, z: 15.0 };
      if (route.nodes.length === 0) {
        route.nodes.push(facilityNode);
      } else {
        route.nodes[0] = facilityNode;
      }
      renderAllCompanyRoutes();
      showEditorToast("Factory base locked! Click '2. Plot Route' to start drawing the road.", '#22c55e');
    } else if (g_editorSubMode === 'draw') {
      if (route.nodes.length === 0) {
        showEditorToast("⚠️ Please establish a Factory Hub (Step 1) before plotting the route!", "#f59e0b");
        return;
      }
      route.nodes.push({ x: targetX, y: targetY, z: 15.0 });
      renderAllCompanyRoutes();
      if (snappedToHub) {
        showEditorToast('🧲 Connected to Hub: ' + hubSnapName, '#f59e0b');
      } else {
        showEditorToast(`📍 Added waypoint #${route.nodes.length - 1} at (${targetX}, ${targetY})`, '#22c55e');
      }
    }
  });

  loadCustomRoutes();
}

let g_highwayPolyline = null;
let g_strategicHubLayer = null;
let g_logisticsStrategicMarkers = [];
let g_latestLogisticsData = null;
let g_showMasterHighway = true;
let g_showLocalFeeders = true;

function toggleLogisticsHighway() {
  g_showMasterHighway = !g_showMasterHighway;
  const btn = document.getElementById('btn-master-highway');
  const txt = document.getElementById('txt-master-highway');
  if (btn) btn.classList.toggle('active', g_showMasterHighway);
  if (txt) txt.textContent = g_showMasterHighway ? 'Master Highway: ON' : 'Master Highway: OFF';
  if (g_showMasterHighway) {
    if (g_highwayPolyline && !g_routeMap.hasLayer(g_highwayPolyline)) g_highwayPolyline.addTo(g_routeMap);
  } else {
    if (g_highwayPolyline && g_routeMap.hasLayer(g_highwayPolyline)) g_routeMap.removeLayer(g_highwayPolyline);
  }
}

function toggleLogisticsFeeders() {
  g_showLocalFeeders = !g_showLocalFeeders;
  const btn = document.getElementById('btn-local-feeders');
  const txt = document.getElementById('txt-local-feeders');
  if (btn) btn.classList.toggle('active', g_showLocalFeeders);
  if (txt) txt.textContent = g_showLocalFeeders ? 'Local Feeders: ON' : 'Local Feeders: OFF';
  if (g_showLocalFeeders) {
    if (g_routeLayerGroup && !g_routeMap.hasLayer(g_routeLayerGroup)) g_routeLayerGroup.addTo(g_routeMap);
    if (g_chevronLayerGroup && !g_routeMap.hasLayer(g_chevronLayerGroup)) g_chevronLayerGroup.addTo(g_routeMap);
  } else {
    if (g_routeLayerGroup && g_routeMap.hasLayer(g_routeLayerGroup)) g_routeMap.removeLayer(g_routeLayerGroup);
    if (g_chevronLayerGroup && g_routeMap.hasLayer(g_chevronLayerGroup)) g_routeMap.removeLayer(g_chevronLayerGroup);
  }
}

function initLogisticsStrategicHubs() {
  if (!g_strategicHubLayer) return;
  if (typeof STRATEGIC_HUBS === 'undefined' || !Array.isArray(STRATEGIC_HUBS)) return;
  g_logisticsStrategicMarkers = [];
  STRATEGIC_HUBS.forEach(hub => {
    const pos = gameToMap(hub.x, hub.y);
    const icon = L.divIcon({
      className: 'strategic-hub-icon-wrap',
      html: `
        <div class="strategic-hub-icon" title="${hub.name}">
          <div class="pulse-ring"></div>
          <div class="hub-dot"></div>
        </div>
      `,
      iconSize: [18, 18],
      iconAnchor: [9, 9],
      popupAnchor: [0, -12]
    });

    const marker = L.marker(pos, { icon: icon, zIndexOffset: 2000 });

    function renderPopup() {
      return `
        <div class="tactical-popup">
          <div class="hub-badge">KEY LOGISTICS HUB</div>
          <div class="hub-title"><span>${hub.icon}</span> <span>${hub.name}</span></div>
          <div class="hub-sub">${hub.subtitle}</div>
          <div class="hub-content">${hub.getDetails(g_latestLogisticsData)}</div>
          <div style="margin-top:8px;font-size:10px;color:#94a3b8;">Coords: X: ${hub.x.toFixed(1)}, Y: ${hub.y.toFixed(1)}</div>
        </div>
      `;
    }

    marker.bindPopup(renderPopup, {
      className: 'tactical-popup-wrap',
      maxWidth: 320,
      autoPan: true
    });

    marker.on('click', (e) => {
      if (g_editorSubMode !== 'idle') {
        if (e) {
          if (e.originalEvent) L.DomEvent.stopPropagation(e.originalEvent);
          L.DomEvent.stopPropagation(e);
        }
        marker.closePopup();

        if (!g_companyRoutes[g_selectedCompanyId]) {
          g_companyRoutes[g_selectedCompanyId] = {
            companyId: g_selectedCompanyId,
            color: COMPANY_META[g_selectedCompanyId].color,
            nodes: []
          };
        }

        const route = g_companyRoutes[g_selectedCompanyId];
        const hubNode = { x: Number(hub.x.toFixed(1)), y: Number(hub.y.toFixed(1)), z: 15.0 };

        if (g_editorSubMode === 'facility') {
          if (route.nodes.length === 0) {
            route.nodes.push(hubNode);
          } else {
            route.nodes[0] = hubNode;
          }
          renderAllCompanyRoutes();
          showEditorToast("Factory base locked! Click '2. Plot Route' to start drawing the road.", '#22c55e');
        } else if (g_editorSubMode === 'draw') {
          if (route.nodes.length === 0) {
            showEditorToast("⚠️ Please establish a Factory Hub (Step 1) before plotting the route!", "#f59e0b");
            return;
          }
          route.nodes.push(hubNode);
          renderAllCompanyRoutes();
          showEditorToast(`🎯 Terminus Locked: ${hub.name}`, '#22c55e');
        }
      } else {
        marker.setPopupContent(renderPopup());
        marker.openPopup();
      }
    });

    g_strategicHubLayer.addLayer(marker);
    g_logisticsStrategicMarkers.push({ marker, hub, renderPopup });
  });
}

function showEditorToast(msg, color = '#38bdf8') {
  const el = document.getElementById('editor-toast');
  if (!el) return;
  el.textContent = msg;
  el.style.color = color;
  el.style.borderColor = color + '66';
  el.style.background = color + '18';
}

function selectCompanyRoute(cId) {
  g_selectedCompanyId = cId;
  [0, 1, 2].forEach(i => {
    const btn = document.getElementById(`btn-comp-${i}`);
    if (btn) btn.classList.toggle('active', i === cId);
  });

  const cm = COMPANY_META[cId];
  const nameEl = document.getElementById('meta-company-name');
  if (nameEl) {
    nameEl.textContent = cm.name;
    nameEl.style.color = cm.color;
  }
  showEditorToast(`Focused on ${cm.name}. Ready.`);
  renderAllCompanyRoutes();
}

const FACILITY_PRESETS = {
  bone_oil: { name: "Bone County Refinery", x: -1038.1, y: -607.4, z: 15.0 },
  flint_farm: { name: "Flint County Agro Silo", x: -133.5, y: -350.1, z: 15.0 },
  ocean_docks: { name: "Ocean Docks Freight Yard", x: 2758.3, y: -2447.0, z: 15.0 }
};

function setEditorSubMode(mode) {
  if (g_editorSubMode === mode) {
    g_editorSubMode = 'idle';
  } else {
    g_editorSubMode = mode;
  }
  g_isEditMode = (g_editorSubMode !== 'idle');

  const btnFacility = document.getElementById('btn-mode-facility');
  const btnDraw = document.getElementById('btn-mode-draw');
  if (btnFacility) btnFacility.classList.toggle('active', g_editorSubMode === 'facility');
  if (btnDraw) btnDraw.classList.toggle('active', g_editorSubMode === 'draw');

  const mapEl = document.getElementById('route-editor-map');
  if (mapEl) mapEl.classList.toggle('edit-active', g_isEditMode);

  if (g_editorSubMode === 'facility') {
    showEditorToast("🏢 Mode: Set Factory Hub. Click map or select a preset to establish the production base.", "#38bdf8");
  } else if (g_editorSubMode === 'draw') {
    const route = g_companyRoutes[g_selectedCompanyId];
    if (!route || !route.nodes || route.nodes.length === 0) {
      showEditorToast("⚠️ Notice: Set Factory Hub first before plotting road, or click map to establish Node 0.", "#f59e0b");
    } else {
      showEditorToast("✏️ Mode: Plot Route. Click along the road to extend path from the factory to the port.", "#22c55e");
    }
  } else {
    showEditorToast("Navigation mode active.", "#38bdf8");
  }

  renderAllCompanyRoutes();
}

function applyPresetFacility(presetKey) {
  if (!presetKey) return;
  if (presetKey === 'custom_click') {
    setEditorSubMode('facility');
    const sel = document.getElementById('select-preset-facility');
    if (sel) sel.value = '';
    return;
  }

  const preset = FACILITY_PRESETS[presetKey];
  if (!preset) return;

  if (!g_companyRoutes[g_selectedCompanyId]) {
    g_companyRoutes[g_selectedCompanyId] = {
      companyId: g_selectedCompanyId,
      color: COMPANY_META[g_selectedCompanyId].color,
      nodes: []
    };
  }

  const route = g_companyRoutes[g_selectedCompanyId];
  const factoryNode = { x: preset.x, y: preset.y, z: preset.z };
  if (route.nodes.length === 0) {
    route.nodes.push(factoryNode);
  } else {
    route.nodes[0] = factoryNode;
  }

  if (g_routeMap) {
    g_routeMap.panTo(gameToMap(preset.x, preset.y), { animate: true, duration: 0.8 });
  }

  renderAllCompanyRoutes();
  showEditorToast(`🏭 Established Factory Base: ${preset.name}. Click '2. Plot Route' to start drawing the road.`, '#22c55e');

  const sel = document.getElementById('select-preset-facility');
  if (sel) sel.value = '';
}

function exportCompanyRouteCpp() {
  const route = g_companyRoutes[g_selectedCompanyId];
  if (!route || !route.nodes || route.nodes.length === 0) {
    alert('No waypoints to export for this company!');
    return;
  }
  const compName = COMPANY_META[g_selectedCompanyId].name;
  let code = `// ${compName} Feeder Corridor (${route.nodes.length} nodes)\n`;
  code += `static const std::vector<HighwayWaypoint> s_feederRoute_${g_selectedCompanyId} = {\n`;
  route.nodes.forEach(p => {
    code += `    { ${p.x.toFixed(1)}f, ${p.y.toFixed(1)}f, 15.0f },\n`;
  });
  code += `};\n`;
  navigator.clipboard.writeText(code);
  alert(`Copied ${route.nodes.length} waypoints for ${compName} to clipboard!`);
}

function openImportModal() {
  const modal = document.getElementById('import-route-modal');
  if (modal) {
    modal.classList.add('active');
    const ta = document.getElementById('import-route-textarea');
    if (ta) {
      ta.value = '';
      setTimeout(() => ta.focus(), 60);
    }
  }
}

function closeImportModal() {
  const modal = document.getElementById('import-route-modal');
  if (modal) modal.classList.remove('active');
}

function handleModalBackdropClick(e) {
  if (e.target && e.target.id === 'import-route-modal') {
    closeImportModal();
  }
}

function submitImportedRoute() {
  const ta = document.getElementById('import-route-textarea');
  if (!ta) return;
  const ok = parseAndApplyImportedRoute(ta.value);
  if (ok) {
    closeImportModal();
  }
}

function parseAndApplyImportedRoute(text) {
  if (!text || !text.trim()) {
    showEditorToast('⚠️ Please paste route coordinates or JSON first!', '#ef4444');
    return false;
  }
  const raw = text.trim();
  let nodes = [];

  // 1. Try JSON parsing
  if (raw.startsWith('[') || raw.startsWith('{')) {
    try {
      const data = JSON.parse(raw);
      const list = Array.isArray(data) ? data : (data.nodes || data.routes || []);
      if (Array.isArray(list) && list.length > 0) {
        for (const item of list) {
          let x = undefined, y = undefined, z = 15.0;
          if (Array.isArray(item)) {
            x = Number(item[0]);
            y = Number(item[1]);
            if (item.length > 2) z = Number(item[2]);
          } else if (item && typeof item === 'object') {
            x = Number(item.x !== undefined ? item.x : item.lng);
            y = Number(item.y !== undefined ? item.y : item.lat);
            if (item.z !== undefined) z = Number(item.z);
          }
          if (!isNaN(x) && !isNaN(y)) {
            nodes.push({
              x: Number(x.toFixed(1)),
              y: Number(y.toFixed(1)),
              z: isNaN(z) ? 15.0 : Number(z.toFixed(1))
            });
          }
        }
      }
    } catch (e) {
      // Fall through to regex
    }
  }

  // 2. Try C++ coordinate regex parsing
  if (nodes.length === 0) {
    const cppRegex = /\{\s*([-\d\.]+)(?:f)?\s*,\s*([-\d\.]+)(?:f)?(?:\s*,\s*([-\d\.]+)(?:f)?)?\s*\}/g;
    let match;
    while ((match = cppRegex.exec(raw)) !== null) {
      const x = parseFloat(match[1]);
      const y = parseFloat(match[2]);
      const z = match[3] !== undefined ? parseFloat(match[3]) : 15.0;
      if (!isNaN(x) && !isNaN(y)) {
        nodes.push({
          x: Number(x.toFixed(1)),
          y: Number(y.toFixed(1)),
          z: isNaN(z) ? 15.0 : Number(z.toFixed(1))
        });
      }
    }
  }

  if (nodes.length === 0) {
    showEditorToast('❌ No valid coordinates found. Paste JSON or C++ { x, y, z } format.', '#ef4444');
    return false;
  }

  if (!g_companyRoutes[g_selectedCompanyId]) {
    g_companyRoutes[g_selectedCompanyId] = {
      companyId: g_selectedCompanyId,
      color: COMPANY_META[g_selectedCompanyId].color,
      nodes: []
    };
  }

  g_companyRoutes[g_selectedCompanyId].nodes = nodes;
  renderAllCompanyRoutes();
  showEditorToast('✅ Imported ' + nodes.length + ' nodes successfully!', '#22c55e');
  return true;
}

document.addEventListener('keydown', function(e) {
  if (e.key === 'Escape') {
    closeImportModal();
  }
});

function undoLastWaypoint() {
  const route = g_companyRoutes[g_selectedCompanyId];
  if (route && route.nodes.length > 0) {
    route.nodes.pop();
    showEditorToast(`↩️ Removed last waypoint. (${route.nodes.length} remaining)`, '#f59e0b');
    renderAllCompanyRoutes();
  }
}

function clearCurrentRoute() {
  const route = g_companyRoutes[g_selectedCompanyId];
  if (route) {
    route.nodes = [];
    showEditorToast(`🗑️ Route cleared for Company #${g_selectedCompanyId}.`, '#ef4444');
    renderAllCompanyRoutes();
  }
}

async function saveAndDeployRoute() {
  const route = g_companyRoutes[g_selectedCompanyId];
  if (!route || route.nodes.length < 2) {
    showEditorToast('⚠️ Route must contain at least 2 waypoints before saving!', '#ef4444');
    return;
  }

  try {
    const payload = {
      companyId: g_selectedCompanyId,
      color: route.color || COMPANY_META[g_selectedCompanyId].color,
      nodes: route.nodes
    };
    const res = await fetch('/api/routes/save', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });
    const data = await res.json();
    if (data.status === 'ok') {
      showEditorToast(`✅ SAVED & DEPLOYED! 8 Trucks bound to new route (${route.nodes.length} nodes)`, '#22c55e');
    } else {
      showEditorToast(`❌ Error: ${data.message || 'Save failed'}`, '#ef4444');
    }
  } catch (err) {
    showEditorToast(`❌ Network error: ${err.message}`, '#ef4444');
  }
}

async function loadCustomRoutes() {
  try {
    const res = await fetch('/api/routes/load');
    if (res.ok) {
      const data = await res.json();
      if (data.routes && Array.isArray(data.routes)) {
        data.routes.forEach(r => {
          if (r.companyId >= 0 && r.companyId < 3) {
            g_companyRoutes[r.companyId] = {
              companyId: r.companyId,
              color: r.color || COMPANY_META[r.companyId].color,
              nodes: r.nodes || []
            };
          }
        });
        renderAllCompanyRoutes();
      }
    }
  } catch (e) {
    console.error('Failed to load custom routes', e);
  }
}

function renderAllCompanyRoutes() {
  if (!g_routeLayerGroup || !g_chevronLayerGroup) return;
  g_routeLayerGroup.clearLayers();
  g_chevronLayerGroup.clearLayers();

  let totalDistMeters = 0.0;
  let activeNodeCount = 0;

  for (let c = 0; c < 3; ++c) {
    const route = g_companyRoutes[c];
    if (!route || !route.nodes || route.nodes.length === 0) continue;

    const isSelected = (c === g_selectedCompanyId);
    const cm = (typeof COMPANY_META !== 'undefined' && COMPANY_META[c]) ? COMPANY_META[c] : { name: `Company #${c}`, color: '#38bdf8' };
    const compColor = route.color || cm.color;
    const compName = cm.name || `Company #${c}`;
    const latlngs = route.nodes.map(n => gameToMap(n.x, n.y));

    if (isSelected) {
      activeNodeCount = route.nodes.length;
      for (let k = 0; k < route.nodes.length - 1; ++k) {
        totalDistMeters += Math.hypot(route.nodes[k+1].x - route.nodes[k].x, route.nodes[k+1].y - route.nodes[k].y);
      }
    }

    // Polyline styling: render active company corridors at full opacity (1.0) with their respective colors!
    const polyline = L.polyline(latlngs, {
      color: compColor,
      weight: isSelected ? 5 : 3.5,
      opacity: 1.0,
      dashArray: isSelected ? null : '6, 4',
      interactive: !g_isEditMode
    });
    polyline.on('click', () => {
      if (!g_isEditMode) {
        selectCompanyRoute(c);
      }
    });
    g_routeLayerGroup.addLayer(polyline);

    // Render node markers: Dedicated Glowing Factory Base (0), Terminal (N-1), and intermediate Waypoints
    route.nodes.forEach((n, idx) => {
      const isStart = (idx === 0);
      const isEnd = (idx === route.nodes.length - 1);
      const pt = gameToMap(n.x, n.y);

      if (isStart) {
        // Dedicated glowing Factory Icon on Node 0 labeled: 🏭 [Company] Production Hub
        const factoryIcon = L.divIcon({
          className: 'factory-hub-marker-wrap',
          html: `<div class="factory-hub-glow" title="${compName} Production Hub">🏭</div>`,
          iconSize: [34, 34],
          iconAnchor: [17, 17],
          popupAnchor: [0, -18]
        });
        const factoryMarker = L.marker(pt, {
          icon: factoryIcon,
          interactive: !g_isEditMode,
          zIndexOffset: 3000
        });
        factoryMarker.bindTooltip(`🏭 ${compName} Production Hub`, {
          permanent: isSelected,
          direction: 'top',
          offset: [0, -18],
          className: 'factory-hub-tooltip'
        });
        g_routeLayerGroup.addLayer(factoryMarker);
      } else {
        const circle = L.circleMarker(pt, {
          radius: isEnd ? 8 : (isSelected ? 4.5 : 3.5),
          fillColor: isEnd ? '#f59e0b' : compColor,
          color: '#ffffff',
          weight: isEnd ? 2.5 : 1.5,
          fillOpacity: 1.0,
          interactive: !g_isEditMode
        });

        let tooltipText = `Waypoint #${idx}`;
        if (isEnd) {
          tooltipText = `📦 CARGO UNLOADING TERMINAL (Depot Drop-off)`;
        }

        circle.bindTooltip(tooltipText, {
          direction: 'top',
          offset: [0, -6]
        });

        g_routeLayerGroup.addLayer(circle);
      }

      // Directional chevrons along segments for selected route
      if (isSelected && idx < route.nodes.length - 1) {
        const nextPt = gameToMap(route.nodes[idx+1].x, route.nodes[idx+1].y);
        const midLat = (pt[0] + nextPt[0]) / 2;
        const midLng = (pt[1] + nextPt[1]) / 2;
        const angle = Math.atan2(nextPt[0] - pt[0], nextPt[1] - pt[1]) * 180 / Math.PI;

        const chevronIcon = L.divIcon({
          className: 'chevron-marker',
          iconSize: [20, 20],
          iconAnchor: [10, 10],
          html: `<svg class="chevron-svg" viewBox="0 0 20 20" width="20" height="20" style="transform: rotate(${angle}deg); display:block;">
                   <polygon points="5,3 15,10 5,17 8,10" fill="${compColor}" />
                 </svg>`
        });
        const chevronMarker = L.marker([midLat, midLng], { icon: chevronIcon, interactive: false });
        g_chevronLayerGroup.addLayer(chevronMarker);
      }
    });
  }

  // Ensure visibility respects the filter toggles
  if (!g_showLocalFeeders) {
    if (g_routeMap.hasLayer(g_routeLayerGroup)) g_routeMap.removeLayer(g_routeLayerGroup);
    if (g_routeMap.hasLayer(g_chevronLayerGroup)) g_routeMap.removeLayer(g_chevronLayerGroup);
  }

  const countEl = document.getElementById('meta-node-count');
  if (countEl) countEl.textContent = activeNodeCount;
  const distEl = document.getElementById('meta-route-dist');
  if (distEl) distEl.textContent = (totalDistMeters / 1000.0).toFixed(2) + ' km';
}

function updateTruckMapOverlays(trucks) {
  if (!g_truckLayerGroup || !Array.isArray(trucks)) return;
  g_truckLayerGroup.clearLayers();

  trucks.forEach(t => {
    const compColor = t.company_color || '#38bdf8';
    const isSelectedComp = (t.company_id === g_selectedCompanyId);
    const pos = gameToMap(t.x, t.y);

    const truckIcon = L.divIcon({
      className: '',
      iconSize: [24, 24],
      iconAnchor: [12, 12],
      html: `
        <div style="width:24px; height:24px; border-radius:50%; background:${compColor}; border:2px solid #ffffff; display:flex; align-items:center; justify-content:center; box-shadow:0 0 8px ${compColor}; opacity:${isSelectedComp ? '1.0' : '0.45'};">
          <span style="font-size:11px; line-height:1;">🚚</span>
        </div>
      `
    });

    const m = L.marker(pos, { icon: truckIcon, interactive: false });
    m.bindTooltip(`<b>${t.driver_name || 'Driver'}</b> (#${t.id})<br>State: ${t.state}<br>Cargo: ${t.cargo} (${t.cargo_tons}t)`, {
      direction: 'top',
      offset: [0, -10]
    });
    g_truckLayerGroup.addLayer(m);
  });
}

// Initialize Route Editor map after DOM loads
setTimeout(initRouteEditorMap, 100);

</script>
</body>
</html>
)rawlogistics";

static constexpr char k_municipalHtml[] = R"rawmunicipal(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>San Andreas — Mayor's Cabinet & Municipal Dispatch</title>
<style>
  :root {
    --bg-dark: #090d16;
    --card: rgba(15, 23, 42, 0.75);
    --card-border: rgba(255, 255, 255, 0.08);
    --text-main: #f8fafc;
    --text-muted: #94a3b8;
    --gold: #fbbf24;
    --red: #ef4444;
    --purple: #a855f7;
    --green: #10b981;
    --blue: #38bdf8;
    --amber: #f59e0b;
  }
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
    background-color: var(--bg-dark);
    background-image: 
      radial-gradient(at 0% 0%, rgba(56, 189, 248, 0.08) 0px, transparent 50%),
      radial-gradient(at 100% 100%, rgba(239, 68, 68, 0.06) 0px, transparent 50%);
    color: var(--text-main);
    min-height: 100vh;
    padding: 24px 20px;
  }
  .container { max-width: 1300px; margin: 0 auto; }
  header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    flex-wrap: wrap;
    gap: 16px;
    padding-bottom: 24px;
    border-bottom: 1px solid var(--card-border);
    margin-bottom: 24px;
  }
  .brand h1 {
    font-size: 1.6rem;
    font-weight: 900;
    letter-spacing: -0.02em;
    display: flex;
    align-items: center;
    gap: 10px;
  }
  .brand h1 span {
    background: linear-gradient(135deg, #fbbf24, #f59e0b);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
  }
  .nav-btns { display: flex; align-items: center; gap: 10px; flex-wrap: wrap; }
  .nav-btn {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    padding: 8px 14px;
    background: rgba(255, 255, 255, 0.04);
    border: 1px solid var(--card-border);
    border-radius: 8px;
    color: var(--text-main);
    text-decoration: none;
    font-size: 0.82rem;
    font-weight: 700;
    cursor: pointer;
    transition: all 0.2s ease;
  }
  .nav-btn:hover { background: rgba(255, 255, 255, 0.09); border-color: rgba(255, 255, 255, 0.2); }
  .nav-btn.active {
    background: rgba(251, 191, 36, 0.15);
    border-color: rgba(251, 191, 36, 0.4);
    color: var(--gold);
  }
  .status-pill {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    padding: 6px 12px;
    background: rgba(16, 185, 129, 0.12);
    border: 1px solid rgba(16, 185, 129, 0.3);
    border-radius: 9999px;
    font-size: 0.75rem;
    font-weight: 800;
    color: #34d399;
  }
  .pulse-dot {
    width: 8px;
    height: 8px;
    background: #34d399;
    border-radius: 50%;
    box-shadow: 0 0 8px #34d399;
    animation: pulse 1.8s infinite;
  }
  @keyframes pulse { 0%, 100% { opacity: 1; transform: scale(1); } 50% { opacity: 0.4; transform: scale(0.85); } }

  /* Banners */
  .impeachment-banner {
    display: none;
    background: linear-gradient(135deg, rgba(220, 38, 38, 0.4), rgba(0, 0, 0, 0.8));
    border: 2px solid #ef4444;
    border-radius: 12px;
    padding: 18px 24px;
    margin-bottom: 24px;
    animation: alertGlow 1.5s infinite alternate;
  }
  .impeachment-title { font-size: 1.25rem; font-weight: 900; color: #fecaca; display: flex; align-items: center; gap: 10px; }
  .impeachment-desc { font-size: 0.88rem; color: #fca5a5; margin-top: 6px; line-height: 1.4; }

  .emergency-banner {
    display: none;
    background: linear-gradient(135deg, rgba(239, 68, 68, 0.25), rgba(185, 28, 28, 0.15));
    border: 1px solid rgba(239, 68, 68, 0.5);
    border-radius: 12px;
    padding: 16px 20px;
    margin-bottom: 24px;
    animation: alertGlow 2s infinite alternate;
  }
  @keyframes alertGlow {
    from { box-shadow: 0 0 10px rgba(239, 68, 68, 0.2); }
    to { box-shadow: 0 0 25px rgba(239, 68, 68, 0.5); }
  }
  .emergency-content {
    display: flex;
    justify-content: space-between;
    align-items: center;
    flex-wrap: wrap;
    gap: 14px;
  }
  .emergency-title { font-size: 1.1rem; font-weight: 800; color: #fca5a5; display: flex; align-items: center; gap: 8px; }
  .emergency-desc { font-size: 0.85rem; color: #cbd5e1; margin-top: 4px; }
  .veto-btn {
    padding: 10px 20px;
    background: linear-gradient(135deg, #ef4444, #dc2626);
    border: 1px solid rgba(255, 255, 255, 0.3);
    border-radius: 8px;
    color: #fff;
    font-size: 0.88rem;
    font-weight: 800;
    cursor: pointer;
    box-shadow: 0 4px 14px rgba(239, 68, 68, 0.4);
    transition: all 0.2s ease;
  }
  .veto-btn:hover { transform: translateY(-2px); box-shadow: 0 6px 20px rgba(239, 68, 68, 0.6); }

  /* Interactive Operations Cards */
  .operations-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(360px, 1fr));
    gap: 16px;
    margin-bottom: 24px;
  }
  .op-card {
    background: var(--card);
    border-radius: 12px;
    padding: 18px 20px;
    backdrop-filter: blur(12px);
    border: 1px solid var(--card-border);
  }
  .op-head { display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px; }
  .op-title { font-size: 0.95rem; font-weight: 800; display: flex; align-items: center; gap: 8px; }
  .op-desc { font-size: 0.82rem; color: #cbd5e1; margin-bottom: 14px; line-height: 1.4; }
  .op-meta { display: flex; justify-content: space-between; font-size: 0.76rem; color: var(--text-muted); font-weight: 700; margin-bottom: 14px; }
  .act-btn {
    width: 100%;
    padding: 9px 14px;
    border-radius: 8px;
    border: none;
    font-size: 0.82rem;
    font-weight: 800;
    cursor: pointer;
    transition: all 0.2s ease;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    gap: 6px;
  }
  .btn-subsidy { background: linear-gradient(135deg, #f59e0b, #d97706); color: #fff; }
  .btn-subsidy:hover { transform: translateY(-1px); box-shadow: 0 4px 12px rgba(245, 158, 11, 0.4); }
  .btn-bribe-accept { background: linear-gradient(135deg, #10b981, #059669); color: #fff; }
  .btn-bribe-accept:hover { transform: translateY(-1px); box-shadow: 0 4px 12px rgba(16, 185, 129, 0.4); }
  .btn-bribe-reject { background: rgba(255, 255, 255, 0.08); color: #cbd5e1; border: 1px solid var(--card-border); margin-top: 6px; }
  .btn-bribe-reject:hover { background: rgba(239, 68, 68, 0.2); color: #fca5a5; }

  /* Metrics Grid */
  .metrics-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
    gap: 16px;
    margin-bottom: 28px;
  }
  .metric-card {
    background: var(--card);
    border: 1px solid var(--card-border);
    border-radius: 12px;
    padding: 18px 20px;
    backdrop-filter: blur(12px);
  }
  .metric-head { display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px; }
  .metric-title { font-size: 0.82rem; font-weight: 700; color: var(--text-muted); text-transform: uppercase; letter-spacing: 0.05em; }
  .metric-icon { font-size: 1.3rem; }
  .metric-value { font-size: 2rem; font-weight: 900; letter-spacing: -0.02em; margin-bottom: 8px; }
  .progress-bar-bg { height: 7px; background: rgba(255, 255, 255, 0.08); border-radius: 4px; overflow: hidden; }
  .progress-bar-fill { height: 100%; border-radius: 4px; transition: width 0.4s ease; }
  .metric-footer { display: flex; justify-content: space-between; font-size: 0.75rem; color: var(--text-muted); font-weight: 700; margin-top: 6px; }

  /* Sections */
  .section-title {
    font-size: 1.15rem;
    font-weight: 800;
    margin-bottom: 14px;
    display: flex;
    align-items: center;
    gap: 8px;
  }
  .two-cols {
    display: grid;
    grid-template-columns: 2fr 1fr;
    gap: 20px;
  }
  @media (max-width: 900px) { .two-cols { grid-template-columns: 1fr; } }

  /* Tables & Cards */
  .panel-card {
    background: var(--card);
    border: 1px solid var(--card-border);
    border-radius: 12px;
    padding: 18px 20px;
    backdrop-filter: blur(12px);
  }
  table { width: 100%; border-collapse: collapse; font-size: 0.85rem; }
  th { text-align: left; padding: 10px 12px; color: var(--text-muted); font-size: 0.72rem; text-transform: uppercase; border-bottom: 1px solid var(--card-border); }
  td { padding: 12px; border-bottom: 1px solid rgba(255, 255, 255, 0.04); vertical-align: middle; }
  .status-tag {
    display: inline-flex;
    align-items: center;
    gap: 4px;
    padding: 3px 8px;
    border-radius: 6px;
    font-size: 0.72rem;
    font-weight: 800;
  }
  .tag-dispatched { background: rgba(245, 158, 11, 0.15); border: 1px solid rgba(245, 158, 11, 0.3); color: #fbbf24; }
  .tag-progress { background: rgba(56, 189, 248, 0.15); border: 1px solid rgba(56, 189, 248, 0.3); color: #38bdf8; }
  .tag-resolved { background: rgba(16, 185, 129, 0.15); border: 1px solid rgba(16, 185, 129, 0.3); color: #34d399; }
  .tag-critical { background: rgba(239, 68, 68, 0.15); border: 1px solid rgba(239, 68, 68, 0.3); color: #f87171; }

  /* Decision Log Console */
  .log-console {
    background: rgba(0, 0, 0, 0.4);
    border: 1px solid rgba(255, 255, 255, 0.05);
    border-radius: 8px;
    padding: 12px 14px;
    font-family: 'Consolas', 'Monaco', monospace;
    font-size: 0.78rem;
    height: 380px;
    overflow-y: auto;
    display: flex;
    flex-direction: column;
    gap: 8px;
  }
  .log-line { color: #94a3b8; border-bottom: 1px dashed rgba(255, 255, 255, 0.05); padding-bottom: 4px; }
  .log-line span { color: var(--gold); font-weight: 700; }

  /* Municipal Fiscal Policy & Tax Regulation Panel */
  .tax-panel {
    background: var(--card);
    border: 1px solid rgba(56, 189, 248, 0.3);
    border-radius: 12px;
    padding: 20px;
    backdrop-filter: blur(12px);
    margin-top: 24px;
    box-shadow: 0 4px 20px rgba(0, 0, 0, 0.4);
  }
  .tax-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
    gap: 16px;
    margin-bottom: 20px;
  }
  .tax-slider-box {
    background: rgba(0, 0, 0, 0.35);
    border: 1px solid rgba(255, 255, 255, 0.08);
    border-radius: 10px;
    padding: 14px 16px;
    display: flex;
    flex-direction: column;
    gap: 8px;
  }
  .tax-slider-head {
    display: flex;
    justify-content: space-between;
    align-items: center;
  }
  .tax-label {
    font-size: 0.84rem;
    font-weight: 700;
    color: #e2e8f0;
    display: flex;
    align-items: center;
    gap: 6px;
  }
  .tax-value-badge {
    font-size: 1rem;
    font-weight: 900;
    font-family: 'Consolas', 'Monaco', monospace;
    padding: 2px 8px;
    border-radius: 6px;
    border: 1px solid rgba(56, 189, 248, 0.4);
    background: rgba(56, 189, 248, 0.15);
    color: #38bdf8;
  }
  .tax-slider {
    width: 100%;
    height: 8px;
    border-radius: 4px;
    appearance: none;
    background: rgba(255, 255, 255, 0.15);
    cursor: pointer;
    outline: none;
  }
  .tax-slider::-webkit-slider-thumb {
    appearance: none;
    width: 18px;
    height: 18px;
    border-radius: 50%;
    background: #38bdf8;
    border: 2px solid #ffffff;
    cursor: pointer;
    box-shadow: 0 0 8px rgba(56, 189, 248, 0.8);
  }
  .tax-slider-footer {
    display: flex;
    justify-content: space-between;
    font-size: 0.7rem;
    color: var(--text-muted);
    font-weight: 600;
  }
  .tax-risk-bar {
    padding: 12px 16px;
    border-radius: 8px;
    font-size: 0.88rem;
    font-weight: 800;
    display: flex;
    align-items: center;
    justify-content: space-between;
    border: 1px solid rgba(16, 185, 129, 0.4);
    background: rgba(16, 185, 129, 0.1);
    color: #6ee7b7;
    transition: all 0.3s ease;
  }
  .btn-apply-tax {
    background: linear-gradient(135deg, #0284c7, #0369a1);
    color: #ffffff;
    border: 1px solid #38bdf8;
    padding: 10px 22px;
    border-radius: 8px;
    font-size: 0.88rem;
    font-weight: 800;
    cursor: pointer;
    transition: transform 0.15s, box-shadow 0.15s, background 0.2s;
    display: inline-flex;
    align-items: center;
    gap: 8px;
  }
  .btn-apply-tax:hover {
    transform: translateY(-1px);
    box-shadow: 0 4px 14px rgba(56, 189, 248, 0.4);
    background: linear-gradient(135deg, #0ea5e9, #0284c7);
  }
  .btn-apply-tax:active {
    transform: translateY(0);
  }
</style>
</head>
<body>
<div class="container">
  <header>
    <div class="brand">
      <h1>🏛️ San Andreas <span>Mayor's Cabinet</span></h1>
    </div>
    <div class="nav-btns">
      <a href="/map" class="nav-btn"><span>🗺️</span> Live GPS Radar</a>
      <a href="/" class="nav-btn"><span>🎮</span> Vehicle Remote</a>
      <a href="/logistics" class="nav-btn"><span>🏢</span> Tycoon</a>
      <a href="/municipal" class="nav-btn active"><span>🏛️</span> City Hall</a>
      <button class="nav-btn" id="btn-curfew" onclick="toggleCurfew()">🌙 Curfew: OFF</button>
      <div class="status-pill" id="pill-emergency-state">STATUS: NORMAL</div>
      <div class="status-pill" id="pill-fuel-status">⛽ SF Fuel: 50u</div>
      <div class="status-pill"><div class="pulse-dot"></div> AI DIRECTOR ONLINE</div>
    </div>
  </header>

  <!-- Impeachment Takeover Banner -->
  <div class="impeachment-banner" id="impeachment-banner">
    <div class="impeachment-title">⚠️ MAYOR IMPEACHMENT TRIGGERED: OUSTER IN EFFECT!</div>
    <div class="impeachment-desc">
      The municipal administration has collapsed due to extreme insolvency (&lt; -$500,000) and rampant crime (&gt; 85%).
      The State Council has stripped mayoral authority and deployed National Guard contingents across Los Santos.
    </div>
  </div>

  <!-- Logistics Fuel Crisis Banner -->
  <div class="emergency-banner" id="fuel-crisis-banner" style="background: linear-gradient(135deg, rgba(245, 158, 11, 0.25), rgba(180, 83, 9, 0.15)); border-color: rgba(245, 158, 11, 0.5);">
    <div class="emergency-content">
      <div>
        <div class="emergency-title" style="color:#fde68a;">⛽ LOGISTICS CRISIS: FUEL RESERVES DEPLETED (&lt; 15 UNITS)</div>
        <div class="emergency-desc">
          Police cruisers and emergency responders are grounded without fuel. Criminal syndicates are operating unchecked!
        </div>
      </div>
    </div>
  </div>

  <!-- Emergency Banner -->
  <div class="emergency-banner" id="emergency-banner">
    <div class="emergency-content">
      <div>
        <div class="emergency-title">⚠️ MUNICIPAL EMERGENCY DECREE ACTIVE</div>
        <div class="emergency-desc" id="emergency-desc">
          City Treasury default or extreme unrest has prompted automatic martial dispatching.
        </div>
      </div>
      <button class="veto-btn" onclick="exerciseMayorVeto()">⚖️ EXERCISE MAYORAL VETO</button>
    </div>
  </div>

  <!-- Interactive Civic Operations (Bribes & Strikes) -->
  <div class="operations-grid" id="operations-grid">
    <!-- Active Union Strike Card -->
    <div class="op-card" id="strike-panel" style="display:none; border-color: rgba(245, 158, 11, 0.4); background: rgba(245, 158, 11, 0.05);">
      <div class="op-head">
        <span class="op-title" style="color:#fbbf24;">🪧 Active Union Picket & Roadblock</span>
        <span class="status-tag tag-dispatched" id="strike-duration">120s</span>
      </div>
      <div class="op-desc">
        <b id="strike-union" style="color:#f8fafc;">Teamsters & Dockers Union</b> has formed physical picket lines at <b id="strike-location" style="color:#38bdf8;">Ocean Docks Gate</b>, disrupting commercial freight and transit.
      </div>
      <div class="op-meta">
        <span>Impact: Traffic Blockade</span>
        <span>Resolution: $20,000 Subsidy</span>
      </div>
      <button class="act-btn btn-subsidy" onclick="subsidizeStrike()">💵 Disburse $20,000 Emergency Union Subsidy</button>
    </div>

    <!-- Syndicate Shadow Bribe Card -->
    <div class="op-card" id="bribe-panel" style="display:none; border-color: rgba(168, 85, 247, 0.4); background: rgba(168, 85, 247, 0.05);">
      <div class="op-head">
        <span class="op-title" style="color:#c084fc;">💼 Syndicate Shadow Bribe Offer</span>
        <span class="status-tag tag-progress" id="bribe-expires">45s</span>
      </div>
      <div class="op-desc">
        <b id="bribe-syndicate" style="color:#f8fafc;">Leone Syndicate</b>: <span id="bribe-desc">Arms contraband transshipment protection.</span>
      </div>
      <div class="op-meta">
        <span id="bribe-amount" style="color:#34d399; font-weight:800; font-size:0.9rem;">+$125,000</span>
        <span id="bribe-impact" style="color:#f87171;">Crime +8.5% | Unrest +4.0%</span>
      </div>
      <button class="act-btn btn-bribe-accept" onclick="acceptBribe()">🤝 Accept Shadow Bribe</button>
      <button class="act-btn btn-bribe-reject" onclick="rejectBribe()">🛡️ Reject & Denounce</button>
    </div>
  </div>

  <!-- Metrics Grid -->
  <div class="metrics-grid">
    <div class="metric-card">
      <div class="metric-head">
        <span class="metric-title">City Treasury</span>
        <span class="metric-icon">💰</span>
      </div>
      <div class="metric-value" id="val-treasury" style="color: #fbbf24;">$1,000,000</div>
      <div class="metric-footer">
        <span id="txt-treasury-status">Fiscal Reserve: Solvency Strong</span>
        <span>+ $5,000 / 30s Tax</span>
      </div>
    </div>

    <div class="metric-card">
      <div class="metric-head">
        <span class="metric-title">Crime Index</span>
        <span class="metric-icon">🚨</span>
      </div>
      <div class="metric-value" id="val-crime">25.0%</div>
      <div class="progress-bar-bg">
        <div class="progress-bar-fill" id="bar-crime" style="width: 25%; background: #10b981;"></div>
      </div>
      <div class="metric-footer">
        <span id="txt-crime-status">Order Maintained</span>
        <span>Roadblock: 60% | Max: 85%</span>
      </div>
    </div>

    <div class="metric-card">
      <div class="metric-head">
        <span class="metric-title">Social Unrest</span>
        <span class="metric-icon">📢</span>
      </div>
      <div class="metric-value" id="val-unrest">10.0%</div>
      <div class="progress-bar-bg">
        <div class="progress-bar-fill" id="bar-unrest" style="width: 10%; background: #38bdf8;"></div>
      </div>
      <div class="metric-footer">
        <span id="txt-unrest-status">Civil Stability: Normal</span>
        <span>Strike Risk: 50%</span>
      </div>
    </div>

    <div class="metric-card">
      <div class="metric-head">
        <span class="metric-title">Incidents Resolved</span>
        <span class="metric-icon">🛡️</span>
      </div>
      <div class="metric-value" id="val-handled" style="color: #38bdf8;">0</div>
      <div class="metric-footer">
        <span id="txt-active-count">Active Tactical Ops: 0</span>
        <span>Dispatcher AI 20Hz</span>
      </div>
    </div>
  </div>

  <div class="two-cols">
    <!-- Active Incidents Panel -->
    <div>
      <div class="section-title"><span>🚨</span> Active Municipal Hotspots & Police Dispatches</div>
      <div class="panel-card">
        <table>
          <thead>
            <tr>
              <th>ID</th>
              <th>Incident Type</th>
              <th>District</th>
              <th>Severity</th>
              <th>Status</th>
              <th>Age</th>
            </tr>
          </thead>
          <tbody id="incidents-tbody">
            <tr><td colspan="6" style="text-align:center; color:#64748b; padding:20px;">No critical emergency incidents currently active. State is secure.</td></tr>
          </tbody>
        </table>
      </div>
    </div>

    <!-- AI Dispatcher Log Feed -->
    <div>
      <div class="section-title"><span>📡</span> AI Mayor Dispatcher Log</div>
      <div class="panel-card" style="padding:14px;">
        <div class="log-console" id="log-console">
          <!-- Populated via JS -->
        </div>
      </div>
    </div>
  </div>

  <!-- Municipal Fiscal Policy & Tax Regulation Panel -->
  <div class="tax-panel" id="panel-tax-regulation">
    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 16px; flex-wrap: wrap; gap: 12px;">
      <div class="section-title" style="margin-bottom: 0;"><span>🏛️</span> Municipal Fiscal Policy & Tax Regulation</div>
      <div style="display: flex; align-items: center; gap: 12px;">
        <span id="txt-tax-feedback" style="font-size: 0.8rem; color: #94a3b8; font-weight: 600;"></span>
        <button class="btn-apply-tax" id="btn-apply-tax" onclick="applyFiscalPolicy()">
          <span>⚖️</span> Apply Fiscal Policy
        </button>
      </div>
    </div>

    <div class="tax-grid">
      <!-- 1. Retail Sales Tax (Food/Fuel/Agro) -->
      <div class="tax-slider-box">
        <div class="tax-slider-head">
          <label for="slider-tax-sales-gen" class="tax-label">
            <span>🛒</span> Retail Sales Tax (Food/Fuel/Agro)
          </label>
          <span class="tax-value-badge" id="txt-tax-sales-gen">12.0%</span>
        </div>
        <input type="range" id="slider-tax-sales-gen" class="tax-slider" min="2" max="30" step="1" value="12"
               oninput="onTaxSliderInput('sales-gen', this.value)" />
        <div class="tax-slider-footer">
          <span>Min: 2% (Relief)</span>
          <span>Baseline: 12%</span>
          <span>Max: 30% (Inflation)</span>
        </div>
      </div>

      <!-- 2. Luxury & Weapons Surtax (Ammu/Tech) -->
      <div class="tax-slider-box">
        <div class="tax-slider-head">
          <label for="slider-tax-sales-lux" class="tax-label">
            <span>🔫</span> Luxury & Weapons Surtax (Ammu/Tech)
          </label>
          <span class="tax-value-badge" id="txt-tax-sales-lux">25.0%</span>
        </div>
        <input type="range" id="slider-tax-sales-lux" class="tax-slider" min="5" max="50" step="1" value="25"
               oninput="onTaxSliderInput('sales-lux', this.value)" />
        <div class="tax-slider-footer">
          <span>Min: 5%</span>
          <span>Baseline: 25%</span>
          <span>Max: 50%</span>
        </div>
      </div>

      <!-- 3. Freight & Port Transit Fee -->
      <div class="tax-slider-box">
        <div class="tax-slider-head">
          <label for="slider-tax-hauler-port" class="tax-label">
            <span>🚢</span> Freight & Port Transit Fee
          </label>
          <span class="tax-value-badge" id="txt-tax-hauler-port">15.0%</span>
        </div>
        <input type="range" id="slider-tax-hauler-port" class="tax-slider" min="5" max="40" step="1" value="15"
               oninput="onTaxSliderInput('hauler-port', this.value)" />
        <div class="tax-slider-footer">
          <span>Min: 5%</span>
          <span>Baseline: 15%</span>
          <span>Strike Risk &gt;22%</span>
        </div>
      </div>

      <!-- 4. Corporate Excess Liquidity Tax -->
      <div class="tax-slider-box">
        <div class="tax-slider-head">
          <label for="slider-tax-corp-wealth" class="tax-label">
            <span>🏢</span> Corporate Excess Liquidity Tax
          </label>
          <span class="tax-value-badge" id="txt-tax-corp-wealth">3.0%</span>
        </div>
        <input type="range" id="slider-tax-corp-wealth" class="tax-slider" min="0" max="15" step="0.5" value="3"
               oninput="onTaxSliderInput('corp-wealth', this.value)" />
        <div class="tax-slider-footer">
          <span>Min: 0%</span>
          <span>Baseline: 3.0%</span>
          <span>Max: 15.0%</span>
        </div>
      </div>

      <!-- 5. Master Citizen Poll Tax (Uniform Bulk Preset) -->
      <div class="tax-slider-box" style="grid-column: 1 / -1;">
        <div class="tax-slider-head">
          <label for="slider-tax-citizen-poll" class="tax-label">
            <span>👥</span> Master Citizen Poll Tax (Uniform / Bulk Preset)
          </label>
          <span class="tax-value-badge" id="txt-tax-citizen-poll" style="color:#fbbf24; border-color:rgba(251,191,36,0.4); background:rgba(251,191,36,0.15);">$270</span>
        </div>
        <input type="range" id="slider-tax-citizen-poll" class="tax-slider" min="0" max="1500" step="25" value="270"
               oninput="onTaxSliderInput('citizen-poll', this.value)" />
        <div class="tax-slider-footer">
          <span>Uniform bulk preset across all 4 zones. Fine-tune granular rates in District Cards below.</span>
        </div>
      </div>
    </div>

    <!-- Real-Time Risk Indicator -->
    <div class="tax-risk-bar" id="tax-risk-indicator">
      <div id="tax-risk-text">🟢 Civil Climate: Stable</div>
      <div style="font-size: 0.75rem; color: #94a3b8; font-weight: 600;" id="tax-risk-desc">
        Tax rates within macroeconomic equilibrium tolerances.
      </div>
    </div>
  </div>

  <!-- 📊 District Living Standards & Real Wage Burden -->
  <div class="tax-panel" id="panel-district-living-standards" style="margin-top: 24px;">
    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 16px; flex-wrap: wrap; gap: 12px;">
      <div>
        <div class="section-title" style="margin-bottom: 2px;"><span>📊</span> District Living Standards & Real Wage Burden</div>
        <div style="font-size: 0.78rem; color: #94a3b8; font-weight: 600;">
          Citizen nominal baseline wages, living basket costs, tax burdens, and real disposable purchasing power.
        </div>
      </div>
      <div style="display: flex; align-items: center; gap: 10px;">
        <span id="txt-district-econ-summary" class="tax-value-badge" style="font-size: 0.82rem; padding: 4px 10px; border-color: rgba(56, 189, 248, 0.4); background: rgba(56, 189, 248, 0.15); color: #38bdf8;">
          Live Telemetry Active
        </span>
      </div>
    </div>

    <!-- 4-District Living Standards Grid -->
    <div class="tax-grid" id="district-living-standards-grid">
      <!-- District 0: South Central -->
      <div class="tax-slider-box" id="dist-econ-card-0" style="position: relative; overflow: hidden;">
        <div class="tax-slider-head" style="margin-bottom: 6px;">
          <div class="tax-label" style="font-size: 0.92rem;">
            <span>🏙️</span> South Central
          </div>
          <span class="status-tag" id="dist-econ-status-0" style="font-size: 0.72rem; padding: 2px 8px; border-radius: 9999px;">--</span>
        </div>
        <div style="font-size: 0.78rem; margin-bottom: 5px;">
          <span style="color: #94a3b8;">Income:</span>
          <span style="font-weight: 700; color: #f8fafc;" id="dist-econ-wage-0">$1,100</span>
          <span style="font-size: 0.7rem; color: #64748b;" id="dist-econ-income-detail-0">(Base: $1,100 | Bonus: +$0)</span>
        </div>
        <div style="font-size: 0.7rem; margin-bottom: 5px;">
          <span style="color: #94a3b8; font-weight: 600;" id="dist-econ-ownership-0">Owners: 30% | Renters: 70%</span>
        </div>
        <div style="font-size: 0.78rem; margin-bottom: 5px;">
          <span style="color: #94a3b8;">Expenses:</span>
          <span style="font-weight: 700; color: #f87171;" id="dist-econ-expenses-0">--</span>
          <span style="font-size: 0.68rem; color: #64748b; display: block; margin-top: 2px;" id="dist-econ-breakdown-0">(Housing: $580 | Basket: -- | Crime: -- | Tax: --)</span>
        </div>
        <div style="display: flex; justify-content: space-between; align-items: center; font-size: 0.82rem; margin-bottom: 4px;">
          <span style="font-weight: 700; color: #e2e8f0;">Real Savings Margin:</span>
          <span style="font-weight: 900; font-family: monospace;" id="dist-econ-net-ratio-0">--%</span>
        </div>
        <div style="width: 100%; height: 6px; background: rgba(255,255,255,0.1); border-radius: 3px; overflow: hidden; margin-top: 4px;">
          <div id="dist-econ-bar-0" style="height: 100%; width: 50%; background: #38bdf8; transition: width 0.2s, background 0.2s;"></div>
        </div>
        <div style="margin-top: 10px; padding-top: 8px; border-top: 1px solid rgba(255,255,255,0.08);">
          <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 4px;">
            <label for="slider-dist-tax-0" style="font-size: 0.76rem; color: #94a3b8; font-weight: 700;">
              <span>💵</span> Poll Tax (Cycle):
            </label>
            <span class="tax-value-badge" id="txt-dist-tax-0" style="font-size: 0.75rem; padding: 2px 7px; color: #fbbf24; border-color: rgba(251,191,36,0.3); background: rgba(251,191,36,0.1); font-weight: 800;">$150</span>
          </div>
          <input type="range" id="slider-dist-tax-0" class="tax-slider" min="0" max="1500" step="25" value="150"
                 oninput="onDistrictTaxSliderInput(0, this.value)" />
          <div style="display: flex; justify-content: space-between; font-size: 0.65rem; color: #64748b; margin-top: 2px;">
            <span>$0</span>
            <span>Step: $25</span>
            <span>Max: $1,500</span>
          </div>
        </div>
      </div>

      <!-- District 1: Downtown -->
      <div class="tax-slider-box" id="dist-econ-card-1" style="position: relative; overflow: hidden;">
        <div class="tax-slider-head" style="margin-bottom: 6px;">
          <div class="tax-label" style="font-size: 0.92rem;">
            <span>🏢</span> Downtown & West LS
          </div>
          <span class="status-tag" id="dist-econ-status-1" style="font-size: 0.72rem; padding: 2px 8px; border-radius: 9999px;">--</span>
        </div>
        <div style="font-size: 0.78rem; margin-bottom: 5px;">
          <span style="color: #94a3b8;">Income:</span>
          <span style="font-weight: 700; color: #f8fafc;" id="dist-econ-wage-1">$4,200</span>
          <span style="font-size: 0.7rem; color: #64748b;" id="dist-econ-income-detail-1">(Base: $4,200 | Yield: +$0)</span>
        </div>
        <div style="font-size: 0.7rem; margin-bottom: 5px;">
          <span style="color: #94a3b8; font-weight: 600;" id="dist-econ-ownership-1">Owners: 65% | Renters: 35%</span>
        </div>
        <div style="font-size: 0.78rem; margin-bottom: 5px;">
          <span style="color: #94a3b8;">Expenses:</span>
          <span style="font-weight: 700; color: #f87171;" id="dist-econ-expenses-1">--</span>
          <span style="font-size: 0.68rem; color: #64748b; display: block; margin-top: 2px;" id="dist-econ-breakdown-1">(Housing: $2,400 | Basket: -- | Crime: -- | Tax: --)</span>
        </div>
        <div style="display: flex; justify-content: space-between; align-items: center; font-size: 0.82rem; margin-bottom: 4px;">
          <span style="font-weight: 700; color: #e2e8f0;">Real Savings Margin:</span>
          <span style="font-weight: 900; font-family: monospace;" id="dist-econ-net-ratio-1">--%</span>
        </div>
        <div style="width: 100%; height: 6px; background: rgba(255,255,255,0.1); border-radius: 3px; overflow: hidden; margin-top: 4px;">
          <div id="dist-econ-bar-1" style="height: 100%; width: 50%; background: #38bdf8; transition: width 0.2s, background 0.2s;"></div>
        </div>
        <div style="margin-top: 10px; padding-top: 8px; border-top: 1px solid rgba(255,255,255,0.08);">
          <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 4px;">
            <label for="slider-dist-tax-1" style="font-size: 0.76rem; color: #94a3b8; font-weight: 700;">
              <span>💵</span> Poll Tax (Cycle):
            </label>
            <span class="tax-value-badge" id="txt-dist-tax-1" style="font-size: 0.75rem; padding: 2px 7px; color: #fbbf24; border-color: rgba(251,191,36,0.3); background: rgba(251,191,36,0.1); font-weight: 800;">$500</span>
          </div>
          <input type="range" id="slider-dist-tax-1" class="tax-slider" min="0" max="1500" step="25" value="500"
                 oninput="onDistrictTaxSliderInput(1, this.value)" />
          <div style="display: flex; justify-content: space-between; font-size: 0.65rem; color: #64748b; margin-top: 2px;">
            <span>$0</span>
            <span>Step: $25</span>
            <span>Max: $1,500</span>
          </div>
        </div>
      </div>

      <!-- District 2: Industrial Port -->
      <div class="tax-slider-box" id="dist-econ-card-2" style="position: relative; overflow: hidden;">
        <div class="tax-slider-head" style="margin-bottom: 6px;">
          <div class="tax-label" style="font-size: 0.92rem;">
            <span>⚓</span> Industrial Port & Logistics
          </div>
          <span class="status-tag" id="dist-econ-status-2" style="font-size: 0.72rem; padding: 2px 8px; border-radius: 9999px;">--</span>
        </div>
        <div style="font-size: 0.78rem; margin-bottom: 5px;">
          <span style="color: #94a3b8;">Income:</span>
          <span style="font-weight: 700; color: #f8fafc;" id="dist-econ-wage-2">$2,400</span>
          <span style="font-size: 0.7rem; color: #64748b;" id="dist-econ-income-detail-2">(Base: $2,400 | Surge: +$0)</span>
        </div>
        <div style="font-size: 0.7rem; margin-bottom: 5px;">
          <span style="color: #94a3b8; font-weight: 600;" id="dist-econ-ownership-2">Owners: 35% | Renters: 65%</span>
        </div>
        <div style="font-size: 0.78rem; margin-bottom: 5px;">
          <span style="color: #94a3b8;">Expenses:</span>
          <span style="font-weight: 700; color: #f87171;" id="dist-econ-expenses-2">--</span>
          <span style="font-size: 0.68rem; color: #64748b; display: block; margin-top: 2px;" id="dist-econ-breakdown-2">(Housing: $1,320 | Basket: -- | Crime: -- | Tax: --)</span>
        </div>
        <div style="display: flex; justify-content: space-between; align-items: center; font-size: 0.82rem; margin-bottom: 4px;">
          <span style="font-weight: 700; color: #e2e8f0;">Real Savings Margin:</span>
          <span style="font-weight: 900; font-family: monospace;" id="dist-econ-net-ratio-2">--%</span>
        </div>
        <div style="width: 100%; height: 6px; background: rgba(255,255,255,0.1); border-radius: 3px; overflow: hidden; margin-top: 4px;">
          <div id="dist-econ-bar-2" style="height: 100%; width: 50%; background: #38bdf8; transition: width 0.2s, background 0.2s;"></div>
        </div>
        <div style="margin-top: 10px; padding-top: 8px; border-top: 1px solid rgba(255,255,255,0.08);">
          <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 4px;">
            <label for="slider-dist-tax-2" style="font-size: 0.76rem; color: #94a3b8; font-weight: 700;">
              <span>💵</span> Poll Tax (Cycle):
            </label>
            <span class="tax-value-badge" id="txt-dist-tax-2" style="font-size: 0.75rem; padding: 2px 7px; color: #fbbf24; border-color: rgba(251,191,36,0.3); background: rgba(251,191,36,0.1); font-weight: 800;">$250</span>
          </div>
          <input type="range" id="slider-dist-tax-2" class="tax-slider" min="0" max="1500" step="25" value="250"
                 oninput="onDistrictTaxSliderInput(2, this.value)" />
          <div style="display: flex; justify-content: space-between; font-size: 0.65rem; color: #64748b; margin-top: 2px;">
            <span>$0</span>
            <span>Step: $25</span>
            <span>Max: $1,500</span>
          </div>
        </div>
      </div>

      <!-- District 3: Country / Rural -->
      <div class="tax-slider-box" id="dist-econ-card-3" style="position: relative; overflow: hidden;">
        <div class="tax-slider-head" style="margin-bottom: 6px;">
          <div class="tax-label" style="font-size: 0.92rem;">
            <span>🌲</span> Country / Rural Highways
          </div>
          <span class="status-tag" id="dist-econ-status-3" style="font-size: 0.72rem; padding: 2px 8px; border-radius: 9999px;">--</span>
        </div>
        <div style="font-size: 0.78rem; margin-bottom: 5px;">
          <span style="color: #94a3b8;">Income:</span>
          <span style="font-weight: 700; color: #f8fafc;" id="dist-econ-wage-3">$1,400</span>
          <span style="font-size: 0.7rem; color: #64748b;" id="dist-econ-income-detail-3">(Base: $1,400 | Bonus: +$0)</span>
        </div>
        <div style="font-size: 0.7rem; margin-bottom: 5px;">
          <span style="color: #94a3b8; font-weight: 600;" id="dist-econ-ownership-3">Owners: 75% | Renters: 25%</span>
        </div>
        <div style="font-size: 0.78rem; margin-bottom: 5px;">
          <span style="color: #94a3b8;">Expenses:</span>
          <span style="font-weight: 700; color: #f87171;" id="dist-econ-expenses-3">--</span>
          <span style="font-size: 0.68rem; color: #64748b; display: block; margin-top: 2px;" id="dist-econ-breakdown-3">(Housing: $750 | Basket: -- | Crime: -- | Tax: --)</span>
        </div>
        <div style="display: flex; justify-content: space-between; align-items: center; font-size: 0.82rem; margin-bottom: 4px;">
          <span style="font-weight: 700; color: #e2e8f0;">Real Savings Margin:</span>
          <span style="font-weight: 900; font-family: monospace;" id="dist-econ-net-ratio-3">--%</span>
        </div>
        <div style="width: 100%; height: 6px; background: rgba(255,255,255,0.1); border-radius: 3px; overflow: hidden; margin-top: 4px;">
          <div id="dist-econ-bar-3" style="height: 100%; width: 50%; background: #38bdf8; transition: width 0.2s, background 0.2s;"></div>
        </div>
        <div style="margin-top: 10px; padding-top: 8px; border-top: 1px solid rgba(255,255,255,0.08);">
          <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 4px;">
            <label for="slider-dist-tax-3" style="font-size: 0.76rem; color: #94a3b8; font-weight: 700;">
              <span>💵</span> Poll Tax (Cycle):
            </label>
            <span class="tax-value-badge" id="txt-dist-tax-3" style="font-size: 0.75rem; padding: 2px 7px; color: #fbbf24; border-color: rgba(251,191,36,0.3); background: rgba(251,191,36,0.1); font-weight: 800;">$180</span>
          </div>
          <input type="range" id="slider-dist-tax-3" class="tax-slider" min="0" max="1500" step="25" value="180"
                 oninput="onDistrictTaxSliderInput(3, this.value)" />
          <div style="display: flex; justify-content: space-between; font-size: 0.65rem; color: #64748b; margin-top: 2px;">
            <span>$0</span>
            <span>Step: $25</span>
            <span>Max: $1,500</span>
          </div>
        </div>
      </div>
    </div>
  </div>

  <!-- Secure Municipal Debug & Override Console -->
  <div style="margin-top: 28px;">
    <div class="section-title"><span>🛠️</span> Secure Municipal Debug & Override Console</div>
    <div class="panel-card" style="border: 1px solid rgba(168, 85, 247, 0.3); background: rgba(15, 23, 42, 0.9);">
      <div style="display: flex; gap: 12px; align-items: center; margin-bottom: 16px; flex-wrap: wrap;">
        <label style="font-size: 0.8rem; color: #a855f7; font-weight: 700;">ADMIN DEBUG KEY:</label>
        <input type="password" id="input-debug-key" value="mayor_sec_8941_auth" style="background: rgba(0,0,0,0.5); border: 1px solid rgba(255,255,255,0.1); color: #fff; padding: 6px 12px; border-radius: 6px; font-family: monospace; font-size: 0.82rem; width: 240px;" />
        <span id="txt-debug-status" style="font-size: 0.75rem; color: #94a3b8; font-weight: 600;">Authorized Key Loaded</span>
      </div>

      <!-- Interactive Sliders for Crime & Social Unrest -->
      <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(320px, 1fr)); gap: 16px; margin-bottom: 18px; background: rgba(0,0,0,0.35); padding: 16px; border-radius: 10px; border: 1px solid rgba(255,255,255,0.08);">
        <!-- Crime Slider -->
        <div>
          <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px;">
            <label for="slider-crime" style="font-size: 0.85rem; font-weight: 800; color: #f87171; display: flex; align-items: center; gap: 6px;">
              <span>🚨</span> Crime Rate Override:
            </label>
            <span id="txt-slider-crime-val" style="font-size: 1.05rem; font-weight: 900; color: #fca5a5; font-family: monospace; background: rgba(239,68,68,0.2); padding: 2px 10px; border-radius: 6px; border: 1px solid rgba(239,68,68,0.4);">25.0%</span>
          </div>
          <input type="range" id="slider-crime" min="0" max="100" step="1" value="25"
                 style="width: 100%; height: 10px; border-radius: 5px; appearance: none; background: linear-gradient(to right, #10b981 0%, #f59e0b 60%, #ef4444 85%); cursor: pointer; outline: none;"
                 oninput="onCrimeSliderInput(this.value)" onchange="onCrimeSliderChange(this.value)" />
          <div style="display: flex; justify-content: space-between; font-size: 0.72rem; color: #94a3b8; margin-top: 4px; font-weight: 600;">
            <span>0% (Peace)</span>
            <span>50% (Tension)</span>
            <span>75% (Riots & Shooters)</span>
            <span>100% (War)</span>
          </div>
        </div>

        <!-- Social Unrest Slider -->
        <div>
          <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px;">
            <label for="slider-unrest" style="font-size: 0.85rem; font-weight: 800; color: #38bdf8; display: flex; align-items: center; gap: 6px;">
              <span>📢</span> Social Unrest Override:
            </label>
            <span id="txt-slider-unrest-val" style="font-size: 1.05rem; font-weight: 900; color: #7dd3fc; font-family: monospace; background: rgba(56,189,248,0.2); padding: 2px 10px; border-radius: 6px; border: 1px solid rgba(56,189,248,0.4);">10.0%</span>
          </div>
          <input type="range" id="slider-unrest" min="0" max="100" step="1" value="10"
                 style="width: 100%; height: 10px; border-radius: 5px; appearance: none; background: linear-gradient(to right, #38bdf8 0%, #a855f7 50%, #ef4444 80%); cursor: pointer; outline: none;"
                 oninput="onUnrestSliderInput(this.value)" onchange="onUnrestSliderChange(this.value)" />
          <div style="display: flex; justify-content: space-between; font-size: 0.72rem; color: #94a3b8; margin-top: 4px; font-weight: 600;">
            <span>0% (Calm)</span>
            <span>40% (Protest)</span>
            <span>55% (SWAT Roadblocks)</span>
            <span>100% (Anarchy)</span>
          </div>
        </div>
      </div>
      <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(180px, 1fr)); gap: 10px;">
        <button class="nav-btn" onclick="sendDebug('spawn_all_crisis')" style="border-color: #ef4444; color: #fff; background: rgba(239, 68, 68, 0.4); font-weight: 900; grid-column: 1 / -1; padding: 12px;">🚨 DEPLOY FULL CITY CRISIS (ALL INCIDENTS)</button>
        <button class="nav-btn" onclick="sendDebug('spike_crime')" style="border-color: rgba(239, 68, 68, 0.4); color: #fca5a5;">💥 Force Crime 75%</button>
        <button class="nav-btn" onclick="sendDebug('bankruptcy')" style="border-color: rgba(239, 68, 68, 0.4); color: #fca5a5;">📉 Force Bankruptcy ($0)</button>
        <button class="nav-btn" onclick="sendDebug('inject_budget')" style="border-color: rgba(16, 185, 129, 0.4); color: #6ee7b7;">💵 Inject +$100,000</button>
        <button class="nav-btn" onclick="sendDebug('drain_budget')" style="border-color: rgba(245, 158, 11, 0.4); color: #fde68a;">💸 Drain -$100,000</button>
        <button class="nav-btn" onclick="sendDebug('spawn_strike')" style="border-color: rgba(245, 158, 11, 0.4); color: #fde68a;">🪧 Spawn Union Strike</button>
        <button class="nav-btn" onclick="sendDebug('spawn_incident')" style="border-color: rgba(56, 189, 248, 0.4); color: #7dd3fc;">🚨 Spawn Ganton Riot</button>
        <button class="nav-btn" onclick="sendDebug('spawn_roadblock')" style="border-color: rgba(59, 130, 246, 0.4); color: #93c5fd;">🚧 Deploy Roadblock</button>
      </div>
    </div>
  </div>
</div>

<script>
let isUserDraggingTax = false;
let g_districtLivingData = [
  { id: 0, name: "South Central", wage: 1100, effWage: 1100, bonus: 0, housing: 380, crimeSurcharge: 0, basket: 480, pollTax: 150, netRatio: 0.10, status: "STRAINED", wealthMod: 0.8, storePrice: 60.0 },
  { id: 1, name: "Downtown & West LS", wage: 4200, effWage: 4200, bonus: 0, housing: 2400, crimeSurcharge: 0, basket: 480, pollTax: 500, netRatio: 0.11, status: "STRAINED", wealthMod: 1.4, storePrice: 60.0 },
  { id: 2, name: "Industrial Port", wage: 2400, effWage: 2400, bonus: 0, housing: 1320, crimeSurcharge: 0, basket: 480, pollTax: 250, netRatio: 0.10, status: "STRAINED", wealthMod: 1.0, storePrice: 60.0 },
  { id: 3, name: "Country / Rural", wage: 1400, effWage: 1400, bonus: 0, housing: 320, crimeSurcharge: 0, basket: 480, pollTax: 180, netRatio: 0.10, status: "STRAINED", wealthMod: 0.9, storePrice: 60.0 }
];
let g_foodScarcityMult = 1.0;
let g_fuelPriceMul = 1.0;

// Static constants matching C++ engine for client-side preview
// Homeownership-based housing model: effectiveCost = rent * (1 - rate) + maintenance * rate
const K_RENT = [480.0, 2600.0, 1500.0, 650.0];
const K_MAINTENANCE = [140.0, 800.0, 400.0, 120.0];
const K_OWNERSHIP_RATE = [0.30, 0.65, 0.35, 0.75]; // fraction of homeowners per district
const K_HOUSING = K_RENT.map((r, i) => r * (1 - K_OWNERSHIP_RATE[i]) + K_MAINTENANCE[i] * K_OWNERSHIP_RATE[i]);
const K_FAMILY_MULT = [1.15, 1.05, 1.15, 1.15];
const K_BASE_WAGES = [1100.0, 4200.0, 2400.0, 1400.0];
const K_CRIME_POV_MUL = [1.3, 0.6, 0.6, 1.3]; // Dist 0,3 = high poverty multiplier
const K_BONUS_LABELS = ['Bonus', 'Yield', 'Surge', 'Bonus'];

function updateDistrictLivingStandardsPreview() {
  const genEl = document.getElementById('slider-tax-sales-gen');
  if (!genEl) return;

  const salesGen = parseFloat(genEl.value) / 100.0;
  // Get crime from the debug slider if available, fallback to telemetry
  const crimeSlider = document.getElementById('slider-crime');
  const crimeRate = crimeSlider ? parseFloat(crimeSlider.value) : 25.0;

  let deficitCount = 0;
  let vulnerableCount = 0;
  let strainedCount = 0;

  for (let d = 0; d < 4; ++d) {
    const item = g_districtLivingData[d];
    const storeP = item.storePrice || 60.0;
    const baseWage = K_BASE_WAGES[d];
    const bonus = item.bonus || 0;
    const effectiveWage = baseWage + bonus;

    // Shelter (homeownership-weighted)
    const shelter = K_HOUSING[d];

    // Grocery basket with family weighting
    const rawBasket = (storeP * 6.5 * g_foodScarcityMult) * (1.0 + salesGen) + (g_fuelPriceMul * 75.0);
    const basketCost = rawBasket * K_FAMILY_MULT[d];

    // Crime surcharge
    const crimeBurden = (crimeRate / 100.0) * 130.0 * K_CRIME_POV_MUL[d];

    // Poll tax
    const distSlider = document.getElementById('slider-dist-tax-' + d);
    const localPoll = distSlider ? parseFloat(distSlider.value) : (item.pollTax !== undefined ? item.pollTax : 150);
    const taxDed = localPoll * item.wealthMod;

    const totalExpenses = shelter + basketCost + crimeBurden + taxDed;
    const netDelta = effectiveWage - totalExpenses;
    const safeWage = Math.max(effectiveWage, 1.0);
    const netRatio = netDelta / safeWage;

    // Multi-tiered status evaluation:
    // - DEFICIT: netDelta < 0.0 -> Red, austerity active
    // - VULNERABLE: netDelta >= 0.0 && netDelta < 250.0 -> Orange, paycheck-to-paycheck
    // - STRAINED: netDelta >= 250.0 && (netRatio < 0.15 || netDelta < 400.0) -> Amber, stable poverty
    // - PROSPEROUS: netRatio >= 0.15 && netDelta >= 400.0 -> Green, capital accumulation
    let statusText = '🟢 Prosperous';
    let statusTagClass = 'tag-resolved';
    let barColor = '#10b981';
    let cardBorder = 'rgba(255, 255, 255, 0.08)';

    if (netDelta < 0.0) {
      statusText = '🔴 Deficit / Riot Risk';
      statusTagClass = 'tag-critical';
      barColor = '#ef4444';
      cardBorder = 'rgba(239, 68, 68, 0.4)';
      deficitCount++;
    } else if (netDelta < 250.0) {
      statusText = '🟠 Vulnerable (+$' + Math.round(netDelta) + ')';
      statusTagClass = 'tag-dispatched';
      barColor = '#f97316';
      cardBorder = 'rgba(249, 115, 22, 0.4)';
      vulnerableCount++;
    } else if (netRatio < 0.15 || netDelta < 400.0) {
      statusText = '🟡 Strained';
      statusTagClass = 'tag-dispatched';
      barColor = '#f59e0b';
      cardBorder = 'rgba(245, 158, 11, 0.3)';
      strainedCount++;
    }

    const card = document.getElementById('dist-econ-card-' + d);
    if (card) card.style.borderColor = cardBorder;

    // Income line
    const wageEl = document.getElementById('dist-econ-wage-' + d);
    if (wageEl) wageEl.textContent = '$' + Math.round(effectiveWage).toLocaleString();

    const incDetailEl = document.getElementById('dist-econ-income-detail-' + d);
    if (incDetailEl) {
      incDetailEl.textContent = '(Base: $' + baseWage.toLocaleString() + ' | ' + K_BONUS_LABELS[d] + ': +$' + Math.round(bonus).toLocaleString() + ')';
    }

    // Expenses line
    const expEl = document.getElementById('dist-econ-expenses-' + d);
    if (expEl) expEl.textContent = '$' + Math.round(totalExpenses).toLocaleString();

    const bdEl = document.getElementById('dist-econ-breakdown-' + d);
    if (bdEl) bdEl.textContent = '(Housing: $' + Math.round(shelter).toLocaleString() + ' | Basket: $' + Math.round(basketCost) + ' | Crime: $' + Math.round(crimeBurden) + ' | Tax: $' + Math.round(taxDed) + ')';

    // Net ratio / Savings margin
    const netEl = document.getElementById('dist-econ-net-ratio-' + d);
    if (netEl) {
      const pct = (netRatio * 100).toFixed(1);
      netEl.textContent = (netDelta >= 0 ? '+$' : '-$') + Math.abs(Math.round(netDelta)) + ' (' + (netRatio >= 0 ? '+' : '') + pct + '%)';
      netEl.style.color = barColor;
    }

    const statEl = document.getElementById('dist-econ-status-' + d);
    if (statEl) {
      statEl.textContent = statusText;
      statEl.className = 'status-tag ' + statusTagClass;
    }

    // Bar: 100% width = +20% surplus
    const barEl = document.getElementById('dist-econ-bar-' + d);
    if (barEl) {
      const fillW = Math.max(0, Math.min(100, (netRatio / 0.20) * 100));
      barEl.style.width = fillW + '%';
      barEl.style.background = barColor;
    }
  }

  const sumBadge = document.getElementById('txt-district-econ-summary');
  if (sumBadge) {
    if (deficitCount > 0) {
      sumBadge.textContent = '⚠️ ' + deficitCount + ' District(s) In Deficit';
      sumBadge.style.borderColor = '#ef4444';
      sumBadge.style.color = '#f87171';
      sumBadge.style.background = 'rgba(239, 68, 68, 0.15)';
    } else if (vulnerableCount > 0) {
      sumBadge.textContent = '🟠 ' + vulnerableCount + ' District(s) Vulnerable';
      sumBadge.style.borderColor = '#f97316';
      sumBadge.style.color = '#fb923c';
      sumBadge.style.background = 'rgba(249, 115, 22, 0.15)';
    } else if (strainedCount > 0) {
      sumBadge.textContent = '🟡 ' + strainedCount + ' District(s) Under Strain';
      sumBadge.style.borderColor = '#f59e0b';
      sumBadge.style.color = '#fbbf24';
      sumBadge.style.background = 'rgba(245, 158, 11, 0.15)';
    } else {
      sumBadge.textContent = '🟢 All Districts Prosperous';
      sumBadge.style.borderColor = '#10b981';
      sumBadge.style.color = '#34d399';
      sumBadge.style.background = 'rgba(16, 185, 129, 0.15)';
    }
  }
}

function onDistrictTaxSliderInput(distId, val) {
  isUserDraggingTax = true;
  const numVal = parseInt(val, 10);
  const t = document.getElementById('txt-dist-tax-' + distId);
  if (t) t.textContent = '$' + numVal.toLocaleString();
  if (g_districtLivingData[distId]) {
    g_districtLivingData[distId].pollTax = numVal;
  }
  updateTaxRiskIndicator();
  updateDistrictLivingStandardsPreview();
}

function onTaxSliderInput(taxType, val) {
  isUserDraggingTax = true;
  if (taxType === 'sales-gen') {
    const el = document.getElementById('txt-tax-sales-gen');
    if (el) el.textContent = parseFloat(val).toFixed(1) + '%';
  } else if (taxType === 'sales-lux') {
    const el = document.getElementById('txt-tax-sales-lux');
    if (el) el.textContent = parseFloat(val).toFixed(1) + '%';
  } else if (taxType === 'hauler-port') {
    const el = document.getElementById('txt-tax-hauler-port');
    if (el) el.textContent = parseFloat(val).toFixed(1) + '%';
  } else if (taxType === 'corp-wealth') {
    const el = document.getElementById('txt-tax-corp-wealth');
    if (el) el.textContent = parseFloat(val).toFixed(1) + '%';
  } else if (taxType === 'citizen-poll') {
    const el = document.getElementById('txt-tax-citizen-poll');
    const numVal = parseInt(val, 10);
    if (el) el.textContent = '$' + numVal.toLocaleString();
    for (let d = 0; d < 4; ++d) {
      const s = document.getElementById('slider-dist-tax-' + d);
      const t = document.getElementById('txt-dist-tax-' + d);
      if (s) s.value = numVal;
      if (t) t.textContent = '$' + numVal.toLocaleString();
      if (g_districtLivingData[d]) g_districtLivingData[d].pollTax = numVal;
    }
  }
  updateTaxRiskIndicator();
  updateDistrictLivingStandardsPreview();
}

function updateTaxRiskIndicator() {
  const genEl = document.getElementById('slider-tax-sales-gen');
  const luxEl = document.getElementById('slider-tax-sales-lux');
  const portEl = document.getElementById('slider-tax-hauler-port');
  const corpEl = document.getElementById('slider-tax-corp-wealth');
  if (!genEl || !luxEl || !portEl || !corpEl) return;

  const gen = parseFloat(genEl.value) / 100.0;
  const lux = parseFloat(luxEl.value) / 100.0;
  const port = parseFloat(portEl.value) / 100.0;
  const corp = parseFloat(corpEl.value) / 100.0;

  let maxDistrictTax = 0;
  for (let d = 0; d < 4; ++d) {
    const s = document.getElementById('slider-dist-tax-' + d);
    if (s) {
      const val = parseInt(s.value, 10);
      if (val > maxDistrictTax) maxDistrictTax = val;
    }
  }

  const isExcessive = (maxDistrictTax > 800 || port > 0.22 || gen > 0.20 || lux > 0.35 || corp > 0.10);
  const ind = document.getElementById('tax-risk-indicator');
  const txt = document.getElementById('tax-risk-text');
  const desc = document.getElementById('tax-risk-desc');
  if (ind && txt && desc) {
    if (isExcessive) {
      txt.textContent = '🔴 Civil Climate: High Unrest Warning (Riot & Strike Risk)';
      desc.textContent = 'Warning: Elevated levies exceed civic tolerance. Strikes, business defaults, and SWAT barricades expected.';
      ind.style.borderColor = 'rgba(239, 68, 68, 0.6)';
      ind.style.background = 'rgba(239, 68, 68, 0.15)';
      ind.style.color = '#fca5a5';
    } else {
      txt.textContent = '🟢 Civil Climate: Stable';
      desc.textContent = 'Tax rates within macroeconomic equilibrium tolerances.';
      ind.style.borderColor = 'rgba(16, 185, 129, 0.6)';
      ind.style.background = 'rgba(16, 185, 129, 0.15)';
      ind.style.color = '#6ee7b7';
    }
  }
}

async function applyFiscalPolicy() {
  const btn = document.getElementById('btn-apply-tax');
  const fb = document.getElementById('txt-tax-feedback');
  const genEl = document.getElementById('slider-tax-sales-gen');
  const luxEl = document.getElementById('slider-tax-sales-lux');
  const portEl = document.getElementById('slider-tax-hauler-port');
  const corpEl = document.getElementById('slider-tax-corp-wealth');
  const pollEl = document.getElementById('slider-tax-citizen-poll');
  if (!genEl || !luxEl || !portEl || !corpEl) return;

  try {
    if (btn) {
      btn.disabled = true;
      btn.style.opacity = '0.6';
    }
    if (fb) {
      fb.style.color = '#94a3b8';
      fb.textContent = 'Enacting fiscal reform...';
    }

    const distTaxes = [];
    for (let d = 0; d < 4; ++d) {
      const s = document.getElementById('slider-dist-tax-' + d);
      distTaxes.push(s ? parseFloat(s.value) : (g_districtLivingData[d].pollTax || 150));
    }

    const payload = {
      salesGeneral: parseFloat(genEl.value) / 100.0,
      salesLuxury: parseFloat(luxEl.value) / 100.0,
      haulerPort: parseFloat(portEl.value) / 100.0,
      corporateWealth: parseFloat(corpEl.value) / 100.0,
      citizenPoll: pollEl ? parseInt(pollEl.value, 10) : 270,
      districtTaxes: distTaxes
    };

    const res = await fetch('/api/municipal/taxes', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });
    const d = await res.json();
    if (res.ok && d.status === 'ok') {
      if (fb) {
        fb.style.color = '#34d399';
        fb.textContent = '✅ Fiscal Policy Enacted!';
      }
      setTimeout(() => {
        isUserDraggingTax = false;
        if (fb) fb.textContent = '';
      }, 3000);
      fetchMunicipal();
    } else {
      if (fb) {
        fb.style.color = '#ef4444';
        fb.textContent = '❌ Failed: ' + (d.message || res.statusText);
      }
    }
  } catch (err) {
    if (fb) {
      fb.style.color = '#ef4444';
      fb.textContent = '❌ Error: ' + err;
    }
  } finally {
    if (btn) {
      btn.disabled = false;
      btn.style.opacity = '1.0';
    }
  }
}

async function fetchMunicipal() {
  try {
    const res = await fetch('/api/municipal');
    if (!res.ok) return;
    const data = await res.json();

    // Treasury
    const treasuryEl = document.getElementById('val-treasury');
    const treasuryStatusEl = document.getElementById('txt-treasury-status');
    const trVal = data.treasury || 0;
    treasuryEl.textContent = '$' + Number(trVal).toLocaleString();
    if (trVal <= 0) {
      treasuryEl.style.color = '#ef4444';
      treasuryStatusEl.textContent = 'BANKRUPT (Default Emergency)';
    } else if (trVal < 200000) {
      treasuryEl.style.color = '#f59e0b';
      treasuryStatusEl.textContent = 'Fiscal Depletion Warning';
    } else {
      treasuryEl.style.color = '#fbbf24';
      treasuryStatusEl.textContent = 'Fiscal Reserve: Solvency Strong';
    }

    // Crime
    const crime = data.crime_rate || 0;
    document.getElementById('val-crime').textContent = crime.toFixed(1) + '%';
    const barCrime = document.getElementById('bar-crime');
    barCrime.style.width = Math.min(100, Math.max(0, crime)) + '%';
    const crimeStatusEl = document.getElementById('txt-crime-status');
    if (crime >= 75) {
      barCrime.style.background = '#ef4444';
      crimeStatusEl.textContent = 'Critical Gang Uprising';
      crimeStatusEl.style.color = '#f87171';
    } else if (crime >= 60) {
      barCrime.style.background = '#f59e0b';
      crimeStatusEl.textContent = 'Elevated: Tactical Dispatches Active';
      crimeStatusEl.style.color = '#fbbf24';
    } else {
      barCrime.style.background = '#10b981';
      crimeStatusEl.textContent = 'Order Maintained';
      crimeStatusEl.style.color = '#94a3b8';
    }
    if (!isUserDraggingCrime) {
      const sCrime = document.getElementById('slider-crime');
      if (sCrime) {
        sCrime.value = Math.round(crime);
        const sVal = document.getElementById('txt-slider-crime-val');
        if (sVal) sVal.textContent = crime.toFixed(1) + '%';
      }
    }

    // Unrest
    const unrest = data.social_unrest || 0;
    document.getElementById('val-unrest').textContent = unrest.toFixed(1) + '%';
    const barUnrest = document.getElementById('bar-unrest');
    barUnrest.style.width = Math.min(100, Math.max(0, unrest)) + '%';
    const unrestStatus = document.getElementById('txt-unrest-status');
    if (unrest >= 60) {
      barUnrest.style.background = '#ef4444';
      unrestStatus.textContent = 'Civil Riots & Unrest';
    } else if (unrest >= 30) {
      barUnrest.style.background = '#a855f7';
      unrestStatus.textContent = 'Citizen Grievance Simmering';
    } else {
      barUnrest.style.background = '#38bdf8';
      unrestStatus.textContent = 'Civil Stability: Normal';
    }
    if (!isUserDraggingUnrest) {
      const sUnrest = document.getElementById('slider-unrest');
      if (sUnrest) {
        sUnrest.value = Math.round(unrest);
        const sVal = document.getElementById('txt-slider-unrest-val');
        if (sVal) sVal.textContent = unrest.toFixed(1) + '%';
      }
    }

    // Taxes Matrix sync
    if (!isUserDraggingTax && data.taxes) {
      const t = data.taxes;
      if (t.salesGeneral !== undefined) {
        const sg = document.getElementById('slider-tax-sales-gen');
        if (sg) {
          sg.value = Math.round(t.salesGeneral * 100);
          const txt = document.getElementById('txt-tax-sales-gen');
          if (txt) txt.textContent = (t.salesGeneral * 100).toFixed(1) + '%';
        }
      }
      if (t.salesLuxury !== undefined) {
        const sl = document.getElementById('slider-tax-sales-lux');
        if (sl) {
          sl.value = Math.round(t.salesLuxury * 100);
          const txt = document.getElementById('txt-tax-sales-lux');
          if (txt) txt.textContent = (t.salesLuxury * 100).toFixed(1) + '%';
        }
      }
      if (t.haulerPort !== undefined) {
        const hp = document.getElementById('slider-tax-hauler-port');
        if (hp) {
          hp.value = Math.round(t.haulerPort * 100);
          const txt = document.getElementById('txt-tax-hauler-port');
          if (txt) txt.textContent = (t.haulerPort * 100).toFixed(1) + '%';
        }
      }
      if (t.corporateWealth !== undefined) {
        const cw = document.getElementById('slider-tax-corp-wealth');
        if (cw) {
          cw.value = (t.corporateWealth * 100).toFixed(1);
          const txt = document.getElementById('txt-tax-corp-wealth');
          if (txt) txt.textContent = (t.corporateWealth * 100).toFixed(1) + '%';
        }
      }
      if (t.citizenPoll !== undefined) {
        const cp = document.getElementById('slider-tax-citizen-poll');
        if (cp) {
          cp.value = t.citizenPoll;
          const txt = document.getElementById('txt-tax-citizen-poll');
          if (txt) txt.textContent = '$' + parseInt(t.citizenPoll, 10).toLocaleString();
        }
      }
      if (Array.isArray(t.districtTaxes)) {
        for (let d = 0; d < 4 && d < t.districtTaxes.length; ++d) {
          const val = t.districtTaxes[d];
          const s = document.getElementById('slider-dist-tax-' + d);
          const txt = document.getElementById('txt-dist-tax-' + d);
          if (s) s.value = Math.round(val);
          if (txt) txt.textContent = '$' + Math.round(val).toLocaleString();
          if (g_districtLivingData[d]) g_districtLivingData[d].pollTax = val;
        }
      }
      updateTaxRiskIndicator();
    }

    // Total Handled
    document.getElementById('val-handled').textContent = data.total_handled || 0;

    // Curfew Nav Button
    const btnCurfew = document.getElementById('btn-curfew');
    if (data.curfew_active) {
      btnCurfew.textContent = '🌙 Curfew: ACTIVE';
      btnCurfew.style.background = 'rgba(168, 85, 247, 0.25)';
      btnCurfew.style.borderColor = 'rgba(168, 85, 247, 0.6)';
      btnCurfew.style.color = '#c084fc';
    } else {
      btnCurfew.textContent = '🌙 Curfew: OFF';
      btnCurfew.style.background = 'rgba(255, 255, 255, 0.04)';
      btnCurfew.style.borderColor = 'var(--card-border)';
      btnCurfew.style.color = 'var(--text-main)';
    }

    // Emergency State Pill
    const pillState = document.getElementById('pill-emergency-state');
    const emState = data.emergency_state || 0;
    if (emState === 3) {
      pillState.textContent = 'STATE: RIOT / OUSTER';
      pillState.style.borderColor = '#ef4444';
      pillState.style.color = '#f87171';
    } else if (emState === 2) {
      pillState.textContent = 'STATE: DEFAULT / INSOLVENT';
      pillState.style.borderColor = '#f59e0b';
      pillState.style.color = '#fbbf24';
    } else if (emState === 1) {
      pillState.textContent = 'STATE: CRIME ALERT';
      pillState.style.borderColor = '#38bdf8';
      pillState.style.color = '#38bdf8';
    } else {
      pillState.textContent = 'STATE: NORMAL';
      pillState.style.borderColor = 'rgba(16, 185, 129, 0.3)';
      pillState.style.color = '#34d399';
    }

    // Fuel Status Pill & Banner
    const pillFuel = document.getElementById('pill-fuel-status');
    const fuelStock = (data.logistics && data.logistics.fuel_stock !== undefined) ? data.logistics.fuel_stock : 50;
    const fuelCrit = data.fuel_crisis || false;
    pillFuel.textContent = '⛽ SF Fuel: ' + fuelStock + 'u';
    const fuelBanner = document.getElementById('fuel-crisis-banner');
    if (fuelCrit) {
      pillFuel.style.borderColor = '#ef4444';
      pillFuel.style.color = '#f87171';
      fuelBanner.style.display = 'block';
    } else {
      pillFuel.style.borderColor = 'rgba(16, 185, 129, 0.3)';
      pillFuel.style.color = '#34d399';
      fuelBanner.style.display = 'none';
    }

    // Impeachment Banner
    const impBanner = document.getElementById('impeachment-banner');
    if (data.impeachment_triggered) {
      impBanner.style.display = 'block';
    } else {
      impBanner.style.display = 'none';
    }

    // Emergency Banner
    const banner = document.getElementById('emergency-banner');
    if (data.is_emergency || crime >= 75 || trVal <= 0) {
      banner.style.display = 'block';
      if (trVal <= 0) {
        document.getElementById('emergency-desc').textContent = 'City treasury has defaulted ($0). Law enforcement payroll frozen, inciting citywide riots. Veto decree to inject emergency state grant.';
      } else {
        document.getElementById('emergency-desc').textContent = 'Citywide crime surge has triggered emergency tactical deployment. Exercise Mayoral Veto to repeal aggressive orders.';
      }
    } else {
      banner.style.display = 'none';
    }

    // Syndicate Shadow Bribe Panel
    const bribePanel = document.getElementById('bribe-panel');
    if (data.bribe_offer && data.bribe_offer.active) {
      bribePanel.style.display = 'block';
      document.getElementById('bribe-syndicate').textContent = data.bribe_offer.syndicate;
      document.getElementById('bribe-desc').textContent = data.bribe_offer.desc;
      document.getElementById('bribe-amount').textContent = '+$' + Number(data.bribe_offer.amount).toLocaleString();
      document.getElementById('bribe-impact').textContent = 'Crime +' + data.bribe_offer.crime_delta + '% | Unrest +' + data.bribe_offer.unrest_delta + '%';
      document.getElementById('bribe-expires').textContent = data.bribe_offer.expires + 's remaining';
    } else {
      bribePanel.style.display = 'none';
    }

    // Union Strike Panel
    const strikePanel = document.getElementById('strike-panel');
    if (data.strike_event && data.strike_event.active) {
      strikePanel.style.display = 'block';
      document.getElementById('strike-union').textContent = data.strike_event.union;
      document.getElementById('strike-location').textContent = data.strike_event.location;
      document.getElementById('strike-duration').textContent = data.strike_event.duration + 's remaining';
    } else {
      strikePanel.style.display = 'none';
    }

    // Incidents Table
    const tbody = document.getElementById('incidents-tbody');
    const allIncidents = data.incidents || [];
    const activeIncidents = allIncidents.filter(inc => inc.active === true && inc.id > 0);
    document.getElementById('txt-active-count').textContent = 'Active Tactical Ops: ' + activeIncidents.length;

    if (activeIncidents.length === 0) {
      tbody.innerHTML = `<tr><td colspan="6" style="text-align:center; color:#64748b; padding:20px;">No critical emergency incidents currently active. State is secure.</td></tr>`;
    } else {
      tbody.innerHTML = activeIncidents.map(inc => {
        const incType = inc.type;
        const district = inc.location || "Los Santos";
        const severity = (inc.type === "Gang Shootout" || inc.type === 2) ? "CRITICAL" : ((inc.type === "SWAT Roadblock" || inc.type === 0) ? "HIGH" : "MODERATE");
        let sevClass = 'tag-progress';
        if (severity === 'CRITICAL') sevClass = 'tag-critical';
        else if (severity === 'HIGH') sevClass = 'tag-dispatched';

        const status = inc.in_combat ? "ENGAGED" : (inc.materialized ? "ON SCENE" : "DISPATCHED");
        let statClass = 'tag-dispatched';
        if (status === 'ENGAGED') statClass = 'tag-critical';
        else if (status === 'ON SCENE') statClass = 'tag-progress';

        const age = (inc.expires_in_ms ? Math.round(inc.expires_in_ms / 1000) + "s" : "0s");

        return `
          <tr>
            <td><b style="color:#94a3b8;">#${inc.id}</b></td>
            <td><b style="color:#f1f5f9;">${incType}</b></td>
            <td><span style="color:#38bdf8; font-weight:700;">${district}</span></td>
            <td><span class="status-tag ${sevClass}">${severity}</span></td>
            <td><span class="status-tag ${statClass}">${status}</span></td>
            <td style="color:#64748b;">${age}</td>
          </tr>
        `;
      }).join('');
    }

    // Decisions Log Feed
    const logConsole = document.getElementById('log-console');
    const logs = data.decisions_log || [];
    if (logs.length > 0) {
      logConsole.innerHTML = logs.map(l => `
        <div class="log-line">${l.message}</div>
      `).join('');
    }

    // District Living Standards Telemetry Sync
    try {
      const telemRes = await fetch('/telemetry');
      if (telemRes.ok) {
        const telem = await telemRes.json();
        if (telem.districtEcon && Array.isArray(telem.districtEcon)) {
          for (let i = 0; i < telem.districtEcon.length && i < 4; ++i) {
            const de = telem.districtEcon[i];
            if (de.wage) g_districtLivingData[i].wage = de.wage;
            if (de.effWage !== undefined) g_districtLivingData[i].effWage = de.effWage;
            if (de.bonus !== undefined) g_districtLivingData[i].bonus = de.bonus;
            if (de.housing !== undefined) g_districtLivingData[i].housing = de.housing;
            if (de.crime !== undefined) g_districtLivingData[i].crimeSurcharge = de.crime;
            if (de.basket) g_districtLivingData[i].basket = de.basket;
            if (de.tax !== undefined) g_districtLivingData[i].tax = de.tax;
            if (de.pollTax !== undefined) {
              g_districtLivingData[i].pollTax = de.pollTax;
              if (!isUserDraggingTax) {
                const s = document.getElementById('slider-dist-tax-' + i);
                const txt = document.getElementById('txt-dist-tax-' + i);
                if (s) s.value = Math.round(de.pollTax);
                if (txt) txt.textContent = '$' + Math.round(de.pollTax).toLocaleString();
              }
            }
            if (de.netRatio !== undefined) g_districtLivingData[i].netRatio = de.netRatio;
            if (de.status) g_districtLivingData[i].status = de.status;
            if (de.basket && de.effWage) {
              const genTax = (data.taxes && data.taxes.salesGeneral !== undefined) ? data.taxes.salesGeneral : 0.12;
              const bNet = (de.basket / K_FAMILY_MULT[i] - (g_fuelPriceMul * 75.0)) / (1.0 + genTax);
              if (bNet > 0) g_districtLivingData[i].storePrice = bNet / (6.5 * g_foodScarcityMult);
            }
          }
        }
        if (telem.port && telem.warehouses) {
          const totalFood = (telem.warehouses.food || 60) + Math.floor((telem.port.food || 120) / 2);
          g_foodScarcityMult = (totalFood < 50) ? 1.6 : ((totalFood < 100) ? 1.25 : 1.0);
        }
      }
    } catch (tErr) {
      /* non-fatal telemetry sync */
    }

    if (!isUserDraggingTax) {
      updateDistrictLivingStandardsPreview();
    }
  } catch (err) {
    console.error('Municipal polling error:', err);
  }
}

async function exerciseMayorVeto() {
  try {
    const res = await fetch('/api/municipal/veto');
    if (res.ok) {
      alert('⚖️ MAYORAL VETO EXERCISED:\nEmergency decree overturned. Emergency stabilization grant (+$25,000) deployed!');
      fetchMunicipal();
    }
  } catch (err) {
    alert('Failed to exercise veto: ' + err);
  }
}

async function toggleCurfew() {
  try {
    const res = await fetch('/api/municipal/curfew');
    if (res.ok) {
      fetchMunicipal();
    }
  } catch (err) {
    console.error('Failed to toggle curfew:', err);
  }
}

async function subsidizeStrike() {
  try {
    const res = await fetch('/api/municipal/strike/subsidize');
    if (res.ok) {
      const d = await res.json();
      if (d.status === 'ok') {
        alert('💵 Union Subsidy Disbursed ($20,000): Strikers disbanded, road cleared.');
      } else {
        alert('Subsidy failed: ' + (d.message || 'Insufficient funds'));
      }
      fetchMunicipal();
    }
  } catch (err) {
    alert('Subsidy call error: ' + err);
  }
}

async function acceptBribe() {
  try {
    const res = await fetch('/api/municipal/bribe/accept');
    if (res.ok) {
      const d = await res.json();
      if (d.status === 'ok') {
        alert('🤝 Shadow Bribe Accepted: Treasury credited, syndicate protection active.');
      }
      fetchMunicipal();
    }
  } catch (err) {
    alert('Failed to accept bribe: ' + err);
  }
}

async function rejectBribe() {
  try {
    const res = await fetch('/api/municipal/bribe/reject');
    if (res.ok) {
      alert('🛡️ Syndicate bribe rejected and denounced.');
      fetchMunicipal();
    }
  } catch (err) {
    alert('Failed to reject bribe: ' + err);
  }
}

let isUserDraggingCrime = false;
let isUserDraggingUnrest = false;
let crimeDebounceTimer = null;
let unrestDebounceTimer = null;

function onCrimeSliderInput(val) {
  isUserDraggingCrime = true;
  const sVal = document.getElementById('txt-slider-crime-val');
  if (sVal) sVal.textContent = parseFloat(val).toFixed(1) + '%';
  clearTimeout(crimeDebounceTimer);
  crimeDebounceTimer = setTimeout(() => {
    sendDebug('set_crime', val);
  }, 180);
}

function onCrimeSliderChange(val) {
  sendDebug('set_crime', val);
  setTimeout(() => { isUserDraggingCrime = false; }, 2000);
}

function onUnrestSliderInput(val) {
  isUserDraggingUnrest = true;
  const sVal = document.getElementById('txt-slider-unrest-val');
  if (sVal) sVal.textContent = parseFloat(val).toFixed(1) + '%';
  clearTimeout(unrestDebounceTimer);
  unrestDebounceTimer = setTimeout(() => {
    sendDebug('set_unrest', val);
  }, 180);
}

function onUnrestSliderChange(val) {
  sendDebug('set_unrest', val);
  setTimeout(() => { isUserDraggingUnrest = false; }, 2000);
}

async function sendDebug(action, val) {
  const key = document.getElementById('input-debug-key').value.trim();
  const statusEl = document.getElementById('txt-debug-status');
  try {
    statusEl.textContent = 'Executing ' + action + (val !== undefined ? ' (' + val + '%)' : '') + '...';
    let url = '/api/municipal/debug?debug_key=' + encodeURIComponent(key) + '&action=' + encodeURIComponent(action);
    if (val !== undefined && val !== null) {
      url += '&value=' + encodeURIComponent(val);
    }
    const res = await fetch(url);
    const d = await res.json();
    if (res.ok && d.status === 'ok') {
      statusEl.style.color = '#34d399';
      statusEl.textContent = 'Success: ' + action + (val !== undefined ? ' (' + val + '%)' : '');
      fetchMunicipal();
    } else {
      statusEl.style.color = '#ef4444';
      statusEl.textContent = 'Failed: ' + (d.message || res.statusText);
    }
  } catch (err) {
    statusEl.style.color = '#ef4444';
    statusEl.textContent = 'Network error: ' + err;
  }
}

setInterval(fetchMunicipal, 1000);
updateDistrictLivingStandardsPreview();
fetchMunicipal();
</script>
</body>
</html>
)rawmunicipal";
