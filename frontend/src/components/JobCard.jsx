import React from 'react';
import { Link } from 'react-router-dom';
import SkillBadge from './SkillBadge';
import { formatScore, getScoreBadgeClass } from '../utils/formatters';
import { MapPinIcon, BriefcaseIcon, ClockIcon, BookmarkIcon } from './Icons';

export const JobCard = ({
  job,
  matchScore = null,
  isSaved = false,
  onSaveToggle = null,
  onApply = null,
  isCandidate = false,
}) => {
  const reqSkills = job.requiredSkills || [];
  const prefSkills = job.preferredSkills || [];

  return (
    <div className="card card-interactive" style={{ display: 'flex', flexDirection: 'column', height: '100%' }}>
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start', gap: '1rem', marginBottom: '0.75rem' }}>
        <div>
          <span style={{ fontSize: '0.8rem', fontWeight: 700, color: 'var(--primary)', textTransform: 'uppercase', letterSpacing: '0.05em' }}>
            {job.company}
          </span>
          <h3 style={{ fontSize: '1.25rem', marginTop: '0.2rem', marginBottom: '0.25rem' }}>
            {job.title}
          </h3>
          <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.75rem', fontSize: '0.85rem', color: 'var(--text-muted)' }}>
            <span style={{ display: 'inline-flex', alignItems: 'center', gap: '0.3rem' }}>
              <MapPinIcon size={13} /> {job.location || 'Remote'}
            </span>
            <span style={{ display: 'inline-flex', alignItems: 'center', gap: '0.3rem' }}>
              <BriefcaseIcon size={13} /> {job.employmentType || 'Full Time'}
            </span>
            {job.minimumExperience !== undefined && (
              <span style={{ display: 'inline-flex', alignItems: 'center', gap: '0.3rem' }}>
                <ClockIcon size={13} /> {job.minimumExperience} yr{job.minimumExperience === 1 ? '' : 's'} exp
              </span>
            )}
          </div>
        </div>

        {matchScore !== null && matchScore !== undefined && (
          <div style={{ textAlign: 'right', flexShrink: 0 }}>
            <span className={`badge ${getScoreBadgeClass(matchScore)}`} style={{ fontSize: '0.85rem', padding: '0.35rem 0.75rem' }}>
              {formatScore(matchScore)} Match
            </span>
          </div>
        )}
      </div>

      <p style={{
        fontSize: '0.9rem',
        color: 'var(--text-muted)',
        marginBottom: '1rem',
        display: '-webkit-box',
        WebkitLineClamp: 2,
        WebkitBoxOrient: 'vertical',
        overflow: 'hidden',
        lineHeight: 1.5,
      }}>
        {job.description || 'No description provided.'}
      </p>

      <div style={{ marginTop: 'auto', marginBottom: '1.25rem' }}>
        <div style={{ fontSize: '0.75rem', fontWeight: 700, color: 'var(--text-dim)', marginBottom: '0.4rem', textTransform: 'uppercase' }}>
          Key Skills
        </div>
        <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.3rem', maxHeight: '68px', overflow: 'hidden' }}>
          {reqSkills.slice(0, 4).map((s, idx) => (
            <SkillBadge key={idx} name={s} />
          ))}
          {prefSkills.slice(0, 2).map((s, idx) => (
            <SkillBadge key={`pref-${idx}`} name={s} type="category" />
          ))}
          {reqSkills.length + prefSkills.length > 6 && (
            <span style={{ fontSize: '0.75rem', color: 'var(--text-dim)', alignSelf: 'center', marginLeft: '0.25rem' }}>
              +{reqSkills.length + prefSkills.length - 6} more
            </span>
          )}
        </div>
      </div>

      <div style={{
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'space-between',
        paddingTop: '0.85rem',
        borderTop: '1px solid var(--border-subtle)',
        gap: '0.5rem',
      }}>
        <div style={{ display: 'flex', gap: '0.5rem' }}>
          <Link
            to={isCandidate ? `/candidate/jobs/${job.jobId}` : `/jobs/${job.jobId}`}
            className="btn btn-sm btn-secondary"
          >
            View Details
          </Link>
          {isCandidate && matchScore !== null && (
            <Link
              to={`/candidate/matches/${job.jobId}`}
              className="btn btn-sm btn-outline"
            >
              Analyze Match
            </Link>
          )}
        </div>

        <div style={{ display: 'flex', gap: '0.5rem' }}>
          {isCandidate && onSaveToggle && (
            <button
              onClick={() => onSaveToggle(job.jobId)}
              className="btn btn-sm btn-outline"
              title={isSaved ? 'Remove bookmark' : 'Bookmark job'}
              style={{ display: 'inline-flex', alignItems: 'center', gap: '0.35rem', color: isSaved ? '#f59e0b' : 'inherit' }}
            >
              <BookmarkIcon size={14} filled={isSaved} />
              <span>{isSaved ? 'Saved' : 'Save'}</span>
            </button>
          )}

          {isCandidate && onApply && (
            <button
              onClick={() => onApply(job.jobId, job.title)}
              className="btn btn-sm btn-primary"
            >
              Apply Now
            </button>
          )}
        </div>
      </div>
    </div>
  );
};

export default JobCard;
