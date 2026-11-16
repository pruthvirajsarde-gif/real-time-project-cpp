import React, { useState, useEffect } from 'react';
import { useParams, Link } from 'react-router-dom';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import ComparisonTable from '../components/ComparisonTable';
import ApplicationStatus from '../components/ApplicationStatus';
import { analyticsApi } from '../api/analyticsApi';
import { formatScore, formatDate } from '../utils/formatters';
import { UsersIcon, ScaleIcon } from '../components/Icons';

export const CandidateComparisonPage = () => {
  const { id } = useParams();
  const [candidates, setCandidates] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const fetchCandidates = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await analyticsApi.getRecruiterJobCandidateComparison(id);
      if (Array.isArray(res?.data)) {
        setCandidates(res.data);
      }
    } catch (err) {
      setError(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    if (id) fetchCandidates();
  }, [id]);

  const columns = [
    {
      key: 'candidateName',
      label: 'Candidate',
      render: (val, row) => (
        <div>
          <div style={{ fontWeight: 700, color: 'var(--text)' }}>{val}</div>
          <div style={{ fontSize: '0.75rem', color: 'var(--text-muted)' }}>
            Applied {formatDate(row.appliedDate)}
          </div>
        </div>
      ),
    },
    {
      key: 'matchScore',
      label: 'Match Score',
      align: 'center',
      render: (val) => (
        <span style={{
          fontWeight: 800,
          fontSize: '0.95rem',
          color: val >= 80 ? '#10b981' : val >= 60 ? '#6366f1' : '#f59e0b',
        }}>
          {formatScore(val)}
        </span>
      ),
    },
    {
      key: 'requiredSkillsMatched',
      label: 'Required Skills',
      align: 'center',
      render: (val, row) => (
        <span style={{ fontWeight: 600 }}>
          {val} / {row.totalRequiredSkills}
        </span>
      ),
    },
    {
      key: 'preferredSkillsMatched',
      label: 'Preferred Skills',
      align: 'center',
      render: (val, row) => (
        <span style={{ color: 'var(--text-muted)' }}>
          {val} / {row.totalPreferredSkills}
        </span>
      ),
    },
    {
      key: 'totalExperience',
      label: 'Experience',
      render: (val) => <span style={{ fontWeight: 600 }}>{val}</span>,
    },
    {
      key: 'educationSummary',
      label: 'Education',
      render: (val) => (
        <span style={{ fontSize: '0.85rem', color: 'var(--text-secondary)' }}>
          {val || 'Not specified'}
        </span>
      ),
    },
    {
      key: 'status',
      label: 'Status',
      align: 'center',
      render: (val) => <ApplicationStatus status={val} />,
    },
    {
      key: 'matchedSkills',
      label: 'Key Matched Skills',
      render: (val) => (
        <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.25rem', maxWidth: '240px' }}>
          {Array.isArray(val) && val.slice(0, 3).map((s, idx) => (
            <span key={idx} className="badge badge-success" style={{ fontSize: '0.7rem', padding: '0.15rem 0.4rem' }}>
              {s}
            </span>
          ))}
          {Array.isArray(val) && val.length > 3 && (
            <span style={{ fontSize: '0.7rem', color: 'var(--text-muted)' }}>
              +{val.length - 3} more
            </span>
          )}
        </div>
      ),
    },
  ];

  return (
    <div className="portal-layout">
      <Sidebar type="recruiter" />

      <main className="portal-main">
        <div className="portal-content">
          <div className="page-header">
            <div>
              <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem', marginBottom: '0.25rem' }}>
                <Link to={`/recruiter/jobs/${id}/analytics`} style={{ fontSize: '0.85rem', color: 'var(--text-muted)' }}>
                  ← Back to Requisition Analytics
                </Link>
              </div>
              <h1 className="page-title">Candidate Comparison Matrix</h1>
              <p className="page-subtitle">
                Objective, side-by-side comparison of applicant credentials for Job Requisition {id}
              </p>
            </div>
            <div style={{ display: 'flex', gap: '0.75rem' }}>
              <Link to={`/recruiter/jobs/${id}/applicants`} className="btn btn-outline btn-sm">
                <UsersIcon size={14} />
                <span>View Applicants List</span>
              </Link>
            </div>
          </div>

          {/* Academic Transparency Note */}
          <div style={{
            background: 'rgba(99, 102, 241, 0.08)',
            border: '1px solid rgba(99, 102, 241, 0.2)',
            borderRadius: 'var(--radius-md)',
            padding: '1rem 1.25rem',
            marginBottom: '1.75rem',
            display: 'flex',
            alignItems: 'center',
            gap: '0.85rem',
            fontSize: '0.85rem',
            color: 'var(--text-muted)',
          }}>
            <ScaleIcon size={20} color="#818cf8" />
            <span>
              <strong style={{ color: 'var(--text-main)' }}>Factual Comparison Notice:</strong> This comparison table presents objective resume and qualification facts. SmartHire does not compute arbitrary "hireability labels" or replace human decision-making; final hiring evaluation remains with the recruiter.
            </span>
          </div>

          <ErrorMessage error={error} onRetry={fetchCandidates} />

          {loading ? (
            <Loading message="Synthesizing candidate comparison records..." />
          ) : (
            <div className="card" style={{ padding: '1rem', overflow: 'hidden' }}>
              <ComparisonTable
                columns={columns}
                data={candidates}
                emptyMessage="No applicants have submitted applications to this job requisition yet."
              />
            </div>
          )}
        </div>
      </main>
    </div>
  );
};

export default CandidateComparisonPage;
