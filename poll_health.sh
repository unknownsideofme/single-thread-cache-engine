#!/usr/bin/env bash
# Script to continuously poll the /health endpoint of the cache server.

PORT="${1:-${PORT:-8080}}"
INTERVAL="${2:-1}"

echo "Continuously polling http://localhost:${PORT}/health every ${INTERVAL}s..."
echo "Press Ctrl+C to stop."
echo "--------------------------------------------------------"

while true; do
    TIMESTAMP=$(date +"%Y-%m-%d %H:%M:%S")
    RESPONSE=$(curl -s -w "\nHTTP_STATUS:%{http_code}" "http://localhost:${PORT}/health")
    HTTP_STATUS=$(echo "$RESPONSE" | grep "HTTP_STATUS:" | cut -d':' -f2)
    BODY=$(echo "$RESPONSE" | grep -v "HTTP_STATUS:")

    if [ "$HTTP_STATUS" = "200" ]; then
        echo "[$TIMESTAMP] [HEALTH OK] Status: $HTTP_STATUS | Response: $BODY"
    else
        echo "[$TIMESTAMP] [HEALTH FAIL] Status: $HTTP_STATUS | Server may be down or unreachable"
    fi
    sleep "$INTERVAL"
done
