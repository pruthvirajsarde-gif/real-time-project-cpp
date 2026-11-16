import React, { useState, useEffect } from 'react';
import { useAuth } from '../context/AuthContext';
import Sidebar from '../components/Sidebar';
import { authApi } from '../api/authApi';
import { dashboardApi } from '../api/dashboardApi';
import { validatePassword } from '../utils/validators';
import {
  UserIcon,
  ShieldIcon,
  CheckIcon,
  XIcon,
  ZapIcon,
  RotateCcwIcon
} from '../components/Icons';

export const Settings = () => {
  const { user, logout } = useAuth();
  const role = user?.role || 'CANDIDATE';

  // Password Form State
  const [passwords, setPasswords] = useState({
    currentPassword: '',
    newPassword: '',
    confirmPassword: '',
  });
  const [submittingPassword, setSubmittingPassword] = useState(false);
  const [passwordSuccess, setPasswordSuccess] = useState(null);
  const [passwordError, setPasswordError] = useState(null);

  // Health Diagnostics State
  const [healthData, setHealthData] = useState(null);
  const [healthLoading, setHealthLoading] = useState(false);
  const [healthError, setHealthError] = useState(null);

  const checkHealth = async () => {
    setHealthLoading(true);
    setHealthError(null);
    try {
      const res = await dashboardApi.getHealth();
      if (res && res.data) {
        setHealthData(res.data);
      }
    } catch (err) {
      setHealthError('SmartHire server is unavailable. Please start: SmartHire.exe --server');
      setHealthData(null);
    } finally {
      setHealthLoading(false);
    }
  };

  useEffect(() => {
    checkHealth();
  }, []);

  const handlePasswordSubmit = async (e) => {
    e.preventDefault();
    setPasswordError(null);
    setPasswordSuccess(null);

    if (passwords.newPassword !== passwords.confirmPassword) {
      setPasswordError('New password and confirmation do not match.');
      return;
    }

    const valResult = validatePassword(passwords.newPassword);
    if (!valResult.valid) {
      setPasswordError(valResult.message);
      return;
    }

    setSubmittingPassword(true);
    try {
      const res = await authApi.changePassword({
        currentPassword: passwords.currentPassword,
        newPassword: passwords.newPassword,
      });

      setPasswordSuccess(res.message || 'Password changed successfully. Please log in again.');
      setPasswords({
        currentPassword: '',
        newPassword: '',
        confirmPassword: '',
      });

      // Automatically log out after 3 seconds so the user can re-authenticate
      setTimeout(() => {
        logout();
      }, 3500);
    } catch (err) {
      setPasswordError(err.message || 'Failed to change password. Please verify current password.');
    } finally {
      setSubmittingPassword(false);
    }
  };

  const apiUrl =
    import.meta.env.VITE_API_BASE_URL ||
    (typeof window !== 'undefined' && window.location.hostname
      ? `http://${window.location.hostname}:8080/api/v1`
      : 'http://127.0.0.1:8080/api/v1');

  return (
    <div className="portal-layout">
      <Sidebar role={role} />

      <main className="portal-content">
        <div className="page-header">
          <h1 className="page-title">Account & System Settings</h1>
          <p className="page-subtitle">
            Manage your account security, credentials, and verify local C++ REST server connectivity.
          </p>
        </div>

        <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(340px, 1fr))', gap: '1.5rem' }}>
          {/* Account Profile Card */}
          <div className="card">
            <h3 style={{ fontSize: '1.25rem', marginBottom: '1.25rem', display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
              <UserIcon size={18} color="var(--primary)" />
              <span>Account Profile</span>
            </h3>

            <div style={{ display: 'flex', flexDirection: 'column', gap: '1rem' }}>
              <div>
                <span style={{ fontSize: '0.8rem', color: 'var(--text-muted)', textTransform: 'uppercase' }}>Username</span>
                <div style={{ fontWeight: 600, fontSize: '1.1rem', marginTop: '0.2rem' }}>
                  {user?.username || '—'}
                </div>
              </div>

              <div>
                <span style={{ fontSize: '0.8rem', color: 'var(--text-muted)', textTransform: 'uppercase' }}>Account ID</span>
                <div style={{ fontFamily: 'var(--font-mono)', fontWeight: 600, marginTop: '0.2rem' }}>
                  {user?.accountId || user?.id || '—'}
                </div>
              </div>

              <div>
                <span style={{ fontSize: '0.8rem', color: 'var(--text-muted)', textTransform: 'uppercase' }}>Assigned Role</span>
                <div style={{ marginTop: '0.25rem' }}>
                  <span
                    className={`badge ${role === 'RECRUITER' ? 'badge-primary' : 'badge-success'}`}
                    style={{ fontSize: '0.85rem', padding: '0.35rem 0.75rem' }}
                  >
                    {role === 'RECRUITER' ? 'Recruiter' : 'Candidate'}
                  </span>
                </div>
              </div>

              <div>
                <span style={{ fontSize: '0.8rem', color: 'var(--text-muted)', textTransform: 'uppercase' }}>Email Address</span>
                <div style={{ fontWeight: 500, marginTop: '0.2rem' }}>
                  {user?.email || '—'}
                </div>
              </div>
            </div>

            <hr style={{ borderColor: 'var(--border)', margin: '1.5rem 0' }} />

            <div>
              <h4 style={{ fontSize: '0.95rem', marginBottom: '0.5rem' }}>Session Storage Architecture</h4>
              <p style={{ fontSize: '0.85rem', color: 'var(--text-muted)', lineHeight: '1.5' }}>
                Authentication Bearer tokens are stored exclusively in <code>sessionStorage</code>. Passwords, hashes, and salts are never stored in client browser storage.
              </p>
              <button
                onClick={logout}
                className="btn btn-outline"
                style={{ marginTop: '1rem', width: '100%', borderColor: 'var(--danger)', color: 'var(--danger)' }}
              >
                Log Out of SmartHire
              </button>
            </div>
          </div>

          {/* Change Password Card */}
          <div className="card">
            <h3 style={{ fontSize: '1.25rem', marginBottom: '1.25rem', display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
              <ShieldIcon size={18} color="var(--primary)" />
              <span>Change Password</span>
            </h3>

            {passwordSuccess && (
              <div className="banner banner-success" style={{ marginBottom: '1.25rem', display: 'flex', alignItems: 'center', gap: '0.4rem' }}>
                <CheckIcon size={14} color="#10b981" />
                <span>{passwordSuccess}</span>
              </div>
            )}

            {passwordError && (
              <div className="banner banner-danger" style={{ marginBottom: '1.25rem', display: 'flex', alignItems: 'center', gap: '0.4rem' }}>
                <XIcon size={14} color="#ef4444" />
                <span>{passwordError}</span>
              </div>
            )}

            <form onSubmit={handlePasswordSubmit}>
              <div className="form-group">
                <label className="form-label">Current Password *</label>
                <input
                  type="password"
                  className="form-input"
                  placeholder="Enter current password"
                  value={passwords.currentPassword}
                  onChange={(e) => setPasswords({ ...passwords, currentPassword: e.target.value })}
                  required
                />
              </div>

              <div className="form-group">
                <label className="form-label">New Password *</label>
                <input
                  type="password"
                  className="form-input"
                  placeholder="Minimum 8 characters"
                  value={passwords.newPassword}
                  onChange={(e) => setPasswords({ ...passwords, newPassword: e.target.value })}
                  required
                />
                <span style={{ fontSize: '0.75rem', color: 'var(--text-muted)', display: 'block', marginTop: '0.25rem' }}>
                  Must contain uppercase, lowercase, digit, and special symbol.
                </span>
              </div>

              <div className="form-group">
                <label className="form-label">Confirm New Password *</label>
                <input
                  type="password"
                  className="form-input"
                  placeholder="Re-enter new password"
                  value={passwords.confirmPassword}
                  onChange={(e) => setPasswords({ ...passwords, confirmPassword: e.target.value })}
                  required
                />
              </div>

              <button
                type="submit"
                className="btn btn-primary"
                style={{ width: '100%', marginTop: '0.5rem' }}
                disabled={submittingPassword}
              >
                {submittingPassword ? 'Updating Password...' : 'Update Password'}
              </button>
            </form>
          </div>
        </div>

        {/* Server Diagnostics & Health Card */}
        <div className="card" style={{ marginTop: '1.5rem' }}>
          <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '1.25rem' }}>
            <h3 style={{ fontSize: '1.25rem', display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
              <ZapIcon size={18} color="var(--primary)" />
              <span>Backend REST Server Diagnostics</span>
            </h3>
            <button
              onClick={checkHealth}
              className="btn btn-sm btn-secondary"
              disabled={healthLoading}
              style={{ display: 'inline-flex', alignItems: 'center', gap: '0.4rem' }}
            >
              <RotateCcwIcon size={13} />
              <span>{healthLoading ? 'Checking...' : 'Ping Server'}</span>
            </button>
          </div>


          {healthError ? (
            <div className="banner banner-danger" style={{ marginBottom: '1.25rem' }}>
              <div style={{ fontWeight: 700, marginBottom: '0.25rem' }}>Server Unavailable</div>
              <p style={{ margin: 0, fontSize: '0.9rem' }}>{healthError}</p>
            </div>
          ) : healthData ? (
            <div
              style={{
                display: 'grid',
                gridTemplateColumns: 'repeat(auto-fit, minmax(200px, 1fr))',
                gap: '1rem',
                marginBottom: '1rem',
              }}
            >
              <div style={{ padding: '0.75rem', background: 'rgba(255,255,255,0.02)', borderRadius: 'var(--radius)' }}>
                <div style={{ fontSize: '0.75rem', color: 'var(--text-muted)', textTransform: 'uppercase' }}>Server Status</div>
                <div style={{ fontSize: '1.1rem', fontWeight: 700, color: 'var(--success)', marginTop: '0.2rem' }}>
                  ● {healthData.status || 'UP'}
                </div>
              </div>

              <div style={{ padding: '0.75rem', background: 'rgba(255,255,255,0.02)', borderRadius: 'var(--radius)' }}>
                <div style={{ fontSize: '0.75rem', color: 'var(--text-muted)', textTransform: 'uppercase' }}>Application</div>
                <div style={{ fontSize: '1.1rem', fontWeight: 700, marginTop: '0.2rem' }}>
                  {healthData.application || 'SmartHire'}
                </div>
              </div>

              <div style={{ padding: '0.75rem', background: 'rgba(255,255,255,0.02)', borderRadius: 'var(--radius)' }}>
                <div style={{ fontSize: '0.75rem', color: 'var(--text-muted)', textTransform: 'uppercase' }}>Database Engine</div>
                <div style={{ fontSize: '1.1rem', fontWeight: 700, color: 'var(--success)', marginTop: '0.2rem' }}>
                  {healthData.database || 'CONNECTED'} (SQLite)
                </div>
              </div>

              <div style={{ padding: '0.75rem', background: 'rgba(255,255,255,0.02)', borderRadius: 'var(--radius)' }}>
                <div style={{ fontSize: '0.75rem', color: 'var(--text-muted)', textTransform: 'uppercase' }}>API Version</div>
                <div style={{ fontSize: '1.1rem', fontWeight: 700, marginTop: '0.2rem' }}>
                  v{healthData.version || '9.0'}
                </div>
              </div>
            </div>
          ) : (
            <div style={{ color: 'var(--text-muted)', fontSize: '0.9rem' }}>No diagnostics data retrieved yet.</div>
          )}

          <div style={{ fontSize: '0.85rem', color: 'var(--text-muted)' }}>
            Configured REST Base URL:{' '}
            <code style={{ fontFamily: 'var(--font-mono)', color: 'var(--primary)' }}>{apiUrl}</code>
          </div>
        </div>
      </main>
    </div>
  );
};

export default Settings;
