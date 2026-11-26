import apiClient from './apiClient';
import { MOCK_JOBS } from '../data/mockData';

export const jobsApi = {
  getJobs: async (params = {}) => {
    const query = new URLSearchParams();
    if (params.page) query.append('page', params.page);
    if (params.limit) query.append('limit', params.limit);
    if (params.title) query.append('title', params.title);
    if (params.company) query.append('company', params.company);
    if (params.location) query.append('location', params.location);
    if (params.skill) query.append('skill', params.skill);
    if (params.employmentType) query.append('employmentType', params.employmentType);

    const qs = query.toString();
    try {
      return await apiClient.get(`/jobs${qs ? `?${qs}` : ''}`);
    } catch (err) {
      if (err.code === 'SERVER_UNAVAILABLE' || err.message?.includes('SmartHire server is unavailable')) {
        let items = [...MOCK_JOBS];
        if (params.title) items = items.filter(j => j.title.toLowerCase().includes(params.title.toLowerCase()));
        if (params.company) items = items.filter(j => j.company.toLowerCase().includes(params.company.toLowerCase()));
        if (params.location) items = items.filter(j => j.location.toLowerCase().includes(params.location.toLowerCase()));
        if (params.skill) items = items.filter(j => 
          j.requiredSkills.some(s => s.toLowerCase().includes(params.skill.toLowerCase())) ||
          j.preferredSkills.some(s => s.toLowerCase().includes(params.skill.toLowerCase()))
        );
        if (params.employmentType && params.employmentType !== 'All Types') {
          items = items.filter(j => j.employmentType === params.employmentType);
        }

        const page = parseInt(params.page || '1', 10);
        const limit = parseInt(params.limit || '6', 10);
        const startIndex = (page - 1) * limit;
        const paginatedItems = items.slice(startIndex, startIndex + limit);

        return {
          success: true,
          isOfflineDemo: true,
          data: {
            items: paginatedItems,
            total: items.length,
            page,
            limit,
            totalPages: Math.max(1, Math.ceil(items.length / limit)),
          },
        };
      }
      throw err;
    }
  },
  getJobById: async (id) => {
    try {
      return await apiClient.get(`/jobs/${encodeURIComponent(id)}`);
    } catch (err) {
      if (err.code === 'SERVER_UNAVAILABLE' || err.message?.includes('SmartHire server is unavailable')) {
        const job = MOCK_JOBS.find(j => j.id.toLowerCase() === id.toLowerCase()) || MOCK_JOBS[0];
        return { success: true, isOfflineDemo: true, data: job };
      }
      throw err;
    }
  },
};

export default jobsApi;
