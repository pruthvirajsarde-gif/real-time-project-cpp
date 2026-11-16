import apiClient from './apiClient';

export const authApi = {
  registerCandidate: (data) => apiClient.post('/auth/register/candidate', data),
  registerRecruiter: (data) => apiClient.post('/auth/register/recruiter', data),
  login: (credentials) => apiClient.post('/auth/login', credentials),
  logout: () => apiClient.post('/auth/logout'),
  getMe: () => apiClient.get('/auth/me'),
  changePassword: (data) => apiClient.post('/auth/change-password', data),
};

export default authApi;
