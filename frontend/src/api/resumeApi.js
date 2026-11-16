import apiClient from './apiClient';

export const resumeApi = {
  uploadResume: (file) => {
    const formData = new FormData();
    formData.append('resume', file);
    return apiClient.upload('/resume/upload', formData);
  },
  importResume: (filePath) => apiClient.post('/resume/import', { filePath }),
  getResume: () => apiClient.get('/resume'),
  getAnalysis: () => apiClient.get('/resume/analysis'),
  getStrengths: () => apiClient.get('/resume/strengths'),
  getRecommendations: () => apiClient.get('/resume/recommendations'),
};

export default resumeApi;
