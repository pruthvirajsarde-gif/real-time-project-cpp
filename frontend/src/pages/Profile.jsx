import React, { useState, useEffect } from 'react';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import { candidateApi } from '../api/candidateApi';
import { CheckIcon, XIcon, UserIcon, EditIcon } from '../components/Icons';

export const Profile = () => {
  const [profile, setProfile] = useState(null);
  const [loading, setLoading] = useState(true);
  const [saving, setSaving] = useState(false);
  const [error, setError] = useState(null);
  const [successMsg, setSuccessMsg] = useState(null);

  const [phone, setPhone] = useState('');
  const [location, setLocation] = useState('');

  const fetchProfile = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await candidateApi.getProfile();
      if (res && res.data) {
        setProfile(res.data);
        setPhone(res.data.phone || '');
        setLocation(res.data.location || '');
      }
    } catch (err) {
      setError(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchProfile();
  }, []);

  const handleUpdate = async (e) => {
    e.preventDefault();
    setError(null);
    setSaving(true);
    setSuccessMsg(null);
    try {
      const res = await candidateApi.updateProfile({
        phone: phone.trim(),
        location: location.trim(),
      });
      setSuccessMsg(res?.message || 'Profile contact details updated successfully!');
      if (res && res.data) {
        setProfile((prev) => ({ ...prev, ...res.data }));
      }
    } catch (err) {
      setError(err);
    } finally {
      setSaving(false);
    }
  };

  return (
    <div className="portal-layout">
      <Sidebar type="candidate" />

      <main className="portal-main">
        <div className="portal-content">
          <div className="page-header">
            <div>
              <h1 className="page-title">Candidate Profile</h1>
              <p className="page-subtitle">
                Inspect your verified account profile and update your contact preferences
              </p>
            </div>
          </div>

          <ErrorMessage error={error} onRetry={fetchProfile} />

          {successMsg && (
            <div style={{
              background: 'rgba(16, 185, 129, 0.12)',
              border: '1px solid rgba(16, 185, 129, 0.3)',
              borderRadius: 'var(--radius-md)',
              padding: '0.85rem 1.25rem',
              color: '#6ee7b7',
              marginBottom: '1.5rem',
              display: 'flex',
              justifyContent: 'space-between',
              alignItems: 'center'
            }}>
              <span style={{ display: 'flex', alignItems: 'center', gap: '0.4rem' }}>
                <CheckIcon size={14} color="#10b981" />
                <span>{successMsg}</span>
              </span>
              <button
                onClick={() => setSuccessMsg(null)}
                style={{ background: 'transparent', border: 'none', color: '#6ee7b7', cursor: 'pointer', display: 'flex', alignItems: 'center' }}
              >
                <XIcon size={14} />
              </button>
            </div>
          )}

          {loading ? (
            <Loading message="Loading profile dossier..." />
          ) : !profile ? (
            <div className="card" style={{ textAlign: 'center', padding: '3rem 1rem' }}>
              <p style={{ color: 'var(--text-muted)' }}>Candidate profile not found.</p>
            </div>
          ) : (
            <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(320px, 1fr))', gap: '2rem' }}>
              {/* Account Readonly Dossier */}
              <div className="card">
                <h3 className="section-title" style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                  <UserIcon size={18} color="var(--primary)" />
                  <span>Verified Dossier</span>
                </h3>
                <div style={{ display: 'flex', flexDirection: 'column', gap: '1.25rem', marginTop: '1.25rem' }}>
                  <div>
                    <div style={{ fontSize: '0.75rem', color: 'var(--text-dim)', textTransform: 'uppercase', fontWeight: 700 }}>Full Name</div>
                    <div style={{ fontSize: '1.15rem', fontWeight: 700, marginTop: '0.2rem' }}>{profile.name}</div>
                  </div>
                  <div>
                    <div style={{ fontSize: '0.75rem', color: 'var(--text-dim)', textTransform: 'uppercase', fontWeight: 700 }}>Email Address</div>
                    <div style={{ fontSize: '0.95rem', color: 'var(--text-muted)', marginTop: '0.2rem' }}>{profile.email}</div>
                  </div>
                  <div>
                    <div style={{ fontSize: '0.75rem', color: 'var(--text-dim)', textTransform: 'uppercase', fontWeight: 700 }}>Account Identifier</div>
                    <div style={{ fontSize: '0.9rem', fontFamily: 'var(--font-mono)', color: 'var(--primary)' }}>Candidate #{profile.candidateId}</div>
                  </div>
                </div>
              </div>

              {/* Editable Contact Form */}
              <div className="card">
                <h3 className="section-title" style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                  <EditIcon size={18} color="var(--primary)" />
                  <span>Contact Preferences</span>
                </h3>

                <form onSubmit={handleUpdate} style={{ marginTop: '1.25rem' }}>
                  <div className="form-group">
                    <label className="form-label" htmlFor="prof-phone">Phone Number</label>
                    <input
                      id="prof-phone"
                      type="text"
                      className="form-input"
                      placeholder="e.g. 555-0199"
                      value={phone}
                      onChange={(e) => setPhone(e.target.value)}
                    />
                  </div>

                  <div className="form-group">
                    <label className="form-label" htmlFor="prof-location">Location / City</label>
                    <input
                      id="prof-location"
                      type="text"
                      className="form-input"
                      placeholder="e.g. Boston, MA"
                      value={location}
                      onChange={(e) => setLocation(e.target.value)}
                    />
                  </div>

                  <button
                    type="submit"
                    className="btn btn-primary"
                    disabled={saving}
                    style={{ marginTop: '0.5rem' }}
                  >
                    {saving ? 'Updating...' : 'Save Profile Changes'}
                  </button>
                </form>
              </div>
            </div>
          )}
        </div>
      </main>
    </div>
  );
};

export default Profile;
