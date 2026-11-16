import React, { useState, useEffect } from 'react';
import { Link } from 'react-router-dom';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import ProgressBar from '../components/ProgressBar';
import ScoreCard from '../components/ScoreCard';
import { resumeApi } from '../api/resumeApi';
import {
  TargetIcon,
  BarChartIcon,
  StarIcon,
  CheckIcon,
  TrendingUpIcon,
  LightbulbIcon,
  ArrowRightIcon
} from '../components/Icons';

export const ResumeAnalysis = () => {
  const [analysis, setAnalysis] = useState(null);
  const [strengths, setStrengths] = useState(null);
  const [recommendations, setRecommendations] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const fetchAnalysisData = async () => {
    setLoading(true);
    setError(null);
    try {
      const [anaRes, strRes, recRes] = await Promise.allSettled([
        resumeApi.getAnalysis(),
        resumeApi.getStrengths(),
        resumeApi.getRecommendations(),
      ]);

      if (anaRes.status === 'fulfilled' && anaRes.value?.data) {
        setAnalysis(anaRes.value.data);
      }
      if (strRes.status === 'fulfilled' && strRes.value?.data) {
        setStrengths(strRes.value.data);
      }
      if (recRes.status === 'fulfilled' && Array.isArray(recRes.value?.data)) {
        setRecommendations(recRes.value.data);
      }
    } catch (err) {
      setError(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchAnalysisData();
  }, []);

  if (loading) {
    return (
      <div className="portal-layout">
        <Sidebar type="candidate" />
        <main className="portal-main">
          <Loading message="Computing deterministic resume quality analysis..." />
        </main>
      </div>
    );
  }

  const breakdown = analysis?.breakdown || {};
  const overall = analysis?.overallScore ?? analysis?.totalQualityScore ?? 0;
  const classification = analysis?.classification || (overall >= 80 ? 'Strong Resume' : overall >= 60 ? 'Moderate Resume' : 'Needs Optimization');

  // Compute percentage factors (0-100) robustly whether backend returns normalized factor or raw points
  const skillsVal = breakdown.skillsFactor ?? (breakdown.skillsScore != null ? Math.round((breakdown.skillsScore / 20) * 100) : 0);
  const expVal = breakdown.experienceFactor ?? (breakdown.experienceScore != null ? Math.round((breakdown.experienceScore / 20) * 100) : 0);
  const eduVal = breakdown.educationFactor ?? (breakdown.educationScore != null ? Math.round((breakdown.educationScore / 15) * 100) : 0);
  const projVal = breakdown.projectsFactor ?? (breakdown.projectsScore != null ? Math.round((breakdown.projectsScore / 20) * 100) : 0);
  const certVal = breakdown.certificationsFactor ?? (breakdown.certificationsScore != null ? Math.round((breakdown.certificationsScore / 10) * 100) : 0);

  const techFreqMap = strengths?.techFrequencies || strengths?.technologyFrequency || {};
  const hasTechFreq = Object.keys(techFreqMap).length > 0;

  return (
    <div className="portal-layout">
      <Sidebar type="candidate" />

      <main className="portal-main">
        <div className="portal-content">
          <div className="page-header">
            <div>
              <h1 className="page-title">Resume Quality & Analytics</h1>
              <p className="page-subtitle">
                Algorithmic section-by-section strength assessment and targeted optimization advice
              </p>
            </div>
            <Link to="/candidate/jobs" className="btn btn-primary" style={{ display: 'inline-flex', alignItems: 'center', gap: '0.4rem' }}>
              <TargetIcon size={15} />
              <span>Explore Matching Jobs</span>
            </Link>
          </div>

          <ErrorMessage error={error} onRetry={fetchAnalysisData} />

          {!analysis ? (
            <div className="card" style={{ textAlign: 'center', padding: '3.5rem 1rem' }}>
              <p style={{ color: 'var(--text-muted)', marginBottom: '1.25rem' }}>
                No resume analysis available yet. Please upload a resume first.
              </p>
              <Link to="/candidate/resume" className="btn btn-primary">
                Upload Resume
              </Link>
            </div>
          ) : (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '2rem' }}>
              {/* Top Overview Cards */}
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(300px, 1fr))', gap: '1.5rem' }}>
                <ScoreCard
                  score={overall}
                  label="Overall Quality Score"
                  classification={classification}
                  size="lg"
                />

                <div className="card" style={{ display: 'flex', flexDirection: 'column', justifyContent: 'center' }}>
                  <h3 style={{ fontSize: '1.2rem', marginBottom: '1rem', display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                    <BarChartIcon size={18} color="var(--primary)" />
                    <span>Section Factor Breakdown</span>
                  </h3>
                  <ProgressBar label="Skills Depth" value={skillsVal} />
                  <ProgressBar label="Experience Density" value={expVal} />
                  <ProgressBar label="Education Completeness" value={eduVal} />
                  <ProgressBar label="Projects Evidence" value={projVal} />
                  <ProgressBar label="Certifications & Accreditations" value={certVal} />
                </div>
              </div>

              {/* Core Strengths & Technology Distribution */}
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(320px, 1fr))', gap: '1.5rem' }}>
                {/* Strengths */}
                <div className="card">
                  <h3 className="section-title" style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                    <StarIcon size={18} color="#10b981" />
                    <span>Verified Resume Strengths</span>
                  </h3>
                  {(!strengths?.strengths || strengths.strengths.length === 0) ? (
                    <p style={{ color: 'var(--text-muted)', fontSize: '0.9rem' }}>
                      Add quantified metrics and specialized skills to highlight candidate strengths.
                    </p>
                  ) : (
                    <ul style={{ listStyle: 'none', display: 'flex', flexDirection: 'column', gap: '0.75rem', padding: 0 }}>
                      {strengths.strengths.map((str, idx) => (
                        <li key={idx} style={{
                          display: 'flex',
                          alignItems: 'flex-start',
                          gap: '0.75rem',
                          background: 'rgba(16, 185, 129, 0.08)',
                          border: '1px solid rgba(16, 185, 129, 0.25)',
                          borderRadius: 'var(--radius-md)',
                          padding: '0.85rem 1rem',
                          fontSize: '0.9rem',
                          color: 'var(--text-main)'
                        }}>
                          <CheckIcon size={15} color="#10b981" />
                          <span>{str}</span>
                        </li>
                      ))}
                    </ul>
                  )}
                </div>

                {/* Technology Frequency */}
                <div className="card">
                  <h3 className="section-title" style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                    <TrendingUpIcon size={18} color="#06b6d4" />
                    <span>Technology Frequency Count</span>
                  </h3>
                  {!hasTechFreq ? (
                    <p style={{ color: 'var(--text-muted)', fontSize: '0.9rem' }}>
                      No technology keywords recognized in current document text.
                    </p>
                  ) : (
                    <div style={{ display: 'flex', flexDirection: 'column', gap: '0.65rem' }}>
                      {Object.entries(techFreqMap).slice(0, 7).map(([tech, count]) => (
                        <div key={tech} style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', fontSize: '0.9rem' }}>
                          <span style={{ fontWeight: 600 }}>{tech}</span>
                          <span className="badge badge-primary">{count} occurrence{count === 1 ? '' : 's'}</span>
                        </div>
                      ))}
                    </div>
                  )}
                </div>
              </div>

              {/* Actionable Recommendations */}
              <div className="card">
                <h3 className="section-title" style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                  <LightbulbIcon size={18} color="#f59e0b" />
                  <span>Actionable Optimization Recommendations</span>
                </h3>
                {recommendations.length === 0 ? (
                  <p style={{ color: 'var(--text-muted)', fontSize: '0.925rem' }}>
                    No critical gaps detected. Your resume structure conforms to high-readiness benchmarks!
                  </p>
                ) : (
                  <div style={{ display: 'flex', flexDirection: 'column', gap: '0.85rem' }}>
                    {recommendations.map((rec, idx) => (
                      <div
                        key={idx}
                        style={{
                          display: 'flex',
                          alignItems: 'flex-start',
                          gap: '0.75rem',
                          background: 'rgba(99, 102, 241, 0.08)',
                          border: '1px solid rgba(99, 102, 241, 0.2)',
                          borderRadius: 'var(--radius-md)',
                          padding: '1rem',
                        }}
                      >
                        <ArrowRightIcon size={16} color="var(--primary)" />
                        <div style={{ fontSize: '0.925rem', color: 'var(--text-main)', lineHeight: 1.5 }}>
                          {rec}
                        </div>
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

export default ResumeAnalysis;
