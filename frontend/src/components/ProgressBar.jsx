import React from 'react';
import { getScoreColor, formatScore } from '../utils/formatters';

export const ProgressBar = ({ label, value = 0, max = 100, showValue = true, customColor = null }) => {
  const percent = Math.min(Math.max((value / max) * 100, 0), 100);
  const color = customColor || getScoreColor(percent);

  return (
    <div style={{ marginBottom: '1rem', width: '100%' }}>
      <div style={{
        display: 'flex',
        justifyContent: 'space-between',
        alignItems: 'center',
        marginBottom: '0.4rem',
        fontSize: '0.875rem'
      }}>
        <span style={{ fontWeight: 600, color: 'var(--text-main)' }}>{label}</span>
        {showValue && (
          <span style={{ fontWeight: 700, color: color, fontFamily: 'var(--font-mono)' }}>
            {formatScore(value)}
          </span>
        )}
      </div>
      <div style={{
        width: '100%',
        height: '8px',
        backgroundColor: 'rgba(255, 255, 255, 0.08)',
        borderRadius: 'var(--radius-full)',
        overflow: 'hidden',
      }}>
        <div style={{
          width: `${percent}%`,
          height: '100%',
          backgroundColor: color,
          borderRadius: 'var(--radius-full)',
          transition: 'width 0.6s cubic-bezier(0.16, 1, 0.3, 1)',
          boxShadow: `0 0 10px ${color}66`
        }} />
      </div>
    </div>
  );
};

export default ProgressBar;
