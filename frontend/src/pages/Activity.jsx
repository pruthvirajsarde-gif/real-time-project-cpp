import React, { useState, useEffect } from 'react';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import { dashboardApi } from '../api/dashboardApi';
import { useAuth } from '../context/AuthContext';
import { formatDate } from '../utils/formatters';
import { RotateCcwIcon } from '../components/Icons';

export const Activity = () => {
  const { role } = useAuth();
  const [activities, setActivities] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const fetchActivity = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await dashboardApi.getActivity(40);
      if (res && Array.isArray(res.data)) {
        setActivities(res.data);
      } else {
        setActivities([]);
      }
    } catch (err) {
      setError(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchActivity();
  }, []);

  const getActionBadge = (action) => {
    switch (action) {
      case 'LOGIN_SUCCESS':
      case 'REGISTRATION_SUCCESS':
        return <span className="badge badge-success">Auth</span>;
      case 'RESUME_UPLOADED':
        return <span className="badge badge-primary">Resume</span>;
      case 'APPLICATION_SUBMITTED':
      case 'APPLICATION_WITHDRAWN':
        return <span className="badge badge-info">Application</span>;
      case 'JOB_CREATED':
      case 'JOB_UPDATED':
      case 'JOB_DELETED':
        return <span className="badge badge-warning">Job Posting</span>;
      default:
        return <span className="badge badge-neutral">{action}</span>;
    }
  };

  return (
    <div className="portal-layout">
      <Sidebar type={role === 'RECRUITER' ? 'recruiter' : 'candidate'} />

      <main className="portal-main">
        <div className="portal-content">
          <div className="page-header">
            <div>
              <h1 className="page-title">Operational Audit Trail</h1>
              <p className="page-subtitle">
                User-isolated audit records verifying security events, resume updates, and application transitions
              </p>
            </div>
            <button onClick={fetchActivity} className="btn btn-outline" style={{ display: 'inline-flex', alignItems: 'center', gap: '0.4rem' }}>
              <RotateCcwIcon size={14} />
              <span>Refresh Log</span>
            </button>
          </div>

          <ErrorMessage error={error} onRetry={fetchActivity} />

          {loading ? (
            <Loading message="Fetching operational audit history..." />
          ) : activities.length === 0 ? (
            <div className="card" style={{ textAlign: 'center', padding: '3.5rem 1rem' }}>
              <p style={{ color: 'var(--text-muted)' }}>No audit events recorded for this account yet.</p>
            </div>
          ) : (
            <div className="card">
              <div style={{ display: 'flex', flexDirection: 'column', gap: '1rem' }}>
                {activities.map((item, idx) => (
                  <div
                    key={idx}
                    style={{
                      display: 'flex',
                      alignItems: 'center',
                      justifyContent: 'space-between',
                      padding: '0.85rem 1rem',
                      borderRadius: 'var(--radius-md)',
                      background: 'rgba(15, 23, 42, 0.4)',
                      border: '1px solid var(--border-subtle)',
                      flexWrap: 'wrap',
                      gap: '0.75rem'
                    }}
                  >
                    <div style={{ display: 'flex', alignItems: 'center', gap: '0.85rem' }}>
                      {getActionBadge(item.action)}
                      <div>
                        <div style={{ fontWeight: 600, fontSize: '0.925rem' }}>{item.action}</div>
                        <div style={{ fontSize: '0.825rem', color: 'var(--text-muted)', marginTop: '0.1rem' }}>
                          {item.details || 'System operation executed.'}
                        </div>
                      </div>
                    </div>

                    <div style={{ fontSize: '0.8rem', color: 'var(--text-dim)', fontFamily: 'var(--font-mono)' }}>
                      {formatDate(item.createdAt)}
                    </div>
                  </div>
                ))}
              </div>
            </div>
          )}
        </div>
      </main>
    </div>
  );
};

export default Activity;
