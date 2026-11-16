import React, { useState, useEffect } from 'react';
import { useSearchParams } from 'react-router-dom';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import { analyticsApi } from '../api/analyticsApi';
import { jobsApi } from '../api/jobsApi';
import { applicationApi } from '../api/applicationApi';
import { formatScore } from '../utils/formatters';
import {
  MapPinIcon,
  CheckIcon,
  XIcon,
  ScaleIcon,
  BarChartIcon
} from '../components/Icons';

export const ResumeJobComparisonPage = () => {
  const [searchParams] = useSearchParams();
  const initialJobId = searchParams.get('jobId') || '';

  const [jobs, setJobs] = useState([]);
  const [selectedJobId, setSelectedJobId] = useState(initialJobId);
  const [comparison, setComparison] = useState(null);
  const [loadingJobs, setLoadingJobs] = useState(true);
  const [loadingCompare, setLoadingCompare] = useState(false);
  const [error, setError] = useState(null);
  const [applied, setApplied] = useState(false);
  const [applyMsg, setApplyMsg] = useState('');

  // 1. Fetch all available jobs for dropdown
  useEffect(() => {
    const fetchJobs = async () => {
      setLoadingJobs(true);
      try {
        const res = await jobsApi.getJobs({ limit: 50 });
        const jobList = res?.data?.items || res?.data || [];
        setJobs(jobList);
        if (!selectedJobId && jobList.length > 0) {
          setSelectedJobId(String(jobList[0].jobId));
        }
      } catch (err) {
        setError(err);
      } finally {
        setLoadingJobs(false);
      }
    };
    fetchJobs();
  }, []);

  // 2. Fetch comparison whenever selectedJobId changes
  useEffect(() => {
    if (!selectedJobId) return;

    const fetchComparison = async () => {
      setLoadingCompare(true);
      setError(null);
      setApplied(false);
      setApplyMsg('');
      try {
        const res = await analyticsApi.getResumeJobComparison(selectedJobId);
        if (res?.data) {
          setComparison(res.data);
        }
      } catch (err) {
        setError(err);
      } finally {
        setLoadingCompare(false);
      }
    };

    fetchComparison();
  }, [selectedJobId]);

  const handleApply = async () => {
    if (!selectedJobId) return;
    try {
      await applicationApi.applyForJob(selectedJobId);
      setApplied(true);
      setApplyMsg('Application submitted successfully!');
    } catch (err) {
      setApplyMsg(err.message || 'Failed to submit application.');
    }
  };

  return (
    <div className="portal-layout">
      <Sidebar type="candidate" />

      <main className="portal-main">
        <div className="portal-content">
          <div className="page-header">
            <div>
              <h1 className="page-title">Resume vs. Job Requisition Comparison</h1>
              <p className="page-subtitle">
                Side-by-side analytical audit comparing your resume credentials against job requirements
              </p>
            </div>
          </div>

          <ErrorMessage error={error} />

          {/* Job Selector Dropdown Card */}
          <div className="card" style={{ padding: '1.4rem 1.6rem', marginBottom: '1.75rem' }}>
            <label style={{ display: 'block', fontSize: '0.8rem', fontWeight: 700, color: 'var(--text-muted)', marginBottom: '0.65rem', textTransform: 'uppercase', letterSpacing: '0.04em' }}>
              Select Job to Compare:
            </label>
            <div style={{ display: 'flex', gap: '1rem', flexWrap: 'wrap' }}>
              <select
                value={selectedJobId}
                onChange={(e) => setSelectedJobId(e.target.value)}
                className="form-select"
                style={{ flex: 1, minWidth: '280px' }}
                disabled={loadingJobs}
              >
                {jobs.map((j) => (
                  <option key={j.jobId} value={j.jobId}>
                    {j.title} — {j.company} ({j.location})
                  </option>
                ))}
              </select>

              <button
                onClick={handleApply}
                disabled={applied}
                className={`btn ${applied ? 'btn-secondary' : 'btn-primary'}`}
              >
                {applied ? (
                  <>
                    <CheckIcon size={16} />
                    <span>Applied</span>
                  </>
                ) : (
                  <span>Apply for this Job</span>
                )}
              </button>
            </div>
            {applyMsg && (
              <div style={{ marginTop: '0.85rem', fontSize: '0.85rem', color: applied ? '#10b981' : '#f87171' }}>
                {applyMsg}
              </div>
            )}
          </div>

          {loadingCompare ? (
            <Loading message="Computing multidimensional compatibility comparison..." />
          ) : comparison ? (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '1.75rem' }}>
              {/* Overall Compatibility Banner */}
              <div className="card" style={{
                background: 'linear-gradient(135deg, rgba(99, 102, 241, 0.12) 0%, rgba(6, 182, 212, 0.08) 100%)',
                padding: '1.75rem',
                border: '1px solid rgba(99, 102, 241, 0.3)',
              }}>
                <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', flexWrap: 'wrap', gap: '1.25rem' }}>
                  <div>
                    <span className="badge badge-primary" style={{ marginBottom: '0.65rem', display: 'inline-block' }}>
                      {comparison.classification || 'Match Analysis'}
                    </span>
                    <h2 style={{ fontSize: '1.75rem', fontWeight: 800, margin: '0 0 0.35rem 0', letterSpacing: '-0.02em' }}>
                      {comparison.jobTitle}
                    </h2>
                    <p style={{ margin: 0, color: 'var(--text-muted)', fontSize: '0.9rem', display: 'flex', alignItems: 'center', gap: '0.4rem' }}>
                      <span>{comparison.company}</span>
                      <span>&bull;</span>
                      <span style={{ display: 'inline-flex', alignItems: 'center', gap: '0.25rem' }}>
                        <MapPinIcon size={14} color="var(--text-dim)" />
                        <span>{comparison.location}</span>
                      </span>
                    </p>
                  </div>

                  <div style={{ textAlign: 'right' }}>
                    <div style={{ fontSize: '2.5rem', fontWeight: 900, color: 'var(--primary)', letterSpacing: '-0.03em', lineHeight: 1 }}>
                      {formatScore(comparison.overallScore)}
                    </div>
                    <div style={{ fontSize: '0.75rem', color: 'var(--text-muted)', fontWeight: 600, textTransform: 'uppercase', letterSpacing: '0.04em', marginTop: '0.25rem' }}>
                      Overall Compatibility Score
                    </div>
                  </div>
                </div>
              </div>

              {/* Grid: Education & Experience Comparison */}
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(320px, 1fr))', gap: '1.5rem' }}>
                {/* Education */}
                <div className="card" style={{ padding: '1.75rem' }}>
                  <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1.25rem' }}>
                    <h3 style={{ fontSize: '1.1rem', fontWeight: 700, margin: 0 }}>
                      Education Alignment
                    </h3>
                    <span className={`badge ${comparison.education?.compatibility === 'Compatible' ? 'badge-success' : 'badge-warning'}`}>
                      {comparison.education?.compatibility || 'Evaluated'}
                    </span>
                  </div>

                  <div style={{ display: 'flex', flexDirection: 'column', gap: '0.85rem', fontSize: '0.875rem' }}>
                    <div style={{ background: 'var(--surface)', padding: '0.85rem 1rem', borderRadius: 'var(--radius-sm)', border: '1px solid var(--border-subtle)' }}>
                      <span style={{ color: 'var(--text-muted)', display: 'block', fontSize: '0.72rem', fontWeight: 700, textTransform: 'uppercase', letterSpacing: '0.04em', marginBottom: '0.25rem' }}>
                        JOB REQUIREMENT
                      </span>
                      <strong style={{ color: 'var(--text-main)' }}>
                        {comparison.education?.required || 'Relevant Degree'}
                      </strong>
                    </div>

                    <div style={{ background: 'var(--surface)', padding: '0.85rem 1rem', borderRadius: 'var(--radius-sm)', border: '1px solid var(--border-subtle)' }}>
                      <span style={{ color: 'var(--text-muted)', display: 'block', fontSize: '0.72rem', fontWeight: 700, textTransform: 'uppercase', letterSpacing: '0.04em', marginBottom: '0.25rem' }}>
                        YOUR CREDENTIAL
                      </span>
                      <strong style={{ color: 'var(--text-main)' }}>
                        {comparison.education?.candidate || 'Not specified'}
                      </strong>
                    </div>
                  </div>
                </div>

                {/* Experience */}
                <div className="card" style={{ padding: '1.75rem' }}>
                  <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1.25rem' }}>
                    <h3 style={{ fontSize: '1.1rem', fontWeight: 700, margin: 0 }}>
                      Experience Compatibility
                    </h3>
                    <span className={`badge ${comparison.experience?.compatibility?.includes('Meets') ? 'badge-success' : 'badge-warning'}`}>
                      {comparison.experience?.compatibility || 'Evaluated'}
                    </span>
                  </div>

                  <div style={{ display: 'flex', flexDirection: 'column', gap: '0.85rem', fontSize: '0.875rem' }}>
                    <div style={{ background: 'var(--surface)', padding: '0.85rem 1rem', borderRadius: 'var(--radius-sm)', border: '1px solid var(--border-subtle)' }}>
                      <span style={{ color: 'var(--text-muted)', display: 'block', fontSize: '0.72rem', fontWeight: 700, textTransform: 'uppercase', letterSpacing: '0.04em', marginBottom: '0.25rem' }}>
                        JOB REQUIREMENT
                      </span>
                      <strong style={{ color: 'var(--text-main)' }}>
                        {comparison.experience?.required || '0 years'}
                      </strong>
                    </div>

                    <div style={{ background: 'var(--surface)', padding: '0.85rem 1rem', borderRadius: 'var(--radius-sm)', border: '1px solid var(--border-subtle)' }}>
                      <span style={{ color: 'var(--text-muted)', display: 'block', fontSize: '0.72rem', fontWeight: 700, textTransform: 'uppercase', letterSpacing: '0.04em', marginBottom: '0.25rem' }}>
                        YOUR EXPERIENCE
                      </span>
                      <strong style={{ color: 'var(--text-main)' }}>
                        {comparison.experience?.candidate || '0 years'}
                      </strong>
                    </div>
                  </div>
                </div>
              </div>

              {/* Skills Matrix Comparison */}
              <div className="card" style={{ padding: '1.75rem' }}>
                <h3 style={{ fontSize: '1.15rem', fontWeight: 700, marginBottom: '1.35rem' }}>
                  Technical Skills Alignment
                </h3>

                <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(280px, 1fr))', gap: '1.5rem' }}>
                  {/* Matched */}
                  <div>
                    <span style={{ fontSize: '0.75rem', fontWeight: 700, color: '#10b981', textTransform: 'uppercase', display: 'block', marginBottom: '0.65rem', letterSpacing: '0.04em' }}>
                      Matched Skills ({comparison.skills?.matched?.length || 0})
                    </span>
                    <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.45rem' }}>
                      {comparison.skills?.matched?.length > 0 ? (
                        comparison.skills.matched.map((s, idx) => (
                          <span key={idx} className="badge badge-success" style={{ fontSize: '0.8rem', padding: '0.35rem 0.75rem' }}>
                            {s}
                          </span>
                        ))
                      ) : (
                        <span style={{ color: 'var(--text-muted)', fontSize: '0.85rem' }}>No exact matches</span>
                      )}
                    </div>
                  </div>

                  {/* Related */}
                  {comparison.skills?.related?.length > 0 && (
                    <div>
                      <span style={{ fontSize: '0.75rem', fontWeight: 700, color: '#06b6d4', textTransform: 'uppercase', display: 'block', marginBottom: '0.65rem', letterSpacing: '0.04em' }}>
                        Related Competencies ({comparison.skills.related.length})
                      </span>
                      <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.45rem' }}>
                        {comparison.skills.related.map((r, idx) => (
                          <span key={idx} style={{
                            background: 'rgba(6, 182, 212, 0.12)',
                            color: '#67e8f9',
                            border: '1px solid rgba(6, 182, 212, 0.25)',
                            padding: '0.25rem 0.65rem',
                            borderRadius: 'var(--radius-sm)',
                            fontSize: '0.8rem'
                          }}>
                            {r}
                          </span>
                        ))}
                      </div>
                    </div>
                  )}

                  {/* Missing Required */}
                  <div>
                    <span style={{ fontSize: '0.75rem', fontWeight: 700, color: '#ef4444', textTransform: 'uppercase', display: 'block', marginBottom: '0.65rem', letterSpacing: '0.04em' }}>
                      Missing Required Skills ({comparison.skills?.missingRequired?.length || 0})
                    </span>
                    <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.45rem' }}>
                      {comparison.skills?.missingRequired?.length > 0 ? (
                        comparison.skills.missingRequired.map((s, idx) => (
                          <span key={idx} className="badge badge-danger" style={{ fontSize: '0.8rem', padding: '0.35rem 0.75rem' }}>
                            {s}
                          </span>
                        ))
                      ) : (
                        <span style={{ color: '#10b981', fontSize: '0.85rem', fontWeight: 600 }}>All required skills satisfied</span>
                      )}
                    </div>
                  </div>

                  {/* Missing Preferred */}
                  {comparison.skills?.missingPreferred?.length > 0 && (
                    <div>
                      <span style={{ fontSize: '0.75rem', fontWeight: 700, color: '#f59e0b', textTransform: 'uppercase', display: 'block', marginBottom: '0.65rem', letterSpacing: '0.04em' }}>
                        Missing Preferred Skills ({comparison.skills.missingPreferred.length})
                      </span>
                      <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.45rem' }}>
                        {comparison.skills.missingPreferred.map((s, idx) => (
                          <span key={idx} style={{
                            background: 'rgba(245, 158, 11, 0.1)',
                            border: '1px solid rgba(245, 158, 11, 0.25)',
                            color: '#fcd34d',
                            padding: '0.25rem 0.65rem',
                            borderRadius: 'var(--radius-sm)',
                            fontSize: '0.8rem',
                          }}>
                            {s}
                          </span>
                        ))}
                      </div>
                    </div>
                  )}
                </div>
              </div>

              {/* Compatibility Factor Scores */}
              {comparison.scores && (
                <div className="card" style={{ padding: '1.75rem' }}>
                  <h3 style={{ fontSize: '1.1rem', fontWeight: 700, marginBottom: '1.15rem' }}>
                    Detailed Scoring Breakdown
                  </h3>
                  <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(140px, 1fr))', gap: '1rem', textAlign: 'center' }}>
                    <div style={{ background: 'var(--surface)', padding: '1.15rem', borderRadius: 'var(--radius-md)', border: '1px solid var(--border-subtle)' }}>
                      <div style={{ fontSize: '1.6rem', fontWeight: 800, color: 'var(--primary)' }}>
                        {formatScore(comparison.scores.skill)}
                      </div>
                      <div style={{ fontSize: '0.75rem', color: 'var(--text-muted)', fontWeight: 600, marginTop: '0.25rem' }}>Skill Match (60%)</div>
                    </div>
                    <div style={{ background: 'var(--surface)', padding: '1.15rem', borderRadius: 'var(--radius-md)', border: '1px solid var(--border-subtle)' }}>
                      <div style={{ fontSize: '1.6rem', fontWeight: 800, color: '#06b6d4' }}>
                        {formatScore(comparison.scores.text)}
                      </div>
                      <div style={{ fontSize: '0.75rem', color: 'var(--text-muted)', fontWeight: 600, marginTop: '0.25rem' }}>Semantic Relevance</div>
                    </div>
                    <div style={{ background: 'var(--surface)', padding: '1.15rem', borderRadius: 'var(--radius-md)', border: '1px solid var(--border-subtle)' }}>
                      <div style={{ fontSize: '1.6rem', fontWeight: 800, color: '#8b5cf6' }}>
                        {formatScore(comparison.scores.experience)}
                      </div>
                      <div style={{ fontSize: '0.75rem', color: 'var(--text-muted)', fontWeight: 600, marginTop: '0.25rem' }}>Experience Match</div>
                    </div>
                    <div style={{ background: 'var(--surface)', padding: '1.15rem', borderRadius: 'var(--radius-md)', border: '1px solid var(--border-subtle)' }}>
                      <div style={{ fontSize: '1.6rem', fontWeight: 800, color: '#10b981' }}>
                        {formatScore(comparison.scores.education)}
                      </div>
                      <div style={{ fontSize: '0.75rem', color: 'var(--text-muted)', fontWeight: 600, marginTop: '0.25rem' }}>Education Match</div>
                    </div>
                  </div>
                </div>
              )}
            </div>
          ) : (
            <div className="card" style={{ padding: '3rem', textAlign: 'center', color: 'var(--text-muted)' }}>
              Select a job requisition above to generate side-by-side compatibility analysis.
            </div>
          )}
        </div>
      </main>
    </div>
  );
};

export default ResumeJobComparisonPage;
