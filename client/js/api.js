/**
 * API Service for remote telemetry ingestion.
 * Fetches encrypted datagrams and performs key-value extraction.
 */
export async function fetchTelemetry(url) {
    try {
        const response = await fetch(url);
        const data = await response.json();
        
        if (data.decrypted_payload) {
            const payload = data.decrypted_payload;
            
            /**
             * Internal helper to extract values via Regular Expressions.
             * Matches key patterns like "T:28.5" from CSV-style strings.
             */
            const extract = (key) => {
                const match = payload.match(new RegExp(`${key}:([^,]+)`));
                return match ? match[1] : null;
            };

            return {
                id: extract("ID"),
                temp: parseFloat(extract("T")),
                hum: parseFloat(extract("H")),
                soil: parseFloat(extract("S")),
                timestamp: new Date().toLocaleTimeString()
            };
        }
        return null;
    } catch (error) {
        console.error("[API_FATAL]: Data ingestion failed", error);
        return null;
    }
}