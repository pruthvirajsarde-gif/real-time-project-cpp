import React, { useState, useEffect } from 'react';
import { Link } from 'react-router-dom';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import ApplicationStatus from '../components/ApplicationStatus';
import LiveSyncBadge from '../components/LiveSyncBadge';
import { useLiveSync } from '../utils/useLiveSync';
import { dashboardApi } from '../api/dashboardApi';
import { recruiterApi } from '../api/recruiterApi';
import { formatScore, formatDate } from '../utils/formatters';
import { PlusIcon, UsersIcon, BriefcaseIcon, StarIcon, ClockIcon, ArrowRightIcon } from '../components/Icons';

export const RecruiterDashboard = () => {
  const [data, setData] = useState(null);
  const [applications, setApplications] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const fetchRecruiterData = async (isBackground = false) => {
    if (!isBackground) setLoading(true);
    setError(null);
    try {
      const [dashRes, appsRes] = await Promise.allSettled([
        dashboardApi.getRecruiterDashboard(),
        recruiterApi.getRecruiterApplications(),
      ]);

      if (dashRes.status === 'fulfilled' && dashRes.value?.data) {
        setData(dashRes.value.data);
      }
      if (appsRes.status === 'fulfilled' && Array.isArray(appsRes.value?.data)) {
        setApplications(appsRes.value.data);
      }
    } catch (err) {
      if (!isBackground) setError(err);
    } finally {
      if (!isBackground) setLoading(false);
    }
  };

  const { lastSynced, isSyncing, syncNow, isLive, setIsLive } = useLiveSync(
    () => fetchRecruiterData(true),
    5000
  );

  useEffect(() => {
    fetchRecruiterData(false);
  }, []);

  if (loading) {
    return (
      <div className="portal-layout">
        <Sidebar type="recruiter" />
        <main className="portal-main">
          <Loading message="Loading recruiter dashboard..." />
        </main>
      </div>
    );
  }

  const jobsPosted = data?.jobsPosted ?? 0;
  const appsReceived = data?.applicationsReceived ?? applications.length;
  const shortlisted = data?.shortlistedCount ?? applications.filter(a => a.status === 'SHORTLISTED').length;
  const inReview = data?.underReviewCount ?? applications.filter(a => a.status === 'APPLIED' || a.status === 'UNDER_REVIEW').length;

  return (
    <div className="portal-layout">
      <Sidebar type="recruiter" />

      <main className="portal-main">
        <div className="portal-content">
          <div className="page-header">
            <div>
              <h1 className="page-title">Recruiter Console</h1>
              <p className="page-subtitle">
                Manage your job postings, inspect incoming candidates, and track recruitment progress
              </p>
            </div>
            <div style={{ display: 'flex', gap: '0.75rem', alignItems: 'center', flexWrap: 'wrap' }}>
              <LiveSyncBadge
                lastSynced={lastSynced}
                isSyncing={isSyncing}
                syncNow={() => fetchRecruiterData(false)}
                isLive={isLive}
                setIsLive={setIsLive}
                label="Live Pipeline"
              />
              <Link to="/recruiter/jobs/new" className="btn btn-primary">
                <PlusIcon size={16} />
                <span>Post New Job</span>
              </Link>
              <Link to="/recruiter/applicants" className="btn btn-outline">
                <UsersIcon size={16} />
                <span>View Applicants</span>
              </Link>
            </div>
          </div>

          <ErrorMessage error={error} onRetry={fetchRecruiterData} />

          {/* Recruiter Metrics Grid */}
          <div className="stats-grid">
            <div className="stat-card">
              <div className="stat-icon" style={{ background: 'rgba(99, 102, 241, 0.15)', color: '#a5b4fc' }}>
                <BriefcaseIcon size={22} />
              </div>
              <div className="stat-info">
                <span className="stat-value">{jobsPosted}</span>
                <span className="stat-label">Active Jobs Posted</span>
              </div>
            </div>

            <div className="stat-card">
              <div className="stat-icon" style={{ background: 'rgba(59, 130, 246, 0.15)', color: '#93c5fd' }}>
                <UsersIcon size={22} />
              </div>
              <div className="stat-info">
                <span className="stat-value">{appsReceived}</span>
                <span className="stat-label">Total Applicants</span>
              </div>
            </div>

            <div className="stat-card">
              <div className="stat-icon" style={{ background: 'rgba(245, 158, 11, 0.15)', color: '#fcd34d' }}>
                <StarIcon size={22} />
              </div>
              <div className="stat-info">
                <span className="stat-value" style={{ color: '#fcd34d' }}>{shortlisted}</span>
                <span className="stat-label">Shortlisted Candidates</span>
              </div>
            </div>

            <div className="stat-card">
              <div className="stat-icon" style={{ background: 'rgba(16, 185, 129, 0.15)', color: '#6ee7b7' }}>
                <ClockIcon size={22} />
              </div>
              <div className="stat-info">
                <span className="stat-value">{inReview}</span>
                <span className="stat-label">Pending Review</span>
              </div>
            </div>
          </div>

          {/* Recent Candidate Submissions */}
          <div style={{ marginTop: '2.5rem' }}>
            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1.15rem' }}>
              <h2 className="section-title" style={{ margin: 0 }}>Recent Applicant Inflow</h2>
              <Link to="/recruiter/applicants" style={{ fontSize: '0.875rem', color: 'var(--primary)', fontWeight: 600, display: 'inline-flex', alignItems: 'center', gap: '0.35rem' }}>
                <span>Manage All Pipeline ({applications.length})</span>
                <ArrowRightIcon size={14} />
              </Link>
            </div>

            {applications.length === 0 ? (
              <div className="card" style={{ textAlign: 'center', padding: '3rem 1rem' }}>
                <p style={{ color: 'var(--text-muted)', marginBottom: '1rem' }}>
                  No candidate submissions received yet for your job requisitions.
                </p>
                <Link to="/recruiter/jobs/new" className="btn btn-sm btn-primary">
                  Post Your First Job Opening
                </Link>
              </div>
            ) : (
              <div className="table-container">
                <table className="data-table">
                  <thead>
                    <tr>
                      <th>Candidate Name</th>
                      <th>Applied Position</th>
                      <th>Match Score</th>
                      <th>Status</th>
                      <th>Submission Date</th>
                      <th>Action</th>
                    </tr>
                  </thead>
                  <tbody>
                    {applications.slice(0, 6).map((app) => (
                      <tr key={app.id || app.applicationId}>
                        <td style={{ fontWeight: 600 }}>{app.candidateName || `Candidate #${app.candidateId}`}</td>
                        <td>{app.jobTitle || app.jobId}</td>
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
                          <Link to="/recruiter/applicants" className="btn btn-sm btn-outline">
                            Review
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

export default RecruiterDashboard;
