import apiClient from './apiClient';

export const candidateApi = {
  getProfile: () => apiClient.get('/candidates/me'),
  updateProfile: (data) => apiClient.put('/candidates/me', data),
};

export default candidateApi;
