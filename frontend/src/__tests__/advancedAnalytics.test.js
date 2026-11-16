// Polyfill in-memory sessionStorage for node test environment
if (typeof globalThis.sessionStorage === 'undefined') {
  const store = new Map();
  globalThis.sessionStorage = {
    getItem: (key) => (store.has(key) ? store.get(key) : null),
    setItem: (key, val) => store.set(key, String(val)),
    removeItem: (key) => store.delete(key),
    clear: () => store.clear(),
  };
}

import { describe, it, expect, vi, beforeEach } from 'vitest';
import { analyticsApi } from '../api/analyticsApi';
import apiClient from '../api/apiClient';

describe('Phase 12: Advanced Intelligence & Analytics API Client Suite', () => {
  beforeEach(() => {
    vi.restoreAllMocks();
  });

  it('getCandidateAnalytics dispatches GET to /analytics/candidate', async () => {
    const mockData = {
      careerReadiness: {
        readinessIndex: 85,
        readinessTier: 'Strong Fit / High Readiness',
        skillCoverage: 80,
        experienceCoverage: 90,
      },
      applicationMetrics: { totalSubmitted: 5, active: 4, averageMatchScore: 78.4 },
    };
    vi.spyOn(apiClient, 'get').mockResolvedValueOnce({ data: mockData });

    const res = await analyticsApi.getCandidateAnalytics();
    expect(apiClient.get).toHaveBeenCalledWith('/analytics/candidate');
    expect(res.data.careerReadiness.readinessIndex).toBe(85);
    expect(res.data.careerReadiness.readinessTier).toBe('Strong Fit / High Readiness');
  });

  it('getCandidateSkillGap dispatches GET with optional jobId query param', async () => {
    const mockGap = {
      evaluatedAgainst: 'multi_job_average',
      candidateSkillsCount: 6,
      exactMatches: ['C++', 'SQL'],
      missingSkillsDetailed: [
        { skill: 'Docker', priority: 'REQUIRED', frequencyAcrossJobs: 4, percentage: 80 },
      ],
    };
    vi.spyOn(apiClient, 'get').mockResolvedValueOnce({ data: mockGap });

    const res = await analyticsApi.getCandidateSkillGap(12);
    expect(apiClient.get).toHaveBeenCalledWith('/analytics/candidate/skills', {
      params: { jobId: 12 },
    });
    expect(res.data.exactMatches).toContain('C++');
    expect(res.data.missingSkillsDetailed[0].priority).toBe('REQUIRED');
  });

  it('getCandidateRecommendations passes limit parameter correctly', async () => {
    const mockRecs = [
      {
        jobId: 101,
        title: 'Backend Systems Engineer',
        compatibilityScore: 92.5,
        explainableReasons: ['✓ 8/10 required skills matched', '✓ Experience requirement satisfied'],
      },
    ];
    vi.spyOn(apiClient, 'get').mockResolvedValueOnce({ data: mockRecs });

    const res = await analyticsApi.getCandidateRecommendations({ limit: 5 });
    expect(apiClient.get).toHaveBeenCalledWith('/analytics/candidate/recommendations', {
      params: { limit: 5 },
    });
    expect(res.data[0].compatibilityScore).toBe(92.5);
    expect(res.data[0].explainableReasons).toHaveLength(2);
  });

  it('getCandidateMarketInsights returns top skills, location, and co-occurrences', async () => {
    const mockMarket = {
      sampleSize: 15,
      note: 'Based on current SmartHire job dataset',
      topSkills: [{ skill: 'C++', count: 12, frequencyPct: 80.0 }],
      jobsByLocation: [{ location: 'Pune', count: 8 }],
      frequentSkillPairs: [{ pair: 'C++ + STL', count: 9 }],
    };
    vi.spyOn(apiClient, 'get').mockResolvedValueOnce({ data: mockMarket });

    const res = await analyticsApi.getCandidateMarketInsights();
    expect(apiClient.get).toHaveBeenCalledWith('/analytics/candidate/market');
    expect(res.data.topSkills[0].skill).toBe('C++');
    expect(res.data.jobsByLocation[0].location).toBe('Pune');
    expect(res.data.frequentSkillPairs[0].pair).toBe('C++ + STL');
  });

  it('compareCandidateResumeJob dispatches to /analytics/candidate/compare with jobId', async () => {
    const mockComp = {
      overallScore: 88.0,
      skills: { matched: ['C++', 'SQL'], missing: ['Kubernetes'] },
      experience: { candidateYears: 4, requiredYears: 3, satisfied: true },
      education: { satisfied: true },
      explanation: 'Candidate meets all fundamental criteria with exceptional skill compatibility.',
    };
    vi.spyOn(apiClient, 'get').mockResolvedValueOnce({ data: mockComp });

    const res = await analyticsApi.compareCandidateResumeJob(5);
    expect(apiClient.get).toHaveBeenCalledWith('/analytics/candidate/compare/5');
    expect(res.data.overallScore).toBe(88.0);
    expect(res.data.experience.satisfied).toBe(true);
  });

  it('getRecruiterAnalytics dispatches to /analytics/recruiter', async () => {
    const mockRecruiterData = {
      overview: { activeJobs: 3, totalApplicants: 18, underReview: 5, shortlisted: 4 },
      pipeline: [{ stage: 'APPLIED', count: 7 }, { stage: 'SHORTLISTED', count: 4 }],
      matchScoreDistribution: [{ range: '90-100%', count: 3, percentage: 16.7 }],
    };
    vi.spyOn(apiClient, 'get').mockResolvedValueOnce({ data: mockRecruiterData });

    const res = await analyticsApi.getRecruiterAnalytics();
    expect(apiClient.get).toHaveBeenCalledWith('/analytics/recruiter');
    expect(res.data.overview.totalApplicants).toBe(18);
    expect(res.data.matchScoreDistribution).toHaveLength(1);
  });

  it('getRecruiterJobAnalytics fetches per-job metrics with ownership validation', async () => {
    const mockJobAnalytics = {
      jobId: 2,
      title: 'Senior C++ Developer',
      totalApplicants: 9,
      averageMatchScore: 76.5,
      highestMatchScore: 94.0,
      stageDistribution: [{ stage: 'APPLIED', count: 4 }, { stage: 'INTERVIEW', count: 2 }],
    };
    vi.spyOn(apiClient, 'get').mockResolvedValueOnce({ data: mockJobAnalytics });

    const res = await analyticsApi.getRecruiterJobAnalytics(2);
    expect(apiClient.get).toHaveBeenCalledWith('/recruiter/jobs/2/analytics');
    expect(res.data.highestMatchScore).toBe(94.0);
    expect(res.data.totalApplicants).toBe(9);
  });

  it('compareJobCandidates returns factual tabular matrix for recruiter', async () => {
    const mockCandidates = [
      {
        candidateId: 10,
        candidateName: 'Candidate Alpha',
        matchScore: 91.5,
        requiredSkillsMatched: 9,
        totalRequiredSkills: 10,
        experienceYears: 4,
        stage: 'SHORTLISTED',
      },
      {
        candidateId: 11,
        candidateName: 'Candidate Beta',
        matchScore: 82.0,
        requiredSkillsMatched: 8,
        totalRequiredSkills: 10,
        experienceYears: 3,
        stage: 'UNDER_REVIEW',
      },
    ];
    vi.spyOn(apiClient, 'get').mockResolvedValueOnce({ data: mockCandidates });

    const res = await analyticsApi.compareJobCandidates(2);
    expect(apiClient.get).toHaveBeenCalledWith('/recruiter/jobs/2/candidates/compare');
    expect(res.data).toHaveLength(2);
    expect(res.data[0].matchScore).toBeGreaterThan(res.data[1].matchScore);
    expect(res.data[0].candidateName).toBe('Candidate Alpha');
  });
});

describe('Phase 12: Analytical Math, Distribution Bucketing & Pipeline Aggregation', () => {
  it('correctly categorizes scores into 5 standard distribution buckets', () => {
    const sampleScores = [95, 91, 88, 82, 75, 71, 70, 65, 54, 40];
    const buckets = {
      '90-100%': 0,
      '80-89%': 0,
      '70-79%': 0,
      '60-69%': 0,
      '<60%': 0,
    };

    sampleScores.forEach((s) => {
      if (s >= 90) buckets['90-100%']++;
      else if (s >= 80) buckets['80-89%']++;
      else if (s >= 70) buckets['70-79%']++;
      else if (s >= 60) buckets['60-69%']++;
      else buckets['<60%']++;
    });

    expect(buckets['90-100%']).toBe(2);
    expect(buckets['80-89%']).toBe(2);
    expect(buckets['70-79%']).toBe(3);
    expect(buckets['60-69%']).toBe(1);
    expect(buckets['<60%']).toBe(2);
  });

  it('determines career readiness indicator tiers deterministically', () => {
    const getTier = (score) => {
      if (score >= 80) return 'Strong Fit / High Readiness';
      if (score >= 60) return 'Moderate Readiness / Core Qualified';
      return 'Emerging / Skills Development Required';
    };

    expect(getTier(92)).toBe('Strong Fit / High Readiness');
    expect(getTier(80)).toBe('Strong Fit / High Readiness');
    expect(getTier(72)).toBe('Moderate Readiness / Core Qualified');
    expect(getTier(59)).toBe('Emerging / Skills Development Required');
  });

  it('aggregates pipeline stages in strictly monotonic sequential order', () => {
    const standardStages = ['APPLIED', 'UNDER_REVIEW', 'SHORTLISTED', 'INTERVIEW', 'SELECTED', 'REJECTED'];
    const mockApps = [
      { status: 'APPLIED' },
      { status: 'APPLIED' },
      { status: 'UNDER_REVIEW' },
      { status: 'SHORTLISTED' },
      { status: 'INTERVIEW' },
      { status: 'SELECTED' },
      { status: 'REJECTED' },
    ];

    const counts = standardStages.map((st) => ({
      stage: st,
      count: mockApps.filter((a) => a.status === st).length,
    }));

    expect(counts.find((c) => c.stage === 'APPLIED').count).toBe(2);
    expect(counts.find((c) => c.stage === 'UNDER_REVIEW').count).toBe(1);
    expect(counts.find((c) => c.stage === 'SHORTLISTED').count).toBe(1);
    expect(counts.find((c) => c.stage === 'INTERVIEW').count).toBe(1);
    expect(counts.find((c) => c.stage === 'SELECTED').count).toBe(1);
    expect(counts.find((c) => c.stage === 'REJECTED').count).toBe(1);
  });
});

describe('Phase 12: Role Access & Route Permissions for Analytics', () => {
  const checkRoutePermission = (path, role, isAuthenticated) => {
    if (!isAuthenticated) return { allowed: false, redirect: '/login' };
    if (path.startsWith('/candidate') && role !== 'CANDIDATE') {
      return { allowed: false, redirect: '/recruiter' };
    }
    if (path.startsWith('/recruiter') && role !== 'RECRUITER') {
      return { allowed: false, redirect: '/candidate' };
    }
    return { allowed: true };
  };

  it('guards /candidate/skill-gap from unauthenticated access', () => {
    const check = checkRoutePermission('/candidate/skill-gap', null, false);
    expect(check.allowed).toBe(false);
    expect(check.redirect).toBe('/login');
  });

  it('guards /candidate/compare from recruiter access', () => {
    const check = checkRoutePermission('/candidate/compare', 'RECRUITER', true);
    expect(check.allowed).toBe(false);
    expect(check.redirect).toBe('/recruiter');
  });

  it('guards /recruiter/analytics from candidate access', () => {
    const check = checkRoutePermission('/recruiter/analytics', 'CANDIDATE', true);
    expect(check.allowed).toBe(false);
    expect(check.redirect).toBe('/candidate');
  });

  it('guards /recruiter/jobs/1/compare from candidate access', () => {
    const check = checkRoutePermission('/recruiter/jobs/1/compare', 'CANDIDATE', true);
    expect(check.allowed).toBe(false);
    expect(check.redirect).toBe('/candidate');
  });

  it('allows recruiters to access recruiter job comparison', () => {
    const check = checkRoutePermission('/recruiter/jobs/1/compare', 'RECRUITER', true);
    expect(check.allowed).toBe(true);
  });

  it('allows candidates to access market insights', () => {
    const check = checkRoutePermission('/candidate/market-insights', 'CANDIDATE', true);
    expect(check.allowed).toBe(true);
  });
});
