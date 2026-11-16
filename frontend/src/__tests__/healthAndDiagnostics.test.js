import { describe, it, expect, vi, beforeEach } from 'vitest';
import apiClient from '../api/apiClient';
import { dashboardApi } from '../api/dashboardApi';
import { clearAuthSession, setToken } from '../utils/auth';

// Polyfill in-memory sessionStorage for node test runner
if (typeof globalThis.sessionStorage === 'undefined') {
  const store = new Map();
  globalThis.sessionStorage = {
    getItem: (key) => (store.has(key) ? store.get(key) : null),
    setItem: (key, val) => store.set(key, String(val)),
    removeItem: (key) => store.delete(key),
    clear: () => store.clear(),
  };
}

describe('Health & Readiness Diagnostics API', () => {
  beforeEach(() => {
    clearAuthSession();
    vi.restoreAllMocks();
  });

  it('unwraps system health check with liveness and readiness data', async () => {
    const mockHealthPayload = {
      status: 'UP',
      liveness: 'OK',
      readiness: 'READY',
      application: 'SmartHire',
      version: '11.0',
      database: 'CONNECTED',
      databaseQuery: 'OK',
      uploadsDirectory: 'OK',
    };

    global.fetch = vi.fn().mockResolvedValue({
      ok: true,
      status: 200,
      headers: new Headers({ 'content-type': 'application/json' }),
      text: async () =>
        JSON.stringify({
          success: true,
          message: 'System is healthy.',
          data: mockHealthPayload,
        }),
    });

    const res = await dashboardApi.getHealth();
    expect(res.success).toBe(true);
    expect(res.data.liveness).toBe('OK');
    expect(res.data.readiness).toBe('READY');
    expect(res.data.database).toBe('CONNECTED');
  });

  it('handles liveness check endpoint (/health/live)', async () => {
    global.fetch = vi.fn().mockResolvedValue({
      ok: true,
      status: 200,
      headers: new Headers({ 'content-type': 'application/json' }),
      text: async () =>
        JSON.stringify({
          success: true,
          data: { status: 'ok', liveness: 'OK' },
        }),
    });

    const res = await apiClient.get('/health/live');
    expect(res.success).toBe(true);
    expect(res.data.status).toBe('ok');
    expect(res.data.liveness).toBe('OK');
  });

  it('handles readiness check endpoint (/health/ready) with component checks', async () => {
    global.fetch = vi.fn().mockResolvedValue({
      ok: true,
      status: 200,
      headers: new Headers({ 'content-type': 'application/json' }),
      text: async () =>
        JSON.stringify({
          success: true,
          data: {
            status: 'ok',
            readiness: 'READY',
            database: 'ok',
            databaseQuery: 'ok',
            uploads: 'ok',
          },
        }),
    });

    const res = await apiClient.get('/health/ready');
    expect(res.success).toBe(true);
    expect(res.data.readiness).toBe('READY');
    expect(res.data.database).toBe('ok');
    expect(res.data.uploads).toBe('ok');
  });
});

describe('Production Error Normalization Matrix', () => {
  const errorScenarios = [
    { status: 400, code: 'VALIDATION_FAILED', message: 'Name is required' },
    { status: 401, code: 'UNAUTHORIZED_EXPIRED', message: 'Session expired' },
    { status: 403, code: 'FORBIDDEN_CANDIDATE_ONLY', message: 'Candidate only' },
    { status: 404, code: 'JOB_NOT_FOUND', message: 'Job not found' },
    { status: 409, code: 'DUPLICATE_APPLICATION', message: 'Already applied' },
    { status: 429, code: 'RATE_LIMIT_EXCEEDED', message: 'Too many requests' },
    { status: 500, code: 'DATABASE_ERROR', message: 'Internal error' },
  ];

  errorScenarios.forEach(({ status, code, message }) => {
    it(`normalizes HTTP ${status} error with code '${code}'`, async () => {
      global.fetch = vi.fn().mockResolvedValue({
        ok: false,
        status,
        headers: new Headers({ 'content-type': 'application/json' }),
        text: async () =>
          JSON.stringify({
            success: false,
            message,
            error: { code, details: {} },
          }),
      });

      await expect(apiClient.get('/test')).rejects.toMatchObject({
        status,
        code,
        message,
      });
    });
  });
});
