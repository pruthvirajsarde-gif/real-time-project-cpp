import React from 'react';

export const BarChart = ({
  items = [],
  maxVal = null,
  color = 'var(--primary)',
  showPercentage = true,
  emptyMessage = 'No data available for display.',
  style = {}
}) => {
  if (!items || items.length === 0) {
    return (
      <div style={{ textAlign: 'center', padding: '2rem 1rem', color: 'var(--text-muted)', fontSize: '0.9rem' }}>
        {emptyMessage}
      </div>
    );
  }

  // Calculate highest value for relative scaling if not provided
  const highest = maxVal || Math.max(...items.map((i) => i.value || i.count || 0), 1);

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: '0.85rem', width: '100%', ...style }}>
      {items.map((item, idx) => {
        const val = item.value ?? item.count ?? 0;
        const pct = item.percentage ?? (highest > 0 ? Math.round((val / highest) * 100) : 0);
        const barColor = item.color || color;

        return (
          <div key={item.label || idx} style={{ width: '100%' }}>
            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '0.35rem', fontSize: '0.875rem' }}>
              <span style={{ fontWeight: 600, color: 'var(--text)' }}>
                {item.label}
                {item.sublabel && (
                  <span style={{ fontSize: '0.75rem', color: 'var(--text-muted)', marginLeft: '0.4rem', fontWeight: 400 }}>
                    ({item.sublabel})
                  </span>
                )}
              </span>
              <span style={{ fontWeight: 700, color: 'var(--text-muted)', fontSize: '0.85rem' }}>
                {item.displayValue || val}
                {showPercentage && item.percentage !== undefined && (
                  <span style={{ marginLeft: '0.35rem', color: 'var(--text-secondary)' }}>
                    ({item.percentage}%)
                  </span>
                )}
              </span>
            </div>

            {/* Track & Bar */}
            <div style={{
              width: '100%',
              height: '8px',
              backgroundColor: 'rgba(255, 255, 255, 0.08)',
              borderRadius: 'var(--radius-full)',
              overflow: 'hidden',
            }}>
              <div
                style={{
                  width: `${Math.min(pct, 100)}%`,
                  height: '100%',
                  background: barColor,
                  borderRadius: 'var(--radius-full)',
                  transition: 'width 0.4s ease-in-out',
                }}
                aria-label={`${item.label}: ${val}`}
                role="progressbar"
                aria-valuenow={val}
                aria-valuemin="0"
                aria-valuemax={highest}
              />
            </div>
          </div>
        );
      })}
    </div>
  );
};

export default BarChart;
