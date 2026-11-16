import React, { useState, useEffect, useRef } from 'react';
import { Link } from 'react-router-dom';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import SkillBadge from '../components/SkillBadge';
import { resumeApi } from '../api/resumeApi';
import { formatScore } from '../utils/formatters';
import {
  BarChartIcon,
  ZapIcon,
  CheckIcon,
  XIcon,
  InfoIcon,
  MailIcon,
  MapPinIcon,
  CodeIcon,
  LayersIcon,
  BriefcaseIcon,
  SparklesIcon,
  ShieldIcon,
  UploadCloudIcon,
  StarIcon,
  ArrowRightIcon,
} from '../components/Icons';

export const Resume = () => {
  const [resume, setResume] = useState(null);
  const [analysis, setAnalysis] = useState(null);
  const [strengths, setStrengths] = useState(null);
  const [loading, setLoading] = useState(true);
  const [uploading, setUploading] = useState(false);
  const [uploadSuccess, setUploadSuccess] = useState(null);
  const [error, setError] = useState(null);
  const [importPath, setImportPath] = useState('');
  const [isDragOver, setIsDragOver] = useState(false);

  const fileInputRef = useRef(null);

  const fetchResumeData = async () => {
    setLoading(true);
    setError(null);
    try {
      const [resRes, anaRes, strRes] = await Promise.allSettled([
        resumeApi.getResume(),
        resumeApi.getAnalysis(),
        resumeApi.getStrengths(),
      ]);

      if (resRes.status === 'fulfilled' && resRes.value?.data) {
        setResume(resRes.value.data);
      } else {
        setResume(null);
      }

      if (anaRes.status === 'fulfilled' && anaRes.value?.data) {
        setAnalysis(anaRes.value.data);
      }
      if (strRes.status === 'fulfilled' && strRes.value?.data) {
        setStrengths(strRes.value.data);
      }
    } catch (err) {
      setError(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchResumeData();
  }, []);

  const handleFileUpload = async (file) => {
    if (!file) return;

    const ext = file.name.toLowerCase().slice(file.name.lastIndexOf('.'));
    if (ext !== '.txt' && ext !== '.pdf') {
      setError('Only .pdf and .txt document formats are supported.');
      return;
    }

    if (file.size > 10 * 1024 * 1024) {
      setError('File size exceeds the 10 MB maximum limit.');
      return;
    }

    setError(null);
    setUploading(true);
    setUploadSuccess(null);

    try {
      const res = await resumeApi.uploadResume(file);
      setUploadSuccess(res?.message || 'Resume uploaded and parsed successfully!');
      await fetchResumeData();
    } catch (err) {
      setError(err);
    } finally {
      setUploading(false);
    }
  };

  const handleDrop = (e) => {
    e.preventDefault();
    setIsDragOver(false);
    if (e.dataTransfer.files && e.dataTransfer.files.length > 0) {
      handleFileUpload(e.dataTransfer.files[0]);
    }
  };

  const handleLocalImport = async (e) => {
    e.preventDefault();
    if (!importPath.trim()) return;

    setError(null);
    setUploading(true);
    setUploadSuccess(null);

    try {
      const res = await resumeApi.importResume(importPath.trim());
      setUploadSuccess(res?.message || 'Local resume imported successfully!');
      setImportPath('');
      await fetchResumeData();
    } catch (err) {
      setError(err);
    } finally {
      setUploading(false);
    }
  };

  return (
    <div className="portal-layout">
      <Sidebar type="candidate" />

      <main className="portal-main">
        <div className="portal-content">
          <div className="page-header">
            <div>
              <h1 className="page-title">Resume Management</h1>
              <p className="page-subtitle">
                Upload or inspect your active resume document and structured profile sections
              </p>
            </div>
            <div style={{ display: 'flex', gap: '0.6rem', flexWrap: 'wrap' }}>
              <Link to="/candidate/realtime-analyzer" className="btn btn-secondary" style={{ display: 'inline-flex', alignItems: 'center', gap: '0.4rem', border: '1px solid #10b981', color: '#10b981' }}>
                <ZapIcon size={15} />
                <span>⚡ Real-Time Scanner</span>
              </Link>
              {resume && (
                <Link to="/candidate/resume-analysis" className="btn btn-primary" style={{ display: 'inline-flex', alignItems: 'center', gap: '0.4rem' }}>
                  <BarChartIcon size={15} />
                  <span>View Quality Analysis</span>
                </Link>
              )}
            </div>
          </div>

          <ErrorMessage error={error} onRetry={() => setError(null)} />

          {uploadSuccess && (
            <div style={{
              background: 'rgba(16, 185, 129, 0.12)',
              border: '1px solid rgba(16, 185, 129, 0.3)',
              borderRadius: 'var(--radius-md)',
              padding: '1rem 1.25rem',
              color: '#6ee7b7',
              marginBottom: '1.5rem',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'space-between'
            }}>
              <span style={{ display: 'flex', alignItems: 'center', gap: '0.4rem' }}>
                <CheckIcon size={16} color="#10b981" />
                <span>{uploadSuccess}</span>
              </span>
              <button
                onClick={() => setUploadSuccess(null)}
                style={{ background: 'transparent', border: 'none', color: '#6ee7b7', cursor: 'pointer', display: 'flex', alignItems: 'center' }}
              >
                <XIcon size={14} />
              </button>
            </div>
          )}


          {/* Upload Area */}
          <div
            onDragOver={(e) => { e.preventDefault(); setIsDragOver(true); }}
            onDragLeave={() => setIsDragOver(false)}
            onDrop={handleDrop}
            style={{
              border: `2px dashed ${isDragOver ? 'var(--primary)' : 'var(--border-card)'}`,
              borderRadius: 'var(--radius-lg)',
              padding: '2.5rem 1.5rem',
              textAlign: 'center',
              backgroundColor: isDragOver ? 'rgba(99, 102, 241, 0.08)' : 'rgba(15, 23, 42, 0.4)',
              transition: 'all var(--transition-fast)',
              marginBottom: '2rem',
              cursor: 'pointer',
            }}
            onClick={() => fileInputRef.current?.click()}
          >
            <input
              type="file"
              ref={fileInputRef}
              style={{ display: 'none' }}
              accept=".pdf,.txt"
              onChange={(e) => {
                if (e.target.files && e.target.files.length > 0) {
                  handleFileUpload(e.target.files[0]);
                }
              }}
            />

            <div style={{ marginBottom: '1.25rem', display: 'flex', justifyContent: 'center' }}>
              {uploading ? (
                <div style={{
                  width: '64px',
                  height: '64px',
                  borderRadius: '50%',
                  backgroundColor: 'rgba(99, 102, 241, 0.12)',
                  border: '2px solid rgba(99, 102, 241, 0.3)',
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center'
                }}>
                  <div className="spinner" style={{ width: '28px', height: '28px', border: '3px solid rgba(99, 102, 241, 0.3)', borderTopColor: 'var(--primary)', borderRadius: '50%', animation: 'spin 0.8s linear infinite' }} />
                </div>
              ) : (
                <div style={{
                  width: '68px',
                  height: '68px',
                  borderRadius: '50%',
                  backgroundColor: isDragOver ? 'rgba(99, 102, 241, 0.2)' : 'rgba(99, 102, 241, 0.12)',
                  border: `2px solid ${isDragOver ? 'var(--primary)' : 'rgba(99, 102, 241, 0.25)'}`,
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                  color: 'var(--primary)',
                  boxShadow: '0 4px 20px -2px rgba(99, 102, 241, 0.25)',
                  transition: 'all var(--transition-fast)'
                }}>
                  <UploadCloudIcon size={34} />
                </div>
              )}
            </div>

            <h3 style={{ fontSize: '1.25rem', fontWeight: 700, marginBottom: '0.5rem' }}>
              {uploading ? 'Decompressing & Deeply Analyzing Resume...' : 'Drag & Drop Resume Here or Click to Browse'}
            </h3>
            <p style={{ color: 'var(--text-muted)', fontSize: '0.9rem', maxWidth: '520px', margin: '0 auto 1.25rem', lineHeight: 1.5 }}>
              Universal document ingestion: <strong>.PDF</strong> (including modern compressed streams &amp; Identity-H font CMaps) or <strong>.TXT</strong> up to 10 MB.
            </p>

            <button
              type="button"
              className="btn btn-secondary"
              disabled={uploading}
              onClick={(e) => {
                e.stopPropagation();
                fileInputRef.current?.click();
              }}
            >
              {uploading ? 'Analyzing Structure...' : 'Choose Resume File'}
            </button>
          </div>

          {/* Demonstration Server Import Accordion */}
          <details style={{
            background: 'var(--surface-card)',
            border: '1px solid var(--border-subtle)',
            borderRadius: 'var(--radius-md)',
            padding: '0.85rem 1.25rem',
            marginBottom: '2.5rem',
          }}>
            <summary style={{ cursor: 'pointer', fontWeight: 600, color: 'var(--text-muted)', fontSize: '0.9rem', display: 'flex', alignItems: 'center', gap: '0.4rem' }}>
              <InfoIcon size={15} color="var(--primary)" />
              <span>Offline Academic Demonstration: Load Server-Side Sample Resume</span>
            </summary>
            <form onSubmit={handleLocalImport} style={{ marginTop: '1rem', display: 'flex', gap: '0.75rem', flexWrap: 'wrap' }}>
              <input
                type="text"
                className="form-input"
                style={{ flex: 1, minWidth: '220px' }}
                placeholder="e.g. data/sample_resume.pdf or data/sample_resume.txt"
                value={importPath}
                onChange={(e) => setImportPath(e.target.value)}
              />
              <button type="submit" className="btn btn-outline" disabled={uploading || !importPath.trim()}>
                Import Server Path
              </button>
            </form>
          </details>

          {/* Parsed Sections Display */}
          {loading ? (
            <Loading message="Fetching active resume profile..." />
          ) : !resume ? (
            <div className="card" style={{ textAlign: 'center', padding: '3rem 1.5rem' }}>
              <p style={{ color: 'var(--text-muted)', fontSize: '1rem' }}>
                No active resume found for this account. Upload a PDF or TXT resume to begin.
              </p>
            </div>
          ) : (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '1.75rem' }}>
              {/* Header Card */}
              <div className="card" style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', flexWrap: 'wrap', gap: '1rem' }}>
                <div>
                  <span style={{ fontSize: '0.8rem', fontWeight: 700, color: 'var(--primary)', textTransform: 'uppercase' }}>
                    Active Candidate Profile
                  </span>
                  <h2 style={{ fontSize: '1.75rem', marginTop: '0.2rem' }}>{resume.name}</h2>
                  <div style={{ display: 'flex', flexWrap: 'wrap', gap: '1rem', color: 'var(--text-muted)', fontSize: '0.9rem', marginTop: '0.25rem' }}>
                    {resume.email && (
                      <span style={{ display: 'inline-flex', alignItems: 'center', gap: '0.35rem' }}>
                        <MailIcon size={14} /> {resume.email}
                      </span>
                    )}
                    {resume.phone && (
                      <span style={{ display: 'inline-flex', alignItems: 'center', gap: '0.35rem' }}>
                        {resume.phone}
                      </span>
                    )}
                    {resume.location && (
                      <span style={{ display: 'inline-flex', alignItems: 'center', gap: '0.35rem' }}>
                        <MapPinIcon size={14} /> {resume.location}
                      </span>
                    )}
                  </div>
                </div>

                {analysis && (
                  <div style={{ textAlign: 'right' }}>
                    <div style={{ fontSize: '0.8rem', color: 'var(--text-muted)', textTransform: 'uppercase', fontWeight: 600 }}>
                      Quality Score
                    </div>
                    <div style={{ fontSize: '1.75rem', fontWeight: 800, color: '#6ee7b7' }}>
                      {formatScore(analysis.overallScore)}
                    </div>
                  </div>
                )}
              </div>

              {/* Deep Analysis & Quality Assessment Card */}
              {analysis && (
                <div className="card" style={{
                  background: 'linear-gradient(135deg, rgba(30, 41, 59, 0.7) 0%, rgba(15, 23, 42, 0.85) 100%)',
                  border: '1px solid rgba(99, 102, 241, 0.25)',
                  boxShadow: '0 8px 32px -4px rgba(0, 0, 0, 0.3)'
                }}>
                  <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', flexWrap: 'wrap', gap: '1rem', marginBottom: '1.25rem' }}>
                    <div>
                      <span style={{
                        display: 'inline-flex',
                        alignItems: 'center',
                        gap: '0.35rem',
                        fontSize: '0.75rem',
                        fontWeight: 700,
                        textTransform: 'uppercase',
                        letterSpacing: '0.05em',
                        color: 'var(--primary)',
                        background: 'rgba(99, 102, 241, 0.12)',
                        padding: '0.2rem 0.6rem',
                        borderRadius: 'var(--radius-sm)',
                        marginBottom: '0.4rem'
                      }}>
                        <SparklesIcon size={13} color="var(--primary)" />
                        <span>Automated Deep Resume Analytics</span>
                      </span>
                      <h3 style={{ fontSize: '1.25rem', margin: 0, fontWeight: 700 }}>
                        Resume Readiness &amp; Structural Evaluation
                      </h3>
                    </div>

                    <div style={{ display: 'flex', alignItems: 'center', gap: '1rem' }}>
                      <div style={{
                        display: 'flex',
                        alignItems: 'center',
                        gap: '0.75rem',
                        background: 'rgba(16, 185, 129, 0.1)',
                        border: '1px solid rgba(16, 185, 129, 0.25)',
                        padding: '0.45rem 1rem',
                        borderRadius: 'var(--radius-md)'
                      }}>
                        <StarIcon size={18} color="#10b981" />
                        <div>
                          <div style={{ fontSize: '0.7rem', color: 'var(--text-muted)', textTransform: 'uppercase', fontWeight: 600 }}>
                            Quality Index
                          </div>
                          <div style={{ fontSize: '1.35rem', fontWeight: 800, color: '#6ee7b7', lineHeight: 1 }}>
                            {formatScore(analysis.overallScore)}
                          </div>
                        </div>
                      </div>

                      <Link to="/candidate/resume-analysis" className="btn btn-outline btn-sm" style={{ display: 'inline-flex', alignItems: 'center', gap: '0.35rem' }}>
                        <span>Detailed Report</span>
                        <ArrowRightIcon size={14} />
                      </Link>
                    </div>
                  </div>

                  {/* Factor Breakdown Mini-Grid */}
                  <div style={{
                    display: 'grid',
                    gridTemplateColumns: 'repeat(auto-fit, minmax(130px, 1fr))',
                    gap: '0.75rem',
                    marginBottom: '1.25rem'
                  }}>
                    {[
                      { label: 'Skills Depth', val: analysis.breakdown?.skillsFactor ?? 0, color: '#818cf8' },
                      { label: 'Experience Density', val: analysis.breakdown?.experienceFactor ?? 0, color: '#38bdf8' },
                      { label: 'Education Quality', val: analysis.breakdown?.educationFactor ?? 0, color: '#34d399' },
                      { label: 'Projects Evidence', val: analysis.breakdown?.projectsFactor ?? 0, color: '#fbbf24' },
                      { label: 'Certifications', val: analysis.breakdown?.certificationsFactor ?? 0, color: '#f472b6' }
                    ].map((item, i) => (
                      <div key={i} style={{
                        background: 'rgba(15, 23, 42, 0.55)',
                        border: '1px solid var(--border-subtle)',
                        borderRadius: 'var(--radius-md)',
                        padding: '0.65rem 0.85rem'
                      }}>
                        <div style={{ fontSize: '0.75rem', color: 'var(--text-muted)', marginBottom: '0.35rem' }}>{item.label}</div>
                        <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
                          <span style={{ fontWeight: 700, fontSize: '1.05rem', color: item.color }}>{item.val}%</span>
                          <div style={{ width: '45px', height: '5px', background: 'rgba(255,255,255,0.08)', borderRadius: '3px', overflow: 'hidden' }}>
                            <div style={{ width: `${Math.min(100, Math.max(0, item.val))}%`, height: '100%', background: item.color, borderRadius: '3px' }} />
                          </div>
                        </div>
                      </div>
                    ))}
                  </div>

                  {/* Highlights / Strengths badges */}
                  {strengths?.strengths && strengths.strengths.length > 0 && (
                    <div style={{ borderTop: '1px solid var(--border-subtle)', paddingTop: '0.9rem', display: 'flex', flexWrap: 'wrap', alignItems: 'center', gap: '0.5rem' }}>
                      <span style={{ fontSize: '0.8rem', color: 'var(--text-muted)', fontWeight: 600, display: 'inline-flex', alignItems: 'center', gap: '0.3rem' }}>
                        <CheckIcon size={14} color="#10b981" />
                        <span>Key Strengths:</span>
                      </span>
                      {strengths.strengths.slice(0, 4).map((s, idx) => (
                        <span key={idx} style={{
                          fontSize: '0.78rem',
                          background: 'rgba(16, 185, 129, 0.08)',
                          color: '#6ee7b7',
                          border: '1px solid rgba(16, 185, 129, 0.2)',
                          padding: '0.2rem 0.6rem',
                          borderRadius: 'var(--radius-sm)'
                        }}>
                          {s}
                        </span>
                      ))}
                    </div>
                  )}
                </div>
              )}

              {/* Skills Section */}
              <div className="card">
                <h3 className="section-title" style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                  <CodeIcon size={18} color="var(--primary)" />
                  <span>Technical Skills ({resume.skills?.length || 0})</span>
                </h3>
                {(!resume.skills || resume.skills.length === 0) ? (
                  <p style={{ color: 'var(--text-dim)', fontSize: '0.9rem' }}>No skills identified.</p>
                ) : (
                  <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.4rem' }}>
                    {resume.skills.map((s, idx) => (
                      <SkillBadge key={idx} name={typeof s === 'string' ? s : s.name} />
                    ))}
                  </div>
                )}
              </div>

              {/* Education Section */}
              <div className="card">
                <h3 className="section-title" style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                  <LayersIcon size={18} color="var(--primary)" />
                  <span>Education History</span>
                </h3>
                {(!resume.education || resume.education.length === 0) ? (
                  <p style={{ color: 'var(--text-dim)', fontSize: '0.9rem' }}>No education records identified.</p>
                ) : (
                  <div style={{ display: 'flex', flexDirection: 'column', gap: '1rem' }}>
                    {resume.education.map((edu, idx) => (
                      <div key={idx} style={{ borderBottom: idx < resume.education.length - 1 ? '1px solid var(--border-subtle)' : 'none', paddingBottom: '0.85rem' }}>
                        <div style={{ fontWeight: 700, fontSize: '1.05rem' }}>{edu.degree} in {edu.fieldOfStudy}</div>
                        <div style={{ color: 'var(--text-muted)', fontSize: '0.9rem' }}>{edu.institution}</div>
                        {(edu.startYear || edu.endYear) && (
                          <div style={{ color: 'var(--text-dim)', fontSize: '0.825rem', marginTop: '0.2rem' }}>
                            {edu.startYear} - {edu.endYear || 'Present'} {edu.score ? `• CGPA/Score: ${edu.score}` : ''}
                          </div>
                        )}
                      </div>
                    ))}
                  </div>
                )}
              </div>

              {/* Experience Section */}
              <div className="card">
                <h3 className="section-title" style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                  <BriefcaseIcon size={18} color="var(--primary)" />
                  <span>Work Experience</span>
                </h3>
                {(!resume.experience || resume.experience.length === 0) ? (
                  <p style={{ color: 'var(--text-dim)', fontSize: '0.9rem' }}>No work experience entries identified.</p>
                ) : (
                  <div style={{ display: 'flex', flexDirection: 'column', gap: '1rem' }}>
                    {resume.experience.map((exp, idx) => (
                      <div key={idx} style={{ borderBottom: idx < resume.experience.length - 1 ? '1px solid var(--border-subtle)' : 'none', paddingBottom: '0.85rem' }}>
                        <div style={{ fontWeight: 700, fontSize: '1.05rem' }}>{exp.position}</div>
                        <div style={{ color: 'var(--primary)', fontWeight: 600, fontSize: '0.9rem' }}>{exp.company} • {exp.duration}</div>
                        {exp.responsibilities && exp.responsibilities.length > 0 && (
                          <ul style={{ paddingLeft: '1.25rem', marginTop: '0.5rem', fontSize: '0.875rem', color: 'var(--text-muted)' }}>
                            {exp.responsibilities.map((r, rIdx) => <li key={rIdx}>{r}</li>)}
                          </ul>
                        )}
                      </div>
                    ))}
                  </div>
                )}
              </div>

              {/* Projects Section */}
              <div className="card">
                <h3 className="section-title" style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                  <SparklesIcon size={18} color="var(--primary)" />
                  <span>Projects & Initiatives</span>
                </h3>
                {(!resume.projects || resume.projects.length === 0) ? (
                  <p style={{ color: 'var(--text-dim)', fontSize: '0.9rem' }}>No project records identified.</p>
                ) : (
                  <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(280px, 1fr))', gap: '1rem' }}>
                    {resume.projects.map((proj, idx) => (
                      <div key={idx} style={{ background: 'rgba(15, 23, 42, 0.5)', padding: '1.15rem', borderRadius: 'var(--radius-md)', border: '1px solid var(--border-subtle)' }}>
                        <div style={{ fontWeight: 700, fontSize: '1.05rem', marginBottom: '0.25rem' }}>{proj.projectName || proj.name}</div>
                        <div style={{ fontSize: '0.85rem', color: 'var(--text-muted)', marginBottom: '0.5rem' }}>{proj.description}</div>
                        {proj.technologies && proj.technologies.length > 0 && (
                          <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.25rem', marginTop: '0.5rem' }}>
                            {proj.technologies.map((t, tIdx) => (
                              <span key={tIdx} style={{ fontSize: '0.75rem', background: 'rgba(99, 102, 241, 0.1)', color: '#a5b4fc', padding: '0.15rem 0.5rem', borderRadius: 'var(--radius-sm)' }}>
                                {t}
                              </span>
                            ))}
                          </div>
                        )}
                      </div>
                    ))}
                  </div>
                )}
              </div>

              {/* Certifications Section */}
              {resume.certifications && resume.certifications.length > 0 && (
                <div className="card">
                  <h3 className="section-title" style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                    <ShieldIcon size={18} color="var(--primary)" />
                    <span>Certifications & Accreditations</span>
                  </h3>
                  <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.75rem' }}>
                    {resume.certifications.map((c, idx) => (
                      <div key={idx} style={{ background: 'rgba(15, 23, 42, 0.6)', padding: '0.75rem 1.25rem', borderRadius: 'var(--radius-md)', border: '1px solid var(--border-subtle)' }}>
                        <div style={{ fontWeight: 700, fontSize: '0.95rem' }}>{c.name || c.certificationName}</div>
                        <div style={{ fontSize: '0.825rem', color: 'var(--text-muted)' }}>{c.issuer || c.issuingOrganization} {c.year ? `(${c.year})` : ''}</div>
                      </div>
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

export default Resume;
