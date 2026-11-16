import React, { useState, useEffect } from 'react';
import { useParams, Link, useNavigate } from 'react-router-dom';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import ScoreCard from '../components/ScoreCard';
import ProgressBar from '../components/ProgressBar';
import SkillBadge from '../components/SkillBadge';
import { matchingApi } from '../api/matchingApi';
import { applicationApi } from '../api/applicationApi';
import { savedJobsApi } from '../api/savedJobsApi';
import {
  BookmarkIcon,
  XIcon,
  LayersIcon,
  SearchIcon,
  CheckIcon,
  ChevronRightIcon,
  TargetIcon,
  LightbulbIcon
} from '../components/Icons';

export const MatchAnalysis = () => {
  const { id } = useParams();
  const navigate = useNavigate();

  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const [applySubmitting, setApplySubmitting] = useState(false);
  const [feedbackNotice, setFeedbackNotice] = useState(null);

  const fetchExplanation = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await matchingApi.getMatchExplanation(id);
      if (res && res.data) {
        setData(res.data);
      }
    } catch (err) {
      setError(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchExplanation();
  }, [id]);

  const handleApply = async () => {
    setApplySubmitting(true);
    try {
      await applicationApi.applyForJob(id);
      setFeedbackNotice({ type: 'success', msg: `Applied to ${id} successfully!` });
    } catch (err) {
      setFeedbackNotice({ type: 'danger', msg: err.message });
    } finally {
      setApplySubmitting(false);
    }
  };

  const handleSave = async () => {
    try {
      await savedJobsApi.saveJob(id);
      setFeedbackNotice({ type: 'success', msg: `Saved job ${id} to bookmarks.` });
    } catch (err) {
      setFeedbackNotice({ type: 'danger', msg: err.message });
    }
  };

  if (loading) {
    return (
      <div className="portal-layout">
        <Sidebar type="candidate" />
        <main className="portal-main">
          <Loading message="Generating deterministic match explanation and skill gap breakdown..." />
        </main>
      </div>
    );
  }

  const scores = data?.scores || {};
  const skills = data?.skills || {};
  const recs = data?.recommendations || [];
  const overall = scores.overall ?? 0;

  return (
    <div className="portal-layout">
      <Sidebar type="candidate" />

      <main className="portal-main">
        <div className="portal-content">
          <div style={{ marginBottom: '1.5rem', display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
            <button onClick={() => navigate(-1)} className="btn btn-sm btn-outline">
              ← Back
            </button>
            <div style={{ display: 'flex', gap: '0.75rem' }}>
              <button onClick={handleSave} className="btn btn-sm btn-secondary" style={{ display: 'inline-flex', alignItems: 'center', gap: '0.4rem' }}>
                <BookmarkIcon size={14} />
                <span>Bookmark Job</span>
              </button>
              <button onClick={handleApply} disabled={applySubmitting} className="btn btn-sm btn-primary">
                {applySubmitting ? 'Applying...' : 'Apply for this Role'}
              </button>
            </div>
          </div>

          <div className="page-header">
            <div>
              <span style={{ fontSize: '0.8rem', fontWeight: 700, color: 'var(--primary)', textTransform: 'uppercase' }}>
                Requisition #{id}
              </span>
              <h1 className="page-title">Explainable Match Breakdown</h1>
              <p className="page-subtitle">
                Mathematical multi-factor scoring showing exactly why your resume aligns with this requisition
              </p>
            </div>
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

          <ErrorMessage error={error} onRetry={fetchExplanation} />

          {!data ? (
            <div className="card" style={{ textAlign: 'center', padding: '3rem 1rem' }}>
              <p style={{ color: 'var(--text-muted)' }}>No match explanation found for job ID {id}.</p>
            </div>
          ) : (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '2rem' }}>
              {/* Overall Score Banner */}
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(280px, 1fr))', gap: '1.5rem' }}>
                <ScoreCard
                  score={overall}
                  label="Deterministic Overall Match"
                  size="lg"
                />

                <div className="card" style={{ display: 'flex', flexDirection: 'column', justifyContent: 'center' }}>
                  <h3 style={{ fontSize: '1.2rem', marginBottom: '1rem', display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                    <LayersIcon size={18} color="var(--primary)" />
                    <span>Multi-Factor Weight Distribution</span>
                  </h3>
                  <ProgressBar label="Technical Skill Alignment (60% Weight)" value={scores.skill ?? 0} />
                  <ProgressBar label="Experience Requirement (20% Weight)" value={scores.experience ?? 0} />
                  <ProgressBar label="Education Discipline (20% Weight)" value={scores.education ?? 0} />
                  {scores.text !== undefined && (
                    <ProgressBar label="NLP Semantic / Text Relevance" value={scores.text} customColor="var(--cyan)" />
                  )}
                </div>
              </div>

              {/* Skills Overlap & Gaps Matrix */}
              <div className="card">
                <h3 className="section-title" style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                  <SearchIcon size={18} color="var(--primary)" />
                  <span>Comprehensive Skill Alignment Matrix</span>
                </h3>
                <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(280px, 1fr))', gap: '1.5rem', marginTop: '1rem' }}>
                  {/* Exact Matches */}
                  <div style={{ background: 'rgba(16, 185, 129, 0.06)', border: '1px solid rgba(16, 185, 129, 0.25)', borderRadius: 'var(--radius-md)', padding: '1.25rem' }}>
                    <div style={{ fontWeight: 700, color: '#6ee7b7', marginBottom: '0.75rem', display: 'flex', alignItems: 'center', gap: '0.4rem' }}>
                      <CheckIcon size={14} color="#10b981" />
                      <span>Exact Matched Skills ({skills.exact?.length || 0})</span>
                    </div>
                    {(!skills.exact || skills.exact.length === 0) ? (
                      <p style={{ color: 'var(--text-dim)', fontSize: '0.85rem' }}>No exact required skills identified in your resume.</p>
                    ) : (
                      <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.35rem' }}>
                        {skills.exact.map((s, idx) => (
                          <SkillBadge key={idx} name={s} type="exact" />
                        ))}
                      </div>
                    )}
                  </div>

                  {/* Missing Required */}
                  <div style={{ background: 'rgba(239, 68, 68, 0.06)', border: '1px solid rgba(239, 68, 68, 0.25)', borderRadius: 'var(--radius-md)', padding: '1.25rem' }}>
                    <div style={{ fontWeight: 700, color: '#fca5a5', marginBottom: '0.75rem', display: 'flex', alignItems: 'center', gap: '0.4rem' }}>
                      <XIcon size={14} color="#ef4444" />
                      <span>Missing Required Skills ({skills.missingRequired?.length || 0})</span>
                    </div>
                    {(!skills.missingRequired || skills.missingRequired.length === 0) ? (
                      <p style={{ color: 'var(--text-dim)', fontSize: '0.85rem' }}>All mandatory job requirements are met!</p>
                    ) : (
                      <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.35rem' }}>
                        {skills.missingRequired.map((s, idx) => (
                          <SkillBadge key={idx} name={s} type="missing-required" />
                        ))}
                      </div>
                    )}
                  </div>

                  {/* Missing Preferred */}
                  <div style={{ background: 'rgba(245, 158, 11, 0.06)', border: '1px solid rgba(245, 158, 11, 0.25)', borderRadius: 'var(--radius-md)', padding: '1.25rem' }}>
                    <div style={{ fontWeight: 700, color: '#fcd34d', marginBottom: '0.75rem', display: 'flex', alignItems: 'center', gap: '0.4rem' }}>
                      <ChevronRightIcon size={14} color="#f59e0b" />
                      <span>Missing Preferred Skills ({skills.missingPreferred?.length || 0})</span>
                    </div>
                    {(!skills.missingPreferred || skills.missingPreferred.length === 0) ? (
                      <p style={{ color: 'var(--text-dim)', fontSize: '0.85rem' }}>No missing preferred skills.</p>
                    ) : (
                      <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.35rem' }}>
                        {skills.missingPreferred.map((s, idx) => (
                          <SkillBadge key={idx} name={s} type="missing-preferred" />
                        ))}
                      </div>
                    )}
                  </div>
                </div>
              </div>

              {/* Actionable Advice for this Job */}
              <div className="card">
                <h3 className="section-title" style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                  <TargetIcon size={18} color="var(--primary)" />
                  <span>Tailored Recommendations for this Requisition</span>
                </h3>
                {recs.length === 0 ? (
                  <p style={{ color: 'var(--text-muted)', fontSize: '0.9rem' }}>
                    Your candidate profile aligns strongly with this position's expectations!
                  </p>
                ) : (
                  <div style={{ display: 'flex', flexDirection: 'column', gap: '0.75rem' }}>
                    {recs.map((r, idx) => (
                      <div
                        key={idx}
                        style={{
                          display: 'flex',
                          alignItems: 'flex-start',
                          gap: '0.75rem',
                          background: 'rgba(15, 23, 42, 0.6)',
                          border: '1px solid var(--border-subtle)',
                          borderRadius: 'var(--radius-md)',
                          padding: '0.85rem 1rem',
                          fontSize: '0.9rem',
                        }}
                      >
                        <LightbulbIcon size={16} color="var(--primary)" />
                        <span>{r}</span>
                      </div>
                    ))}
                  </div>
                )}
              </div>
            </div>
          )}
        </div>

      </main>
    </div>
  );
};

export default MatchAnalysis;
