import React, { useState, useEffect } from 'react';
import { useParams, Link, useNavigate } from 'react-router-dom';
import Navbar from '../components/Navbar';
import Sidebar from '../components/Sidebar';
import SkillBadge from '../components/SkillBadge';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import { jobsApi } from '../api/jobsApi';
import { savedJobsApi } from '../api/savedJobsApi';
import { applicationApi } from '../api/applicationApi';
import { analyticsApi } from '../api/analyticsApi';
import { useAuth } from '../context/AuthContext';
import { formatScore } from '../utils/formatters';
import {
  MapPinIcon,
  BriefcaseIcon,
  ClockIcon,
  BookmarkIcon,
  TargetIcon,
  SparklesIcon,
  CheckIcon,
  LayersIcon,
  XIcon,
  ScaleIcon,
} from '../components/Icons';

export const JobDetails = () => {
  const { id } = useParams();
  const navigate = useNavigate();
  const { isAuthenticated, role } = useAuth();
  const isCandidate = isAuthenticated && role === 'CANDIDATE';

  const [job, setJob] = useState(null);
  const [matchData, setMatchData] = useState(null);
  const [isSaved, setIsSaved] = useState(false);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  // Apply modal
  const [showApplyModal, setShowApplyModal] = useState(false);
  const [applySubmitting, setApplySubmitting] = useState(false);
  const [feedbackNotice, setFeedbackNotice] = useState(null);

  const fetchJob = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await jobsApi.getJobById(id);
      if (res && res.data) {
        setJob(res.data);
      }

      if (isCandidate) {
        try {
          const savedRes = await savedJobsApi.getSavedJobs();
          if (savedRes && Array.isArray(savedRes.data)) {
            setIsSaved(savedRes.data.some((s) => s.jobId === id));
          }
        } catch (e) {}

        try {
          const matchRes = await analyticsApi.getResumeJobComparison(id);
          if (matchRes?.data) {
            setMatchData(matchRes.data);
          }
        } catch (e) {}
      }
    } catch (err) {
      setError(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchJob();
  }, [id]);

  const handleSaveToggle = async () => {
    try {
      if (isSaved) {
        await savedJobsApi.removeSavedJob(id);
        setIsSaved(false);
        setFeedbackNotice({ type: 'info', msg: 'Job removed from saved bookmarks.' });
      } else {
        await savedJobsApi.saveJob(id);
        setIsSaved(true);
        setFeedbackNotice({ type: 'success', msg: 'Job bookmarked successfully!' });
      }
    } catch (err) {
      setFeedbackNotice({ type: 'danger', msg: err.message });
    }
  };

  const handleApply = async () => {
    setApplySubmitting(true);
    try {
      await applicationApi.applyForJob(id);
      setFeedbackNotice({
        type: 'success',
        msg: `Application submitted successfully for ${job.title}! Check the Applications tab for status.`
      });
      setShowApplyModal(false);
    } catch (err) {
      setFeedbackNotice({ type: 'danger', msg: err.message });
      setShowApplyModal(false);
    } finally {
      setApplySubmitting(false);
    }
  };

  const pageContent = (
    <div className="portal-content">
      <div style={{ marginBottom: '1.5rem' }}>
        <button
          onClick={() => navigate(-1)}
          className="btn btn-sm btn-outline"
        >
          ← Back to Jobs
        </button>
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

      <ErrorMessage error={error} onRetry={fetchJob} />

      {loading ? (
        <Loading message="Loading requisition details..." />
      ) : !job ? (
        <div className="card" style={{ textAlign: 'center', padding: '3rem 1rem' }}>
          <p style={{ color: 'var(--text-muted)' }}>Job requisition not found.</p>
        </div>
      ) : (
        <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(320px, 1fr))', gap: '2rem' }}>
          {/* Main Details Column */}
          <div style={{ gridColumn: 'span 2' }}>
            <div className="card" style={{ marginBottom: '2rem' }}>
              <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start', flexWrap: 'wrap', gap: '1rem', marginBottom: '1rem' }}>
                <div>
                  <span style={{ fontSize: '0.85rem', fontWeight: 700, color: 'var(--primary)', textTransform: 'uppercase' }}>
                    {job.company}
                  </span>
                  <h1 style={{ fontSize: '2rem', marginTop: '0.25rem' }}>{job.title}</h1>
                  <div style={{ display: 'flex', flexWrap: 'wrap', gap: '1rem', color: 'var(--text-muted)', fontSize: '0.9rem', marginTop: '0.5rem' }}>
                    <span style={{ display: 'inline-flex', alignItems: 'center', gap: '0.35rem' }}>
                      <MapPinIcon size={14} /> {job.location || 'Remote'}
                    </span>
                    <span style={{ display: 'inline-flex', alignItems: 'center', gap: '0.35rem' }}>
                      <BriefcaseIcon size={14} /> {job.employmentType || 'Full Time'}
                    </span>
                    <span style={{ display: 'inline-flex', alignItems: 'center', gap: '0.35rem' }}>
                      <ClockIcon size={14} /> {job.minimumExperience} Year{job.minimumExperience === 1 ? '' : 's'} Experience
                    </span>
                  </div>
                </div>

                <div style={{ display: 'flex', gap: '0.75rem', flexWrap: 'wrap' }}>
                  {isCandidate && (
                    <>
                      <button
                        onClick={handleSaveToggle}
                        className="btn btn-secondary"
                        style={{ display: 'inline-flex', alignItems: 'center', gap: '0.4rem', color: isSaved ? '#f59e0b' : 'inherit' }}
                      >
                        <BookmarkIcon size={15} filled={isSaved} />
                        <span>{isSaved ? 'Bookmarked' : 'Bookmark'}</span>
                      </button>
                      <Link to={`/candidate/matches/${job.jobId}`} className="btn btn-outline" style={{ display: 'inline-flex', alignItems: 'center', gap: '0.4rem' }}>
                        <TargetIcon size={15} />
                        <span>Match Breakdown</span>
                      </Link>
                      <button onClick={() => setShowApplyModal(true)} className="btn btn-primary">
                        Apply Now
                      </button>
                    </>
                  )}
                </div>
              </div>

              <div style={{ paddingTop: '1.5rem', borderTop: '1px solid var(--border-subtle)' }}>
                <h3 style={{ fontSize: '1.15rem', marginBottom: '0.75rem' }}>Role Description</h3>
                <p style={{ color: 'var(--text-main)', fontSize: '0.95rem', lineHeight: 1.7, whiteSpace: 'pre-line' }}>
                  {job.description || 'No detailed description provided.'}
                </p>
              </div>
            </div>

            {/* Why This Job Matches You Card (Phase 12) */}
            {isCandidate && matchData && (
              <div className="card" style={{
                marginBottom: '2rem',
                background: 'linear-gradient(135deg, rgba(99, 102, 241, 0.08) 0%, rgba(16, 185, 129, 0.05) 100%)',
                border: '1px solid rgba(99, 102, 241, 0.25)',
              }}>
                <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1rem', flexWrap: 'wrap', gap: '0.5rem' }}>
                  <h3 style={{ fontSize: '1.2rem', fontWeight: 800, margin: 0, display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                    <SparklesIcon size={18} color="var(--primary)" /> Why This Job Matches You
                  </h3>
                  <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                    <span className="badge badge-primary">{matchData.classification}</span>
                    <span style={{ fontSize: '1.25rem', fontWeight: 900, color: 'var(--primary)' }}>
                      {formatScore(matchData.overallScore)}
                    </span>
                  </div>
                </div>

                <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(240px, 1fr))', gap: '1rem', marginBottom: '1.25rem' }}>
                  {/* Strong Matches */}
                  <div style={{ background: 'var(--surface)', padding: '0.85rem', borderRadius: 'var(--radius-md)' }}>
                    <span style={{ fontSize: '0.75rem', fontWeight: 700, color: '#10b981', textTransform: 'uppercase', display: 'flex', alignItems: 'center', gap: '0.35rem', marginBottom: '0.4rem' }}>
                      <CheckIcon size={13} color="#10b981" /> Strong Matched Skills ({matchData.skills?.matched?.length || 0})
                    </span>
                    <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.35rem' }}>
                      {matchData.skills?.matched?.length > 0 ? (
                        matchData.skills.matched.map((s, idx) => (
                          <span key={idx} className="badge badge-success" style={{ fontSize: '0.75rem' }}>
                            {s}
                          </span>
                        ))
                      ) : (
                        <span style={{ fontSize: '0.8rem', color: 'var(--text-muted)' }}>None matched</span>
                      )}
                    </div>
                  </div>

                  {/* Related Matches */}
                  {matchData.skills?.related?.length > 0 && (
                    <div style={{ background: 'var(--surface)', padding: '0.85rem', borderRadius: 'var(--radius-md)' }}>
                      <span style={{ fontSize: '0.75rem', fontWeight: 700, color: '#06b6d4', textTransform: 'uppercase', display: 'flex', alignItems: 'center', gap: '0.35rem', marginBottom: '0.4rem' }}>
                        <LayersIcon size={13} color="#06b6d4" /> Related Competencies ({matchData.skills.related.length})
                      </span>
                      <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.35rem' }}>
                        {matchData.skills.related.map((r, idx) => (
                          <span key={idx} style={{
                            fontSize: '0.75rem',
                            background: 'rgba(6, 182, 212, 0.15)',
                            color: '#67e8f9',
                            padding: '0.2rem 0.4rem',
                            borderRadius: 'var(--radius-sm)',
                          }}>
                            {r}
                          </span>
                        ))}
                      </div>
                    </div>
                  )}

                  {/* Missing Skills */}
                  <div style={{ background: 'var(--surface)', padding: '0.85rem', borderRadius: 'var(--radius-md)' }}>
                    <span style={{ fontSize: '0.75rem', fontWeight: 700, color: '#ef4444', textTransform: 'uppercase', display: 'flex', alignItems: 'center', gap: '0.35rem', marginBottom: '0.4rem' }}>
                      <XIcon size={13} color="#ef4444" /> Missing Skills ({matchData.skills?.missingRequired?.length || 0})
                    </span>
                    <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.35rem' }}>
                      {matchData.skills?.missingRequired?.length > 0 ? (
                        matchData.skills.missingRequired.map((s, idx) => (
                          <span key={idx} className="badge badge-danger" style={{ fontSize: '0.75rem' }}>
                            {s}
                          </span>
                        ))
                      ) : (
                        <span style={{ fontSize: '0.8rem', color: '#10b981' }}>None missing!</span>
                      )}
                    </div>
                  </div>

                  {/* Experience Alignment */}
                  <div style={{ background: 'var(--surface)', padding: '0.85rem', borderRadius: 'var(--radius-md)' }}>
                    <span style={{ fontSize: '0.75rem', fontWeight: 700, color: '#8b5cf6', textTransform: 'uppercase', display: 'flex', alignItems: 'center', gap: '0.35rem', marginBottom: '0.4rem' }}>
                      <ClockIcon size={13} color="#8b5cf6" /> Experience Comparison
                    </span>
                    <div style={{ fontSize: '0.85rem', display: 'flex', flexDirection: 'column', gap: '0.2rem' }}>
                      <div><span style={{ color: 'var(--text-muted)' }}>Required:</span> {matchData.experience?.required}</div>
                      <div><span style={{ color: 'var(--text-muted)' }}>Candidate:</span> {matchData.experience?.candidate}</div>
                      <div style={{ marginTop: '0.25rem' }}>
                        <span className={`badge ${matchData.experience?.compatibility?.includes('Meets') ? 'badge-success' : 'badge-warning'}`} style={{ fontSize: '0.7rem' }}>
                          {matchData.experience?.compatibility}
                        </span>
                      </div>
                    </div>
                  </div>
                </div>

                <div style={{ display: 'flex', justifyContent: 'flex-end', paddingTop: '0.5rem', borderTop: '1px solid var(--border-subtle)' }}>
                  <Link to={`/candidate/compare?jobId=${job.jobId}`} className="btn btn-sm btn-outline" style={{ display: 'inline-flex', alignItems: 'center', gap: '0.4rem' }}>
                    <ScaleIcon size={14} />
                    <span>Open Full Side-by-Side Comparison →</span>
                  </Link>
                </div>
              </div>

            )}

            {/* Skills Card */}
            <div className="card" style={{ marginBottom: '2rem' }}>
              <h3 style={{ fontSize: '1.15rem', marginBottom: '1.25rem' }}>Technical Skill Requirements</h3>

              <div style={{ marginBottom: '1.5rem' }}>
                <div style={{ fontSize: '0.8rem', fontWeight: 700, color: 'var(--text-dim)', textTransform: 'uppercase', marginBottom: '0.5rem' }}>
                  Mandatory Required Skills (70% Weight)
                </div>
                <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.4rem' }}>
                  {(job.requiredSkills || []).map((s, idx) => (
                    <SkillBadge key={idx} name={s} />
                  ))}
                </div>
              </div>

              {job.preferredSkills && job.preferredSkills.length > 0 && (
                <div>
                  <div style={{ fontSize: '0.8rem', fontWeight: 700, color: 'var(--text-dim)', textTransform: 'uppercase', marginBottom: '0.5rem' }}>
                    Preferred Nice-to-Have Skills (30% Weight)
                  </div>
                  <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.4rem' }}>
                    {job.preferredSkills.map((s, idx) => (
                      <SkillBadge key={idx} name={s} type="category" />
                    ))}
                  </div>
                </div>
              )}
            </div>
          </div>

          {/* Side Overview Column */}
          <div>
            <div className="card">
              <h3 style={{ fontSize: '1.15rem', marginBottom: '1.25rem' }}>Position Summary</h3>
              <div style={{ display: 'flex', flexDirection: 'column', gap: '1rem', fontSize: '0.9rem' }}>
                <div>
                  <div style={{ color: 'var(--text-dim)', fontSize: '0.8rem', textTransform: 'uppercase' }}>Job ID</div>
                  <div style={{ fontWeight: 600, fontFamily: 'var(--font-mono)' }}>{job.jobId}</div>
                </div>
                <div>
                  <div style={{ color: 'var(--text-dim)', fontSize: '0.8rem', textTransform: 'uppercase' }}>Company</div>
                  <div style={{ fontWeight: 600 }}>{job.company}</div>
                </div>
                <div>
                  <div style={{ color: 'var(--text-dim)', fontSize: '0.8rem', textTransform: 'uppercase' }}>Location</div>
                  <div style={{ fontWeight: 600 }}>{job.location || 'Remote'}</div>
                </div>
                <div>
                  <div style={{ color: 'var(--text-dim)', fontSize: '0.8rem', textTransform: 'uppercase' }}>Employment Type</div>
                  <div style={{ fontWeight: 600 }}>{job.employmentType}</div>
                </div>
                <div>
                  <div style={{ color: 'var(--text-dim)', fontSize: '0.8rem', textTransform: 'uppercase' }}>Education Requirement</div>
                  <div style={{ fontWeight: 600 }}>{job.educationRequirement || 'Any Degree'}</div>
                </div>
                <div>
                  <div style={{ color: 'var(--text-dim)', fontSize: '0.8rem', textTransform: 'uppercase' }}>Minimum Experience</div>
                  <div style={{ fontWeight: 600 }}>{job.minimumExperience} Year(s)</div>
                </div>
              </div>

              {isCandidate && (
                <div style={{ marginTop: '1.75rem', paddingTop: '1.25rem', borderTop: '1px solid var(--border-subtle)' }}>
                  <button
                    onClick={() => setShowApplyModal(true)}
                    className="btn btn-primary"
                    style={{ width: '100%', marginBottom: '0.5rem' }}
                  >
                    Apply Now
                  </button>
                  <Link
                    to={`/candidate/matches/${job.jobId}`}
                    className="btn btn-outline"
                    style={{ width: '100%', justifyContent: 'center' }}
                  >
                    Inspect Match Score
                  </Link>
                </div>
              )}
            </div>
          </div>
        </div>
      )}

      {/* Apply Modal */}
      {showApplyModal && (
        <div className="modal-overlay">
          <div className="modal-content" style={{ maxWidth: '480px' }}>
            <h3 style={{ fontSize: '1.25rem', marginBottom: '0.75rem' }}>Apply for Position</h3>
            <p style={{ color: 'var(--text-muted)', fontSize: '0.925rem', marginBottom: '1.5rem', lineHeight: 1.5 }}>
              Are you sure you want to apply for <strong>{job?.title}</strong> at <strong>{job?.company}</strong>?
              Your profile and active resume will be submitted.
            </p>
            <div style={{ display: 'flex', gap: '0.75rem', justifyContent: 'flex-end' }}>
              <button
                className="btn btn-secondary"
                disabled={applySubmitting}
                onClick={() => setShowApplyModal(false)}
              >
                Cancel
              </button>
              <button
                className="btn btn-primary"
                disabled={applySubmitting}
                onClick={handleApply}
              >
                {applySubmitting ? 'Submitting...' : 'Confirm & Apply'}
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );

  if (isCandidate) {
    return (
      <div className="portal-layout">
        <Sidebar type="candidate" />
        <main className="portal-main">{pageContent}</main>
      </div>
    );
  }

  return (
    <div className="app-container">
      <Navbar />
      <main style={{ maxWidth: '1400px', width: '100%', margin: '0 auto', padding: '2rem 1.5rem' }}>
        {pageContent}
      </main>
    </div>
  );
};

export default JobDetails;
