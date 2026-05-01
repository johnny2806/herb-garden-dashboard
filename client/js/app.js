import { CONFIG } from './config.js';
import { fetchTelemetry } from './api.js';
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