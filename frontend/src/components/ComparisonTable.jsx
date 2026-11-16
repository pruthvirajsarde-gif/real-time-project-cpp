import React from 'react';

export const ComparisonTable = ({ columns = [], data = [], emptyMessage = 'No comparison records available.' }) => {
  if (!data || data.length === 0) {
    return (
      <div style={{ textAlign: 'center', padding: '2.5rem 1rem', color: 'var(--text-muted)' }}>
        {emptyMessage}
      </div>
    );
  }

  return (
    <div style={{ overflowX: 'auto', width: '100%' }}>
      <table style={{
        width: '100%',
        borderCollapse: 'separate',
        borderSpacing: '0',
        fontSize: '0.9rem',
      }}>
        <thead>
          <tr style={{ background: 'var(--surface-hover)', borderBottom: '1px solid var(--border-card)' }}>
            {columns.map((col, idx) => (
              <th
                key={col.key || idx}
                style={{
                  padding: '0.85rem 1rem',
                  textAlign: col.align || 'left',
                  fontWeight: 700,
                  fontSize: '0.8rem',
                  textTransform: 'uppercase',
                  letterSpacing: '0.05em',
                  color: 'var(--text-muted)',
                  borderBottom: '1px solid var(--border-card)',
                }}
              >
                {col.label}
              </th>
            ))}
          </tr>
        </thead>
        <tbody>
          {data.map((row, rowIdx) => (
            <tr
              key={row.id || rowIdx}
              style={{
                borderBottom: '1px solid var(--border-subtle)',
                backgroundColor: rowIdx % 2 === 0 ? 'transparent' : 'rgba(255, 255, 255, 0.015)',
                transition: 'background-color 0.15s ease',
              }}
              onMouseEnter={(e) => (e.currentTarget.style.backgroundColor = 'var(--surface)')}
              onMouseLeave={(e) => (e.currentTarget.style.backgroundColor = rowIdx % 2 === 0 ? 'transparent' : 'rgba(255, 255, 255, 0.015)')}
            >
              {columns.map((col, colIdx) => {
                const val = row[col.key];
                return (
                  <td
                    key={col.key || colIdx}
                    style={{
                      padding: '0.9rem 1rem',
                      textAlign: col.align || 'left',
                      verticalAlign: 'middle',
                      borderBottom: '1px solid var(--border-subtle)',
                      color: 'var(--text)',
                    }}
                  >
                    {col.render ? col.render(val, row) : (val !== undefined && val !== null ? val : '—')}
                  </td>
                );
              })}
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
};

export default ComparisonTable;
