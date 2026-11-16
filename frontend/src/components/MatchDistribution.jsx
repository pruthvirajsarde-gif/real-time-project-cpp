import React from 'react';

const BUCKETS = [
  { key: '90-100%', label: '90 – 100%', color: '#10b981', desc: 'Exceptional Alignment' },
  { key: '80-89%', label: '80 – 89%', color: '#06b6d4', desc: 'Strong Match' },
  { key: '70-79%', label: '70 – 79%', color: '#6366f1', desc: 'Good Fit' },
  { key: '60-69%', label: '60 – 69%', color: '#f59e0b', desc: 'Moderate Match' },
  { key: '<60%', label: '< 60%', color: '#ef4444', desc: 'Low Alignment' },
];

export const MatchDistribution = ({ distribution = {} }) => {
  const total = Object.values(distribution).reduce((acc, curr) => acc + (Number(curr) || 0), 0);

  if (total === 0) {
    return (
      <div style={{ textAlign: 'center', padding: '2rem 1rem', color: 'var(--text-muted)' }}>
        No match score data available yet.
      </div>
    );
  }

  const maxCount = Math.max(...Object.values(distribution).map(Number), 1);

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: '0.85rem', width: '100%' }}>
      {BUCKETS.map((b) => {
        const count = Number(distribution[b.key]) || 0;
        const pct = total > 0 ? Math.round((count / total) * 100) : 0;
        const barHeight = Math.max(Math.round((count / maxCount) * 100), 2);

        return (
          <div key={b.key} style={{ display: 'flex', flexDirection: 'column', gap: '0.25rem' }}>
            <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: '0.85rem' }}>
              <span style={{ fontWeight: 600, color: 'var(--text)' }}>
                {b.label}
                <span style={{ color: 'var(--text-muted)', marginLeft: '0.4rem', fontWeight: 400, fontSize: '0.75rem' }}>
                  ({b.desc})
                </span>
              </span>
              <span style={{ fontWeight: 700, color: b.color }}>
                {count} {count === 1 ? 'applicant' : 'applicants'} ({pct}%)
              </span>
            </div>

            <div style={{
              width: '100%',
              height: '8px',
              backgroundColor: 'rgba(255, 255, 255, 0.08)',
              borderRadius: 'var(--radius-full)',
              overflow: 'hidden',
            }}>
              <div
                style={{
                  width: `${pct}%`,
                  height: '100%',
                  backgroundColor: b.color,
                  borderRadius: 'var(--radius-full)',
                  transition: 'width 0.4s ease',
                }}
              />
            </div>
          </div>
        );
      })}
    </div>
  );
};

export default MatchDistribution;
