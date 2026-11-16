import React, { useState, useEffect } from 'react';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import AnalyticsCard from '../components/AnalyticsCard';
import PipelineChart from '../components/PipelineChart';
import MatchDistribution from '../components/MatchDistribution';
import { analyticsApi } from '../api/analyticsApi';
import { formatScore } from '../utils/formatters';
import {
  BriefcaseIcon,
  UsersIcon,
  TargetIcon,
  LayersIcon,
  BarChartIcon
} from '../components/Icons';

export const RecruiterAnalyticsPage = () => {
  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const fetchRecruiterAnalytics = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await analyticsApi.getRecruiterAnalytics();
      if (res.data) {
        setData(res.data);
      }
    } catch (err) {
      setError(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchRecruiterAnalytics();
  }, []);

  return (
    <div className="portal-layout">
      <Sidebar type="recruiter" />

      <main className="portal-main">
        <div className="portal-content">
          <div className="page-header">
            <div>
              <h1 className="page-title">Hiring Analytics</h1>
              <p className="page-subtitle">
                Comprehensive pipeline volume, stage progression, and applicant compatibility distribution
              </p>
            </div>
            <button onClick={fetchRecruiterAnalytics} className="btn btn-outline">
              Refresh Analytics
            </button>
          </div>

          <ErrorMessage error={error} onRetry={fetchRecruiterAnalytics} />

          {loading ? (
            <Loading message="Synthesizing candidate pipeline metrics..." />
          ) : (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '1.75rem' }}>
              {/* Stat Cards */}
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(220px, 1fr))', gap: '1.25rem' }}>
                <AnalyticsCard
                  title="Active Requisitions"
                  value={data?.activeJobs ?? 0}
                  subtitle="Open requisitions under your account"
                  icon={<BriefcaseIcon size={20} />}
                />
                <AnalyticsCard
                  title="Total Applications Received"
                  value={data?.totalApplications ?? 0}
                  subtitle="Incoming candidate submissions"
                  icon={<UsersIcon size={20} />}
                />
                <AnalyticsCard
                  title="Average Match Score"
                  value={formatScore(data?.averageMatchScore ?? 0)}
                  subtitle="Mean TF-IDF compatibility across applicants"
                  icon={<TargetIcon size={20} />}
                  badgeType="success"
                />
              </div>

              {/* Grid: Pipeline Funnel & Match Score Distribution */}
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(340px, 1fr))', gap: '1.5rem' }}>
                {/* Pipeline Funnel */}
                <div className="card" style={{ padding: '1.75rem' }}>
                  <h3 style={{ fontSize: '1.15rem', fontWeight: 700, marginBottom: '0.4rem', display: 'flex', alignItems: 'center', gap: '0.65rem' }}>
                    <LayersIcon size={20} color="#818cf8" />
                    <span>Application Pipeline Stages</span>
                  </h3>
                  <p style={{ color: 'var(--text-muted)', fontSize: '0.85rem', marginBottom: '1.35rem' }}>
                    Current status breakdown of candidates across review stages.
                  </p>
                  <PipelineChart stages={data?.pipelineFunnel || {}} total={data?.totalApplications || 0} />
                </div>

                {/* Match Distribution */}
                <div className="card" style={{ padding: '1.75rem' }}>
                  <h3 style={{ fontSize: '1.15rem', fontWeight: 700, marginBottom: '0.4rem', display: 'flex', alignItems: 'center', gap: '0.65rem' }}>
                    <BarChartIcon size={20} color="#818cf8" />
                    <span>Applicant Match Score Distribution</span>
                  </h3>
                  <p style={{ color: 'var(--text-muted)', fontSize: '0.85rem', marginBottom: '1.35rem' }}>
                    Spread of candidate compatibility scores across all your job postings.
                  </p>
                  <MatchDistribution distribution={data?.matchScoreDistribution || {}} />
                </div>
              </div>
            </div>
          )}
        </div>
      </main>
    </div>
  );
};

export default RecruiterAnalyticsPage;
