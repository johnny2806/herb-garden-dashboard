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
                timestamp: new Date().toLocaleTimeString()
            };
        }
        return null;
    } catch (error) {
        console.error("[API_ERROR]: Connection refused or malformed response.", error);
        return null;
    }
}