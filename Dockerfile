# ============================================================
# SmartHire — Multi-Stage Production Dockerfile
# Compiles React Frontend + C++17 REST Server into a single,
# lightweight, self-contained production container.
# ============================================================

# ── Stage 1: Build React Frontend ───────────────────────────
FROM node:20-alpine AS frontend-builder
WORKDIR /app/frontend

COPY frontend/package*.json ./
RUN npm ci || npm install

COPY frontend/ ./
RUN npm run build

# ── Stage 2: Build C++ Backend Server ───────────────────────
FROM alpine:3.20 AS backend-builder
RUN apk add --no-cache build-base cmake linux-headers

WORKDIR /app
COPY CMakeLists.txt ./
COPY backend/ ./backend/
COPY tests/ ./tests/

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --target SmartHire -j$(nproc)

# ── Stage 3: Lean Production Runtime ─────────────────────────
FROM alpine:3.20
RUN apk add --no-cache libstdc++ libgcc ca-certificates

WORKDIR /app

# Copy compiled single-binary backend
COPY --from=backend-builder /app/build/SmartHire ./SmartHire

# Copy static frontend distribution (served directly by SmartHire)
COPY --from=frontend-builder /app/frontend/dist ./frontend/dist

# Copy configurations, database schema, and templates
COPY config/ ./config/
COPY data/ ./data/

# Ensure required runtime write directories exist
RUN mkdir -p data/uploads reports

# Dynamic port binding support (Cloud hosts like Railway/Render set PORT)
EXPOSE 8080

ENV SMARTHIRE_HOST="0.0.0.0" \
    SMARTHIRE_PORT="8080" \
    SMARTHIRE_DB_PATH="data/smarthire.db" \
    SMARTHIRE_CORS_ORIGIN="*"

CMD ["./SmartHire", "--server"]
