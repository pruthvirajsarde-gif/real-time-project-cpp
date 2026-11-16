import apiClient from './apiClient';

export const applicationApi = {
  getApplications: () => apiClient.get('/applications'),
  applyForJob: (jobId) => apiClient.post('/applications', { jobId }),
  getApplicationById: (id) => apiClient.get(`/applications/${encodeURIComponent(id)}`),
  withdrawApplication: (id) =>
    apiClient.post(`/applications/${encodeURIComponent(id)}/withdraw`),
};

export default applicationApi;
