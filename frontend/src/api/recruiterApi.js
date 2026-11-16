import apiClient from './apiClient';

export const recruiterApi = {
  createJob: (data) => apiClient.post('/recruiter/jobs', data),
  updateJob: (id, data) => apiClient.put(`/recruiter/jobs/${encodeURIComponent(id)}`, data),
  deleteJob: (id) => apiClient.del(`/recruiter/jobs/${encodeURIComponent(id)}`),
  getRecruiterApplications: () => apiClient.get('/recruiter/applications'),
  updateApplicationStatus: (id, status) =>
    apiClient.put(`/recruiter/applications/${encodeURIComponent(id)}/status`, { status }),
};

export default recruiterApi;
