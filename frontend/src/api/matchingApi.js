import apiClient from './apiClient';

export const matchingApi = {
  getMatches: (params = {}) => {
    const query = new URLSearchParams();
    if (params.limit) query.append('limit', params.limit);
    if (params.minScore) query.append('minScore', params.minScore);
    const qs = query.toString();
    return apiClient.get(`/matches${qs ? `?${qs}` : ''}`);
  },
  getMatchExplanation: (jobId) =>
    apiClient.get(`/matches/${encodeURIComponent(jobId)}/explanation`),
};

export default matchingApi;
