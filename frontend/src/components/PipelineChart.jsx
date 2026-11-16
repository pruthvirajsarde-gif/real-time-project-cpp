import React from 'react';
import {
  SendIcon,
  ClockIcon,
  StarIcon,
  UsersIcon,
  CheckIcon,
  XIcon
} from './Icons';

const STAGE_CONFIG = [
  { key: 'APPLIED', label: 'Applied', color: '#6366f1', Icon: SendIcon },
  { key: 'UNDER_REVIEW', label: 'Under Review', color: '#f59e0b', Icon: ClockIcon },
  { key: 'SHORTLISTED', label: 'Shortlisted', color: '#06b6d4', Icon: StarIcon },
  { key: 'INTERVIEW', label: 'Interview', color: '#8b5cf6', Icon: UsersIcon },
  { key: 'SELECTED', label: 'Selected / Offer', color: '#10b981', Icon: CheckIcon },
  { key: 'REJECTED', label: 'Rejected', color: '#ef4444', Icon: XIcon },
];

export const PipelineChart = ({ stages = {}, total = 0 }) => {
  const effectiveTotal = total || Object.values(stages).reduce((acc, curr) => acc + (Number(curr) || 0), 0);

  if (effectiveTotal === 0) {
    return (
      <div style={{ textAlign: 'center', padding: '2rem 1rem', color: 'var(--text-muted)' }}>
        No applications in the pipeline yet.
      </div>
    );
  }

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: '0.75rem', width: '100%' }}>
      <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(130px, 1fr))', gap: '0.75rem' }}>
        {STAGE_CONFIG.map((stage) => {
          const count = Number(stages[stage.key]) || 0;
          const pct = effectiveTotal > 0 ? Math.round((count / effectiveTotal) * 100) : 0;
          const IconComponent = stage.Icon;

          return (
            <div
              key={stage.key}
              style={{
                background: 'var(--surface)',
                border: '1px solid var(--border-card)',
                borderRadius: 'var(--radius-md)',
                padding: '0.85rem',
                display: 'flex',
                flexDirection: 'column',
                gap: '0.4rem',
                borderTop: `3px solid ${stage.color}`,
              }}
            >
              <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                <span style={{ fontSize: '0.75rem', fontWeight: 600, color: 'var(--text-muted)', display: 'inline-flex', alignItems: 'center', gap: '0.35rem' }}>
                  <IconComponent size={12} color={stage.color} />
                  <span>{stage.label}</span>
                </span>
                <span style={{ fontSize: '0.75rem', fontWeight: 700, color: stage.color }}>
                  {pct}%
                </span>
              </div>
              <span style={{ fontSize: '1.5rem', fontWeight: 800, color: 'var(--text)' }}>
                {count}
              </span>
            </div>
          );
        })}
      </div>

      {/* Aggregate Visual Funnel Bar */}
      <div style={{
        width: '100%',
        height: '12px',
        backgroundColor: 'rgba(255, 255, 255, 0.06)',
        borderRadius: 'var(--radius-full)',
        overflow: 'hidden',
        display: 'flex',
        marginTop: '0.5rem',
      }}>
        {STAGE_CONFIG.map((stage) => {
          const count = Number(stages[stage.key]) || 0;
          const pct = effectiveTotal > 0 ? (count / effectiveTotal) * 100 : 0;
          if (pct === 0) return null;

          return (
            <div
              key={stage.key}
              style={{
                width: `${pct}%`,
                height: '100%',
                backgroundColor: stage.color,
                transition: 'width 0.4s ease',
              }}
              title={`${stage.label}: ${count} (${Math.round(pct)}%)`}
            />
          );
        })}
      </div>
    </div>
  );
};

export default PipelineChart;
