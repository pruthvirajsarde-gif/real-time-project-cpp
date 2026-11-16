import React, { useState, useEffect } from 'react';
import { Link, useLocation } from 'react-router-dom';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import { jobsApi } from '../api/jobsApi';
import { recruiterApi } from '../api/recruiterApi';
import { PlusIcon, XIcon, BarChartIcon, ScaleIcon } from '../components/Icons';

export const RecruiterJobs = () => {
  const location = useLocation();
  const [jobs, setJobs] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const [feedbackNotice, setFeedbackNotice] = useState(null);

  // Create / Edit Modal State
  const [modalOpen, setModalOpen] = useState(location.pathname.endsWith('/new'));
  const [editingJob, setEditingJob] = useState(null);
  const [submitting, setSubmitting] = useState(false);

  // Form State
  const [formData, setFormData] = useState({
    jobId: '',
    title: '',
    company: '',
    location: '',
    employmentType: 'Full Time',
    educationRequirement: 'Computer Science',
    minimumExperience: 1,
    requiredSkills: '',
    preferredSkills: '',
    description: '',
  });

  // Delete State
  const [deletingId, setDeletingId] = useState(null);

  const fetchJobs = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await jobsApi.getJobs({ limit: 100 });
      if (res && res.data) {
        setJobs(res.data.items || []);
      }
    } catch (err) {
      setError(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchJobs();
  }, []);

  const openCreateModal = () => {
    setEditingJob(null);
    setFormData({
      jobId: `J_REC_${Date.now().toString().slice(-4)}`,
      title: '',
      company: '',
      location: 'Boston, MA',
      employmentType: 'Full Time',
      educationRequirement: 'Computer Science',
      minimumExperience: 2,
      requiredSkills: 'C++, SQL, Git',
      preferredSkills: 'Docker, Linux',
      description: '',
    });
    setModalOpen(true);
  };

  const openEditModal = (job) => {
    setEditingJob(job);
    setFormData({
      jobId: job.jobId,
      title: job.title,
      company: job.company,
      location: job.location,
      employmentType: job.employmentType,
      educationRequirement: job.educationRequirement,
      minimumExperience: job.minimumExperience,
      requiredSkills: (job.requiredSkills || []).join(', '),
      preferredSkills: (job.preferredSkills || []).join(', '),
      description: job.description,
    });
    setModalOpen(true);
  };

  const handleSubmit = async (e) => {
    e.preventDefault();
    setSubmitting(true);
    setError(null);

    const payload = {
      jobId: formData.jobId.trim(),
      title: formData.title.trim(),
      company: formData.company.trim(),
      location: formData.location.trim(),
      employmentType: formData.employmentType,
      educationRequirement: formData.educationRequirement.trim(),
      minimumExperience: Number(formData.minimumExperience) || 0,
      requiredSkills: formData.requiredSkills.split(',').map((s) => s.trim()).filter(Boolean),
      preferredSkills: formData.preferredSkills.split(',').map((s) => s.trim()).filter(Boolean),
      description: formData.description.trim(),
    };

    try {
      if (editingJob) {
        await recruiterApi.updateJob(editingJob.jobId, payload);
        setFeedbackNotice({ type: 'success', msg: `Job ${editingJob.jobId} updated successfully!` });
      } else {
        await recruiterApi.createJob(payload);
        setFeedbackNotice({ type: 'success', msg: `Job ${payload.jobId} created and published successfully!` });
      }
      setModalOpen(false);
      await fetchJobs();
    } catch (err) {
      setError(err);
    } finally {
      setSubmitting(false);
    }
  };

  const handleDeleteConfirm = async () => {
    if (!deletingId) return;
    try {
      await recruiterApi.deleteJob(deletingId);
      setJobs((prev) => prev.filter((j) => j.jobId !== deletingId));
      setFeedbackNotice({ type: 'info', msg: `Job ${deletingId} deleted successfully.` });
      setDeletingId(null);
    } catch (err) {
      setFeedbackNotice({ type: 'danger', msg: err.message });
      setDeletingId(null);
    }
  };

  return (
    <div className="portal-layout">
      <Sidebar type="recruiter" />

      <main className="portal-main">
        <div className="portal-content">
          <div className="page-header">
            <div>
              <h1 className="page-title">Manage Job Postings</h1>
              <p className="page-subtitle">
                Create new requisitions, update technical skill expectations, or review applicant submissions
              </p>
            </div>
            <button onClick={openCreateModal} className="btn btn-primary">
              <PlusIcon size={16} />
              <span>Post New Job</span>
            </button>
          </div>

          {feedbackNotice && (
            <div style={{
              background: feedbackNotice.type === 'success' ? 'rgba(16, 185, 129, 0.12)' : 'rgba(239, 68, 68, 0.12)',
              border: `1px solid ${feedbackNotice.type === 'success' ? 'rgba(16, 185, 129, 0.3)' : 'rgba(239, 68, 68, 0.3)'}`,
              borderRadius: 'var(--radius-md)',
              padding: '0.85rem 1.25rem',
              color: feedbackNotice.type === 'success' ? '#6ee7b7' : '#fca5a5',
              marginBottom: '1.5rem',
              display: 'flex',
              justifyContent: 'space-between',
              alignItems: 'center'
            }}>
              <span>{feedbackNotice.msg}</span>
              <button
                onClick={() => setFeedbackNotice(null)}
                style={{ background: 'transparent', border: 'none', color: 'inherit', cursor: 'pointer', display: 'flex', alignItems: 'center' }}
              >
                <XIcon size={16} />
              </button>
            </div>
          )}

          <ErrorMessage error={error} onRetry={fetchJobs} />

          {loading ? (
            <Loading message="Loading job requisitions..." />
          ) : jobs.length === 0 ? (
            <div className="card" style={{ textAlign: 'center', padding: '3.5rem 1rem' }}>
              <p style={{ color: 'var(--text-muted)', marginBottom: '1.25rem' }}>
                No job postings created yet.
              </p>
              <button onClick={openCreateModal} className="btn btn-primary">
                Create First Job Posting
              </button>
            </div>
          ) : (
            <div className="table-container">
              <table className="data-table">
                <thead>
                  <tr>
                    <th>Job ID</th>
                    <th>Position Title</th>
                    <th>Company</th>
                    <th>Location</th>
                    <th>Required Skills</th>
                    <th>Actions</th>
                  </tr>
                </thead>
                <tbody>
                  {jobs.map((job) => (
                    <tr key={job.jobId}>
                      <td style={{ fontFamily: 'var(--font-mono)', fontWeight: 600 }}>{job.jobId}</td>
                      <td style={{ fontWeight: 600 }}>{job.title}</td>
                      <td>{job.company}</td>
                      <td>{job.location || 'Remote'}</td>
                      <td>
                        <span style={{ fontSize: '0.825rem', color: 'var(--text-muted)' }}>
                          {(job.requiredSkills || []).slice(0, 3).join(', ')}
                          {(job.requiredSkills || []).length > 3 ? ` +${job.requiredSkills.length - 3}` : ''}
                        </span>
                      </td>
                      <td>
                        <div style={{ display: 'flex', gap: '0.4rem', flexWrap: 'wrap' }}>
                          <button onClick={() => openEditModal(job)} className="btn btn-sm btn-secondary">
                            Edit
                          </button>
                          <Link to={`/recruiter/jobs/${job.jobId}/analytics`} className="btn btn-sm btn-primary" title="Job Analytics" style={{ gap: '0.3rem' }}>
                            <BarChartIcon size={14} />
                            <span>Analytics</span>
                          </Link>
                          <Link to={`/recruiter/jobs/${job.jobId}/compare`} className="btn btn-sm btn-outline" title="Compare Applicants" style={{ gap: '0.3rem' }}>
                            <ScaleIcon size={14} />
                            <span>Compare</span>
                          </Link>
                          <Link to="/recruiter/applicants" className="btn btn-sm btn-secondary">
                            Applicants
                          </Link>
                          <button
                            onClick={() => setDeletingId(job.jobId)}
                            className="btn btn-sm btn-danger"
                            title="Delete job requisition"
                            style={{ padding: '0.42rem 0.55rem' }}
                          >
                            <XIcon size={14} />
                          </button>
                        </div>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          )}
        </div>
      </main>

      {/* Create / Edit Modal */}
      {modalOpen && (
        <div className="modal-overlay">
          <div className="modal-content" style={{ maxWidth: '640px' }}>
            <h3 style={{ fontSize: '1.4rem', marginBottom: '0.25rem' }}>
              {editingJob ? `Edit Job #${editingJob.jobId}` : 'Create New Job Opening'}
            </h3>
            <p style={{ color: 'var(--text-muted)', fontSize: '0.875rem', marginBottom: '1.5rem' }}>
              Define the requirements, required skills, and candidate profile
            </p>

            <form onSubmit={handleSubmit}>
              <div style={{ display: 'grid', gridTemplateColumns: '1fr 2fr', gap: '1rem' }}>
                <div className="form-group">
                  <label className="form-label">Job ID *</label>
                  <input
                    type="text"
                    className="form-input"
                    value={formData.jobId}
                    onChange={(e) => setFormData({ ...formData, jobId: e.target.value })}
                    disabled={!!editingJob}
                    required
                  />
                </div>
                <div className="form-group">
                  <label className="form-label">Position Title *</label>
                  <input
                    type="text"
                    className="form-input"
                    placeholder="e.g. Senior C++ Engineer"
                    value={formData.title}
                    onChange={(e) => setFormData({ ...formData, title: e.target.value })}
                    required
                  />
                </div>
              </div>

              <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '1rem' }}>
                <div className="form-group">
                  <label className="form-label">Company Name *</label>
                  <input
                    type="text"
                    className="form-input"
                    value={formData.company}
                    onChange={(e) => setFormData({ ...formData, company: e.target.value })}
                    required
                  />
                </div>
                <div className="form-group">
                  <label className="form-label">Location</label>
                  <input
                    type="text"
                    className="form-input"
                    placeholder="e.g. Boston, MA / Remote"
                    value={formData.location}
                    onChange={(e) => setFormData({ ...formData, location: e.target.value })}
                  />
                </div>
              </div>

              <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr 1fr', gap: '1rem' }}>
                <div className="form-group">
                  <label className="form-label">Employment Type</label>
                  <select
                    className="form-select"
                    value={formData.employmentType}
                    onChange={(e) => setFormData({ ...formData, employmentType: e.target.value })}
                  >
                    <option value="Full Time">Full Time</option>
                    <option value="Part Time">Part Time</option>
                    <option value="Contract">Contract</option>
                    <option value="Remote">Remote</option>
                  </select>
                </div>
                <div className="form-group">
                  <label className="form-label">Min. Experience (Yrs)</label>
                  <input
                    type="number"
                    min="0"
                    className="form-input"
                    value={formData.minimumExperience}
                    onChange={(e) => setFormData({ ...formData, minimumExperience: e.target.value })}
                  />
                </div>
                <div className="form-group">
                  <label className="form-label">Education Discipline</label>
                  <input
                    type="text"
                    className="form-input"
                    placeholder="e.g. Computer Science"
                    value={formData.educationRequirement}
                    onChange={(e) => setFormData({ ...formData, educationRequirement: e.target.value })}
                  />
                </div>
              </div>

              <div className="form-group">
                <label className="form-label">Required Skills (Comma-separated) *</label>
                <input
                  type="text"
                  className="form-input"
                  placeholder="e.g. C++, SQL, Linux"
                  value={formData.requiredSkills}
                  onChange={(e) => setFormData({ ...formData, requiredSkills: e.target.value })}
                  required
                />
              </div>

              <div className="form-group">
                <label className="form-label">Preferred Skills (Comma-separated)</label>
                <input
                  type="text"
                  className="form-input"
                  placeholder="e.g. Docker, Python, ROS"
                  value={formData.preferredSkills}
                  onChange={(e) => setFormData({ ...formData, preferredSkills: e.target.value })}
                />
              </div>

              <div className="form-group">
                <label className="form-label">Role Description</label>
                <textarea
                  className="form-textarea"
                  rows={3}
                  placeholder="Summarize the core engineering responsibilities..."
                  value={formData.description}
                  onChange={(e) => setFormData({ ...formData, description: e.target.value })}
                />
              </div>

              <div style={{ display: 'flex', justifyContent: 'flex-end', gap: '0.75rem', marginTop: '1.5rem' }}>
                <button
                  type="button"
                  className="btn btn-secondary"
                  disabled={submitting}
                  onClick={() => setModalOpen(false)}
                >
                  Cancel
                </button>
                <button
                  type="submit"
                  className="btn btn-primary"
                  disabled={submitting}
                >
                  {submitting ? 'Saving...' : editingJob ? 'Update Requisition' : 'Publish Opening'}
                </button>
              </div>
            </form>
          </div>
        </div>
      )}

      {/* Delete Confirmation Modal */}
      {deletingId && (
        <div className="modal-overlay">
          <div className="modal-content" style={{ maxWidth: '440px' }}>
            <h3 style={{ fontSize: '1.25rem', marginBottom: '0.75rem' }}>Delete Job Posting</h3>
            <p style={{ color: 'var(--text-muted)', fontSize: '0.925rem', marginBottom: '1.5rem', lineHeight: 1.5 }}>
              Are you sure you want to delete job <strong>{deletingId}</strong>? All associated application submissions will also be removed.
            </p>
            <div style={{ display: 'flex', gap: '0.75rem', justifyContent: 'flex-end' }}>
              <button
                className="btn btn-secondary"
                onClick={() => setDeletingId(null)}
              >
                Cancel
              </button>
              <button
                className="btn btn-danger"
                onClick={handleDeleteConfirm}
              >
                Confirm Delete
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default RecruiterJobs;
