import { apiClient } from './apiClient';

export const analyticsApi = {
  // Candidate Analytics
  getCandidateAnalytics: () => apiClient.get('/analytics/candidate'),
  getCandidateSkillsAnalytics: (params) => apiClient.get('/analytics/candidate/skills', { params }),
  getCandidateSkillGap: (jobId) =>
    apiClient.get('/analytics/candidate/skills', {
      params: jobId ? { jobId } : {},
    }),
  getCandidateMarketInsights: () => apiClient.get('/analytics/candidate/market'),
  getCandidateRecommendations: (params = {}) =>
    apiClient.get('/analytics/candidate/recommendations', { params }),
  getResumeJobComparison: (jobId) => apiClient.get(`/analytics/candidate/compare/${jobId}`),
  compareCandidateResumeJob: (jobId) => apiClient.get(`/analytics/candidate/compare/${jobId}`),

  // Recruiter Analytics
  getRecruiterAnalytics: () => apiClient.get('/analytics/recruiter'),
  getRecruiterJobAnalytics: (jobId) => apiClient.get(`/recruiter/jobs/${jobId}/analytics`),
  getRecruiterJobCandidateComparison: (jobId) =>
    apiClient.get(`/recruiter/jobs/${jobId}/candidates/compare`),
  compareJobCandidates: (jobId) =>
    apiClient.get(`/recruiter/jobs/${jobId}/candidates/compare`),
};

export default analyticsApi;
