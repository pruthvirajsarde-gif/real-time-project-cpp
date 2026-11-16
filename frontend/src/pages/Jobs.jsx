import React, { useState, useEffect } from 'react';
import Navbar from '../components/Navbar';
import Sidebar from '../components/Sidebar';
import JobCard from '../components/JobCard';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import { jobsApi } from '../api/jobsApi';
import { matchingApi } from '../api/matchingApi';
import { savedJobsApi } from '../api/savedJobsApi';
import { applicationApi } from '../api/applicationApi';
import { useAuth } from '../context/AuthContext';
import { SearchIcon, XIcon } from '../components/Icons';

export const Jobs = () => {
  const { isAuthenticated, role } = useAuth();
  const isCandidate = isAuthenticated && role === 'CANDIDATE';

  const [jobs, setJobs] = useState([]);
  const [total, setTotal] = useState(0);
  const [totalPages, setTotalPages] = useState(1);
  const [page, setPage] = useState(1);
  const [limit, setLimit] = useState(6);

  // Filters
  const [titleFilter, setTitleFilter] = useState('');
  const [companyFilter, setCompanyFilter] = useState('');
  const [locationFilter, setLocationFilter] = useState('');
  const [skillFilter, setSkillFilter] = useState('');
  const [typeFilter, setTypeFilter] = useState('');

  // Candidate Match & Saved state
  const [matchMap, setMatchMap] = useState({});
  const [savedSet, setSavedSet] = useState(new Set());

  // Apply Modal state
  const [applyingJob, setApplyingJob] = useState(null);
  const [applySubmitting, setApplySubmitting] = useState(false);
  const [feedbackNotice, setFeedbackNotice] = useState(null);

  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const fetchJobs = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await jobsApi.getJobs({
        page,
        limit,
        title: titleFilter.trim(),
        company: companyFilter.trim(),
        location: locationFilter.trim(),
        skill: skillFilter.trim(),
        employmentType: typeFilter,
      });

      if (res && res.data) {
        setJobs(res.data.items || []);
        setTotal(res.data.total || 0);
        setTotalPages(res.data.totalPages || 1);
      }

      // If candidate, load match scores and saved bookmarks
      if (isCandidate) {
        try {
          const [matchesRes, savedRes] = await Promise.allSettled([
            matchingApi.getMatches({ limit: 50 }),
            savedJobsApi.getSavedJobs(),
          ]);

          if (matchesRes.status === 'fulfilled' && Array.isArray(matchesRes.value?.data)) {
            const map = {};
            matchesRes.value.data.forEach((m) => {
              map[m.jobId] = m.overallScore;
            });
            setMatchMap(map);
          }

          if (savedRes.status === 'fulfilled' && Array.isArray(savedRes.value?.data)) {
            const set = new Set();
            savedRes.value.data.forEach((s) => set.add(s.jobId));
            setSavedSet(set);
          }
        } catch (e) {
          // Non-critical background fetch failure
        }
      }
    } catch (err) {
      setError(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchJobs();
  }, [page, limit, typeFilter]);

  const handleSearchSubmit = (e) => {
    e.preventDefault();
    setPage(1);
    fetchJobs();
  };

  const handleResetFilters = () => {
    setTitleFilter('');
    setCompanyFilter('');
    setLocationFilter('');
    setSkillFilter('');
    setTypeFilter('');
    setPage(1);
  };

  const handleSaveToggle = async (jobId) => {
    try {
      if (savedSet.has(jobId)) {
        await savedJobsApi.removeSavedJob(jobId);
        setSavedSet((prev) => {
          const next = new Set(prev);
          next.delete(jobId);
          return next;
        });
        setFeedbackNotice({ type: 'info', msg: `Job ${jobId} removed from saved list.` });
      } else {
        await savedJobsApi.saveJob(jobId);
        setSavedSet((prev) => new Set(prev).add(jobId));
        setFeedbackNotice({ type: 'success', msg: `Job ${jobId} saved to your bookmarks!` });
      }
    } catch (err) {
      setFeedbackNotice({ type: 'danger', msg: err.message });
    }
  };

  const handleApplyConfirm = async () => {
    if (!applyingJob) return;
    setApplySubmitting(true);
    try {
      await applicationApi.applyForJob(applyingJob.id);
      setFeedbackNotice({
        type: 'success',
        msg: `Successfully applied to ${applyingJob.title}! Check your Applications tab for status updates.`
      });
      setApplyingJob(null);
    } catch (err) {
      setFeedbackNotice({ type: 'danger', msg: err.message });
      setApplyingJob(null);
    } finally {
      setApplySubmitting(false);
    }
  };

  const content = (
    <div className="portal-content">
      <div className="page-header">
        <div>
          <h1 className="page-title">Explore Career Opportunities</h1>
          <p className="page-subtitle">
            Find openings matching your technical profile or filter by skill and company criteria
          </p>
        </div>
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
            <XIcon size={14} />
          </button>
        </div>
      )}

      {/* Filter & Search Bar */}
      <form onSubmit={handleSearchSubmit} className="card" style={{ marginBottom: '2rem', padding: '1.25rem' }}>
        <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(200px, 1fr))', gap: '1rem', marginBottom: '1rem' }}>
          <div>
            <label className="form-label">Job Title / Keyword</label>
            <input
              type="text"
              className="form-input"
              placeholder="e.g. Systems Engineer"
              value={titleFilter}
              onChange={(e) => setTitleFilter(e.target.value)}
            />
          </div>

          <div>
            <label className="form-label">Company Name</label>
            <input
              type="text"
              className="form-input"
              placeholder="e.g. CyberDyne"
              value={companyFilter}
              onChange={(e) => setCompanyFilter(e.target.value)}
            />
          </div>

          <div>
            <label className="form-label">Required Skill</label>
            <input
              type="text"
              className="form-input"
              placeholder="e.g. C++ or SQL"
              value={skillFilter}
              onChange={(e) => setSkillFilter(e.target.value)}
            />
          </div>

          <div>
            <label className="form-label">Employment Type</label>
            <select
              className="form-select"
              value={typeFilter}
              onChange={(e) => setTypeFilter(e.target.value)}
            >
              <option value="">All Types</option>
              <option value="Full Time">Full Time</option>
              <option value="Part Time">Part Time</option>
              <option value="Contract">Contract</option>
              <option value="Remote">Remote</option>
            </select>
          </div>
        </div>

        <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', flexWrap: 'wrap', gap: '0.75rem' }}>
          <div style={{ fontSize: '0.85rem', color: 'var(--text-muted)' }}>
            Showing <strong>{jobs.length}</strong> of <strong>{total}</strong> open positions
          </div>

          <div style={{ display: 'flex', gap: '0.5rem' }}>
            <button type="button" onClick={handleResetFilters} className="btn btn-sm btn-outline">
              Clear Filters
            </button>
            <button type="submit" className="btn btn-sm btn-primary" style={{ display: 'inline-flex', alignItems: 'center', gap: '0.4rem' }}>
              <SearchIcon size={14} />
              <span>Filter Results</span>
            </button>
          </div>
        </div>
      </form>


      <ErrorMessage error={error} onRetry={fetchJobs} />

      {/* Jobs Grid */}
      {loading ? (
        <Loading message="Fetching job openings..." />
      ) : jobs.length === 0 ? (
        <div className="card" style={{ textAlign: 'center', padding: '3.5rem 1rem' }}>
          <p style={{ color: 'var(--text-muted)', fontSize: '1rem', marginBottom: '1rem' }}>
            No matching job openings found for your criteria.
          </p>
          <button onClick={handleResetFilters} className="btn btn-sm btn-secondary">
            Reset Filters
          </button>
        </div>
      ) : (
        <>
          <div style={{
            display: 'grid',
            gridTemplateColumns: 'repeat(auto-fit, minmax(340px, 1fr))',
            gap: '1.5rem',
            marginBottom: '2.5rem'
          }}>
            {jobs.map((job) => (
              <JobCard
                key={job.jobId}
                job={job}
                matchScore={matchMap[job.jobId] ?? null}
                isSaved={savedSet.has(job.jobId)}
                onSaveToggle={isCandidate ? handleSaveToggle : null}
                onApply={isCandidate ? (id, title) => setApplyingJob({ id, title }) : null}
                isCandidate={isCandidate}
              />
            ))}
          </div>

          {/* Pagination Controls */}
          {totalPages > 1 && (
            <div style={{
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              gap: '0.5rem',
              marginTop: '1rem',
            }}>
              <button
                className="btn btn-sm btn-secondary"
                disabled={page <= 1}
                onClick={() => setPage(page - 1)}
              >
                Previous
              </button>

              {Array.from({ length: totalPages }, (_, i) => i + 1).map((p) => (
                <button
                  key={p}
                  className={`btn btn-sm ${p === page ? 'btn-primary' : 'btn-outline'}`}
                  style={{ minWidth: '36px' }}
                  onClick={() => setPage(p)}
                >
                  {p}
                </button>
              ))}

              <button
                className="btn btn-sm btn-secondary"
                disabled={page >= totalPages}
                onClick={() => setPage(page + 1)}
              >
                Next
              </button>
            </div>
          )}
        </>
      )}

      {/* Apply Confirmation Modal */}
      {applyingJob && (
        <div className="modal-overlay">
          <div className="modal-content" style={{ maxWidth: '480px' }}>
            <h3 style={{ fontSize: '1.25rem', marginBottom: '0.75rem' }}>Confirm Job Application</h3>
            <p style={{ color: 'var(--text-muted)', fontSize: '0.925rem', marginBottom: '1.5rem', lineHeight: 1.5 }}>
              Are you sure you want to apply for <strong>{applyingJob.title}</strong>? Your active resume and contact profile will be submitted for recruiter review.
            </p>
            <div style={{ display: 'flex', gap: '0.75rem', justifyContent: 'flex-end' }}>
              <button
                className="btn btn-secondary"
                disabled={applySubmitting}
                onClick={() => setApplyingJob(null)}
              >
                Cancel
              </button>
              <button
                className="btn btn-primary"
                disabled={applySubmitting}
                onClick={handleApplyConfirm}
              >
                {applySubmitting ? 'Submitting...' : 'Confirm Application'}
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );

  if (isCandidate) {
    return (
      <div className="portal-layout">
        <Sidebar type="candidate" />
        <main className="portal-main">{content}</main>
      </div>
    );
  }

  return (
    <div className="app-container">
      <Navbar />
      <main style={{ maxWidth: '1400px', width: '100%', margin: '0 auto', padding: '2rem 1.5rem' }}>
        {content}
      </main>
    </div>
  );
};

export default Jobs;
