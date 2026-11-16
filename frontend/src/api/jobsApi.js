import apiClient from './apiClient';

export const jobsApi = {
  getJobs: (params = {}) => {
    const query = new URLSearchParams();
    if (params.page) query.append('page', params.page);
    if (params.limit) query.append('limit', params.limit);
    if (params.title) query.append('title', params.title);
    if (params.company) query.append('company', params.company);
    if (params.location) query.append('location', params.location);
    if (params.skill) query.append('skill', params.skill);
    if (params.employmentType) query.append('employmentType', params.employmentType);

    const qs = query.toString();
    return apiClient.get(`/jobs${qs ? `?${qs}` : ''}`);
  },
  getJobById: (id) => apiClient.get(`/jobs/${encodeURIComponent(id)}`),
};

export default jobsApi;
