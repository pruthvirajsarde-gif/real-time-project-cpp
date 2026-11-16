import { getToken, removeToken } from '../utils/auth';

const API_BASE_URL =
  import.meta.env.VITE_API_BASE_URL ||
  (typeof window !== 'undefined' && window.location.hostname
    ? `http://${window.location.hostname}:8080/api/v1`
    : 'http://127.0.0.1:8080/api/v1');

let onUnauthorizedCallback = null;

export const setOnUnauthorizedCallback = (callback) => {
  onUnauthorizedCallback = callback;
};

/**
 * Core fetch wrapper with envelope parsing and error normalization
 */
async function request(endpoint, options = {}) {
  const url = endpoint.startsWith('http') ? endpoint : `${API_BASE_URL}${endpoint}`;
  const token = getToken();

  const headers = { ...options.headers };

  // Attach Bearer token if available and not explicitly skipped
  if (token && !options.skipAuth) {
    headers['Authorization'] = `Bearer ${token}`;
  }

  // Set JSON Content-Type only if body is NOT FormData
  if (!(options.body instanceof FormData) && !headers['Content-Type']) {
    headers['Content-Type'] = 'application/json';
  }

  try {
    const response = await fetch(url, {
      ...options,
      headers,
    });

    // Handle 204 No Content
    if (response.status === 204) {
      return { success: true, message: 'Success', data: null };
    }

    let payload = null;
    const text = await response.text();
    if (text) {
      try {
        payload = JSON.parse(text);
      } catch (err) {
        payload = { message: text };
      }
    }

    if (!response.ok) {
      // 401 Unauthorized handling
      if (response.status === 401) {
        if (token) {
          removeToken();
          if (onUnauthorizedCallback) {
            onUnauthorizedCallback(payload?.message || 'Your session has expired. Please log in again.');
          }
        }
      }

      const errorMsg =
        payload?.message ||
        payload?.error?.details ||
        `Request failed with status ${response.status}`;

      const error = new Error(errorMsg);
      error.status = response.status;
      error.code = payload?.error?.code || 'UNKNOWN_ERROR';
      error.data = payload?.data || null;
      throw error;
    }

    return payload;
  } catch (err) {
    if (err.name === 'TypeError' && err.message.includes('fetch')) {
      const serverErr = new Error(
        'SmartHire server is unavailable. Please verify the backend is running with: SmartHire.exe --server'
      );
      serverErr.status = 0;
      serverErr.code = 'SERVER_UNAVAILABLE';
      throw serverErr;
    }
    throw err;
  }
}

export const apiClient = {
  get: (endpoint, headers = {}) => request(endpoint, { method: 'GET', headers }),
  post: (endpoint, body = {}, headers = {}) =>
    request(endpoint, {
      method: 'POST',
      headers,
      body: body instanceof FormData ? body : JSON.stringify(body),
    }),
  put: (endpoint, body = {}, headers = {}) =>
    request(endpoint, {
      method: 'PUT',
      headers,
      body: body instanceof FormData ? body : JSON.stringify(body),
    }),
  del: (endpoint, headers = {}) => request(endpoint, { method: 'DELETE', headers }),
  upload: (endpoint, formData, headers = {}) =>
    request(endpoint, {
      method: 'POST',
      headers,
      body: formData,
    }),
};

export default apiClient;
