import { describe, it, expect, beforeEach } from 'vitest';
import {
  getToken,
  setToken,
  removeToken,
  getCachedUser,
  setCachedUser,
  clearAuthSession,
} from '../utils/auth';
import { formatScore, getScoreColor, getStatusBadgeClass, formatDate } from '../utils/formatters';
import { validateEmail, validatePassword } from '../utils/validators';

// Polyfill in-memory sessionStorage if not present in node runner
if (typeof globalThis.sessionStorage === 'undefined') {
  const store = new Map();
  globalThis.sessionStorage = {
    getItem: (key) => (store.has(key) ? store.get(key) : null),
    setItem: (key, val) => store.set(key, String(val)),
    removeItem: (key) => store.delete(key),
    clear: () => store.clear(),
  };
}

describe('Auth Utilities & Session Storage', () => {
  beforeEach(() => {
    sessionStorage.clear();
  });

  it('stores and retrieves bearer token in sessionStorage', () => {
    expect(getToken()).toBeNull();
    setToken('token_test_12345');
    expect(getToken()).toBe('token_test_12345');
    expect(sessionStorage.getItem('smarthire_token')).toBe('token_test_12345');
  });

  it('removes bearer token on logout', () => {
    setToken('token_test_12345');
    removeToken();
    expect(getToken()).toBeNull();
  });

  it('stores and retrieves cached user object in sessionStorage', () => {
    const user = { username: 'alice', role: 'CANDIDATE', email: 'alice@example.com' };
    setCachedUser(user);
    expect(getCachedUser()).toEqual(user);
  });

  it('clears complete auth session', () => {
    setToken('token_abc');
    setCachedUser({ username: 'bob', role: 'RECRUITER' });
    clearAuthSession();
    expect(getToken()).toBeNull();
    expect(getCachedUser()).toBeNull();
  });
});

describe('Formatters', () => {
  it('formats score to 1 decimal place with percentage', () => {
    expect(formatScore(72.954)).toBe('73.0%');
    expect(formatScore(85)).toBe('85.0%');
    expect(formatScore(null)).toBe('0.0%');
    expect(formatScore(undefined)).toBe('0.0%');
  });

  it('assigns correct color tokens based on score thresholds', () => {
    expect(getScoreColor(85)).toBe('var(--success)');
    expect(getScoreColor(70)).toBe('var(--info)');
    expect(getScoreColor(60)).toBe('var(--info)');
    expect(getScoreColor(50)).toBe('var(--warning)');
    expect(getScoreColor(40)).toBe('var(--warning)');
    expect(getScoreColor(35)).toBe('var(--danger)');
  });

  it('returns valid status badge classes for all application states', () => {
    expect(getStatusBadgeClass('APPLIED')).toBe('badge-info');
    expect(getStatusBadgeClass('UNDER_REVIEW')).toBe('badge-primary');
    expect(getStatusBadgeClass('SHORTLISTED')).toBe('badge-warning');
    expect(getStatusBadgeClass('INTERVIEW')).toBe('badge-info');
    expect(getStatusBadgeClass('SELECTED')).toBe('badge-success');
    expect(getStatusBadgeClass('REJECTED')).toBe('badge-danger');
    expect(getStatusBadgeClass('WITHDRAWN')).toBe('badge-neutral');
    expect(getStatusBadgeClass('UNKNOWN')).toBe('badge-neutral');
  });

  it('formats valid dates and handles invalid inputs gracefully', () => {
    const formatted = formatDate('2026-09-26T12:00:00Z');
    expect(formatted).not.toBe('N/A');
    expect(formatDate(null)).toBe('N/A');
    expect(formatDate('')).toBe('N/A');
  });
});

describe('Validators', () => {
  it('validates email addresses properly', () => {
    expect(validateEmail('test@example.com')).toBeNull();
    expect(validateEmail('user.name+tag@sub.domain.org')).toBeNull();
    expect(validateEmail('plainaddress')).not.toBeNull();
    expect(validateEmail('@missinguser.com')).not.toBeNull();
    expect(validateEmail('')).not.toBeNull();
  });

  it('validates password security requirements', () => {
    // Valid password
    expect(validatePassword('SecureP@ss123')).toBeNull();

    // Too short (< 8 chars)
    expect(validatePassword('Short1!')).not.toBeNull();

    // Missing uppercase
    expect(validatePassword('nouppercase123!')).not.toBeNull();

    // Missing lowercase
    expect(validatePassword('NOLOWERCASE123!')).not.toBeNull();

    // Missing digit
    expect(validatePassword('NoDigitSpecial!')).not.toBeNull();
  });
});
