import apiClient from './apiClient';

export const dashboardApi = {
  getCandidateDashboard: () => apiClient.get('/dashboard/candidate'),
  getRecruiterDashboard: () => apiClient.get('/dashboard/recruiter'),
  getActivity: (limit = 20) => apiClient.get(`/activity?limit=${limit}`),
  getHealth: () => apiClient.get('/health'),
};

export default dashboardApi;
