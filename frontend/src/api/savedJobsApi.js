import apiClient from './apiClient';

export const savedJobsApi = {
  getSavedJobs: async () => {
    try {
      return await apiClient.get('/saved-jobs');
    } catch (err) {
      if (err.code === 'SERVER_UNAVAILABLE' || err.message?.includes('SmartHire server is unavailable')) {
        return { success: true, data: [] };
      }
      throw err;
    }
  },
  saveJob: (jobId) => apiClient.post(`/saved-jobs/${encodeURIComponent(jobId)}`),
  removeSavedJob: (jobId) => apiClient.del(`/saved-jobs/${encodeURIComponent(jobId)}`),
};

export default savedJobsApi;
