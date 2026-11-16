import React, { useState, useEffect } from 'react';
import { Link } from 'react-router-dom';
import Navbar from '../components/Navbar';
import { dashboardApi } from '../api/dashboardApi';
import { useAuth } from '../context/AuthContext';
import {
  ZapIcon,
  SearchIcon,
  LayersIcon,
  FileTextIcon,
  TargetIcon,
  UsersIcon,
} from '../components/Icons';

export const Home = () => {
  const { isAuthenticated, role } = useAuth();
  const [health, setHealth] = useState(null);

  useEffect(() => {
    dashboardApi
      .getHealth()
      .then((res) => setHealth(res?.data || null))
      .catch(() => setHealth({ status: 'OFFLINE' }));
  }, []);

  return (
    <div className="app-container">
      <Navbar />

      {/* Hero Section */}
      <section style={{
        padding: '5rem 1.5rem 4rem',
        maxWidth: '1200px',
        margin: '0 auto',
        textAlign: 'center',
        position: 'relative',
      }}>
        {health && (
          <div style={{
            display: 'inline-flex',
            alignItems: 'center',
            gap: '0.5rem',
            padding: '0.35rem 0.85rem',
            borderRadius: 'var(--radius-full)',
            background: health.status === 'UP' ? 'rgba(16, 185, 129, 0.1)' : 'rgba(239, 68, 68, 0.1)',
            border: `1px solid ${health.status === 'UP' ? 'rgba(16, 185, 129, 0.3)' : 'rgba(239, 68, 68, 0.3)'}`,
            marginBottom: '1.75rem',
            fontSize: '0.825rem',
            color: health.status === 'UP' ? '#6ee7b7' : '#fca5a5'
          }}>
            <span style={{
              width: '8px',
              height: '8px',
              borderRadius: '50%',
              backgroundColor: health.status === 'UP' ? '#10b981' : '#ef4444',
              boxShadow: `0 0 8px ${health.status === 'UP' ? '#10b981' : '#ef4444'}`
            }} />
            <span>C++ Engine v{health.version || '9.0'} • Database: {health.database || 'CONNECTED'}</span>
          </div>
        )}

        <h1 style={{
          fontSize: 'clamp(2.5rem, 5vw, 4rem)',
          fontWeight: 800,
          letterSpacing: '-0.03em',
          lineHeight: 1.15,
          marginBottom: '1.25rem'
        }}>
          Intelligent Resume Analysis & <br />
          <span className="gradient-text">Explainable Job Matching</span>
        </h1>

        <p style={{
          fontSize: 'clamp(1rem, 2vw, 1.25rem)',
          color: 'var(--text-muted)',
          maxWidth: '750px',
          margin: '0 auto 2.5rem',
          lineHeight: 1.6
        }}>
          Analyze your resume structure, detect missing critical skills, and discover perfectly matched career opportunities backed by deterministic, explainable intelligence.
        </p>

        <div style={{ display: 'flex', gap: '1rem', justifyContent: 'center', flexWrap: 'wrap' }}>
          {isAuthenticated ? (
            <Link
              to={role === 'RECRUITER' ? '/recruiter' : '/candidate'}
              className="btn btn-lg btn-primary"
              style={{ display: 'inline-flex', alignItems: 'center', gap: '0.5rem' }}
            >
              <ZapIcon size={18} />
              <span>Launch {role === 'RECRUITER' ? 'Recruiter Console' : 'Candidate Workspace'}</span>
            </Link>
          ) : (
            <>
              <Link to="/register" className="btn btn-lg btn-primary">
                Get Started Free →
              </Link>
              <Link to="/login" className="btn btn-lg btn-secondary">
                Sign In to Account
              </Link>
            </>
          )}
          <Link to="/jobs" className="btn btn-lg btn-outline" style={{ display: 'inline-flex', alignItems: 'center', gap: '0.5rem' }}>
            <SearchIcon size={16} />
            <span>Browse All Jobs</span>
          </Link>
        </div>
      </section>

      {/* Diploma Project Highlights */}
      <section style={{
        maxWidth: '1200px',
        margin: '1rem auto 4rem',
        padding: '0 1.5rem',
      }}>
        <div style={{
          background: 'linear-gradient(135deg, rgba(99, 102, 241, 0.1) 0%, rgba(6, 182, 212, 0.05) 100%)',
          border: '1px solid rgba(99, 102, 241, 0.25)',
          borderRadius: 'var(--radius-lg)',
          padding: '2rem',
          boxShadow: 'var(--shadow-lg)'
        }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '0.75rem', marginBottom: '1rem' }}>
            <div style={{
              width: '40px',
              height: '40px',
              borderRadius: 'var(--radius-md)',
              background: 'rgba(99, 102, 241, 0.15)',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              color: 'var(--primary)'
            }}>
              <LayersIcon size={22} />
            </div>
            <div>
              <h3 style={{ fontSize: '1.25rem', fontWeight: 800 }}>
                Academic Project Demonstration Features
              </h3>
              <p style={{ fontSize: '0.875rem', color: 'var(--text-muted)' }}>
                Deterministic, offline C++17 platform with high-fidelity explainable intelligence
              </p>
            </div>
          </div>

          <div style={{
            display: 'grid',
            gridTemplateColumns: 'repeat(auto-fit, minmax(260px, 1fr))',
            gap: '1.25rem',
            marginTop: '1.25rem'
          }}>
            <div style={{ background: 'rgba(15, 23, 42, 0.5)', padding: '1rem', borderRadius: 'var(--radius-md)', border: '1px solid var(--border-subtle)' }}>
              <div style={{ fontWeight: 700, color: '#a5b4fc', marginBottom: '0.25rem' }}>
                1. Native Document Ingestion
              </div>
              <p style={{ fontSize: '0.85rem', color: 'var(--text-muted)' }}>
                Multi-format parsing supporting both standard .txt and binary .pdf resumes directly via C++ document extractors.
              </p>
            </div>

            <div style={{ background: 'rgba(15, 23, 42, 0.5)', padding: '1rem', borderRadius: 'var(--radius-md)', border: '1px solid var(--border-subtle)' }}>
              <div style={{ fontWeight: 700, color: '#6ee7b7', marginBottom: '0.25rem' }}>
                2. Explainable Matching Engine
              </div>
              <p style={{ fontSize: '0.85rem', color: 'var(--text-muted)' }}>
                Breaks matching into transparent factors: exact skills, semantic cosine similarity, education, and experience.
              </p>
            </div>

            <div style={{ background: 'rgba(15, 23, 42, 0.5)', padding: '1rem', borderRadius: 'var(--radius-md)', border: '1px solid var(--border-subtle)' }}>
              <div style={{ fontWeight: 700, color: '#fcd34d', marginBottom: '0.25rem' }}>
                3. Skill Gap & Readiness
              </div>
              <p style={{ fontSize: '0.85rem', color: 'var(--text-muted)' }}>
                Pinpoints missing required and preferred skills with prioritized, actionable optimization advice.
              </p>
            </div>

            <div style={{ background: 'rgba(15, 23, 42, 0.5)', padding: '1rem', borderRadius: 'var(--radius-md)', border: '1px solid var(--border-subtle)' }}>
              <div style={{ fontWeight: 700, color: '#93c5fd', marginBottom: '0.25rem' }}>
                4. Multi-Tenant Security & RBAC
              </div>
              <p style={{ fontSize: '0.85rem', color: 'var(--text-muted)' }}>
                PBKDF2-HMAC-SHA256 hashing, Bearer tokens, IP rate limiting, ownership isolation, and privacy-preserving audit logs.
              </p>
            </div>
          </div>
        </div>
      </section>

      {/* Feature Cards Grid */}
      <section style={{
        maxWidth: '1200px',
        margin: '0 auto 5rem',
        padding: '0 1.5rem',
      }}>
        <h2 style={{ textAlign: 'center', fontSize: '2rem', marginBottom: '2.5rem' }}>
          Engineered for <span className="gradient-text">Candidates & Recruiters</span>
        </h2>

        <div style={{
          display: 'grid',
          gridTemplateColumns: 'repeat(auto-fit, minmax(320px, 1fr))',
          gap: '1.5rem'
        }}>
          <div className="card">
            <div style={{
              width: '48px',
              height: '48px',
              borderRadius: 'var(--radius-md)',
              background: 'rgba(99, 102, 241, 0.12)',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              color: 'var(--primary)',
              marginBottom: '1.25rem'
            }}>
              <FileTextIcon size={24} />
            </div>
            <h3 style={{ fontSize: '1.25rem', marginBottom: '0.5rem' }}>Resume Quality Scoring</h3>
            <p style={{ color: 'var(--text-muted)', fontSize: '0.925rem' }}>
              Evaluates section completeness, formatting depth, technical density, and produces a granular 100-point strength scorecard.
            </p>
          </div>

          <div className="card">
            <div style={{
              width: '48px',
              height: '48px',
              borderRadius: 'var(--radius-md)',
              background: 'rgba(16, 185, 129, 0.12)',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              color: '#10b981',
              marginBottom: '1.25rem'
            }}>
              <TargetIcon size={24} />
            </div>
            <h3 style={{ fontSize: '1.25rem', marginBottom: '0.5rem' }}>Multi-Factor Matching</h3>
            <p style={{ color: 'var(--text-muted)', fontSize: '0.925rem' }}>
              Ranks job matches using a weighted mathematical formula combining skill overlap, duration of experience, and educational alignment.
            </p>
          </div>

          <div className="card">
            <div style={{
              width: '48px',
              height: '48px',
              borderRadius: 'var(--radius-md)',
              background: 'rgba(6, 182, 212, 0.12)',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              color: '#06b6d4',
              marginBottom: '1.25rem'
            }}>
              <UsersIcon size={24} />
            </div>
            <h3 style={{ fontSize: '1.25rem', marginBottom: '0.5rem' }}>Recruiter Pipeline</h3>
            <p style={{ color: 'var(--text-muted)', fontSize: '0.925rem' }}>
              Allows hiring teams to post requisitions, inspect incoming applicants, evaluate candidate readiness, and update hiring statuses.
            </p>
          </div>
        </div>
      </section>


      {/* Footer */}
      <footer style={{
        marginTop: 'auto',
        borderTop: '1px solid var(--border-card)',
        padding: '2rem 1.5rem',
        textAlign: 'center',
        color: 'var(--text-dim)',
        fontSize: '0.875rem'
      }}>
        <p>SmartHire © 2026 • Powered by C++17 Embedded REST Architecture & SQLite</p>
      </footer>
    </div>
  );
};

export default Home;
