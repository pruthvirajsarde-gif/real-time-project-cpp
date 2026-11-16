import { describe, it, expect, beforeEach, vi, afterEach } from 'vitest';

// Polyfill in-memory sessionStorage and window event dispatching for node test environment
if (typeof globalThis.sessionStorage === 'undefined') {
  const store = new Map();
  globalThis.sessionStorage = {
    getItem: (key) => (store.has(key) ? store.get(key) : null),
    setItem: (key, val) => store.set(key, String(val)),
    removeItem: (key) => store.delete(key),
    clear: () => store.clear(),
  };
}

import apiClient, { setOnUnauthorizedCallback } from '../api/apiClient';
import { authApi } from '../api/authApi';
import { candidateApi } from '../api/candidateApi';
import { recruiterApi } from '../api/recruiterApi';
import { jobsApi } from '../api/jobsApi';
import { resumeApi } from '../api/resumeApi';
import { matchingApi } from '../api/matchingApi';
import { applicationApi } from '../api/applicationApi';
import { savedJobsApi } from '../api/savedJobsApi';
import { dashboardApi } from '../api/dashboardApi';
import { setToken, clearAuthSession } from '../utils/auth';

describe('API Client & HTTP Envelopes', () => {
  beforeEach(() => {
    clearAuthSession();
    vi.restoreAllMocks();
  });

  afterEach(() => {
    vi.restoreAllMocks();
  });

  it('unwraps successful API response envelope', async () => {
    const mockData = { id: 101, username: 'testuser' };
    const envelope = {
      success: true,
      message: 'Account details retrieved.',
      data: mockData,
    };

    global.fetch = vi.fn().mockResolvedValue({
      ok: true,
      status: 200,
      headers: new Headers({ 'content-type': 'application/json' }),
      text: async () => JSON.stringify(envelope),
    });

    const result = await apiClient.get('/auth/me');
    expect(result.success).toBe(true);
    expect(result.data).toEqual(mockData);
    expect(result.message).toBe('Account details retrieved.');
  });

  it('automatically attaches Authorization Bearer header when token exists', async () => {
    setToken('jwt_bearer_token_xyz');

    global.fetch = vi.fn().mockImplementation((url, options) => {
      expect(options.headers['Authorization']).toBe('Bearer jwt_bearer_token_xyz');
      return Promise.resolve({
        ok: true,
        status: 200,
        headers: new Headers({ 'content-type': 'application/json' }),
        text: async () => JSON.stringify({ success: true, data: { ok: true } }),
      });
    });

    await apiClient.get('/candidate/profile');
  });

  it('rejects with standardized error when API returns success: false', async () => {
    const errEnvelope = {
      success: false,
      message: 'Validation failed for registration.',
      error: { code: 'VALIDATION_FAILED', details: {} },
    };

    global.fetch = vi.fn().mockResolvedValue({
      ok: false,
      status: 400,
      headers: new Headers({ 'content-type': 'application/json' }),
      text: async () => JSON.stringify(errEnvelope),
    });

    await expect(apiClient.post('/auth/register/candidate', {})).rejects.toMatchObject({
      message: 'Validation failed for registration.',
      code: 'VALIDATION_FAILED',
      status: 400,
    });
  });

  it('formats friendly server offline message on network failure', async () => {
    global.fetch = vi.fn().mockRejectedValue(new TypeError('Failed to fetch'));

    await expect(apiClient.get('/health')).rejects.toMatchObject({
      message: expect.stringContaining('SmartHire server is unavailable'),
    });
  });

  it('invokes onUnauthorizedCallback and clears token on HTTP 401', async () => {
    setToken('expired_token');
    let callbackInvoked = false;
    setOnUnauthorizedCallback((msg) => {
      callbackInvoked = true;
    });

    const errEnvelope = {
      success: false,
      message: 'Session has expired or token is invalid.',
      error: { code: 'UNAUTHORIZED_EXPIRED' },
    };

    global.fetch = vi.fn().mockResolvedValue({
      ok: false,
      status: 401,
      headers: new Headers({ 'content-type': 'application/json' }),
      text: async () => JSON.stringify(errEnvelope),
    });

    try {
      await apiClient.get('/dashboard/candidate');
    } catch {
      // Expected rejection
    }

    expect(callbackInvoked).toBe(true);
    setOnUnauthorizedCallback(null);
  });
});

describe('API Services Endpoint Contracts', () => {
  beforeEach(() => {
    clearAuthSession();
    vi.restoreAllMocks();
  });

  it('calls authApi methods correctly', async () => {
    const spy = vi.spyOn(apiClient, 'post').mockResolvedValue({ success: true });
    await authApi.login({ username: 'u', password: 'p' });
    expect(spy).toHaveBeenCalledWith('/auth/login', { username: 'u', password: 'p' });

    await authApi.registerCandidate({ name: 'Candidate' });
    expect(spy).toHaveBeenCalledWith('/auth/register/candidate', { name: 'Candidate' });

    await authApi.registerRecruiter({ name: 'Recruiter' });
    expect(spy).toHaveBeenCalledWith('/auth/register/recruiter', { name: 'Recruiter' });

    await authApi.changePassword({ currentPassword: 'c', newPassword: 'n' });
    expect(spy).toHaveBeenCalledWith('/auth/change-password', { currentPassword: 'c', newPassword: 'n' });
  });

  it('calls jobsApi with query params respecting backend limit <= 100', async () => {
    const spy = vi.spyOn(apiClient, 'get').mockResolvedValue({ success: true, data: { items: [] } });

    await jobsApi.getJobs({ page: 2, limit: 25, title: 'developer' });
    expect(spy).toHaveBeenCalledWith('/jobs?page=2&limit=25&title=developer');

    await jobsApi.getJobById('J001');
    expect(spy).toHaveBeenCalledWith('/jobs/J001');
  });

  it('calls matchingApi explanation endpoint correctly', async () => {
    const spy = vi.spyOn(apiClient, 'get').mockResolvedValue({ success: true, data: {} });

    await matchingApi.getMatchExplanation('J001');
    expect(spy).toHaveBeenCalledWith('/matches/J001/explanation');
  });

  it('calls savedJobsApi endpoints correctly', async () => {
    const getSpy = vi.spyOn(apiClient, 'get').mockResolvedValue({ success: true, data: [] });
    const postSpy = vi.spyOn(apiClient, 'post').mockResolvedValue({ success: true });
    const delSpy = vi.spyOn(apiClient, 'del').mockResolvedValue({ success: true });

    await savedJobsApi.getSavedJobs();
    expect(getSpy).toHaveBeenCalledWith('/saved-jobs');

    await savedJobsApi.saveJob('J002');
    expect(postSpy).toHaveBeenCalledWith('/saved-jobs/J002');

    await savedJobsApi.removeSavedJob('J002');
    expect(delSpy).toHaveBeenCalledWith('/saved-jobs/J002');
  });

  it('calls applicationApi apply and withdraw endpoints correctly', async () => {
    const postSpy = vi.spyOn(apiClient, 'post').mockResolvedValue({ success: true });
    const getSpy = vi.spyOn(apiClient, 'get').mockResolvedValue({ success: true, data: [] });

    await applicationApi.applyForJob('J001');
    expect(postSpy).toHaveBeenCalledWith('/applications', { jobId: 'J001' });

    await applicationApi.withdrawApplication(42);
    expect(postSpy).toHaveBeenCalledWith('/applications/42/withdraw');

    await applicationApi.getApplications();
    expect(getSpy).toHaveBeenCalledWith('/applications');
  });

  it('calls recruiterApi job CRUD and applicant status endpoints correctly', async () => {
    const postSpy = vi.spyOn(apiClient, 'post').mockResolvedValue({ success: true });
    const putSpy = vi.spyOn(apiClient, 'put').mockResolvedValue({ success: true });
    const delSpy = vi.spyOn(apiClient, 'del').mockResolvedValue({ success: true });
    const getSpy = vi.spyOn(apiClient, 'get').mockResolvedValue({ success: true, data: [] });

    await recruiterApi.createJob({ jobId: 'J_NEW', title: 'Architect' });
    expect(postSpy).toHaveBeenCalledWith('/recruiter/jobs', { jobId: 'J_NEW', title: 'Architect' });

    await recruiterApi.updateJob('J_NEW', { title: 'Lead Architect' });
    expect(putSpy).toHaveBeenCalledWith('/recruiter/jobs/J_NEW', { title: 'Lead Architect' });

    await recruiterApi.deleteJob('J_NEW');
    expect(delSpy).toHaveBeenCalledWith('/recruiter/jobs/J_NEW');

    await recruiterApi.getRecruiterApplications();
    expect(getSpy).toHaveBeenCalledWith('/recruiter/applications');

    await recruiterApi.updateApplicationStatus(15, 'SHORTLISTED');
    expect(putSpy).toHaveBeenCalledWith('/recruiter/applications/15/status', { status: 'SHORTLISTED' });
  });

  it('calls dashboardApi candidate and recruiter endpoints correctly', async () => {
    const getSpy = vi.spyOn(apiClient, 'get').mockResolvedValue({ success: true });

    await dashboardApi.getCandidateDashboard();
    expect(getSpy).toHaveBeenCalledWith('/dashboard/candidate');

    await dashboardApi.getRecruiterDashboard();
    expect(getSpy).toHaveBeenCalledWith('/dashboard/recruiter');

    await dashboardApi.getActivity(10);
    expect(getSpy).toHaveBeenCalledWith('/activity?limit=10');

    await dashboardApi.getHealth();
    expect(getSpy).toHaveBeenCalledWith('/health');
  });

  it('calls resumeApi endpoints correctly', async () => {
    const getSpy = vi.spyOn(apiClient, 'get').mockResolvedValue({ success: true });
    const uploadSpy = vi.spyOn(apiClient, 'upload').mockResolvedValue({ success: true });

    const dummyFile = new Blob(['Resume Content'], { type: 'text/plain' });
    await resumeApi.uploadResume(dummyFile);
    expect(uploadSpy).toHaveBeenCalledWith('/resume/upload', expect.any(FormData));

    await resumeApi.getAnalysis();
    expect(getSpy).toHaveBeenCalledWith('/resume/analysis');

    await resumeApi.getRecommendations();
    expect(getSpy).toHaveBeenCalledWith('/resume/recommendations');

    await resumeApi.getStrengths();
    expect(getSpy).toHaveBeenCalledWith('/resume/strengths');
  });

  it('calls candidateApi profile endpoints correctly', async () => {
    const getSpy = vi.spyOn(apiClient, 'get').mockResolvedValue({ success: true });
    const putSpy = vi.spyOn(apiClient, 'put').mockResolvedValue({ success: true });

    await candidateApi.getProfile();
    expect(getSpy).toHaveBeenCalledWith('/candidates/me');

    await candidateApi.updateProfile({ name: 'Jane' });
    expect(putSpy).toHaveBeenCalledWith('/candidates/me', { name: 'Jane' });
  });
});
