import React, { useState } from 'react';
import { NavLink, useNavigate } from 'react-router-dom';
import { useAuth } from '../context/AuthContext';
import {
  DashboardIcon,
  FileTextIcon,
  TrendingUpIcon,
  ZapIcon,
  ScaleIcon,
  SearchIcon,
  StarIcon,
  SendIcon,
  BriefcaseIcon,
  UsersIcon,
  UserIcon,
  ClockIcon,
  SettingsIcon,
  PlusIcon,
  LogOutIcon,
  BarChartIcon,
  XIcon
} from './Icons';

export const Sidebar = ({ type, role: propRole }) => {
  const { user, role: authRole, logout } = useAuth();
  const navigate = useNavigate();
  const [mobileMenuOpen, setMobileMenuOpen] = useState(false);

  const handleLogout = async () => {
    await logout();
    navigate('/login');
  };

  const isRecruiter =
    propRole === 'RECRUITER' ||
    type === 'recruiter' ||
    (!type && !propRole && authRole === 'RECRUITER');

  const role = isRecruiter ? 'RECRUITER' : 'CANDIDATE';

  const candidateLinks = [
    { to: '/candidate', label: 'Dashboard', icon: <DashboardIcon size={18} />, end: true },
    { to: '/candidate/realtime-analyzer', label: '⚡ Real-Time Scanner', icon: <ZapIcon size={18} /> },
    { to: '/candidate/resume', label: 'My Resume', icon: <FileTextIcon size={18} /> },
    { to: '/candidate/resume-analysis', label: 'Resume Analysis', icon: <TrendingUpIcon size={18} /> },
    { to: '/candidate/skill-gap', label: 'Skill Gap', icon: <ZapIcon size={18} /> },
    { to: '/candidate/compare', label: 'Resume vs Job', icon: <ScaleIcon size={18} /> },
    { to: '/candidate/jobs', label: 'Find Jobs', icon: <SearchIcon size={18} /> },
    { to: '/candidate/saved-jobs', label: 'Saved Jobs', icon: <StarIcon size={18} /> },
    { to: '/candidate/applications', label: 'Applications', icon: <SendIcon size={18} /> },
    { to: '/candidate/analytics', label: 'Career Analytics', icon: <BarChartIcon size={18} /> },
    { to: '/candidate/market-insights', label: 'Market Insights', icon: <TrendingUpIcon size={18} /> },
    { to: '/candidate/profile', label: 'My Profile', icon: <UserIcon size={18} /> },
    { to: '/candidate/activity', label: 'Activity Audit', icon: <ClockIcon size={18} /> },
    { to: '/candidate/settings', label: 'Settings', icon: <SettingsIcon size={18} /> },
  ];

  const recruiterLinks = [
    { to: '/recruiter', label: 'Dashboard', icon: <DashboardIcon size={18} />, end: true },
    { to: '/recruiter/analytics', label: 'Hiring Analytics', icon: <BarChartIcon size={18} /> },
    { to: '/recruiter/jobs', label: 'My Job Postings', icon: <BriefcaseIcon size={18} /> },
    { to: '/recruiter/jobs/new', label: 'Post New Job', icon: <PlusIcon size={18} /> },
    { to: '/recruiter/applicants', label: 'Applicant Pipeline', icon: <UsersIcon size={18} /> },
    { to: '/recruiter/activity', label: 'Activity Audit', icon: <ClockIcon size={18} /> },
    { to: '/recruiter/settings', label: 'Settings', icon: <SettingsIcon size={18} /> },
  ];

  const links = isRecruiter ? recruiterLinks : candidateLinks;

  return (
    <>
      {/* Mobile Bar */}
      <div style={{
        display: 'none',
        alignItems: 'center',
        justifyContent: 'space-between',
        padding: '0.85rem 1.25rem',
        background: 'var(--surface)',
        borderBottom: '1px solid var(--border-card)',
      }} className="mobile-header">
        <span style={{ fontWeight: 700, fontSize: '0.95rem' }}>
          {isRecruiter ? 'Recruiter Console' : 'Candidate Workspace'}
        </span>
        <button
          onClick={() => setMobileMenuOpen(!mobileMenuOpen)}
          className="btn btn-sm btn-secondary"
        >
          {mobileMenuOpen ? <XIcon size={16} /> : 'Menu'}
        </button>
      </div>

      {/* Main Sidebar */}
      <aside
        style={{
          width: '265px',
          backgroundColor: 'var(--bg-secondary)',
          borderRight: '1px solid var(--border-card)',
          display: 'flex',
          flexDirection: 'column',
          flexShrink: 0,
          minHeight: '100vh',
          zIndex: 40,
        }}
        className={mobileMenuOpen ? 'sidebar-open' : ''}
      >
        {/* User Card */}
        <div style={{
          padding: '1.5rem',
          borderBottom: '1px solid var(--border-subtle)',
          display: 'flex',
          alignItems: 'center',
          gap: '0.85rem',
          background: 'linear-gradient(180deg, rgba(255,255,255,0.02) 0%, transparent 100%)',
        }}>
          <div style={{
            width: '42px',
            height: '42px',
            borderRadius: 'var(--radius-md)',
            background: isRecruiter ? 'var(--primary-gradient)' : 'linear-gradient(135deg, #10b981 0%, #06b6d4 100%)',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            color: '#fff',
            fontWeight: 800,
            fontSize: '1.1rem',
            boxShadow: isRecruiter ? '0 4px 12px rgba(99, 102, 241, 0.35)' : '0 4px 12px rgba(16, 185, 129, 0.35)',
          }}>
            {(user?.username || 'U')[0].toUpperCase()}
          </div>
          <div style={{ overflow: 'hidden' }}>
            <div style={{ fontWeight: 700, fontSize: '0.95rem', whiteSpace: 'nowrap', textOverflow: 'ellipsis', overflow: 'hidden' }}>
              {user?.username || 'Authenticated User'}
            </div>
            <span className={`badge ${isRecruiter ? 'badge-primary' : 'badge-success'}`} style={{ fontSize: '0.65rem', marginTop: '0.15rem' }}>
              {role}
            </span>
          </div>
        </div>

        {/* Navigation Items */}
        <nav style={{ padding: '1rem 0.75rem', display: 'flex', flexDirection: 'column', gap: '0.25rem', flex: 1, overflowY: 'auto' }}>
          {links.map((link) => (
            <NavLink
              key={link.to}
              to={link.to}
              end={link.end}
              onClick={() => setMobileMenuOpen(false)}
              style={({ isActive }) => ({
                display: 'flex',
                alignItems: 'center',
                gap: '0.85rem',
                padding: '0.65rem 0.95rem',
                borderRadius: 'var(--radius-md)',
                fontSize: '0.875rem',
                fontWeight: isActive ? 700 : 500,
                color: isActive ? '#ffffff' : 'var(--text-muted)',
                backgroundColor: isActive ? 'rgba(99, 102, 241, 0.18)' : 'transparent',
                border: isActive ? '1px solid rgba(99, 102, 241, 0.4)' : '1px solid transparent',
                boxShadow: isActive ? '0 2px 10px rgba(99, 102, 241, 0.2)' : 'none',
                transition: 'all var(--transition-fast)',
              })}
            >
              <span style={{ display: 'flex', alignItems: 'center', opacity: 0.9 }}>{link.icon}</span>
              <span>{link.label}</span>
            </NavLink>
          ))}
        </nav>

        {/* Footer Actions */}
        <div style={{ padding: '1.25rem', borderTop: '1px solid var(--border-subtle)', background: 'rgba(0,0,0,0.1)' }}>
          <button
            onClick={handleLogout}
            className="btn btn-sm btn-outline"
            style={{ width: '100%', justifyContent: 'center', gap: '0.5rem' }}
          >
            <LogOutIcon size={16} />
            <span>Sign Out</span>
          </button>
        </div>
      </aside>

      <style>{`
        @media (max-width: 768px) {
          .mobile-header { display: flex !important; }
          aside {
            display: none !important;
            width: 100% !important;
            min-height: auto !important;
          }
          aside.sidebar-open {
            display: flex !important;
          }
        }
      `}</style>
    </>
  );
};

export default Sidebar;
