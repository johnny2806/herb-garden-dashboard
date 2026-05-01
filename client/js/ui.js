/**
 * UI Controller Module - Precision Instrumentation Edition
 * Synchronizes custom gauge ticks with Chart.js internal coordinate system.
 */

const gaugeTicksPlugin = {
    id: 'gaugeTicks',
    afterDraw: (chart) => {
        const { ctx } = chart;
        // Retrieve the EXACT coordinates used by the Chart.js engine
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

            // Draw Ticks: Anchored precisely to the outer radius boundary
            ctx.strokeStyle = '#D1D5DB';
            ctx.lineWidth = 1.5;
            ctx.beginPath();
            ctx.moveTo(Math.cos(angle) * (outerRadius + 2), Math.sin(angle) * (outerRadius + 2));
            ctx.lineTo(Math.cos(angle) * (outerRadius + 8), Math.sin(angle) * (outerRadius + 8));
            ctx.stroke();

            // Draw Numeric Labels: Offset for optimal legibility
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
    soilCtx: document.getElementById('soilGauge').getContext('2d')
};

const createGaugeConfig = (color, max) => ({
    type: 'doughnut',
    data: {
        datasets: [{
            data: [0, max],
            backgroundColor: [color, '#F3F4F6'],
            borderWidth: 0,
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
        // Provided padding for external label clearance
        layout: { padding: { top: 30, bottom: 10, left: 35, right: 35 } },
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