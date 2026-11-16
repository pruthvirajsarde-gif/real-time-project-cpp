import React, { useState, useEffect } from 'react';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import { analyticsApi } from '../api/analyticsApi';
import { CheckIcon, TargetIcon, SparklesIcon } from '../components/Icons';

export const SkillGap = () => {
  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const fetchSkillGap = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await analyticsApi.getCandidateSkillsAnalytics();
      if (res?.data) {
        setData(res.data);
      }
    } catch (err) {
      setError(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchSkillGap();
  }, []);

  return (
    <div className="portal-layout">
      <Sidebar type="candidate" />

      <main className="portal-main">
        <div className="portal-content">
          <div className="page-header">
            <div>
              <h1 className="page-title">Skill Gap Analytics</h1>
              <p className="page-subtitle">
                Compare your current technical competencies against aggregated market demand across all job requisitions
              </p>
            </div>
            <button onClick={fetchSkillGap} className="btn btn-outline btn-sm">
              Refresh Analysis
            </button>
          </div>

          <ErrorMessage error={error} onRetry={fetchSkillGap} />

          {loading ? (
            <Loading message="Calculating skill coverage and demand priorities..." />
          ) : (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '1.75rem' }}>
              {/* Coverage Banner */}
              <div className="card" style={{
                background: 'linear-gradient(135deg, rgba(99, 102, 241, 0.14) 0%, rgba(6, 182, 212, 0.09) 100%)',
                borderColor: 'rgba(99, 102, 241, 0.3)',
                display: 'flex',
                justifyContent: 'space-between',
                alignItems: 'center',
                flexWrap: 'wrap',
                gap: '1.25rem',
                padding: '1.75rem'
              }}>
                <div>
                  <span style={{ fontSize: '0.8rem', textTransform: 'uppercase', letterSpacing: '0.05em', color: 'var(--text-muted)', fontWeight: 700 }}>
                    Catalog-Wide Skill Coverage
                  </span>
                  <h2 style={{ fontSize: '2.5rem', fontWeight: 800, margin: '0.25rem 0', color: '#fff', letterSpacing: '-0.02em' }}>
                    {data?.catalogSkillCoverage ?? 0}%
                  </h2>
                  <p style={{ margin: 0, color: 'var(--text-muted)', fontSize: '0.875rem' }}>
                    Proportion of available job skill requirements satisfied by your active resume
                  </p>
                </div>
                <div style={{
                  padding: '1rem 1.75rem',
                  borderRadius: 'var(--radius-md)',
                  background: 'rgba(0, 0, 0, 0.35)',
                  border: '1px solid var(--border-subtle)',
                  textAlign: 'center',
                }}>
                  <div style={{ fontSize: '1.85rem', fontWeight: 800, color: 'var(--primary)', lineHeight: 1.1 }}>
                    {data?.candidateSkills?.length ?? 0}
                  </div>
                  <div style={{ fontSize: '0.775rem', color: 'var(--text-muted)', fontWeight: 600, marginTop: '0.25rem' }}>
                    Skills in Your Resume
                  </div>
                </div>
              </div>

              {/* Skills You Have */}
              <div className="card" style={{ padding: '1.75rem' }}>
                <h3 style={{ fontSize: '1.15rem', fontWeight: 700, marginBottom: '0.4rem', display: 'flex', alignItems: 'center', gap: '0.65rem' }}>
                  <CheckIcon size={20} color="#10b981" />
                  <span>Skills You Have</span>
                </h3>
                <p style={{ color: 'var(--text-muted)', fontSize: '0.85rem', marginBottom: '1.15rem' }}>
                  Technical proficiencies parsed from your resume that directly align with open requisitions.
                </p>

                {(!data?.candidateSkills || data.candidateSkills.length === 0) ? (
                  <div style={{ padding: '1.5rem', textAlign: 'center', color: 'var(--text-muted)' }}>
                    No skills identified in your resume. Please update your resume to populate competencies.
                  </div>
                ) : (
                  <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.6rem' }}>
                    {data.candidateSkills.map((skill, idx) => (
                      <span
                        key={idx}
                        className="badge badge-success"
                        style={{ fontSize: '0.825rem', padding: '0.4rem 0.85rem', fontWeight: 600 }}
                      >
                        {skill}
                      </span>
                    ))}
                  </div>
                )}

                {data?.relatedSkills && data.relatedSkills.length > 0 && (
                  <div style={{ marginTop: '1.5rem', paddingTop: '1.25rem', borderTop: '1px solid var(--border-subtle)' }}>
                    <span style={{ fontSize: '0.75rem', fontWeight: 700, color: 'var(--text-muted)', textTransform: 'uppercase', letterSpacing: '0.04em' }}>
                      Semantic & Related Competency Matches
                    </span>
                    <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.5rem', marginTop: '0.65rem' }}>
                      {data.relatedSkills.map((rel, idx) => (
                        <span
                          key={idx}
                          style={{
                            background: 'rgba(6, 182, 212, 0.12)',
                            color: '#67e8f9',
                            border: '1px solid rgba(6, 182, 212, 0.25)',
                            borderRadius: 'var(--radius-sm)',
                            padding: '0.3rem 0.7rem',
                            fontSize: '0.8rem',
                            fontWeight: 500,
                          }}
                        >
                          {rel}
                        </span>
                      ))}
                    </div>
                  </div>
                )}
              </div>

              {/* Skills You Need (Priority Breakdown) */}
              <div className="card" style={{ padding: '1.75rem' }}>
                <h3 style={{ fontSize: '1.15rem', fontWeight: 700, marginBottom: '0.4rem', display: 'flex', alignItems: 'center', gap: '0.65rem' }}>
                  <TargetIcon size={20} color="#f59e0b" />
                  <span>Priority Missing Required Skills</span>
                </h3>
                <p style={{ color: 'var(--text-muted)', fontSize: '0.85rem', marginBottom: '1.35rem' }}>
                  Critical competencies frequently requested by hiring teams that are currently absent from your profile.
                </p>

                {(!data?.missingRequired || data.missingRequired.length === 0) ? (
                  <div style={{ padding: '1.5rem', textAlign: 'center', color: 'var(--text-muted)' }}>
                    Excellent alignment! No high-priority missing required skills detected in current jobs.
                  </div>
                ) : (
                  <div style={{ display: 'flex', flexDirection: 'column', gap: '1rem' }}>
                    {data.missingRequired.map((item, idx) => {
                      const priorityColor =
                        item.priority === 'HIGH' ? '#ef4444' : item.priority === 'MEDIUM' ? '#f59e0b' : '#6366f1';

                      return (
                        <div key={idx} style={{
                          background: 'var(--surface)',
                          border: '1px solid var(--border-card)',
                          borderRadius: 'var(--radius-md)',
                          padding: '1.15rem',
                        }}>
                          <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '0.5rem' }}>
                            <div style={{ display: 'flex', alignItems: 'center', gap: '0.6rem' }}>
                              <span style={{ fontWeight: 700, fontSize: '0.95rem' }}>{item.skill}</span>
                              <span style={{
                                fontSize: '0.65rem',
                                fontWeight: 800,
                                padding: '0.15rem 0.5rem',
                                borderRadius: 'var(--radius-sm)',
                                backgroundColor: `${priorityColor}22`,
                                color: priorityColor,
                                border: `1px solid ${priorityColor}44`,
                                letterSpacing: '0.04em'
                              }}>
                                {item.priority} PRIORITY
                              </span>
                            </div>
                            <span style={{ fontSize: '0.825rem', fontWeight: 600, color: 'var(--text-muted)' }}>
                              Required in {item.jobCount} {item.jobCount === 1 ? 'job' : 'jobs'} ({item.demandPercentage}%)
                            </span>
                          </div>

                          <div style={{
                            width: '100%',
                            height: '7px',
                            backgroundColor: 'rgba(255, 255, 255, 0.08)',
                            borderRadius: 'var(--radius-full)',
                            overflow: 'hidden',
                          }}>
                            <div style={{
                              width: `${Math.min(item.demandPercentage, 100)}%`,
                              height: '100%',
                              backgroundColor: priorityColor,
                              borderRadius: 'var(--radius-full)',
                            }} />
                          </div>
                        </div>
                      );
                    })}
                  </div>
                )}
              </div>

              {/* Missing Preferred Skills */}
              {data?.missingPreferred && data.missingPreferred.length > 0 && (
                <div className="card" style={{ padding: '1.75rem' }}>
                  <h3 style={{ fontSize: '1.15rem', fontWeight: 700, marginBottom: '0.4rem', display: 'flex', alignItems: 'center', gap: '0.65rem' }}>
                    <SparklesIcon size={20} color="#38bdf8" />
                    <span>Nice-to-Have Preferred Skills</span>
                  </h3>
                  <p style={{ color: 'var(--text-muted)', fontSize: '0.85rem', marginBottom: '1.15rem' }}>
                    Secondary skills that increase competitiveness when present on your profile.
                  </p>
                  <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.6rem' }}>
                    {data.missingPreferred.map((item, idx) => (
                      <span
                        key={idx}
                        style={{
                          background: 'rgba(255, 255, 255, 0.04)',
                          border: '1px solid var(--border-card)',
                          borderRadius: 'var(--radius-md)',
                          padding: '0.45rem 0.85rem',
                          fontSize: '0.85rem',
                          display: 'inline-flex',
                          alignItems: 'center',
                          gap: '0.45rem',
                        }}
                      >
                        <span style={{ fontWeight: 600 }}>{item.skill}</span>
                        <span style={{ fontSize: '0.75rem', color: 'var(--text-muted)' }}>({item.jobCount} jobs)</span>
                      </span>
                    ))}
                  </div>
                </div>
              )}
            </div>
          )}
        </div>
      </main>
    </div>
  );
};

export default SkillGap;
