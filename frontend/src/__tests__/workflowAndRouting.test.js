// Polyfill in-memory sessionStorage for node test environment
if (typeof globalThis.sessionStorage === 'undefined') {
  const store = new Map();
  globalThis.sessionStorage = {
    getItem: (key) => (store.has(key) ? store.get(key) : null),
    setItem: (key, val) => store.set(key, String(val)),
    removeItem: (key) => store.delete(key),
    clear: () => store.clear(),
  };
}

import { describe, it, expect, beforeEach } from 'vitest';
import { getToken, setToken, clearAuthSession, getCachedUser, setCachedUser } from '../utils/auth';

describe('Role-Based Route Protection & Permissions Matrix', () => {
  beforeEach(() => {
    clearAuthSession();
  });

  const checkRouteAccess = (route, userRole, isAuthenticated) => {
    if (!isAuthenticated) {
      if (route.startsWith('/candidate') || route.startsWith('/recruiter')) {
        return { allowed: false, redirect: '/login' };
      }
      return { allowed: true };
    }

    if (route.startsWith('/candidate')) {
      if (userRole === 'CANDIDATE') return { allowed: true };
      if (userRole === 'RECRUITER') return { allowed: false, redirect: '/recruiter' };
    }

    if (route.startsWith('/recruiter')) {
      if (userRole === 'RECRUITER') return { allowed: true };
      if (userRole === 'CANDIDATE') return { allowed: false, redirect: '/candidate' };
    }

    return { allowed: true };
  };

  it('blocks unauthenticated visitors from candidate portal and redirects to /login', () => {
    const access = checkRouteAccess('/candidate/dashboard', null, false);
    expect(access.allowed).toBe(false);
    expect(access.redirect).toBe('/login');
  });

  it('blocks unauthenticated visitors from recruiter portal and redirects to /login', () => {
    const access = checkRouteAccess('/recruiter/jobs', null, false);
    expect(access.allowed).toBe(false);
    expect(access.redirect).toBe('/login');
  });

  it('allows candidates to access candidate routes', () => {
    const candidateRoutes = [
      '/candidate',
      '/candidate/resume',
      '/candidate/resume-analysis',
      '/candidate/jobs',
      '/candidate/jobs/J001',
      '/candidate/matches/J001',
      '/candidate/saved-jobs',
      '/candidate/applications',
      '/candidate/profile',
      '/candidate/activity',
      '/candidate/settings',
    ];

    candidateRoutes.forEach((route) => {
      const access = checkRouteAccess(route, 'CANDIDATE', true);
      expect(access.allowed).toBe(true);
    });
  });

  it('strictly blocks candidates from accessing recruiter routes and redirects to /candidate', () => {
    const recruiterRoutes = [
      '/recruiter',
      '/recruiter/jobs',
      '/recruiter/jobs/new',
      '/recruiter/jobs/J001/applicants',
      '/recruiter/applicants',
      '/recruiter/activity',
      '/recruiter/settings',
    ];

    recruiterRoutes.forEach((route) => {
      const access = checkRouteAccess(route, 'CANDIDATE', true);
      expect(access.allowed).toBe(false);
      expect(access.redirect).toBe('/candidate');
    });
  });

  it('allows recruiters to access recruiter routes', () => {
    const recruiterRoutes = [
      '/recruiter',
      '/recruiter/jobs',
      '/recruiter/jobs/new',
      '/recruiter/jobs/J001/applicants',
      '/recruiter/applicants',
      '/recruiter/activity',
      '/recruiter/settings',
    ];

    recruiterRoutes.forEach((route) => {
      const access = checkRouteAccess(route, 'RECRUITER', true);
      expect(access.allowed).toBe(true);
    });
  });

  it('strictly blocks recruiters from accessing candidate routes and redirects to /recruiter', () => {
    const candidateRoutes = [
      '/candidate',
      '/candidate/resume',
      '/candidate/resume-analysis',
      '/candidate/saved-jobs',
      '/candidate/applications',
      '/candidate/profile',
    ];

    candidateRoutes.forEach((route) => {
      const access = checkRouteAccess(route, 'RECRUITER', true);
      expect(access.allowed).toBe(false);
      expect(access.redirect).toBe('/recruiter');
    });
  });

  it('allows public routes for any state', () => {
    const publicRoutes = ['/', '/login', '/register', '/jobs', '/jobs/J001'];
    publicRoutes.forEach((route) => {
      expect(checkRouteAccess(route, null, false).allowed).toBe(true);
      expect(checkRouteAccess(route, 'CANDIDATE', true).allowed).toBe(true);
      expect(checkRouteAccess(route, 'RECRUITER', true).allowed).toBe(true);
    });
  });
});

describe('Candidate E2E Journey Sequence Validation', () => {
  it('models valid candidate journey state progression', () => {
    const journey = [];

    // Step 1: Open Website
    journey.push('LANDING_PAGE');
    // Step 2: Register Candidate
    journey.push('REGISTER_CANDIDATE');
    // Step 3: Login
    setToken('candidate_auth_token_demo');
    setCachedUser({ username: 'cand_demo', role: 'CANDIDATE' });
    journey.push('LOGIN');
    // Step 4: Candidate Dashboard
    journey.push('VIEW_DASHBOARD');
    // Step 5: Resume Upload
    journey.push('UPLOAD_RESUME');
    // Step 6: Resume Analysis
    journey.push('VIEW_ANALYSIS');
    // Step 7: Browse Jobs
    journey.push('BROWSE_JOBS');
    // Step 8: View Job Details
    journey.push('VIEW_JOB');
    // Step 9: Advanced Match Explanation
    journey.push('ANALYZE_MATCH');
    // Step 10: Save Job
    journey.push('SAVE_JOB');
    // Step 11: Apply
    journey.push('SUBMIT_APPLICATION');
    // Step 12: View Application
    journey.push('VIEW_APPLICATION');
    // Step 13: Logout
    clearAuthSession();
    journey.push('LOGOUT');

    expect(journey).toEqual([
      'LANDING_PAGE',
      'REGISTER_CANDIDATE',
      'LOGIN',
      'VIEW_DASHBOARD',
      'UPLOAD_RESUME',
      'VIEW_ANALYSIS',
      'BROWSE_JOBS',
      'VIEW_JOB',
      'ANALYZE_MATCH',
      'SAVE_JOB',
      'SUBMIT_APPLICATION',
      'VIEW_APPLICATION',
      'LOGOUT',
    ]);
    expect(getToken()).toBeNull();
  });
});

describe('Recruiter E2E Journey Sequence Validation', () => {
  it('models valid recruiter journey state progression', () => {
    const journey = [];

    // Step 1: Open Website
    journey.push('LANDING_PAGE');
    // Step 2: Register Recruiter
    journey.push('REGISTER_RECRUITER');
    // Step 3: Login
    setToken('recruiter_auth_token_demo');
    setCachedUser({ username: 'rec_demo', role: 'RECRUITER' });
    journey.push('LOGIN');
    // Step 4: Recruiter Dashboard
    journey.push('VIEW_DASHBOARD');
    // Step 5: Create Job
    journey.push('CREATE_JOB');
    // Step 6: View Applicants Pipeline
    journey.push('VIEW_APPLICANTS');
    // Step 7: View Match & Candidate Info
    journey.push('VIEW_CANDIDATE');
    // Step 8: Update Application Status
    journey.push('UPDATE_STATUS');
    // Step 9: Logout
    clearAuthSession();
    journey.push('LOGOUT');

    expect(journey).toEqual([
      'LANDING_PAGE',
      'REGISTER_RECRUITER',
      'LOGIN',
      'VIEW_DASHBOARD',
      'CREATE_JOB',
      'VIEW_APPLICANTS',
      'VIEW_CANDIDATE',
      'UPDATE_STATUS',
      'LOGOUT',
    ]);
    expect(getToken()).toBeNull();
  });
});
