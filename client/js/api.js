/**
 * Telemetry Fetcher.
 * Retrieves verified JSON payloads from the REST API.
 */
export async function fetchTelemetry(url) {
    try {
        const response = await fetch(url);
        const data = await response.json();

        // Return structured data only if the node is verified as ONLINE
        if (data.status === "ONLINE") {
            return {
                id: data.identity,
                temp: data.temperature_celsius,
                hum: data.humidity_percentage,
                soil: data.saturation_percentage,
                soil_raw: data.soil_raw_adc,
                timestamp: new Date().toLocaleTimeString(),
                core_temp: data.core_temp,
                free_heap_bytes: data.free_heap_bytes,
                rssi_dbm: data.rssi_dbm,
                uptime_sec: data.uptime_sec
            };
        }
        return null;
    } catch (error) {
        console.error("[API_ERROR]: Connection refused or malformed response.", error);
        return null;
    }
}