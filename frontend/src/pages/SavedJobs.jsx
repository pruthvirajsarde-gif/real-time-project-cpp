import React, { useState, useEffect } from 'react';
import { Link } from 'react-router-dom';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import { savedJobsApi } from '../api/savedJobsApi';
import { applicationApi } from '../api/applicationApi';
import { formatDate } from '../utils/formatters';
import { SearchIcon, BookmarkIcon, XIcon } from '../components/Icons';

export const SavedJobs = () => {
  const [savedJobs, setSavedJobs] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const [feedbackNotice, setFeedbackNotice] = useState(null);

  const fetchSavedJobs = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await savedJobsApi.getSavedJobs();
      if (res && Array.isArray(res.data)) {
        setSavedJobs(res.data);
      } else {
        setSavedJobs([]);
      }
    } catch (err) {
      setError(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchSavedJobs();
  }, []);

  const handleRemove = async (jobId) => {
    try {
      await savedJobsApi.removeSavedJob(jobId);
      setSavedJobs((prev) => prev.filter((j) => j.jobId !== jobId));
      setFeedbackNotice({ type: 'info', msg: `Job ${jobId} removed from saved list.` });
    } catch (err) {
      setFeedbackNotice({ type: 'danger', msg: err.message });
    }
  };

  const handleApply = async (jobId, title) => {
    try {
      await applicationApi.applyForJob(jobId);
      setFeedbackNotice({ type: 'success', msg: `Successfully applied to ${title || jobId}!` });
    } catch (err) {
      setFeedbackNotice({ type: 'danger', msg: err.message });
    }
  };

  return (
    <div className="portal-layout">
      <Sidebar type="candidate" />

      <main className="portal-main">
        <div className="portal-content">
          <div className="page-header">
            <div>
              <h1 className="page-title">Saved Job Bookmarks</h1>
              <p className="page-subtitle">
                Keep track of interesting positions and review them before applying
              </p>
            </div>
            <Link to="/candidate/jobs" className="btn btn-primary" style={{ display: 'inline-flex', alignItems: 'center', gap: '0.4rem' }}>
              <SearchIcon size={14} />
              <span>Explore More Jobs</span>
            </Link>
          </div>

          {feedbackNotice && (
            <div style={{
              background: feedbackNotice.type === 'success' ? 'rgba(16, 185, 129, 0.12)' : 'rgba(239, 68, 68, 0.12)',
              border: `1px solid ${feedbackNotice.type === 'success' ? 'rgba(16, 185, 129, 0.3)' : 'rgba(239, 68, 68, 0.3)'}`,
              borderRadius: 'var(--radius-md)',
              padding: '0.85rem 1.25rem',
              color: feedbackNotice.type === 'success' ? '#6ee7b7' : '#fca5a5',
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

          <ErrorMessage error={error} onRetry={fetchSavedJobs} />

          {loading ? (
            <Loading message="Loading saved bookmarks..." />
          ) : savedJobs.length === 0 ? (
            <div className="card" style={{ textAlign: 'center', padding: '3.5rem 1rem' }}>
              <div style={{ display: 'flex', justifyContent: 'center', marginBottom: '1rem', color: 'var(--primary)' }}>
                <BookmarkIcon size={44} />
              </div>
              <h3 style={{ fontSize: '1.25rem', marginBottom: '0.5rem' }}>No saved jobs yet</h3>
              <p style={{ color: 'var(--text-muted)', marginBottom: '1.5rem' }}>
                When browsing positions, click "Save" to keep them organized here.
              </p>
              <Link to="/candidate/jobs" className="btn btn-primary">
                Browse Open Opportunities
              </Link>
            </div>
          ) : (
            <div className="table-container">
              <table className="data-table">
                <thead>
                  <tr>
                    <th>Job Title</th>
                    <th>Company</th>
                    <th>Location</th>
                    <th>Saved Date</th>
                    <th>Actions</th>
                  </tr>
                </thead>
                <tbody>
                  {savedJobs.map((item) => (
                    <tr key={item.jobId}>
                      <td style={{ fontWeight: 600 }}>
                        <Link to={`/candidate/jobs/${item.jobId}`} style={{ color: 'var(--text-main)' }}>
                          {item.title || item.jobId}
                        </Link>
                      </td>
                      <td>{item.company || '—'}</td>
                      <td>{item.location || 'Remote'}</td>
                      <td style={{ color: 'var(--text-muted)', fontSize: '0.85rem' }}>
                        {formatDate(item.savedAt)}
                      </td>
                      <td>
                        <div style={{ display: 'flex', gap: '0.5rem' }}>
                          <Link to={`/candidate/jobs/${item.jobId}`} className="btn btn-sm btn-secondary">
                            View
                          </Link>
                          <button
                            onClick={() => handleApply(item.jobId, item.title)}
                            className="btn btn-sm btn-primary"
                          >
                            Apply
                          </button>
                          <button
                            onClick={() => handleRemove(item.jobId)}
                            className="btn btn-sm btn-danger"
                            title="Remove bookmark"
                            style={{ display: 'inline-flex', alignItems: 'center', justifyContent: 'center' }}
                          >
                            <XIcon size={13} />
                          </button>
                        </div>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          )}
        </div>
      </main>
    </div>
  );
};

export default SavedJobs;
