import React, { useState, useEffect } from 'react';
import { Link } from 'react-router-dom';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import ApplicationStatus from '../components/ApplicationStatus';
import LiveSyncBadge from '../components/LiveSyncBadge';
import { useLiveSync } from '../utils/useLiveSync';
import { dashboardApi } from '../api/dashboardApi';
import { applicationApi } from '../api/applicationApi';
import { matchingApi } from '../api/matchingApi';
import { analyticsApi } from '../api/analyticsApi';
import { formatScore, formatDate } from '../utils/formatters';
import {
  FileTextIcon,
  SendIcon,
  StarIcon,
  TargetIcon,
  ZapIcon,
  SearchIcon,
  MapPinIcon,
  BriefcaseIcon,
  ArrowRightIcon
} from '../components/Icons';

export const CandidateDashboard = () => {
  const [data, setData] = useState(null);
  const [applications, setApplications] = useState([]);
  const [topMatches, setTopMatches] = useState([]);
  const [recommendedJobs, setRecommendedJobs] = useState([]);
  const [careerReadiness, setCareerReadiness] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const fetchDashboardData = async (isBackground = false) => {
    if (!isBackground) setLoading(true);
    setError(null);
    try {
      const [dashRes, appRes, matchRes, recRes, analRes] = await Promise.allSettled([
        dashboardApi.getCandidateDashboard(),
        applicationApi.getApplications(),
        matchingApi.getMatches({ limit: 4 }),
        analyticsApi.getCandidateRecommendations({ limit: 4 }),
        analyticsApi.getCandidateAnalytics(),
      ]);

      if (dashRes.status === 'fulfilled' && dashRes.value?.data) {
        setData(dashRes.value.data);
      }
      if (appRes.status === 'fulfilled' && Array.isArray(appRes.value?.data)) {
        setApplications(appRes.value.data);
      }
      if (matchRes.status === 'fulfilled' && Array.isArray(matchRes.value?.data)) {
        setTopMatches(matchRes.value.data);
      }
      if (recRes.status === 'fulfilled' && Array.isArray(recRes.value?.data)) {
        setRecommendedJobs(recRes.value.data);
      }
      if (analRes.status === 'fulfilled' && analRes.value?.data?.careerReadiness) {
        setCareerReadiness({
          ...analRes.value.data.careerReadiness,
          ...(analRes.value.data.resumeHealth || {}),
        });
      }
    } catch (err) {
      if (!isBackground) setError(err);
    } finally {
      if (!isBackground) setLoading(false);
    }
  };

  const { lastSynced, isSyncing, syncNow, isLive, setIsLive } = useLiveSync(
    () => fetchDashboardData(true),
    5000
  );

  useEffect(() => {
    fetchDashboardData(false);
  }, []);

  if (loading) {
    return (
      <div className="portal-layout">
        <Sidebar type="candidate" />
        <main className="portal-main">
          <Loading message="Loading candidate workspace..." />
        </main>
      </div>
    );
  }

  const qualityScore = data?.resumeQuality ?? 0;
  const totalApps = applications.length;
  const savedCount = data?.savedJobsCount ?? 0;
  const bestMatchScore = topMatches[0]?.overallScore ?? (data?.bestMatchScore ?? 0);

  return (
    <div className="portal-layout">
      <Sidebar type="candidate" />

      <main className="portal-main">
        <div className="portal-content">
          <div className="page-header">
            <div>
              <h1 className="page-title">Candidate Workspace</h1>
              <p className="page-subtitle">
                Overview of resume strength, active applications, and recommended positions
              </p>
            </div>
            <div style={{ display: 'flex', gap: '0.75rem', alignItems: 'center', flexWrap: 'wrap' }}>
              <LiveSyncBadge
                lastSynced={lastSynced}
                isSyncing={isSyncing}
                syncNow={() => fetchDashboardData(false)}
                isLive={isLive}
                setIsLive={setIsLive}
                label="Live Workspace"
              />
              <Link to="/candidate/realtime-analyzer" className="btn btn-secondary" style={{ display: 'inline-flex', alignItems: 'center', gap: '0.4rem', border: '1px solid #10b981', color: '#10b981' }}>
                <ZapIcon size={16} />
                <span>⚡ Real-Time Scanner</span>
              </Link>
              <Link to="/candidate/resume" className="btn btn-outline">
                <FileTextIcon size={16} />
                <span>Update Resume</span>
              </Link>
              <Link to="/candidate/jobs" className="btn btn-primary">
                <SearchIcon size={16} />
                <span>Find Jobs</span>
              </Link>
            </div>
          </div>

          <ErrorMessage error={error} onRetry={fetchDashboardData} />

          {/* Metric Cards Grid */}
          <div className="stats-grid">
            <div className="stat-card">
              <div className="stat-icon" style={{ background: 'rgba(99, 102, 241, 0.15)', color: '#a5b4fc' }}>
                <FileTextIcon size={22} />
              </div>
              <div className="stat-info">
                <span className="stat-value" style={{ color: qualityScore >= 80 ? '#6ee7b7' : '#a5b4fc' }}>
                  {formatScore(qualityScore)}
                </span>
                <span className="stat-label">Resume Quality</span>
              </div>
            </div>

            <div className="stat-card">
              <div className="stat-icon" style={{ background: 'rgba(59, 130, 246, 0.15)', color: '#93c5fd' }}>
                <SendIcon size={22} />
              </div>
              <div className="stat-info">
                <span className="stat-value">{totalApps}</span>
                <span className="stat-label">Active Applications</span>
              </div>
            </div>

            <div className="stat-card">
              <div className="stat-icon" style={{ background: 'rgba(245, 158, 11, 0.15)', color: '#fcd34d' }}>
                <StarIcon size={22} />
              </div>
              <div className="stat-info">
                <span className="stat-value">{savedCount}</span>
                <span className="stat-label">Saved Bookmarks</span>
              </div>
            </div>

            <div className="stat-card">
              <div className="stat-icon" style={{ background: 'rgba(16, 185, 129, 0.15)', color: '#6ee7b7' }}>
                <TargetIcon size={22} />
              </div>
              <div className="stat-info">
                <span className="stat-value" style={{ color: '#6ee7b7' }}>
                  {formatScore(bestMatchScore)}
                </span>
                <span className="stat-label">Top Job Match</span>
              </div>
            </div>
          </div>

          {/* Career Readiness Indicators Section */}
          {careerReadiness && (
            <div className="card" style={{ marginBottom: '2.25rem', background: 'linear-gradient(135deg, rgba(24, 34, 60, 0.75) 0%, rgba(13, 19, 34, 0.85) 100%)', border: '1px solid rgba(99, 102, 241, 0.28)' }}>
              <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1.25rem', flexWrap: 'wrap', gap: '0.75rem' }}>
                <div>
                  <h3 style={{ fontSize: '1.2rem', margin: '0 0 0.25rem 0', display: 'flex', alignItems: 'center', gap: '0.6rem' }}>
                    <ZapIcon size={20} color="#818cf8" />
                    <span>Career Readiness Indicators</span>
                  </h3>
                  <p style={{ color: 'var(--text-muted)', fontSize: '0.85rem', margin: 0 }}>
                    Deterministic evaluation based on profile completeness, skill coverage, and resume structure
                  </p>
                </div>
                <div style={{ display: 'flex', alignItems: 'center', gap: '1rem' }}>
                  <span className="badge" style={{ background: 'rgba(99, 102, 241, 0.18)', color: '#c7d2fe', border: '1px solid rgba(99, 102, 241, 0.35)', padding: '0.4rem 0.85rem' }}>
                    {careerReadiness.readinessTier || careerReadiness.readinessLevel || 'Evaluated'}
                  </span>
                  <div style={{ textAlign: 'right' }}>
                    <div style={{ fontSize: '1.6rem', fontWeight: 800, color: '#6ee7b7', lineHeight: 1.1 }}>
                      {careerReadiness.readinessIndex ?? careerReadiness.overallReadiness ?? 0}%
                    </div>
                    <div style={{ fontSize: '0.7rem', color: 'var(--text-muted)', textTransform: 'uppercase', letterSpacing: '0.04em' }}>Readiness Index</div>
                  </div>
                </div>
              </div>

              {/* Breakdown Bars */}
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(180px, 1fr))', gap: '1.15rem', marginBottom: '1.35rem' }}>
                <div>
                  <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: '0.8rem', marginBottom: '0.35rem' }}>
                    <span style={{ color: 'var(--text-muted)' }}>Skill Coverage</span>
                    <span style={{ fontWeight: 600 }}>{careerReadiness.skillCoverage ?? careerReadiness.skillReadiness ?? 0}%</span>
                  </div>
                  <div className="progress-bar-bg" style={{ height: '6px' }}>
                    <div style={{ width: `${careerReadiness.skillCoverage ?? careerReadiness.skillReadiness ?? 0}%`, height: '100%', background: 'linear-gradient(90deg, #6366f1, #8b5cf6)', borderRadius: 'var(--radius-full)' }}></div>
                  </div>
                </div>

                <div>
                  <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: '0.8rem', marginBottom: '0.35rem' }}>
                    <span style={{ color: 'var(--text-muted)' }}>Experience Coverage</span>
                    <span style={{ fontWeight: 600 }}>{careerReadiness.experienceCoverage ?? careerReadiness.experienceReadiness ?? 0}%</span>
                  </div>
                  <div className="progress-bar-bg" style={{ height: '6px' }}>
                    <div style={{ width: `${careerReadiness.experienceCoverage ?? careerReadiness.experienceReadiness ?? 0}%`, height: '100%', background: 'linear-gradient(90deg, #10b981, #06b6d4)', borderRadius: 'var(--radius-full)' }}></div>
                  </div>
                </div>

                <div>
                  <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: '0.8rem', marginBottom: '0.35rem' }}>
                    <span style={{ color: 'var(--text-muted)' }}>Education Alignment</span>
                    <span style={{ fontWeight: 600 }}>{careerReadiness.educationCoverage ?? careerReadiness.educationReadiness ?? 0}%</span>
                  </div>
                  <div className="progress-bar-bg" style={{ height: '6px' }}>
                    <div style={{ width: `${careerReadiness.educationCoverage ?? careerReadiness.educationReadiness ?? 0}%`, height: '100%', background: 'linear-gradient(90deg, #06b6d4, #3b82f6)', borderRadius: 'var(--radius-full)' }}></div>
                  </div>
                </div>

                <div>
                  <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: '0.8rem', marginBottom: '0.35rem' }}>
                    <span style={{ color: 'var(--text-muted)' }}>Profile Completeness</span>
                    <span style={{ fontWeight: 600 }}>{careerReadiness.profileCompleteness ?? 0}%</span>
                  </div>
                  <div className="progress-bar-bg" style={{ height: '6px' }}>
                    <div style={{ width: `${careerReadiness.profileCompleteness ?? 0}%`, height: '100%', background: 'linear-gradient(90deg, #f59e0b, #ec4899)', borderRadius: 'var(--radius-full)' }}></div>
                  </div>
                </div>
              </div>

              {/* Diagnostic Factors */}
              {((careerReadiness.readinessIndicators && careerReadiness.readinessIndicators.length > 0) ||
                (careerReadiness.strengths && careerReadiness.strengths.length > 0) ||
                (careerReadiness.improvementAreas && careerReadiness.improvementAreas.length > 0)) && (
                <div style={{ padding: '0.85rem 1.15rem', background: 'rgba(255,255,255,0.03)', borderRadius: 'var(--radius-md)', fontSize: '0.825rem', border: '1px solid var(--border-subtle)' }}>
                  <div style={{ fontWeight: 600, color: '#93c5fd', marginBottom: '0.4rem', textTransform: 'uppercase', fontSize: '0.75rem', letterSpacing: '0.04em' }}>Key Readiness Observations:</div>
                  <ul style={{ margin: 0, paddingLeft: '1.2rem', color: 'var(--text-muted)' }}>
                    {(careerReadiness.readinessIndicators || [
                      ...(careerReadiness.strengths || []).map(s => `Strength: ${s}`),
                      ...(careerReadiness.improvementAreas || []).map(i => `Opportunity: ${i}`)
                    ]).slice(0, 3).map((ind, i) => (
                      <li key={i} style={{ marginBottom: '0.25rem' }}>{ind}</li>
                    ))}
                  </ul>
                </div>
              )}
            </div>
          )}

          {/* Quick Shortcuts Banner if No Resume Uploaded */}
          {qualityScore === 0 && (
            <div style={{
              background: 'linear-gradient(135deg, rgba(99, 102, 241, 0.12) 0%, rgba(6, 182, 212, 0.08) 100%)',
              border: '1px solid rgba(99, 102, 241, 0.3)',
              borderRadius: 'var(--radius-lg)',
              padding: '1.5rem 2rem',
              marginBottom: '2rem',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'space-between',
              flexWrap: 'wrap',
              gap: '1rem'
            }}>
              <div>
                <h3 style={{ fontSize: '1.15rem', marginBottom: '0.25rem' }}>Upload your resume to unlock matching</h3>
                <p style={{ color: 'var(--text-muted)', fontSize: '0.9rem', margin: 0 }}>
                  Upload a PDF or TXT resume to automatically calculate your job readiness and match rankings.
                </p>
              </div>
              <Link to="/candidate/resume" className="btn btn-primary">
                <span>Upload Resume Now</span>
                <ArrowRightIcon size={16} />
              </Link>
            </div>
          )}

          {/* Intelligent Recommendations with Explainable Reasons */}
          <div style={{ marginBottom: '2.5rem' }}>
            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1.15rem' }}>
              <div>
                <h2 className="section-title" style={{ margin: 0 }}>Recommended Positions</h2>
                <p style={{ color: 'var(--text-muted)', fontSize: '0.85rem', margin: '0.2rem 0 0 0' }}>
                  Ranked by multi-factor deterministic compatibility with clear rationale
                </p>
              </div>
              <Link to="/candidate/jobs" style={{ fontSize: '0.875rem', color: 'var(--primary)', fontWeight: 600, display: 'inline-flex', alignItems: 'center', gap: '0.35rem' }}>
                <span>Explore All Jobs</span>
                <ArrowRightIcon size={14} />
              </Link>
            </div>

            {(recommendedJobs.length === 0 && topMatches.length === 0) ? (
              <div className="card" style={{ textAlign: 'center', padding: '2.75rem 1rem' }}>
                <p style={{ color: 'var(--text-muted)', marginBottom: '1.15rem' }}>
                  No match recommendations available yet. Upload your resume to calculate intelligent matches.
                </p>
                <Link to="/candidate/resume" className="btn btn-sm btn-outline">
                  Upload Resume
                </Link>
              </div>
            ) : (
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(320px, 1fr))', gap: '1.15rem' }}>
                {(recommendedJobs.length > 0 ? recommendedJobs : topMatches).map((m) => (
                  <div key={m.jobId} className="card card-interactive" style={{ display: 'flex', flexDirection: 'column', justifyContent: 'space-between' }}>
                    <div>
                      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start', marginBottom: '0.65rem' }}>
                        <span style={{ fontSize: '0.75rem', fontWeight: 700, color: 'var(--primary)', letterSpacing: '0.04em', textTransform: 'uppercase' }}>
                          {m.company}
                        </span>
                        <span className="badge badge-success">
                          {formatScore(m.compatibilityScore ?? m.overallScore)} Match
                        </span>
                      </div>
                      <h4 style={{ fontSize: '1.1rem', marginBottom: '0.35rem' }}>{m.title || m.jobTitle}</h4>
                      <div style={{ fontSize: '0.8rem', color: 'var(--text-muted)', marginBottom: '0.85rem', display: 'flex', alignItems: 'center', gap: '0.75rem', flexWrap: 'wrap' }}>
                        <span style={{ display: 'inline-flex', alignItems: 'center', gap: '0.3rem' }}>
                          <MapPinIcon size={13} color="var(--text-dim)" />
                          <span>{m.location || 'Remote'}</span>
                        </span>
                        <span>&bull;</span>
                        <span style={{ display: 'inline-flex', alignItems: 'center', gap: '0.3rem' }}>
                          <BriefcaseIcon size={13} color="var(--text-dim)" />
                          <span>{m.employmentType || 'Full-time'}</span>
                        </span>
                      </div>

                      {/* Explainable reasons */}
                      {m.explainableReasons && m.explainableReasons.length > 0 ? (
                        <div style={{ background: 'rgba(255,255,255,0.03)', padding: '0.65rem 0.85rem', borderRadius: 'var(--radius-sm)', marginBottom: '1.15rem', fontSize: '0.775rem', border: '1px solid var(--border-subtle)' }}>
                          <div style={{ fontWeight: 600, color: '#a5b4fc', marginBottom: '0.3rem', textTransform: 'uppercase', fontSize: '0.7rem', letterSpacing: '0.04em' }}>Why recommended:</div>
                          <ul style={{ margin: 0, paddingLeft: '1rem', color: 'var(--text-muted)' }}>
                            {m.explainableReasons.slice(0, 3).map((r, ri) => (
                              <li key={ri} style={{ marginBottom: '0.15rem' }}>{r}</li>
                            ))}
                          </ul>
                        </div>
                      ) : (
                        <div style={{ fontSize: '0.85rem', color: 'var(--text-muted)', marginBottom: '1.15rem' }}>
                          Readiness: <strong style={{ color: 'var(--text-main)' }}>{formatScore(m.readinessScore ?? m.overallScore)}</strong>
                        </div>
                      )}
                    </div>

                    <div style={{ display: 'flex', gap: '0.65rem', marginTop: '0.5rem' }}>
                      <Link to={`/candidate/jobs/${m.jobId}`} className="btn btn-sm btn-secondary" style={{ flex: 1, textAlign: 'center' }}>
                        View Job
                      </Link>
                      <Link to={`/candidate/compare?jobId=${m.jobId}`} className="btn btn-sm btn-outline" style={{ flex: 1, textAlign: 'center' }}>
                        Compare
                      </Link>
                    </div>
                  </div>
                ))}
              </div>
            )}
          </div>

          {/* Recent Applications Table */}
          <div>
            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1.15rem' }}>
              <div>
                <h2 className="section-title" style={{ margin: 0 }}>Recent Applications</h2>
                <p style={{ color: 'var(--text-muted)', fontSize: '0.85rem', margin: '0.2rem 0 0 0' }}>
                  Track your active submissions and recruitment milestones
                </p>
              </div>
              <Link to="/candidate/applications" style={{ fontSize: '0.875rem', color: 'var(--primary)', fontWeight: 600, display: 'inline-flex', alignItems: 'center', gap: '0.35rem' }}>
                <span>View All Applications</span>
                <ArrowRightIcon size={14} />
              </Link>
            </div>

            {applications.length === 0 ? (
              <div className="card" style={{ textAlign: 'center', padding: '2.75rem 1rem' }}>
                <p style={{ color: 'var(--text-muted)' }}>You haven't submitted any job applications yet.</p>
                <Link to="/candidate/jobs" className="btn btn-sm btn-primary" style={{ marginTop: '0.85rem' }}>
                  Browse Open Positions
                </Link>
              </div>
            ) : (
              <div className="table-container">
                <table className="data-table">
                  <thead>
                    <tr>
                      <th>Job Position</th>
                      <th>Company</th>
                      <th>Match Score</th>
                      <th>Status</th>
                      <th>Applied Date</th>
                      <th>Action</th>
                    </tr>
                  </thead>
                  <tbody>
                    {applications.slice(0, 5).map((app) => (
                      <tr key={app.id || app.applicationId}>
                        <td style={{ fontWeight: 600 }}>{app.jobTitle || app.jobId}</td>
                        <td>{app.company || '—'}</td>
                        <td style={{ fontWeight: 700, color: '#6ee7b7' }}>
                          {formatScore(app.matchScore)}
                        </td>
                        <td>
                          <ApplicationStatus status={app.status} />
                        </td>
                        <td style={{ color: 'var(--text-muted)', fontSize: '0.85rem' }}>
                          {formatDate(app.appliedAt)}
                        </td>
                        <td>
                          <Link to="/candidate/applications" className="btn btn-sm btn-outline">
                            View
                          </Link>
                        </td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            )}
          </div>
        </div>
      </main>
    </div>
  );
};

export default CandidateDashboard;
