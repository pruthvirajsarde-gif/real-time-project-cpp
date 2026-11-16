import React, { useState } from 'react';
import { Link, useNavigate } from 'react-router-dom';
import { useAuth } from '../context/AuthContext';

export const Navbar = () => {
  const { isAuthenticated, user, role, logout } = useAuth();
  const navigate = useNavigate();
  const [mobileOpen, setMobileOpen] = useState(false);

  const handleLogout = async () => {
    await logout();
    navigate('/login');
  };

  const portalPath = role === 'RECRUITER' ? '/recruiter' : '/candidate';

  return (
    <header style={{
      position: 'sticky',
      top: 0,
      zIndex: 100,
      backgroundColor: 'var(--surface-glass)',
      backdropFilter: 'blur(12px)',
      WebkitBackdropFilter: 'blur(12px)',
      borderBottom: '1px solid var(--border-card)',
    }}>
      <div style={{
        maxWidth: '1400px',
        margin: '0 auto',
        padding: '0.85rem 1.5rem',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'space-between',
      }}>
        {/* Brand Logo */}
        <Link to="/" style={{ display: 'flex', alignItems: 'center', gap: '0.75rem' }}>
          <div style={{
            width: '36px',
            height: '36px',
            borderRadius: 'var(--radius-md)',
            background: 'var(--primary-gradient)',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            color: '#fff',
            fontWeight: 800,
            fontSize: '1.1rem',
            boxShadow: '0 2px 10px rgba(99, 102, 241, 0.4)'
          }}>
            SH
          </div>
          <div>
            <span style={{ fontSize: '1.25rem', fontWeight: 800, letterSpacing: '-0.02em' }}>
              Smart<span className="gradient-text">Hire</span>
            </span>
          </div>
        </Link>

        {/* Desktop Navigation Links */}
        <nav style={{ display: 'flex', alignItems: 'center', gap: '1.5rem' }}>
          <Link to="/" style={{ fontSize: '0.925rem', color: 'var(--text-muted)', fontWeight: 500 }}>
            Home
          </Link>
          <Link to={isAuthenticated && role === 'CANDIDATE' ? '/candidate/jobs' : '/jobs'} style={{ fontSize: '0.925rem', color: 'var(--text-muted)', fontWeight: 500 }}>
            Browse Jobs
          </Link>

          {isAuthenticated ? (
            <div style={{ display: 'flex', alignItems: 'center', gap: '1rem' }}>
              <Link to={portalPath} className="btn btn-sm btn-outline">
                {role === 'RECRUITER' ? 'Recruiter Portal' : 'Candidate Workspace'}
              </Link>

              <div style={{
                display: 'flex',
                alignItems: 'center',
                gap: '0.5rem',
                background: 'rgba(255, 255, 255, 0.05)',
                padding: '0.35rem 0.75rem',
                borderRadius: 'var(--radius-full)',
                border: '1px solid var(--border-subtle)',
              }}>
                <span style={{ fontSize: '0.85rem', fontWeight: 600 }}>
                  {user?.username || 'User'}
                </span>
                <span className={`badge ${role === 'RECRUITER' ? 'badge-primary' : 'badge-success'}`} style={{ fontSize: '0.65rem', padding: '0.15rem 0.45rem' }}>
                  {role}
                </span>
              </div>

              <button onClick={handleLogout} className="btn btn-sm btn-secondary" title="Sign out">
                Logout
              </button>
            </div>
          ) : (
            <div style={{ display: 'flex', alignItems: 'center', gap: '0.75rem' }}>
              <Link to="/login" className="btn btn-sm btn-outline">
                Login
              </Link>
              <Link to="/register" className="btn btn-sm btn-primary">
                Get Started
              </Link>
            </div>
          )}
        </nav>
      </div>
    </header>
  );
};

export default Navbar;
