/**
 * UI Instrumentation Module.
 * Manages gauge rendering and real-time DOM updates.
 */

// Custom Chart.js plugin for technical gauge ticks
const gaugeTicksPlugin = {
    id: 'gaugeTicks',
    afterDraw: (chart) => {
        const { ctx } = chart;
        const meta = chart.getDatasetMeta(0).data[0];
        if (!meta) return;

        const { x: centerX, y: centerY, outerRadius } = meta;
        const maxVal = chart.config.options.customMax || 100;

        ctx.save();
        ctx.translate(centerX, centerY);
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.font = 'bold 10px Inter';

        for (let i = 0; i <= 10; i++) {
            const angle = Math.PI + (i * Math.PI / 10);
            const labelValue = Math.round((i / 10) * maxVal);

            ctx.strokeStyle = '#D1D5DB';
            ctx.beginPath();
            ctx.moveTo(Math.cos(angle) * (outerRadius + 2), Math.sin(angle) * (outerRadius + 2));
            ctx.lineTo(Math.cos(angle) * (outerRadius + 8), Math.sin(angle) * (outerRadius + 8));
            ctx.stroke();

            ctx.fillStyle = '#9CA3AF';
            const labelX = Math.cos(angle) * (outerRadius + 20);
            const labelY = Math.sin(angle) * (outerRadius + 20);
            ctx.fillText(labelValue, labelX, labelY);
        }
        ctx.restore();
    }
};

Chart.register(gaugeTicksPlugin);

export const ui = {
    id: document.getElementById('identity_display'),
    auth: document.getElementById('auth_status'),
    temp: document.getElementById('temperature_display'),
    hum: document.getElementById('air_humidity_display'),
    soil: document.getElementById('moisture_display'),
    time: document.getElementById('update_timestamp'),
    tempCtx: document.getElementById('tempGauge').getContext('2d'),
    humCtx: document.getElementById('humGauge').getContext('2d'),
    soilCtx: document.getElementById('soilGauge').getContext('2d'),
    sys_rssi: document.getElementById('ui_rssi'),
    sys_core: document.getElementById('ui_core_temp'),
    sys_heap: document.getElementById('ui_heap'),
    sys_uptime: document.getElementById('ui_uptime'),
    sys_soil_raw: document.getElementById('ui_soil_raw'),
    sys_pump: document.getElementById('ui_pump_status'),
    sys_mode: document.getElementById('mode_display')
};

const createGaugeConfig = (color, max) => ({
    type: 'doughnut',
    data: {
        datasets: [{
            data: [0, max],
            backgroundColor: [color, '#F3F4F6'],
            circumference: 180,
            rotation: 270,
            cutout: '82%',
            borderRadius: 15
        }]
    },
    options: {
        customMax: max,
        responsive: true,
        maintainAspectRatio: false,
        layout: { padding: 35 },
        plugins: { legend: { display: false }, tooltip: { enabled: false } }
    }
});

export const gauges = {
    temp: new Chart(ui.tempCtx, createGaugeConfig('#10B981', 50)),
    hum: new Chart(ui.humCtx, createGaugeConfig('#8B5CF6', 100)),
    soil: new Chart(ui.soilCtx, createGaugeConfig('#3B82F6', 100))
};

export function updateDashboard(data) {
    if (!data) return;

    ui.id.textContent = data.id;
    ui.temp.textContent = data.temp.toFixed(2);
    ui.hum.textContent = data.hum.toFixed(2);
    ui.soil.textContent = data.soil.toFixed(2);
    ui.time.textContent = `LAST_SYNC: ${data.timestamp}`;
    // Display RSSI with color coding (red if weak signal)
    ui.sys_rssi.textContent = data.rssi_dbm;
    ui.sys_rssi.style.color = data.rssi_dbm < -80 ? "#EF4444" : "#1F2937"; // Red if weak signal

    // Display core temperature with color coding (red if too hot)
    ui.sys_core.textContent = data.core_temp.toFixed(1);
    ui.sys_core.style.color = data.core_temp > 70 ? "#EF4444" : "#1F2937"; // Red if too hot

    // Change heap display to KB and color code (red if low memory)
    ui.sys_heap.textContent = Math.round(data.free_heap_bytes / 1024);
    ui.sys_heap.style.color = data.free_heap_bytes < 10240 ? "#EF4444" : "#1F2937"; // Red if low memory

    // Calculate Uptime (Showing only seconds for the cool factor)
    ui.sys_uptime.textContent = data.uptime_sec;

    ui.sys_soil_raw.textContent = data.soil_raw;

    // Actuator state visual feedback
    ui.sys_pump.textContent = data.pump_is_active ? "ENGAGED" : "STANDBY";
    ui.sys_pump.style.color = data.pump_is_active ? "#10B981" : "#6B7280"; // Emerald if running, Gray if standby

    if (data.current_mode === true) {
        ui.sys_mode.textContent = "MODE: MANUAL - FORCE ON";
        ui.sys_mode.style.color = "#10B981"; // Green for force on
    } else if (data.current_mode === false) {
        ui.sys_mode.textContent = "MODE: MANUAL - FORCE OFF";
        ui.sys_mode.style.color = "#EF4444"; // Red for manual off
    } else {
        ui.sys_mode.textContent = "MODE: AUTOMATIC";
        ui.sys_mode.style.color = "#6B7280"; // Gray for automatic mode
    }

    // Authentication visual feedback
    if (data.id && data.id.includes("28:CD:C1")) {
        ui.auth.textContent = "STEALTH_AUTHENTICATED";
        ui.auth.style.color = "#059669";
    }

    const refreshGauge = (gauge, value, max) => {
        const clampedValue = Math.min(Math.max(value, 0), max);
        gauge.data.datasets[0].data = [clampedValue, max - clampedValue];
        gauge.update();
    };

    refreshGauge(gauges.temp, data.temp, 50);
    refreshGauge(gauges.hum, data.hum, 100);
    refreshGauge(gauges.soil, data.soil, 100);
}