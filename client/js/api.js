/**
 * Telemetry Fetcher.
 * Retrieves verified JSON payloads from the REST API.
 */
export async function fetchTelemetry(url) {
    try {
        const response = await fetch(url);
        const data = await response.json();

        // Return structured data only if the node is verified as ONLINE
        if (data && data.telemetry && data.telemetry.status === "ONLINE") {
            return {
                id: data.telemetry.identity,
                temp: data.telemetry.temperature_celsius,
                hum: data.telemetry.humidity_percentage,
                soil: data.telemetry.saturation_percentage,
                soil_raw: data.telemetry.soil_raw_adc,
                timestamp: new Date().toLocaleTimeString(),
                core_temp: data.telemetry.core_temp,
                free_heap_bytes: data.telemetry.free_heap_bytes,
                rssi_dbm: data.telemetry.rssi_dbm,
                uptime_sec: data.telemetry.uptime_sec,
                pump_is_active: data.telemetry.pump_is_active,
                pump_command: data.command
            };
        }
        return null;
    } catch (error) {
        console.error("[API_ERROR]: Connection refused or malformed response.", error);
        return null;
    }
}

export async function sendCommand(state) {
    try {
        const stateStr = state === null ? "null" : state.toString();
        const host = window.location.hostname || "localhost";
        await fetch(`http://${host}:8000/api/v1/control/pump?state=${stateStr}`, {
            method: 'POST'
        });
    } catch (e) { console.error("Command failed", e); }
}