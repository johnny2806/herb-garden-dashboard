import { CONFIG } from './config.js';
import { fetchTelemetry, sendCommand } from './api.js';
import { updateDashboard, ui } from './ui.js';

async function main() {
    console.log(`[System] Herb Garden Node Dashboard initialized.`);

    setInterval(async () => {
        const data = await fetchTelemetry(CONFIG.API_URL);
        if (data) {
            updateDashboard(data);
        } else {
            ui.time.textContent = "STATUS: OFFLINE";
        }
    }, CONFIG.POLLING_MS);
}

document.addEventListener('DOMContentLoaded', main);
document.getElementById('btn_pump_on').onclick = () => sendCommand(true);
document.getElementById('btn_pump_off').onclick = () => sendCommand(false);
document.getElementById('btn_pump_auto').onclick = () => sendCommand(null);