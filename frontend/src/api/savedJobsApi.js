import apiClient from './apiClient';

export const savedJobsApi = {
  getSavedJobs: () => apiClient.get('/saved-jobs'),
  saveJob: (jobId) => apiClient.post(`/saved-jobs/${encodeURIComponent(jobId)}`),
  removeSavedJob: (jobId) => apiClient.del(`/saved-jobs/${encodeURIComponent(jobId)}`),
};

export default savedJobsApi;
