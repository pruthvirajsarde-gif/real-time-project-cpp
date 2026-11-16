import React, { useState, useEffect } from 'react';
import { useLocation, useParams, Link } from 'react-router-dom';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import ApplicationStatus from '../components/ApplicationStatus';
import ScoreCard from '../components/ScoreCard';
import LiveSyncBadge from '../components/LiveSyncBadge';
import { useLiveSync } from '../utils/useLiveSync';
import { recruiterApi } from '../api/recruiterApi';
import { formatDate } from '../utils/formatters';
import { RotateCcwIcon, CheckIcon, XIcon, FileTextIcon } from '../components/Icons';

export const Applicants = () => {
  const { id: routeJobId } = useParams();
  const location = useLocation();

  const [applications, setApplications] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const [statusMessage, setStatusMessage] = useState(null);

  // Filters & Search
  const [searchQuery, setSearchQuery] = useState('');
  const [selectedJobFilter, setSelectedJobFilter] = useState(routeJobId || 'ALL');
  const [selectedStatusFilter, setSelectedStatusFilter] = useState('ALL');

  // Status Update Modal
  const [selectedApp, setSelectedApp] = useState(null);
  const [newStatus, setNewStatus] = useState('');
  const [updating, setUpdating] = useState(false);
  const [updateError, setUpdateError] = useState(null);

  // Detail Modal
  const [viewDetailApp, setViewDetailApp] = useState(null);

  const fetchApplications = async (isBackground = false) => {
    if (!isBackground) setLoading(true);
    setError(null);
    try {
      const res = await recruiterApi.getRecruiterApplications();
      if (res && res.data) {
        setApplications(Array.isArray(res.data) ? res.data : []);
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

  useEffect(() => {
    if (routeJobId) {
      setSelectedJobFilter(routeJobId);
    }
  }, [routeJobId]);

  // Extract unique Job IDs for filtering
  const uniqueJobs = Array.from(new Set(applications.map((a) => a.jobId)));

  // Filtered List
  const filteredApps = applications.filter((app) => {
    const matchesJob = selectedJobFilter === 'ALL' || app.jobId === selectedJobFilter;
    const matchesStatus = selectedStatusFilter === 'ALL' || app.status === selectedStatusFilter;
    const matchesSearch =
      searchQuery.trim() === '' ||
      (app.candidateName && app.candidateName.toLowerCase().includes(searchQuery.toLowerCase())) ||
      (app.candidateId && app.candidateId.toLowerCase().includes(searchQuery.toLowerCase())) ||
      (app.jobTitle && app.jobTitle.toLowerCase().includes(searchQuery.toLowerCase())) ||
      (app.jobId && app.jobId.toLowerCase().includes(searchQuery.toLowerCase()));

    return matchesJob && matchesStatus && matchesSearch;
  });

  const handleOpenStatusModal = (app) => {
    setSelectedApp(app);
    setNewStatus(app.status);
    setUpdateError(null);
  };

  const handleUpdateStatusSubmit = async (e) => {
    e.preventDefault();
    if (!selectedApp || !newStatus) return;

    setUpdating(true);
    setUpdateError(null);
    try {
      await recruiterApi.updateApplicationStatus(selectedApp.id, newStatus);
      setStatusMessage(`Status for application #${selectedApp.id} updated to ${newStatus}`);
      setSelectedApp(null);
      await fetchApplications();
      setTimeout(() => setStatusMessage(null), 4000);
    } catch (err) {
      setUpdateError(err.message || 'Failed to update application status.');
    } finally {
      setUpdating(false);
    }
  };

  // Status metrics
  const stats = {
    total: applications.length,
    underReview: applications.filter((a) => a.status === 'UNDER_REVIEW').length,
    shortlisted: applications.filter((a) => a.status === 'SHORTLISTED').length,
    interview: applications.filter((a) => a.status === 'INTERVIEW').length,
    selected: applications.filter((a) => a.status === 'SELECTED').length,
    rejected: applications.filter((a) => a.status === 'REJECTED').length,
  };

  return (
    <div className="portal-layout">
      <Sidebar role="RECRUITER" />

      <main className="portal-content">
        <div className="page-header" style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start' }}>
          <div>
            <h1 className="page-title">Candidate Applicants</h1>
            <p className="page-subtitle">
              Review received applications, match analytics, and advance candidates through recruitment stages.
            </p>
          </div>
          <div style={{ display: 'flex', alignItems: 'center', gap: '0.75rem', flexWrap: 'wrap' }}>
            <LiveSyncBadge
              lastSynced={lastSynced}
              isSyncing={isSyncing}
              syncNow={() => fetchApplications(false)}
              isLive={isLive}
              setIsLive={setIsLive}
              label="Live Pipeline"
            />
          </div>
        </div>

        {statusMessage && (
          <div className="banner banner-success" style={{ marginBottom: '1.5rem', display: 'flex', alignItems: 'center', gap: '0.4rem' }}>
            <CheckIcon size={14} color="#10b981" />
            <span>{statusMessage}</span>
          </div>
        )}


        {/* Stats Strip */}
        <div
          style={{
            display: 'grid',
            gridTemplateColumns: 'repeat(auto-fit, minmax(130px, 1fr))',
            gap: '1rem',
            marginBottom: '1.5rem',
          }}
        >
          <div className="card" style={{ padding: '1rem', textAlign: 'center' }}>
            <span style={{ fontSize: '0.8rem', color: 'var(--text-muted)', textTransform: 'uppercase' }}>Total</span>
            <div style={{ fontSize: '1.5rem', fontWeight: 800 }}>{stats.total}</div>
          </div>
          <div className="card" style={{ padding: '1rem', textAlign: 'center' }}>
            <span style={{ fontSize: '0.8rem', color: 'var(--warning)', textTransform: 'uppercase' }}>Review</span>
            <div style={{ fontSize: '1.5rem', fontWeight: 800 }}>{stats.underReview}</div>
          </div>
          <div className="card" style={{ padding: '1rem', textAlign: 'center' }}>
            <span style={{ fontSize: '0.8rem', color: '#60A5FA', textTransform: 'uppercase' }}>Shortlisted</span>
            <div style={{ fontSize: '1.5rem', fontWeight: 800 }}>{stats.shortlisted}</div>
          </div>
          <div className="card" style={{ padding: '1rem', textAlign: 'center' }}>
            <span style={{ fontSize: '0.8rem', color: 'var(--primary)', textTransform: 'uppercase' }}>Interview</span>
            <div style={{ fontSize: '1.5rem', fontWeight: 800 }}>{stats.interview}</div>
          </div>
          <div className="card" style={{ padding: '1rem', textAlign: 'center' }}>
            <span style={{ fontSize: '0.8rem', color: 'var(--success)', textTransform: 'uppercase' }}>Selected</span>
            <div style={{ fontSize: '1.5rem', fontWeight: 800 }}>{stats.selected}</div>
          </div>
          <div className="card" style={{ padding: '1rem', textAlign: 'center' }}>
            <span style={{ fontSize: '0.8rem', color: 'var(--danger)', textTransform: 'uppercase' }}>Rejected</span>
            <div style={{ fontSize: '1.5rem', fontWeight: 800 }}>{stats.rejected}</div>
          </div>
        </div>

        {/* Filters Toolbar */}
        <div
          className="card"
          style={{
            padding: '1.25rem',
            marginBottom: '1.5rem',
            display: 'flex',
            gap: '1rem',
            flexWrap: 'wrap',
            alignItems: 'center',
          }}
        >
          <div style={{ flex: '1 1 240px' }}>
            <input
              type="text"
              className="form-input"
              placeholder="Search candidate name, ID, or job title..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
            />
          </div>

          <div style={{ minWidth: '180px' }}>
            <select
              className="form-select"
              value={selectedJobFilter}
              onChange={(e) => setSelectedJobFilter(e.target.value)}
            >
              <option value="ALL">All Jobs ({applications.length})</option>
              {uniqueJobs.map((jId) => (
                <option key={jId} value={jId}>
                  Job: {jId}
                </option>
              ))}
            </select>
          </div>

          <div style={{ minWidth: '160px' }}>
            <select
              className="form-select"
              value={selectedStatusFilter}
              onChange={(e) => setSelectedStatusFilter(e.target.value)}
            >
              <option value="ALL">All Statuses</option>
              <option value="APPLIED">Applied</option>
              <option value="UNDER_REVIEW">Under Review</option>
              <option value="SHORTLISTED">Shortlisted</option>
              <option value="INTERVIEW">Interview</option>
              <option value="SELECTED">Selected</option>
              <option value="REJECTED">Rejected</option>
              <option value="WITHDRAWN">Withdrawn</option>
            </select>
          </div>
        </div>

        {/* Body Content */}
        {loading ? (
          <Loading message="Loading candidate applications..." />
        ) : error ? (
          <ErrorMessage error={error} onRetry={fetchApplications} />
        ) : filteredApps.length === 0 ? (
          <div className="card empty-state" style={{ padding: '3.5rem 2rem', textAlign: 'center' }}>
            <div style={{ marginBottom: '1rem', display: 'flex', justifyContent: 'center' }}>
              <FileTextIcon size={48} color="var(--text-dim)" />
            </div>
            <h3 style={{ fontSize: '1.3rem', marginBottom: '0.5rem' }}>No Applicants Found</h3>
            <p style={{ color: 'var(--text-muted)', marginBottom: '1.5rem' }}>
              {applications.length === 0
                ? 'No candidates have submitted applications for your posted jobs yet.'
                : 'No applicants match your current search and filter criteria.'}
            </p>
            {applications.length > 0 ? (
              <button
                onClick={() => {
                  setSearchQuery('');
                  setSelectedJobFilter('ALL');
                  setSelectedStatusFilter('ALL');
                }}
                className="btn btn-secondary"
              >
                Clear Filters
              </button>
            ) : (
              <Link to="/recruiter/jobs/new" className="btn btn-primary">
                Post A New Job
              </Link>
            )}
          </div>
        ) : (
          <div className="table-container card" style={{ padding: '0' }}>
            <table className="data-table">
              <thead>
                <tr>
                  <th>App #</th>
                  <th>Candidate</th>
                  <th>Applied Position</th>
                  <th>Match Score</th>
                  <th>Readiness</th>
                  <th>Status</th>
                  <th>Applied On</th>
                  <th>Actions</th>
                </tr>
              </thead>
              <tbody>
                {filteredApps.map((app) => (
                  <tr key={app.id}>
                    <td style={{ fontFamily: 'var(--font-mono)', fontWeight: 600 }}>#{app.id}</td>
                    <td>
                      <div style={{ fontWeight: 600, color: 'var(--text-main)' }}>
                        {app.candidateName || 'Anonymous Candidate'}
                      </div>
                      <div style={{ fontSize: '0.8rem', color: 'var(--text-muted)' }}>
                        ID: {app.candidateId}
                      </div>
                    </td>
                    <td>
                      <div style={{ fontWeight: 600 }}>{app.jobTitle}</div>
                      <div style={{ fontSize: '0.8rem', color: 'var(--text-muted)' }}>
                        {app.jobId} · {app.company}
                      </div>
                    </td>
                    <td>
                      <span
                        style={{
                          fontWeight: 700,
                          fontSize: '0.95rem',
                          color:
                            app.matchScore >= 70
                              ? 'var(--success)'
                              : app.matchScore >= 50
                              ? 'var(--warning)'
                              : 'var(--danger)',
                        }}
                      >
                        {typeof app.matchScore === 'number' ? `${app.matchScore.toFixed(1)}%` : 'N/A'}
                      </span>
                    </td>
                    <td>
                      <span style={{ fontSize: '0.9rem', color: 'var(--text-main)' }}>
                        {typeof app.readinessScore === 'number' ? `${app.readinessScore.toFixed(1)}%` : '—'}
                      </span>
                    </td>
                    <td>
                      <ApplicationStatus status={app.status} />
                    </td>
                    <td style={{ fontSize: '0.85rem', color: 'var(--text-muted)' }}>
                      {formatDate(app.appliedAt)}
                    </td>
                    <td>
                      <div style={{ display: 'flex', gap: '0.5rem' }}>
                        <button
                          onClick={() => setViewDetailApp(app)}
                          className="btn btn-sm btn-secondary"
                          title="View Candidate Information"
                        >
                          View
                        </button>
                        <button
                          onClick={() => handleOpenStatusModal(app)}
                          className="btn btn-sm btn-primary"
                          title="Update Applicant Status"
                        >
                          Status
                        </button>
                      </div>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        )}
      </main>

      {/* Update Status Modal */}
      {selectedApp && (
        <div className="modal-overlay">
          <div className="modal-content" style={{ maxWidth: '480px' }}>
            <h3 style={{ fontSize: '1.3rem', marginBottom: '0.5rem' }}>Update Application Stage</h3>
            <p style={{ color: 'var(--text-muted)', fontSize: '0.9rem', marginBottom: '1.25rem' }}>
              Candidate: <strong>{selectedApp.candidateName || selectedApp.candidateId}</strong>
              <br />
              Position: <strong>{selectedApp.jobTitle}</strong> ({selectedApp.jobId})
            </p>

            {updateError && (
              <div className="banner banner-danger" style={{ marginBottom: '1rem', fontSize: '0.9rem', display: 'flex', alignItems: 'center', gap: '0.4rem' }}>
                <XIcon size={14} color="#ef4444" />
                <span>{updateError}</span>
              </div>
            )}

            <form onSubmit={handleUpdateStatusSubmit}>
              <div className="form-group">
                <label className="form-label">Workflow Status Stage *</label>
                <select
                  className="form-select"
                  value={newStatus}
                  onChange={(e) => setNewStatus(e.target.value)}
                  required
                >
                  <option value="UNDER_REVIEW">Under Review</option>
                  <option value="SHORTLISTED">Shortlisted</option>
                  <option value="INTERVIEW">Interview</option>
                  <option value="SELECTED">Selected</option>
                  <option value="REJECTED">Rejected</option>
                </select>
              </div>

              <div style={{ display: 'flex', justifyContent: 'flex-end', gap: '0.75rem', marginTop: '1.5rem' }}>
                <button
                  type="button"
                  onClick={() => setSelectedApp(null)}
                  className="btn btn-secondary"
                  disabled={updating}
                >
                  Cancel
                </button>
                <button type="submit" className="btn btn-primary" disabled={updating}>
                  {updating ? 'Saving...' : 'Update Status'}
                </button>
              </div>
            </form>
          </div>
        </div>
      )}

      {/* Candidate Application Detail Modal */}
      {viewDetailApp && (
        <div className="modal-overlay">
          <div className="modal-content" style={{ maxWidth: '580px' }}>
            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start', marginBottom: '1rem' }}>
              <div>
                <h3 style={{ fontSize: '1.3rem', marginBottom: '0.25rem' }}>Applicant Details</h3>
                <span style={{ fontSize: '0.85rem', color: 'var(--text-muted)' }}>
                  Application #{viewDetailApp.id} · Received {formatDate(viewDetailApp.appliedAt)}
                </span>
              </div>
              <button
                onClick={() => setViewDetailApp(null)}
                style={{ background: 'none', border: 'none', color: 'var(--text-muted)', cursor: 'pointer', display: 'flex', alignItems: 'center' }}
              >
                <XIcon size={16} />
              </button>
            </div>


            <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '1rem', marginBottom: '1.5rem' }}>
              <div className="card" style={{ padding: '1rem' }}>
                <div style={{ fontSize: '0.8rem', color: 'var(--text-muted)', marginBottom: '0.25rem' }}>
                  Candidate Name
                </div>
                <div style={{ fontWeight: 700, fontSize: '1.1rem' }}>
                  {viewDetailApp.candidateName || 'Candidate'}
                </div>
                <div style={{ fontSize: '0.8rem', color: 'var(--text-muted)' }}>
                  ID: {viewDetailApp.candidateId}
                </div>
              </div>

              <div className="card" style={{ padding: '1rem' }}>
                <div style={{ fontSize: '0.8rem', color: 'var(--text-muted)', marginBottom: '0.25rem' }}>
                  Target Role
                </div>
                <div style={{ fontWeight: 700, fontSize: '1.1rem' }}>{viewDetailApp.jobTitle}</div>
                <div style={{ fontSize: '0.8rem', color: 'var(--text-muted)' }}>
                  {viewDetailApp.company} ({viewDetailApp.jobId})
                </div>
              </div>
            </div>

            <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '1rem', marginBottom: '1.5rem' }}>
              <div className="card" style={{ padding: '1rem', textAlign: 'center' }}>
                <div style={{ fontSize: '0.8rem', color: 'var(--text-muted)', marginBottom: '0.5rem' }}>
                  Match Relevance
                </div>
                <div style={{ fontSize: '1.6rem', fontWeight: 800, color: 'var(--primary)' }}>
                  {typeof viewDetailApp.matchScore === 'number' ? `${viewDetailApp.matchScore.toFixed(1)}%` : 'N/A'}
                </div>
              </div>

              <div className="card" style={{ padding: '1rem', textAlign: 'center' }}>
                <div style={{ fontSize: '0.8rem', color: 'var(--text-muted)', marginBottom: '0.5rem' }}>
                  Readiness Score
                </div>
                <div style={{ fontSize: '1.6rem', fontWeight: 800, color: 'var(--success)' }}>
                  {typeof viewDetailApp.readinessScore === 'number'
                    ? `${viewDetailApp.readinessScore.toFixed(1)}%`
                    : 'N/A'}
                </div>
              </div>
            </div>

            <div style={{ marginBottom: '1.5rem' }}>
              <div style={{ fontSize: '0.85rem', color: 'var(--text-muted)', marginBottom: '0.5rem' }}>
                Current Stage
              </div>
              <ApplicationStatus status={viewDetailApp.status} />
            </div>

            <div style={{ display: 'flex', justifyContent: 'flex-end', gap: '0.75rem' }}>
              <button
                onClick={() => {
                  const target = viewDetailApp;
                  setViewDetailApp(null);
                  handleOpenStatusModal(target);
                }}
                className="btn btn-primary"
              >
                Change Status
              </button>
              <button onClick={() => setViewDetailApp(null)} className="btn btn-secondary">
                Close
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default Applicants;
