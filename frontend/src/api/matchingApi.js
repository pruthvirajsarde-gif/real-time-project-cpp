import apiClient from './apiClient';
import { MOCK_MATCHES } from '../data/mockData';

export const matchingApi = {
  getMatches: async (params = {}) => {
    const query = new URLSearchParams();
    if (params.limit) query.append('limit', params.limit);
    if (params.minScore) query.append('minScore', params.minScore);
    const qs = query.toString();
    try {
      return await apiClient.get(`/matches${qs ? `?${qs}` : ''}`);
    } catch (err) {
      if (err.code === 'SERVER_UNAVAILABLE' || err.message?.includes('SmartHire server is unavailable')) {
        return { success: true, data: MOCK_MATCHES };
      }
      throw err;
    }
  },
  getMatchExplanation: async (jobId) => {
    try {
      return await apiClient.get(`/matches/${encodeURIComponent(jobId)}/explanation`);
    } catch (err) {
      if (err.code === 'SERVER_UNAVAILABLE' || err.message?.includes('SmartHire server is unavailable')) {
        const match = MOCK_MATCHES.find(m => m.jobId.toLowerCase() === jobId.toLowerCase()) || MOCK_MATCHES[0];
        return { success: true, data: match };
      }
      throw err;
    }
  },
};

export default matchingApi;
