import React, { useState, useEffect } from 'react';
import { useParams, Link } from 'react-router-dom';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import AnalyticsCard from '../components/AnalyticsCard';
import PipelineChart from '../components/PipelineChart';
import MatchDistribution from '../components/MatchDistribution';
import { analyticsApi } from '../api/analyticsApi';
import { formatScore } from '../utils/formatters';
import {
  MapPinIcon,
  ScaleIcon,
  UsersIcon,
  TargetIcon,
  StarIcon,
  ZapIcon,
  LayersIcon,
  BarChartIcon
} from '../components/Icons';

export const JobAnalyticsPage = () => {
  const { id } = useParams();
  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const fetchJobAnalytics = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await analyticsApi.getRecruiterJobAnalytics(id);
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
    fetchJobAnalytics();
  }, [id]);

  return (
    <div className="portal-layout">
      <Sidebar type="recruiter" />

      <main className="portal-main">
        <div className="portal-content">
          <div className="page-header">
            <div>
              <h1 className="page-title">{data?.title || 'Job Requisition Analytics'}</h1>
              <p className="page-subtitle" style={{ display: 'flex', alignItems: 'center', gap: '0.4rem', flexWrap: 'wrap' }}>
                {data ? (
                  <>
                    <span>{data.company}</span>
                    <span>&bull;</span>
                    <span style={{ display: 'inline-flex', alignItems: 'center', gap: '0.25rem' }}>
                      <MapPinIcon size={14} color="var(--text-dim)" />
                      <span>{data.location}</span>
                    </span>
                    <span>(ID: {data.jobId})</span>
                  </>
                ) : 'Requisition-level applicant intelligence'}
              </p>
            </div>
            <div style={{ display: 'flex', gap: '0.75rem' }}>
              <Link to={`/recruiter/jobs/${id}/compare`} className="btn btn-primary btn-sm">
                <ScaleIcon size={14} />
                <span>Compare Candidates</span>
              </Link>
              <Link to={`/recruiter/jobs/${id}/applicants`} className="btn btn-outline btn-sm">
                <UsersIcon size={14} />
                <span>View Applicants</span>
              </Link>
            </div>
          </div>

          <ErrorMessage error={error} onRetry={fetchJobAnalytics} />

          {loading ? (
            <Loading message="Aggregating requisition applicant analytics..." />
          ) : (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '1.75rem' }}>
              {/* Metric Cards */}
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(200px, 1fr))', gap: '1.25rem' }}>
                <AnalyticsCard
                  title="Total Applicants"
                  value={data?.totalApplicants ?? 0}
                  subtitle="Submissions received"
                  icon={<UsersIcon size={20} />}
                />
                <AnalyticsCard
                  title="Average Match Score"
                  value={formatScore(data?.averageMatchScore ?? 0)}
                  subtitle="Mean TF-IDF compatibility"
                  icon={<TargetIcon size={20} />}
                  badgeType="success"
                />
                <AnalyticsCard
                  title="Top Match Score"
                  value={formatScore(data?.highestMatchScore ?? 0)}
                  subtitle={`Lowest score: ${formatScore(data?.lowestMatchScore ?? 0)}`}
                  icon={<StarIcon size={20} />}
                />
                <AnalyticsCard
                  title="Average Skill Coverage"
                  value={formatScore(data?.averageSkillCoverage ?? 0)}
                  subtitle="Required + preferred skills"
                  icon={<ZapIcon size={20} />}
                />
              </div>

              {/* Grid: Stages & Match Distribution */}
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(340px, 1fr))', gap: '1.5rem' }}>
                <div className="card" style={{ padding: '1.75rem' }}>
                  <h3 style={{ fontSize: '1.15rem', fontWeight: 700, marginBottom: '0.4rem', display: 'flex', alignItems: 'center', gap: '0.65rem' }}>
                    <LayersIcon size={20} color="#818cf8" />
                    <span>Requisition Stage Distribution</span>
                  </h3>
                  <p style={{ color: 'var(--text-muted)', fontSize: '0.85rem', marginBottom: '1.35rem' }}>
                    Candidate advancement across the hiring pipeline for this position.
                  </p>
                  <PipelineChart stages={data?.stageDistribution || {}} total={data?.totalApplicants || 0} />
                </div>

                <div className="card" style={{ padding: '1.75rem' }}>
                  <h3 style={{ fontSize: '1.15rem', fontWeight: 700, marginBottom: '0.4rem', display: 'flex', alignItems: 'center', gap: '0.65rem' }}>
                    <BarChartIcon size={20} color="#818cf8" />
                    <span>Match Score Spread</span>
                  </h3>
                  <p style={{ color: 'var(--text-muted)', fontSize: '0.85rem', marginBottom: '1.35rem' }}>
                    Histogram of candidate suitability scores specific to this requisition.
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

export default JobAnalyticsPage;
