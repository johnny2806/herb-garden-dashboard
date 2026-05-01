/**
 * Global App Configuration.
 * Handles dynamic API endpoint detection.
 */
export const CONFIG = {
    PORT: "8000",
    ENDPOINT: "/api/v1/telemetry/latest",
    get API_URL() {
        const host = window.location.hostname || "localhost";
        return `http://${host}:${this.PORT}${this.ENDPOINT}`;
    },
    POLLING_MS: 2000
};