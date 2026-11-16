import React from 'react';
import { formatScore, getScoreColor } from '../utils/formatters';

export const ScoreCard = ({
  score = 0,
  label = 'Match Score',
  classification = null,
  size = 'md',
}) => {
  const color = getScoreColor(score);
  const formatted = formatScore(score);

  const getTier = (s) => {
    const num = Number(s) || 0;
    if (num >= 80) return 'Strong Match';
    if (num >= 60) return 'Good Match';
    if (num >= 40) return 'Partial Match';
    return 'Low Match';
  };

  const tier = classification || getTier(score);

  return (
    <div
      style={{
        background: 'var(--surface-card)',
        border: '1px solid var(--border-card)',
        borderRadius: 'var(--radius-lg)',
        padding: size === 'lg' ? '2rem' : '1.25rem',
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        textAlign: 'center',
        position: 'relative',
        overflow: 'hidden',
      }}
    >
      <div
        style={{
          width: size === 'lg' ? '120px' : '84px',
          height: size === 'lg' ? '120px' : '84px',
          borderRadius: '50%',
          border: `4px solid ${color}`,
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          flexDirection: 'column',
          boxShadow: `0 0 20px ${color}33`,
          marginBottom: '0.85rem',
          backgroundColor: 'rgba(15, 23, 42, 0.4)',
        }}
      >
        <span
          style={{
            fontSize: size === 'lg' ? '1.85rem' : '1.35rem',
            fontWeight: 800,
            color: '#fff',
            lineHeight: 1,
          }}
        >
          {formatted}
        </span>
      </div>

      <div style={{ fontSize: '0.9rem', fontWeight: 600, color: 'var(--text-muted)' }}>
        {label}
      </div>

      {tier && (
        <span
          style={{
            marginTop: '0.5rem',
            fontSize: '0.8rem',
            fontWeight: 700,
            color: color,
            padding: '0.2rem 0.65rem',
            borderRadius: 'var(--radius-full)',
            background: `${color}15`,
            border: `1px solid ${color}33`,
          }}
        >
          {tier}
        </span>
      )}
    </div>
  );
};

export default ScoreCard;
