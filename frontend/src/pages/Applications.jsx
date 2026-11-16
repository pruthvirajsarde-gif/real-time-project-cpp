import React, { useState, useEffect } from 'react';
import { Link } from 'react-router-dom';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import ApplicationStatus from '../components/ApplicationStatus';
import LiveSyncBadge from '../components/LiveSyncBadge';
import { useLiveSync } from '../utils/useLiveSync';
import { applicationApi } from '../api/applicationApi';
import { formatScore, formatDate } from '../utils/formatters';
import { SearchIcon, SendIcon, XIcon, CheckIcon } from '../components/Icons';

export const Applications = () => {
  const [applications, setApplications] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  // Detail Modal
  const [selectedApp, setSelectedApp] = useState(null);

  // Withdraw State
  const [withdrawingId, setWithdrawingId] = useState(null);
  const [withdrawSubmitting, setWithdrawSubmitting] = useState(false);
  const [feedbackNotice, setFeedbackNotice] = useState(null);

  const fetchApplications = async (isBackground = false) => {
    if (!isBackground) setLoading(true);
    setError(null);
    try {
      const res = await applicationApi.getApplications();
      if (res && Array.isArray(res.data)) {
        setApplications(res.data);
      } else {
        setApplications([]);
      }
    } catch (err) {
      if (!isBackground) setError(err);
    } finally {
      if (!isBackground) setLoading(false);
    }
  };

  const { lastSynced, isSyncing, syncNow, isLive, setIsLive } = useLiveSync(
    () => fetchApplications(true),
    4000
  );

  useEffect(() => {
    fetchApplications(false);
  }, []);

  const handleWithdrawConfirm = async () => {
    if (!withdrawingId) return;
    setWithdrawSubmitting(true);
    try {
      await applicationApi.withdrawApplication(withdrawingId);
      setFeedbackNotice({
        type: 'info',
        msg: `Application #${withdrawingId} withdrawn successfully.`
      });
      setWithdrawingId(null);
      if (selectedApp && (selectedApp.id === withdrawingId || selectedApp.applicationId === withdrawingId)) {
        setSelectedApp(null);
      }
      await fetchApplications();
    } catch (err) {
      setFeedbackNotice({ type: 'danger', msg: err.message });
      setWithdrawingId(null);
    } finally {
      setWithdrawSubmitting(false);
    }
  };

  const getTimelineStages = (currentStatus) => {
    const normalStages = ['APPLIED', 'UNDER_REVIEW', 'SHORTLISTED', 'INTERVIEW', 'SELECTED'];
    const st = currentStatus?.toUpperCase() || 'APPLIED';

    if (st === 'REJECTED') {
      return [
        { key: 'APPLIED', label: 'Applied', done: true },
        { key: 'UNDER_REVIEW', label: 'Reviewed', done: true },
        { key: 'REJECTED', label: 'Not Selected', done: true, isError: true },
      ];
    }

    if (st === 'WITHDRAWN') {
      return [
        { key: 'APPLIED', label: 'Applied', done: true },
        { key: 'WITHDRAWN', label: 'Withdrawn by Candidate', done: true, isNeutral: true },
      ];
    }

    const currentIndex = normalStages.indexOf(st);
    return normalStages.map((stage, idx) => ({
      key: stage,
      label: stage.replace('_', ' '),
      done: idx <= currentIndex,
      active: idx === currentIndex,
    }));
  };

  return (
    <div className="portal-layout">
      <Sidebar type="candidate" />

      <main className="portal-main">
        <div className="portal-content">
          <div className="page-header">
            <div>
              <h1 className="page-title">Submitted Applications</h1>
              <p className="page-subtitle">
                Track your recruitment progress, review evaluation status, or withdraw candidacies
              </p>
            </div>
            <div style={{ display: 'flex', alignItems: 'center', gap: '0.75rem', flexWrap: 'wrap' }}>
              <LiveSyncBadge
                lastSynced={lastSynced}
                isSyncing={isSyncing}
                syncNow={() => fetchApplications(false)}
                isLive={isLive}
                setIsLive={setIsLive}
                label="Live Status"
              />
              <Link to="/candidate/jobs" className="btn btn-primary" style={{ display: 'inline-flex', alignItems: 'center', gap: '0.4rem' }}>
                <SearchIcon size={14} />
                <span>Apply to More Jobs</span>
              </Link>
            </div>
          </div>

          {feedbackNotice && (
            <div style={{
              background: feedbackNotice.type === 'info' ? 'rgba(59, 130, 246, 0.12)' : 'rgba(239, 68, 68, 0.12)',
              border: `1px solid ${feedbackNotice.type === 'info' ? 'rgba(59, 130, 246, 0.3)' : 'rgba(239, 68, 68, 0.3)'}`,
              borderRadius: 'var(--radius-md)',
              padding: '0.85rem 1.25rem',
              color: feedbackNotice.type === 'info' ? '#93c5fd' : '#fca5a5',
              marginBottom: '1.5rem',
              display: 'flex',
              justifyContent: 'space-between',
              alignItems: 'center'
            }}>
              <span>{feedbackNotice.msg}</span>
              <button
                onClick={() => setFeedbackNotice(null)}
                style={{ background: 'transparent', border: 'none', color: 'inherit', cursor: 'pointer', display: 'flex', alignItems: 'center' }}
              >
                <XIcon size={14} />
              </button>
            </div>
          )}

          <ErrorMessage error={error} onRetry={fetchApplications} />

          {loading ? (
            <Loading message="Loading application statuses..." />
          ) : applications.length === 0 ? (
            <div className="card" style={{ textAlign: 'center', padding: '3.5rem 1rem' }}>
              <div style={{ display: 'flex', justifyContent: 'center', marginBottom: '1rem', color: 'var(--primary)' }}>
                <SendIcon size={44} />
              </div>
              <h3 style={{ fontSize: '1.25rem', marginBottom: '0.5rem' }}>No applications submitted yet</h3>
              <p style={{ color: 'var(--text-muted)', marginBottom: '1.5rem' }}>
                Explore openings and submit your resume to start tracking recruitment progress.
              </p>
              <Link to="/candidate/jobs" className="btn btn-primary">
                Browse Job Requisitions
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
                    <th>Actions</th>
                  </tr>
                </thead>
                <tbody>
                  {applications.map((app) => {
                    const appId = app.id || app.applicationId;
                    const canWithdraw = app.status !== 'WITHDRAWN' && app.status !== 'REJECTED' && app.status !== 'SELECTED';

                    return (
                      <tr key={appId}>
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
                          <div style={{ display: 'flex', gap: '0.5rem' }}>
                            <button
                              onClick={() => setSelectedApp(app)}
                              className="btn btn-sm btn-secondary"
                            >
                              Timeline
                            </button>
                            {canWithdraw && (
                              <button
                                onClick={() => setWithdrawingId(appId)}
                                className="btn btn-sm btn-danger"
                              >
                                Withdraw
                              </button>
                            )}
                          </div>
                        </td>
                      </tr>
                    );
                  })}
                </tbody>
              </table>
            </div>
          )}
        </div>
      </main>

      {/* Application Detail & Stage Timeline Modal */}
      {selectedApp && (
        <div className="modal-overlay">
          <div className="modal-content" style={{ maxWidth: '580px' }}>
            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start', marginBottom: '1.25rem' }}>
              <div>
                <span style={{ fontSize: '0.8rem', fontWeight: 700, color: 'var(--primary)', textTransform: 'uppercase' }}>
                  Application #{selectedApp.id || selectedApp.applicationId}
                </span>
                <h3 style={{ fontSize: '1.4rem', marginTop: '0.2rem' }}>{selectedApp.jobTitle || selectedApp.jobId}</h3>
                <div style={{ color: 'var(--text-muted)', fontSize: '0.9rem' }}>{selectedApp.company}</div>
              </div>
              <ApplicationStatus status={selectedApp.status} />
            </div>

            <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '1rem', background: 'rgba(15, 23, 42, 0.6)', padding: '1rem', borderRadius: 'var(--radius-md)', marginBottom: '1.75rem', border: '1px solid var(--border-subtle)' }}>
              <div>
                <div style={{ fontSize: '0.75rem', color: 'var(--text-dim)', textTransform: 'uppercase' }}>Computed Match Score</div>
                <div style={{ fontSize: '1.25rem', fontWeight: 800, color: '#6ee7b7' }}>{formatScore(selectedApp.matchScore)}</div>
              </div>
              <div>
                <div style={{ fontSize: '0.75rem', color: 'var(--text-dim)', textTransform: 'uppercase' }}>Applied Timestamp</div>
                <div style={{ fontSize: '0.95rem', fontWeight: 600, marginTop: '0.2rem' }}>{formatDate(selectedApp.appliedAt)}</div>
              </div>
            </div>

            {/* Stages Progression */}
            <div style={{ marginBottom: '1.75rem' }}>
              <div style={{ fontSize: '0.85rem', fontWeight: 700, color: 'var(--text-muted)', marginBottom: '1rem', textTransform: 'uppercase' }}>
                Application Lifecycle Pipeline
              </div>
              <div style={{ display: 'flex', flexDirection: 'column', gap: '0.75rem' }}>
                {getTimelineStages(selectedApp.status).map((stage, idx) => (
                  <div
                    key={idx}
                    style={{
                      display: 'flex',
                      alignItems: 'center',
                      gap: '0.85rem',
                      padding: '0.65rem 0.85rem',
                      borderRadius: 'var(--radius-md)',
                      background: stage.active ? 'rgba(99, 102, 241, 0.15)' : 'rgba(255, 255, 255, 0.02)',
                      border: `1px solid ${stage.active ? 'var(--primary)' : 'var(--border-subtle)'}`,
                    }}
                  >
                    <span style={{
                      width: '24px',
                      height: '24px',
                      borderRadius: '50%',
                      background: stage.done ? '#10b981' : 'rgba(255, 255, 255, 0.1)',
                      color: stage.done ? '#fff' : 'var(--text-dim)',
                      display: 'flex',
                      alignItems: 'center',
                      justifyContent: 'center',
                      fontSize: '0.75rem',
                      fontWeight: 800,
                    }}>
                      {stage.done ? <CheckIcon size={12} color="#fff" /> : idx + 1}
                    </span>

                    <span style={{ fontWeight: stage.active ? 700 : 500, color: stage.done ? 'var(--text-main)' : 'var(--text-dim)' }}>
                      {stage.label}
                    </span>
                  </div>
                ))}
              </div>
            </div>

            <div style={{ display: 'flex', justifyContent: 'flex-end', gap: '0.75rem' }}>
              <button
                className="btn btn-secondary"
                onClick={() => setSelectedApp(null)}
              >
                Close
              </button>
            </div>
          </div>
        </div>
      )}

      {/* Withdraw Modal */}
      {withdrawingId && (
        <div className="modal-overlay">
          <div className="modal-content" style={{ maxWidth: '460px' }}>
            <h3 style={{ fontSize: '1.25rem', marginBottom: '0.75rem' }}>Withdraw Application</h3>
            <p style={{ color: 'var(--text-muted)', fontSize: '0.925rem', marginBottom: '1.5rem', lineHeight: 1.5 }}>
              Are you sure you want to withdraw application #{withdrawingId}? The recruiter will be notified and your status will be updated to <strong>WITHDRAWN</strong>.
            </p>
            <div style={{ display: 'flex', gap: '0.75rem', justifyContent: 'flex-end' }}>
              <button
                className="btn btn-secondary"
                disabled={withdrawSubmitting}
                onClick={() => setWithdrawingId(null)}
              >
                Cancel
              </button>
              <button
                className="btn btn-danger"
                disabled={withdrawSubmitting}
                onClick={handleWithdrawConfirm}
              >
                {withdrawSubmitting ? 'Withdrawing...' : 'Confirm Withdrawal'}
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default Applications;
