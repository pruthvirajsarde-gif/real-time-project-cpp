import React, { useState } from 'react';
import { Link, useNavigate } from 'react-router-dom';
import Navbar from '../components/Navbar';
import ErrorMessage from '../components/ErrorMessage';
import { useAuth } from '../context/AuthContext';
import { validateEmail, validatePassword } from '../utils/validators';

export const Register = () => {
  const { registerCandidate, registerRecruiter, login } = useAuth();
  const navigate = useNavigate();

  const [roleTab, setRoleTab] = useState('CANDIDATE'); // 'CANDIDATE' or 'RECRUITER'
  const [formData, setFormData] = useState({
    name: '',
    email: '',
    phone: '',
    location: '',
    company: '',
    username: '',
    password: '',
    confirmPassword: '',
  });

  const [submitting, setSubmitting] = useState(false);
  const [error, setError] = useState(null);

  const handleChange = (e) => {
    setFormData({ ...formData, [e.target.name]: e.target.value });
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    setError(null);

    // Client-side validations
    const emailErr = validateEmail(formData.email);
    if (emailErr) {
      setError(emailErr);
      return;
    }

    const passErr = validatePassword(formData.password);
    if (passErr) {
      setError(passErr);
      return;
    }

    if (formData.password !== formData.confirmPassword) {
      setError('Passwords do not match.');
      return;
    }

    setSubmitting(true);
    try {
      if (roleTab === 'CANDIDATE') {
        await registerCandidate({
          name: formData.name.trim(),
          email: formData.email.trim(),
          phone: formData.phone.trim(),
          location: formData.location.trim(),
          username: formData.username.trim(),
          password: formData.password,
        });
      } else {
        await registerRecruiter({
          name: formData.name.trim(),
          company: formData.company.trim(),
          email: formData.email.trim(),
          username: formData.username.trim(),
          password: formData.password,
        });
      }

      // Auto login upon successful registration
      await login(formData.username.trim(), formData.password);
      navigate(roleTab === 'RECRUITER' ? '/recruiter' : '/candidate');
    } catch (err) {
      setError(err);
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <div className="app-container">
      <Navbar />

      <main style={{
        flex: 1,
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        padding: '3rem 1.5rem',
      }}>
        <div className="card" style={{ maxWidth: '520px', width: '100%', padding: '2.5rem' }}>
          <div style={{ textAlign: 'center', marginBottom: '1.75rem' }}>
            <h2 style={{ fontSize: '1.75rem', fontWeight: 800 }}>Create Your Account</h2>
            <p style={{ color: 'var(--text-muted)', fontSize: '0.9rem', marginTop: '0.25rem' }}>
              Choose your role to get started with SmartHire
            </p>
          </div>

          {/* Role Tabs */}
          <div style={{
            display: 'flex',
            background: 'rgba(15, 23, 42, 0.6)',
            padding: '0.35rem',
            borderRadius: 'var(--radius-md)',
            marginBottom: '1.75rem',
            border: '1px solid var(--border-card)',
          }}>
            <button
              type="button"
              onClick={() => { setRoleTab('CANDIDATE'); setError(null); }}
              style={{
                flex: 1,
                padding: '0.65rem',
                border: 'none',
                borderRadius: 'var(--radius-sm)',
                fontWeight: 700,
                fontSize: '0.9rem',
                cursor: 'pointer',
                background: roleTab === 'CANDIDATE' ? 'var(--primary-gradient)' : 'transparent',
                color: roleTab === 'CANDIDATE' ? '#fff' : 'var(--text-muted)',
                transition: 'all var(--transition-fast)',
              }}
            >
              Candidate
            </button>
            <button
              type="button"
              onClick={() => { setRoleTab('RECRUITER'); setError(null); }}
              style={{
                flex: 1,
                padding: '0.65rem',
                border: 'none',
                borderRadius: 'var(--radius-sm)',
                fontWeight: 700,
                fontSize: '0.9rem',
                cursor: 'pointer',
                background: roleTab === 'RECRUITER' ? 'var(--primary-gradient)' : 'transparent',
                color: roleTab === 'RECRUITER' ? '#fff' : 'var(--text-muted)',
                transition: 'all var(--transition-fast)',
              }}
            >
              Recruiter
            </button>
          </div>

          <ErrorMessage error={error} onRetry={() => setError(null)} />

          <form onSubmit={handleSubmit}>
            <div className="form-group">
              <label className="form-label" htmlFor="name">Full Name *</label>
              <input
                id="name"
                name="name"
                type="text"
                className="form-input"
                placeholder={roleTab === 'CANDIDATE' ? 'e.g. Elena Rostova' : 'e.g. Marcus Vance'}
                value={formData.name}
                onChange={handleChange}
                required
              />
            </div>

            {roleTab === 'RECRUITER' && (
              <div className="form-group">
                <label className="form-label" htmlFor="company">Company / Organization *</label>
                <input
                  id="company"
                  name="company"
                  type="text"
                  className="form-input"
                  placeholder="e.g. CyberDyne Systems"
                  value={formData.company}
                  onChange={handleChange}
                  required
                />
              </div>
            )}

            <div className="form-group">
              <label className="form-label" htmlFor="email">Email Address *</label>
              <input
                id="email"
                name="email"
                type="email"
                className="form-input"
                placeholder="name@example.com"
                value={formData.email}
                onChange={handleChange}
                required
              />
            </div>

            {roleTab === 'CANDIDATE' && (
              <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '1rem' }}>
                <div className="form-group">
                  <label className="form-label" htmlFor="phone">Phone</label>
                  <input
                    id="phone"
                    name="phone"
                    type="text"
                    className="form-input"
                    placeholder="555-0199"
                    value={formData.phone}
                    onChange={handleChange}
                  />
                </div>
                <div className="form-group">
                  <label className="form-label" htmlFor="location">Location</label>
                  <input
                    id="location"
                    name="location"
                    type="text"
                    className="form-input"
                    placeholder="Boston, MA"
                    value={formData.location}
                    onChange={handleChange}
                  />
                </div>
              </div>
            )}

            <div className="form-group">
              <label className="form-label" htmlFor="reg-username">Username *</label>
              <input
                id="reg-username"
                name="username"
                type="text"
                className="form-input"
                placeholder="Choose unique username"
                value={formData.username}
                onChange={handleChange}
                required
              />
            </div>

            <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '1rem' }}>
              <div className="form-group">
                <label className="form-label" htmlFor="reg-password">Password *</label>
                <input
                  id="reg-password"
                  name="password"
                  type="password"
                  className="form-input"
                  placeholder="Min 8 characters"
                  value={formData.password}
                  onChange={handleChange}
                  required
                />
              </div>
              <div className="form-group">
                <label className="form-label" htmlFor="confirmPassword">Confirm Password *</label>
                <input
                  id="confirmPassword"
                  name="confirmPassword"
                  type="password"
                  className="form-input"
                  placeholder="Re-enter password"
                  value={formData.confirmPassword}
                  onChange={handleChange}
                  required
                />
              </div>
            </div>

            <button
              type="submit"
              className="btn btn-primary"
              style={{ width: '100%', marginTop: '1rem', padding: '0.85rem' }}
              disabled={submitting}
            >
              {submitting ? 'Registering...' : `Create ${roleTab === 'RECRUITER' ? 'Recruiter' : 'Candidate'} Account`}
            </button>
          </form>

          <div style={{
            marginTop: '2rem',
            paddingTop: '1.25rem',
            borderTop: '1px solid var(--border-subtle)',
            textAlign: 'center',
            fontSize: '0.9rem',
            color: 'var(--text-muted)'
          }}>
            Already have an account?{' '}
            <Link to="/login" style={{ color: 'var(--primary)', fontWeight: 600 }}>
              Sign In
            </Link>
          </div>
        </div>
      </main>
    </div>
  );
};

export default Register;
