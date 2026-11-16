import React, { useState, useEffect } from 'react';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import AnalyticsCard from '../components/AnalyticsCard';
import PipelineChart from '../components/PipelineChart';
import BarChart from '../components/BarChart';
import { analyticsApi } from '../api/analyticsApi';
import { formatScore } from '../utils/formatters';
import {
  SendIcon,
  TargetIcon,
  BookmarkIcon,
  TrendingUpIcon,
  BarChartIcon,
  ShieldIcon,
  LightbulbIcon,
  RotateCcwIcon
} from '../components/Icons';

export const CandidateAnalyticsPage = () => {
  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const fetchAnalytics = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await analyticsApi.getCandidateAnalytics();
      if (res?.data) {
        setData(res.data);
      }
    } catch (err) {
      setError(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchAnalytics();
  }, []);

  const apps = data?.applications || {};
  const health = data?.resumeHealth || {};
  const readiness = data?.careerReadiness || {};

  const healthBars = [
    { label: 'Resume Quality Score', value: health.qualityScore || 0, percentage: health.qualityScore || 0, color: 'var(--primary)' },
    { label: 'Market Skill Coverage', value: health.skillCoverage || 0, percentage: health.skillCoverage || 0, color: '#06b6d4' },
    { label: 'Experience Alignment', value: health.experienceCoverage || 0, percentage: health.experienceCoverage || 0, color: '#8b5cf6' },
    { label: 'Education Relevance', value: health.educationCoverage || 0, percentage: health.educationCoverage || 0, color: '#10b981' },
    { label: 'Profile Completeness', value: health.profileCompleteness || 0, percentage: health.profileCompleteness || 0, color: '#f59e0b' },
  ];

  return (
    <div className="portal-layout">
      <Sidebar type="candidate" />

      <main className="portal-main">
        <div className="portal-content">
          <div className="page-header">
            <div>
              <h1 className="page-title">Career Intelligence & Analytics</h1>
              <p className="page-subtitle">
                Comprehensive health diagnostics, application pipeline metrics, and readiness indicators
              </p>
            </div>
            <button onClick={fetchAnalytics} className="btn btn-outline btn-sm" style={{ display: 'inline-flex', alignItems: 'center', gap: '0.4rem' }}>
              <RotateCcwIcon size={14} />
              <span>Refresh Analytics</span>
            </button>
          </div>

          <ErrorMessage error={error} onRetry={fetchAnalytics} />

          {loading ? (
            <Loading message="Aggregating candidate performance metrics..." />
          ) : (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '1.5rem' }}>
              {/* Top Overview Cards */}
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(200px, 1fr))', gap: '1rem' }}>
                <AnalyticsCard
                  title="Applications Submitted"
                  value={apps.total ?? 0}
                  subtitle={`${apps.active ?? 0} active, ${apps.withdrawn ?? 0} withdrawn`}
                  icon={<SendIcon size={20} color="var(--primary)" />}
                />
                <AnalyticsCard
                  title="Average Match Score"
                  value={formatScore(apps.avgMatchScore ?? 0)}
                  subtitle={`Highest: ${formatScore(apps.highestMatchScore ?? 0)}`}
                  icon={<TargetIcon size={20} color="#10b981" />}
                  badgeType="success"
                />
                <AnalyticsCard
                  title="Saved Job Bookmarks"
                  value={apps.savedJobs ?? 0}
                  subtitle="Shortlisted positions"
                  icon={<BookmarkIcon size={20} color="#f59e0b" />}
                />
                <AnalyticsCard
                  title="Career Readiness Level"
                  value={formatScore(readiness.overallReadiness ?? 0)}
                  badge={readiness.readinessLevel || 'Evaluated'}
                  badgeType={readiness.overallReadiness >= 75 ? 'success' : readiness.overallReadiness >= 50 ? 'primary' : 'warning'}
                  icon={<TrendingUpIcon size={20} color="#8b5cf6" />}
                />
              </div>

              {/* Application Pipeline Funnel */}
              <div className="card" style={{ padding: '1.5rem' }}>
                <h3 style={{ fontSize: '1.15rem', fontWeight: 700, marginBottom: '0.5rem', display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                  <BarChartIcon size={18} color="var(--primary)" />
                  <span>Application Pipeline Distribution</span>
                </h3>
                <p style={{ color: 'var(--text-muted)', fontSize: '0.875rem', marginBottom: '1.25rem' }}>
                  Current progress of your active applications through recruiter review stages.
                </p>
                <PipelineChart stages={apps.byStatus || {}} total={apps.total || 0} />
              </div>

              {/* Grid: Resume Health & Career Readiness */}
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(340px, 1fr))', gap: '1.5rem' }}>
                {/* Resume Health Breakdown */}
                <div className="card" style={{ padding: '1.5rem' }}>
                  <h3 style={{ fontSize: '1.15rem', fontWeight: 700, marginBottom: '0.5rem', display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                    <ShieldIcon size={18} color="#06b6d4" />
                    <span>Resume Health Dimensions</span>
                  </h3>
                  <p style={{ color: 'var(--text-muted)', fontSize: '0.875rem', marginBottom: '1.25rem' }}>
                    Granular evaluations of your resume credentials and catalog market alignment.
                  </p>
                  <BarChart items={healthBars} maxVal={100} />
                </div>

                {/* Career Readiness Insights */}
                <div className="card" style={{ padding: '1.5rem' }}>
                  <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '0.5rem' }}>
                    <h3 style={{ fontSize: '1.15rem', fontWeight: 700, margin: 0, display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                      <LightbulbIcon size={18} color="#f59e0b" />
                      <span>Readiness Strengths & Tips</span>
                    </h3>
                    <span className="badge badge-primary">{readiness.readinessLevel}</span>
                  </div>

                  <p style={{ color: 'var(--text-muted)', fontSize: '0.875rem', marginBottom: '1.25rem' }}>
                    Data-backed diagnostic strengths and actionable improvement vectors.
                  </p>

                  <div style={{ display: 'flex', flexDirection: 'column', gap: '1rem' }}>
                    {/* Strengths */}
                    <div>
                      <span style={{ fontSize: '0.8rem', fontWeight: 700, color: '#10b981', textTransform: 'uppercase', display: 'block', marginBottom: '0.35rem' }}>
                        Core Profile Strengths
                      </span>
                      {readiness.strengths && readiness.strengths.length > 0 ? (
                        <ul style={{ margin: 0, paddingLeft: '1.25rem', fontSize: '0.875rem', color: 'var(--text)' }}>
                          {readiness.strengths.map((s, idx) => (
                            <li key={idx} style={{ marginBottom: '0.25rem' }}>{s}</li>
                          ))}
                        </ul>
                      ) : (
                        <p style={{ margin: 0, fontSize: '0.85rem', color: 'var(--text-muted)' }}>
                          Populate more resume projects and experience to unlock strengths.
                        </p>
                      )}
                    </div>

                    {/* Improvement Areas */}
                    <div style={{ paddingTop: '0.75rem', borderTop: '1px solid var(--border-subtle)' }}>
                      <span style={{ fontSize: '0.8rem', fontWeight: 700, color: '#f59e0b', textTransform: 'uppercase', display: 'block', marginBottom: '0.35rem' }}>
                        Optimization Recommendations
                      </span>
                      {readiness.improvementAreas && readiness.improvementAreas.length > 0 ? (
                        <ul style={{ margin: 0, paddingLeft: '1.25rem', fontSize: '0.875rem', color: 'var(--text)' }}>
                          {readiness.improvementAreas.map((tip, idx) => (
                            <li key={idx} style={{ marginBottom: '0.25rem' }}>{tip}</li>
                          ))}
                        </ul>
                      ) : (
                        <p style={{ margin: 0, fontSize: '0.85rem', color: 'var(--text-muted)' }}>
                          Profile looks solid! Continue applying for matching opportunities.
                        </p>
                      )}
                    </div>
                  </div>
                </div>
              </div>
            </div>
          )}
        </div>
      </main>
    </div>
  );
};

export default CandidateAnalyticsPage;
