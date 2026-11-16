import React, { useState, useEffect, useMemo, useRef } from 'react';
import { Link } from 'react-router-dom';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import LiveSyncBadge from '../components/LiveSyncBadge';
import { jobsApi } from '../api/jobsApi';
import { resumeApi } from '../api/resumeApi';
import {
  ZapIcon,
  CheckIcon,
  XIcon,
  StarIcon,
  FileTextIcon,
  TargetIcon,
  SparklesIcon,
  LayersIcon,
  BriefcaseIcon,
  ArrowRightIcon,
  RotateCcwIcon,
} from '../components/Icons';

// Comprehensive dictionary for real-time skill extraction
const KNOWN_SKILLS = [
  'c++', 'c', 'python', 'java', 'javascript', 'typescript', 'c#', 'golang', 'rust',
  'html', 'css', 'react', 'vue', 'angular', 'next.js', 'node.js', 'express',
  'spring boot', 'django', 'flask', 'fastapi', 'sql', 'mysql', 'postgresql', 'sqlite',
  'mongodb', 'redis', 'git', 'docker', 'kubernetes', 'linux', 'cmake', 'stl',
  'multithreading', 'concurrency', 'data structures', 'algorithms', 'rest apis',
  'graphql', 'microservices', 'aws', 'azure', 'gcp', 'ci/cd', 'jenkins', 'testing',
  'selenium', 'unit testing', 'kafka', 'rtos', 'embedded c', 'vite', 'redux', 'tailwind',
];

const ACTION_VERBS = [
  'developed', 'architected', 'optimized', 'engineered', 'implemented', 'designed',
  'built', 'led', 'spearheaded', 'orchestrated', 'refactored', 'accelerated',
  'reduced', 'increased', 'managed', 'created', 'deployed', 'automated', 'delivered',
];

const SAMPLE_TEMPLATES = {
  cpp: `[PROFILE]
Name: Alice Smith
Email: alice.smith@example.com
Phone: +91 98765 43210
Location: Pune, India

[SUMMARY]
High-performance C++ Systems Engineer with 3+ years of experience architecting low-latency microservices, concurrent pipelines, and robust backend systems using Modern C++17 and Linux.

[SKILLS]
C++, STL, Data Structures, Multithreading, Linux, CMake, Git, Docker, Python, SQL

[EXPERIENCE]
Senior Systems Engineer | TechCorp Solutions | Jan 2024 - Present
- Architected and optimized mission-critical embedded C++17 microservices, improving throughput by 42%.
- Engineered lock-free concurrency queues reducing inter-thread latency from 1.2ms to 280us.
- Led automated unit testing and CMake build pipelines, increasing code coverage to 94%.

Software Engineer | Alpha Systems | Jun 2022 - Dec 2023
- Developed scalable C++ network daemons handling over 50,000 requests per second.
- Refactored legacy monolithic services into modular Linux microservices with Docker.

[EDUCATION]
B.Tech in Computer Science | Pune University | 2018 - 2022 | CGPA: 8.9 / 10

[PROJECTS]
Distributed Key-Value Store | Modern C++17, RAII, POSIX Threads
- Implemented Raft consensus algorithm from scratch with zero memory leaks.
`,

  react: `[PROFILE]
Name: Rahul Sharma
Email: rahul.sharma@example.com
Phone: +91 91234 56789
Location: Pune, India

[SUMMARY]
Creative Frontend Engineer specializing in React 19, TypeScript, responsive UX design, and state management. Passionate about web performance, accessibility, and clean component architecture.

[SKILLS]
React, JavaScript, TypeScript, HTML, CSS, Vite, Redux, Git, SQL, Tailwind

[EXPERIENCE]
Frontend React Developer | InnovateAI Labs | Mar 2023 - Present
- Built responsive, accessible single-page web applications with React, TypeScript, and Vite.
- Optimized bundle sizes by 38% using code splitting and lazy component loading.
- Collaborated with UI/UX team to deliver 15+ design-system components tested with Vitest.

Junior Web Developer | PixelCraft Studio | Aug 2021 - Feb 2023
- Developed modern frontend interfaces with HTML, CSS, JavaScript, and React.
- Integrated RESTful APIs and implemented token-based authentication flows.

[EDUCATION]
B.Tech in Information Technology | Savitribai Phule Pune University | 2017 - 2021

[PROJECTS]
SmartHire Recruitment Dashboard | React, Vite, CSS Grid
- Designed real-time candidate and recruiter dashboards with interactive analytics.
`,

  fullstack: `[PROFILE]
Name: Priya Nair
Email: priya.nair@example.com
Phone: +91 94567 89012
Location: Pune, India

[SUMMARY]
Fullstack Engineer with 2+ years of experience across the entire web stack, building robust REST APIs with Spring Boot and reactive, elegant web frontends with React and TypeScript.

[SKILLS]
Java, Spring Boot, React, JavaScript, SQL, HTML, CSS, Git, Docker, REST APIs

[EXPERIENCE]
Fullstack Developer | CloudNext Technologies | Jul 2023 - Present
- Engineered scalable REST microservices with Java and Spring Boot connected to PostgreSQL.
- Developed client-facing analytics dashboards using React, CSS Grid, and Chart libraries.
- Automated CI/CD deployment pipelines using Docker and GitHub Actions.

[EDUCATION]
B.E. in Computer Engineering | Mumbai University | 2019 - 2023
`
};

export const RealTimeAnalyzer = () => {
  const [resumeText, setResumeText] = useState(SAMPLE_TEMPLATES.cpp);
  const [jobs, setJobs] = useState([]);
  const [selectedJobId, setSelectedJobId] = useState('');
  const [loadingJobs, setLoadingJobs] = useState(true);
  const [saving, setSaving] = useState(false);
  const [saveStatus, setSaveStatus] = useState(null);
  const [loadingProfileResume, setLoadingProfileResume] = useState(false);

  const textareaRef = useRef(null);

  // Load available jobs from backend
  useEffect(() => {
    const loadJobs = async () => {
      try {
        const res = await jobsApi.getJobs({ limit: 50 });
        const items = res?.data?.items || res?.data || [];
        setJobs(items);
        if (items.length > 0) {
          setSelectedJobId(items[0].jobId);
        }
      } catch (err) {
        console.error('Failed to load jobs for real-time analyzer', err);
      } finally {
        setLoadingJobs(false);
      }
    };
    loadJobs();
  }, []);

  // Try loading active resume on initial mount if available
  const loadExistingResume = async () => {
    setLoadingProfileResume(true);
    setSaveStatus(null);
    try {
      const res = await resumeApi.getResume();
      if (res?.data) {
        const d = res.data;
        let generated = `[PROFILE]\nName: ${d.name || ''}\nEmail: ${d.email || ''}\nPhone: ${d.phone || ''}\nLocation: ${d.location || ''}\n\n`;
        if (d.summary) generated += `[SUMMARY]\n${d.summary}\n\n`;
        if (Array.isArray(d.skills) && d.skills.length > 0) {
          generated += `[SKILLS]\n${d.skills.join(', ')}\n\n`;
        }
        if (Array.isArray(d.experience) && d.experience.length > 0) {
          generated += `[EXPERIENCE]\n`;
          d.experience.forEach(exp => {
            generated += `${exp.role || exp.title || 'Role'} | ${exp.company || 'Company'} | ${exp.duration || ''}\n`;
            if (exp.description) generated += `- ${exp.description}\n`;
          });
          generated += '\n';
        }
        if (Array.isArray(d.education) && d.education.length > 0) {
          generated += `[EDUCATION]\n`;
          d.education.forEach(edu => {
            generated += `${edu.degree || ''} in ${edu.field || ''} | ${edu.institution || ''} | ${edu.year || ''}\n`;
          });
          generated += '\n';
        }
        setResumeText(generated);
        setSaveStatus({ type: 'success', message: 'Loaded your active profile resume into editor!' });
      } else {
        setSaveStatus({ type: 'info', message: 'No existing resume found on profile. Using sample template.' });
      }
    } catch (e) {
      setSaveStatus({ type: 'error', message: 'Could not load existing resume. You can draft or edit text below.' });
    } finally {
      setLoadingProfileResume(false);
    }
  };

  const selectedJob = useMemo(() => {
    return jobs.find(j => j.jobId === selectedJobId) || jobs[0] || null;
  }, [jobs, selectedJobId]);

  // Real-Time Analysis Calculation
  const analysis = useMemo(() => {
    const textLower = resumeText.toLowerCase();

    // 1. Extract matched skills from text
    const extractedSkills = KNOWN_SKILLS.filter(skill => {
      const escaped = skill.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
      const regex = new RegExp(`(?:^|[\\s,;:.()\\[\\]/])${escaped}(?:$|[\\s,;:.()\\[\\]/])`, 'i');
      return regex.test(textLower);
    });

    // 2. Word & Char Counts
    const words = resumeText.trim().split(/\s+/).filter(Boolean);
    const wordCount = words.length;
    const charCount = resumeText.length;

    // 3. Action Verbs Detected
    const detectedVerbs = ACTION_VERBS.filter(verb => {
      const regex = new RegExp(`\\b${verb}\\b`, 'i');
      return regex.test(textLower);
    });

    // 4. Measurable Numbers/Impact metrics (% or numbers)
    const metricsMatches = resumeText.match(/\d+[\d,.]*(?:%|\+|k|M|\/10)?/g) || [];

    // 5. Section Presence Check
    const hasSummary = /\[summary\]/i.test(resumeText);
    const hasSkills = /\[skills\]/i.test(resumeText);
    const hasExperience = /\[experience\]/i.test(resumeText);
    const hasEducation = /\[education\]/i.test(resumeText);
    const hasProjects = /\[projects\]/i.test(resumeText);
    const sectionCount = [hasSummary, hasSkills, hasExperience, hasEducation, hasProjects].filter(Boolean).length;

    // 6. Job Matching against selected job
    let reqMatched = [];
    let reqMissing = [];
    let prefMatched = [];
    let prefMissing = [];
    let matchScore = 0;

    if (selectedJob) {
      const jobReq = selectedJob.requiredSkills || [];
      const jobPref = selectedJob.preferredSkills || [];

      jobReq.forEach(skill => {
        const escaped = skill.trim().toLowerCase().replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
        const regex = new RegExp(`(?:^|[\\s,;:.()\\[\\]/])${escaped}(?:$|[\\s,;:.()\\[\\]/])`, 'i');
        if (regex.test(textLower)) {
          reqMatched.push(skill);
        } else {
          reqMissing.push(skill);
        }
      });

      jobPref.forEach(skill => {
        const escaped = skill.trim().toLowerCase().replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
        const regex = new RegExp(`(?:^|[\\s,;:.()\\[\\]/])${escaped}(?:$|[\\s,;:.()\\[\\]/])`, 'i');
        if (regex.test(textLower)) {
          prefMatched.push(skill);
        } else {
          prefMissing.push(skill);
        }
      });

      const totalReq = jobReq.length || 1;
      const totalPref = jobPref.length || 0;

      if (totalPref > 0) {
        const reqPct = (reqMatched.length / totalReq) * 70;
        const prefPct = (prefMatched.length / totalPref) * 30;
        matchScore = Math.round(reqPct + prefPct);
      } else {
        matchScore = Math.round((reqMatched.length / totalReq) * 100);
      }
      matchScore = Math.min(100, Math.max(0, matchScore));
    }

    // 7. Overall ATS Health Score
    let atsHealth = 20; // baseline
    if (wordCount >= 150) atsHealth += 20;
    if (sectionCount >= 4) atsHealth += 20;
    if (detectedVerbs.length >= 3) atsHealth += 15;
    if (metricsMatches.length >= 2) atsHealth += 15;
    if (extractedSkills.length >= 5) atsHealth += 10;
    atsHealth = Math.min(100, atsHealth);

    return {
      extractedSkills,
      wordCount,
      charCount,
      detectedVerbs,
      metricsMatches,
      sectionCount,
      hasSummary,
      hasSkills,
      hasExperience,
      hasEducation,
      hasProjects,
      reqMatched,
      reqMissing,
      prefMatched,
      prefMissing,
      matchScore,
      atsHealth,
    };
  }, [resumeText, selectedJob]);

  // Quick insertion helper to add missing skill straight into the resume
  const handleInsertSkill = (skill) => {
    let updated = resumeText;
    const skillsSectionMatch = updated.match(/(\[SKILLS\][^\n]*\n)([\s\S]*?)(\n\s*\[|$)/i);
    if (skillsSectionMatch) {
      const header = skillsSectionMatch[1];
      const currentSkills = skillsSectionMatch[2].trim();
      const newSkillsText = currentSkills.length > 0 ? `${currentSkills}, ${skill}` : skill;
      updated = updated.replace(skillsSectionMatch[0], `${header}${newSkillsText}\n\n${skillsSectionMatch[3]}`);
    } else {
      updated += `\n[SKILLS]\n${skill}\n`;
    }
    setResumeText(updated);
  };

  // Save/Upload the real-time resume directly to backend profile
  const handleSaveToProfile = async () => {
    setSaving(true);
    setSaveStatus(null);
    try {
      const blob = new Blob([resumeText], { type: 'text/plain' });
      const file = new File([blob], 'realtime_resume.txt', { type: 'text/plain' });
      await resumeApi.uploadResume(file);
      setSaveStatus({
        type: 'success',
        message: 'Real-time resume successfully saved and synced to your SmartHire profile!',
      });
    } catch (err) {
      setSaveStatus({
        type: 'error',
        message: err.message || 'Failed to save resume to backend database.',
      });
    } finally {
      setSaving(false);
    }
  };

  const handleDownload = () => {
    const blob = new Blob([resumeText], { type: 'text/plain;charset=utf-8' });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.download = 'smart_resume.txt';
    link.click();
    URL.revokeObjectURL(url);
  };

  const matchClassification =
    analysis.matchScore >= 85
      ? { label: 'EXCELLENT MATCH', color: '#10b981' }
      : analysis.matchScore >= 70
      ? { label: 'STRONG MATCH', color: '#3b82f6' }
      : analysis.matchScore >= 50
      ? { label: 'MODERATE MATCH', color: '#f59e0b' }
      : { label: 'LOW MATCH', color: '#ef4444' };

  return (
    <div className="portal-layout">
      <Sidebar role="CANDIDATE" />

      <main className="portal-content">
        {/* Top Header */}
        <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1.5rem', flexWrap: 'wrap', gap: '1rem' }}>
          <div>
            <div style={{ display: 'flex', alignItems: 'center', gap: '0.65rem' }}>
              <h1 style={{ margin: 0, fontSize: '1.75rem', fontWeight: 800 }}>⚡ Real-Time Resume Scanner & Matcher</h1>
              <span className="badge badge-primary" style={{ animation: 'pulse-live 2s infinite' }}>LIVE</span>
            </div>
            <p style={{ color: 'var(--text-muted)', margin: '0.35rem 0 0 0', fontSize: '0.95rem' }}>
              Live interactive resume optimizer. Type or paste your resume text to see ATS score and job matches update on every keystroke.
            </p>
          </div>

          <div style={{ display: 'flex', alignItems: 'center', gap: '0.75rem' }}>
            <LiveSyncBadge label="Engine" />
            <button
              onClick={handleSaveToProfile}
              disabled={saving}
              className="btn btn-primary"
              style={{ gap: '0.5rem', fontWeight: 600 }}
            >
              {saving ? <RotateCcwIcon size={16} className="smart-icon-spin" /> : <SparklesIcon size={16} />}
              <span>{saving ? 'Syncing...' : 'Save to Profile'}</span>
            </button>
          </div>
        </div>

        {saveStatus && (
          <div
            style={{
              padding: '0.75rem 1.25rem',
              borderRadius: 'var(--radius-md)',
              marginBottom: '1.25rem',
              backgroundColor: saveStatus.type === 'success' ? 'rgba(16, 185, 129, 0.15)' : 'rgba(239, 68, 68, 0.15)',
              border: `1px solid ${saveStatus.type === 'success' ? '#10b981' : '#ef4444'}`,
              color: saveStatus.type === 'success' ? '#10b981' : '#ef4444',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'space-between',
            }}
          >
            <span>{saveStatus.message}</span>
            <button onClick={() => setSaveStatus(null)} style={{ background: 'none', border: 'none', color: 'inherit', cursor: 'pointer' }}>✕</button>
          </div>
        )}

        {/* Workspace Grid */}
        <div style={{ display: 'grid', gridTemplateColumns: '1.1fr 0.9fr', gap: '1.5rem', alignItems: 'start' }}>
          
          {/* LEFT: Real-Time Resume Editor */}
          <div className="card" style={{ padding: '1.25rem' }}>
            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '0.75rem', flexWrap: 'wrap', gap: '0.5rem' }}>
              <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                <FileTextIcon size={18} color="var(--primary)" />
                <h3 style={{ margin: 0, fontSize: '1.05rem', fontWeight: 700 }}>Resume Draft Editor</h3>
              </div>

              {/* Sample Switchers */}
              <div style={{ display: 'flex', gap: '0.4rem', flexWrap: 'wrap' }}>
                <button
                  type="button"
                  className="btn btn-sm btn-outline"
                  onClick={loadExistingResume}
                  disabled={loadingProfileResume}
                  title="Load current active resume from profile"
                >
                  {loadingProfileResume ? 'Loading...' : 'Load My Profile'}
                </button>
                <button
                  type="button"
                  className="btn btn-sm btn-outline"
                  onClick={() => setResumeText(SAMPLE_TEMPLATES.cpp)}
                >
                  C++ Template
                </button>
                <button
                  type="button"
                  className="btn btn-sm btn-outline"
                  onClick={() => setResumeText(SAMPLE_TEMPLATES.react)}
                >
                  React Template
                </button>
                <button
                  type="button"
                  className="btn btn-sm btn-outline"
                  onClick={() => setResumeText(SAMPLE_TEMPLATES.fullstack)}
                >
                  Fullstack
                </button>
              </div>
            </div>

            {/* Live Textarea */}
            <div style={{ position: 'relative' }}>
              <textarea
                ref={textareaRef}
                value={resumeText}
                onChange={(e) => setResumeText(e.target.value)}
                rows={22}
                placeholder="Paste or type structured resume text here with [SKILLS], [EXPERIENCE], [EDUCATION]..."
                style={{
                  width: '100%',
                  fontFamily: 'var(--font-mono, "SFMono-Regular", Consolas, monospace)',
                  fontSize: '0.85rem',
                  lineHeight: '1.55',
                  padding: '1rem',
                  backgroundColor: 'rgba(0, 0, 0, 0.25)',
                  color: 'var(--text-main)',
                  border: '1px solid var(--border-card)',
                  borderRadius: 'var(--radius-md)',
                  resize: 'vertical',
                  outline: 'none',
                  boxSizing: 'border-box',
                }}
              />
            </div>

            {/* Bottom Meta & Action Bar */}
            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginTop: '0.75rem', fontSize: '0.8rem', color: 'var(--text-muted)' }}>
              <div style={{ display: 'flex', gap: '1rem' }}>
                <span><strong>{analysis.wordCount}</strong> words</span>
                <span><strong>{analysis.charCount}</strong> chars</span>
                <span><strong>{analysis.extractedSkills.length}</strong> skills detected</span>
              </div>

              <div style={{ display: 'flex', gap: '0.5rem' }}>
                <button
                  type="button"
                  onClick={handleDownload}
                  className="btn btn-sm btn-outline"
                  style={{ fontSize: '0.75rem', padding: '0.25rem 0.6rem' }}
                >
                  Download .txt
                </button>
                <button
                  type="button"
                  onClick={() => {
                    navigator.clipboard.writeText(resumeText);
                    setSaveStatus({ type: 'success', message: 'Copied resume to clipboard!' });
                  }}
                  className="btn btn-sm btn-outline"
                  style={{ fontSize: '0.75rem', padding: '0.25rem 0.6rem' }}
                >
                  Copy Text
                </button>
              </div>
            </div>
          </div>

          {/* RIGHT: Real-Time Intelligence & Match Gauge */}
          <div style={{ display: 'flex', flexDirection: 'column', gap: '1.25rem' }}>
            
            {/* Target Job Selector Card */}
            <div className="card" style={{ padding: '1.25rem' }}>
              <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '0.75rem' }}>
                <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                  <TargetIcon size={18} color="var(--primary)" />
                  <h3 style={{ margin: 0, fontSize: '1.05rem', fontWeight: 700 }}>Target Job Matching</h3>
                </div>
                <span style={{ fontSize: '0.75rem', color: 'var(--text-muted)' }}>{jobs.length} jobs available</span>
              </div>

              {loadingJobs ? (
                <Loading message="Loading database job requisitions..." />
              ) : (
                <>
                  <select
                    value={selectedJobId}
                    onChange={(e) => setSelectedJobId(e.target.value)}
                    style={{
                      width: '100%',
                      padding: '0.65rem 0.85rem',
                      borderRadius: 'var(--radius-md)',
                      backgroundColor: 'rgba(0, 0, 0, 0.2)',
                      border: '1px solid var(--border-card)',
                      color: 'var(--text-main)',
                      fontSize: '0.9rem',
                      outline: 'none',
                      cursor: 'pointer',
                    }}
                  >
                    {jobs.map((j) => (
                      <option key={j.jobId} value={j.jobId} style={{ backgroundColor: '#1e293b' }}>
                        [{j.jobId}] {j.title} — {j.company} ({j.location})
                      </option>
                    ))}
                  </select>

                  {selectedJob && (
                    <div style={{ marginTop: '0.75rem', fontSize: '0.82rem', color: 'var(--text-muted)', display: 'flex', gap: '0.75rem', flexWrap: 'wrap' }}>
                      <span><strong>Min Exp:</strong> {selectedJob.minimumExperience ?? 0} yrs</span>
                      <span><strong>Type:</strong> {selectedJob.employmentType || 'Full Time'}</span>
                      <span><strong>Degree:</strong> {selectedJob.educationRequirement || 'Any'}</span>
                    </div>
                  )}
                </>
              )}
            </div>

            {/* Live Score Display Card */}
            <div className="card" style={{ padding: '1.5rem', background: 'linear-gradient(135deg, rgba(30,41,59,0.7) 0%, rgba(15,23,42,0.9) 100%)' }}>
              <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: '1rem' }}>
                <div>
                  <span style={{ fontSize: '0.75rem', textTransform: 'uppercase', letterSpacing: '0.05em', color: 'var(--text-muted)', fontWeight: 600 }}>
                    Live Match Score
                  </span>
                  <div style={{ fontSize: '2.5rem', fontWeight: 800, color: matchClassification.color, lineHeight: 1.1 }}>
                    {analysis.matchScore}%
                  </div>
                  <div style={{ fontSize: '0.8rem', fontWeight: 700, color: matchClassification.color, marginTop: '0.25rem' }}>
                    {matchClassification.label}
                  </div>
                </div>

                {/* ATS Overall Score Gauge */}
                <div style={{ textAlign: 'right' }}>
                  <span style={{ fontSize: '0.75rem', textTransform: 'uppercase', letterSpacing: '0.05em', color: 'var(--text-muted)', fontWeight: 600 }}>
                    ATS Quality Index
                  </span>
                  <div style={{ fontSize: '2.2rem', fontWeight: 800, color: analysis.atsHealth >= 75 ? '#10b981' : '#f59e0b', lineHeight: 1.1 }}>
                    {analysis.atsHealth}/100
                  </div>
                  <div style={{ fontSize: '0.8rem', color: 'var(--text-muted)', marginTop: '0.25rem' }}>
                    {analysis.atsHealth >= 75 ? 'ATS Optimized' : 'Needs Optimization'}
                  </div>
                </div>
              </div>

              {/* Progress Bar */}
              <div style={{ width: '100%', height: '8px', backgroundColor: 'rgba(255,255,255,0.08)', borderRadius: '9999px', overflow: 'hidden' }}>
                <div
                  style={{
                    height: '100%',
                    width: `${analysis.matchScore}%`,
                    backgroundColor: matchClassification.color,
                    borderRadius: '9999px',
                    transition: 'width 0.25s ease-in-out',
                  }}
                />
              </div>
            </div>

            {/* Real-Time Skill Gap Breakdown */}
            <div className="card" style={{ padding: '1.25rem' }}>
              <h4 style={{ margin: '0 0 0.75rem 0', fontSize: '0.95rem', fontWeight: 700, display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                <ZapIcon size={16} color="var(--primary)" />
                <span>Live Skill Match & Gap Analysis</span>
              </h4>

              {/* Matched Required Skills */}
              <div style={{ marginBottom: '1rem' }}>
                <div style={{ fontSize: '0.78rem', color: 'var(--text-muted)', marginBottom: '0.4rem', fontWeight: 600 }}>
                  ✓ MATCHED REQUIRED SKILLS ({analysis.reqMatched.length})
                </div>
                <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.4rem' }}>
                  {analysis.reqMatched.length > 0 ? (
                    analysis.reqMatched.map(s => (
                      <span key={s} style={{ display: 'inline-flex', alignItems: 'center', gap: '0.3rem', padding: '0.25rem 0.6rem', borderRadius: '9999px', backgroundColor: 'rgba(16, 185, 129, 0.15)', border: '1px solid #10b981', color: '#10b981', fontSize: '0.78rem', fontWeight: 600 }}>
                        ✓ {s}
                      </span>
                    ))
                  ) : (
                    <span style={{ fontSize: '0.8rem', color: 'var(--text-muted)', fontStyle: 'italic' }}>None matched yet</span>
                  )}
                </div>
              </div>

              {/* Missing Required Skills with Quick Insert Button */}
              <div style={{ marginBottom: '1rem' }}>
                <div style={{ fontSize: '0.78rem', color: '#ef4444', marginBottom: '0.4rem', fontWeight: 600 }}>
                  ✕ MISSING REQUIRED SKILLS ({analysis.reqMissing.length}) — Click to insert
                </div>
                <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.4rem' }}>
                  {analysis.reqMissing.length > 0 ? (
                    analysis.reqMissing.map(s => (
                      <button
                        key={s}
                        type="button"
                        onClick={() => handleInsertSkill(s)}
                        title={`Click to insert "${s}" into your resume draft`}
                        style={{
                          display: 'inline-flex',
                          alignItems: 'center',
                          gap: '0.35rem',
                          padding: '0.25rem 0.6rem',
                          borderRadius: '9999px',
                          backgroundColor: 'rgba(239, 68, 68, 0.12)',
                          border: '1px solid rgba(239, 68, 68, 0.4)',
                          color: '#f87171',
                          fontSize: '0.78rem',
                          fontWeight: 600,
                          cursor: 'pointer',
                          transition: 'all 0.15s ease',
                        }}
                        onMouseEnter={(e) => {
                          e.currentTarget.style.backgroundColor = 'rgba(239, 68, 68, 0.25)';
                          e.currentTarget.style.borderColor = '#ef4444';
                        }}
                        onMouseLeave={(e) => {
                          e.currentTarget.style.backgroundColor = 'rgba(239, 68, 68, 0.12)';
                          e.currentTarget.style.borderColor = 'rgba(239, 68, 68, 0.4)';
                        }}
                      >
                        <span>+ {s}</span>
                      </button>
                    ))
                  ) : (
                    <span style={{ fontSize: '0.8rem', color: '#10b981', fontWeight: 600 }}>
                      🎉 All required skills matched!
                    </span>
                  )}
                </div>
              </div>

              {/* Preferred Skills */}
              <div>
                <div style={{ fontSize: '0.78rem', color: 'var(--text-muted)', marginBottom: '0.4rem', fontWeight: 600 }}>
                  PREFERRED SKILLS: {analysis.prefMatched.length} matched / {analysis.prefMissing.length} missing
                </div>
                <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.4rem' }}>
                  {analysis.prefMatched.map(s => (
                    <span key={s} style={{ padding: '0.2rem 0.5rem', borderRadius: '9999px', backgroundColor: 'rgba(59, 130, 246, 0.15)', border: '1px solid #3b82f6', color: '#60a5fa', fontSize: '0.75rem' }}>
                      ✓ {s}
                    </span>
                  ))}
                  {analysis.prefMissing.map(s => (
                    <button
                      key={s}
                      type="button"
                      onClick={() => handleInsertSkill(s)}
                      title={`Click to insert preferred skill "${s}"`}
                      style={{
                        padding: '0.2rem 0.5rem',
                        borderRadius: '9999px',
                        backgroundColor: 'rgba(245, 158, 11, 0.1)',
                        border: '1px solid rgba(245, 158, 11, 0.3)',
                        color: '#fbbf24',
                        fontSize: '0.75rem',
                        cursor: 'pointer',
                      }}
                    >
                      + {s}
                    </button>
                  ))}
                </div>
              </div>
            </div>

            {/* Real-Time ATS Quality Diagnostic Card */}
            <div className="card" style={{ padding: '1.25rem' }}>
              <h4 style={{ margin: '0 0 0.75rem 0', fontSize: '0.95rem', fontWeight: 700, display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                <SparklesIcon size={16} color="var(--primary)" />
                <span>Live Content Quality Diagnostics</span>
              </h4>

              <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '0.75rem', fontSize: '0.82rem' }}>
                <div style={{ padding: '0.6rem', borderRadius: 'var(--radius-sm)', backgroundColor: 'rgba(255,255,255,0.02)', border: '1px solid var(--border-subtle)' }}>
                  <div style={{ color: 'var(--text-muted)', fontSize: '0.74rem' }}>Action Verbs</div>
                  <div style={{ fontSize: '1.1rem', fontWeight: 700, color: analysis.detectedVerbs.length >= 3 ? '#10b981' : '#f59e0b' }}>
                    {analysis.detectedVerbs.length} verbs
                  </div>
                  <div style={{ fontSize: '0.7rem', color: 'var(--text-muted)' }}>
                    {analysis.detectedVerbs.slice(0, 3).join(', ') || 'None found'}
                  </div>
                </div>

                <div style={{ padding: '0.6rem', borderRadius: 'var(--radius-sm)', backgroundColor: 'rgba(255,255,255,0.02)', border: '1px solid var(--border-subtle)' }}>
                  <div style={{ color: 'var(--text-muted)', fontSize: '0.74rem' }}>Quantifiable Metrics</div>
                  <div style={{ fontSize: '1.1rem', fontWeight: 700, color: analysis.metricsMatches.length >= 2 ? '#10b981' : '#f59e0b' }}>
                    {analysis.metricsMatches.length} metrics
                  </div>
                  <div style={{ fontSize: '0.7rem', color: 'var(--text-muted)' }}>
                    {analysis.metricsMatches.slice(0, 3).join(', ') || 'Add %, numbers'}
                  </div>
                </div>

                <div style={{ padding: '0.6rem', borderRadius: 'var(--radius-sm)', backgroundColor: 'rgba(255,255,255,0.02)', border: '1px solid var(--border-subtle)' }}>
                  <div style={{ color: 'var(--text-muted)', fontSize: '0.74rem' }}>Section Coverage</div>
                  <div style={{ fontSize: '1.1rem', fontWeight: 700, color: analysis.sectionCount >= 4 ? '#10b981' : '#f59e0b' }}>
                    {analysis.sectionCount} / 5 sections
                  </div>
                  <div style={{ fontSize: '0.7rem', color: 'var(--text-muted)' }}>
                    {[analysis.hasSummary && 'Summary', analysis.hasSkills && 'Skills', analysis.hasExperience && 'Exp', analysis.hasEducation && 'Edu'].filter(Boolean).join(', ')}
                  </div>
                </div>

                <div style={{ padding: '0.6rem', borderRadius: 'var(--radius-sm)', backgroundColor: 'rgba(255,255,255,0.02)', border: '1px solid var(--border-subtle)' }}>
                  <div style={{ color: 'var(--text-muted)', fontSize: '0.74rem' }}>Resume Volume</div>
                  <div style={{ fontSize: '1.1rem', fontWeight: 700, color: analysis.wordCount >= 150 && analysis.wordCount <= 600 ? '#10b981' : '#f59e0b' }}>
                    {analysis.wordCount} words
                  </div>
                  <div style={{ fontSize: '0.7rem', color: 'var(--text-muted)' }}>
                    Target: 200–500 words
                  </div>
                </div>
              </div>
            </div>

          </div>

        </div>
      </main>

      <style>{`
        @keyframes pulse-live {
          0% { transform: scale(0.95); opacity: 0.7; }
          50% { transform: scale(1.15); opacity: 1; }
          100% { transform: scale(0.95); opacity: 0.7; }
        }
        .smart-icon-spin {
          animation: spin-live 1s linear infinite;
        }
        @keyframes spin-live {
          from { transform: rotate(0deg); }
          to { transform: rotate(360deg); }
        }
        @media (max-width: 900px) {
          .portal-content > div:nth-of-type(2) {
            grid-template-columns: 1fr !important;
          }
        }
      `}</style>
    </div>
  );
};

export default RealTimeAnalyzer;

