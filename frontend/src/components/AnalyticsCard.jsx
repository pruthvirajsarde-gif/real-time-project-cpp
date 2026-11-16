import React from 'react';

export const AnalyticsCard = ({ title, value, subtitle, icon, badge, badgeType = 'primary', style = {} }) => {
  return (
    <div
      className="card"
      style={{
        padding: '1.4rem 1.6rem',
        position: 'relative',
        background: 'var(--surface-card)',
        border: '1px solid var(--border-card)',
        display: 'flex',
        flexDirection: 'column',
        justifyContent: 'space-between',
        boxShadow: 'var(--shadow-sm)',
        ...style,
      }}
    >
      <div>
        <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '0.85rem' }}>
          <span style={{ fontSize: '0.775rem', color: 'var(--text-muted)', fontWeight: 700, textTransform: 'uppercase', letterSpacing: '0.05em' }}>
            {title}
          </span>
          {icon && (
            <div style={{
              width: '38px',
              height: '38px',
              borderRadius: 'var(--radius-md)',
              background: 'rgba(99, 102, 241, 0.12)',
              border: '1px solid rgba(99, 102, 241, 0.25)',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              color: '#a5b4fc',
              flexShrink: 0,
            }}>
              {icon}
            </div>
          )}
        </div>

        <div style={{ display: 'flex', alignItems: 'baseline', gap: '0.75rem', marginBottom: '0.35rem' }}>
          <span style={{ fontSize: '2rem', fontWeight: 800, color: 'var(--text-main)', letterSpacing: '-0.02em', lineHeight: 1.1 }}>
            {value}
          </span>
          {badge && (
            <span className={`badge badge-${badgeType}`} style={{ fontSize: '0.72rem', padding: '0.2rem 0.55rem' }}>
              {badge}
            </span>
          )}
        </div>
      </div>

      {subtitle && (
        <p style={{ margin: '0.4rem 0 0 0', fontSize: '0.825rem', color: 'var(--text-muted)' }}>
          {subtitle}
        </p>
      )}
    </div>
  );
};

export default AnalyticsCard;
