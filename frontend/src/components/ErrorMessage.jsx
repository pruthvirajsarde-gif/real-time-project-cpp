import React from 'react';
import { AlertCircleIcon, InfoIcon, RotateCcwIcon } from './Icons';

export const ErrorMessage = ({ error, onRetry }) => {
  if (!error) return null;

  const isServerDown =
    error?.code === 'SERVER_UNAVAILABLE' ||
    (typeof error === 'string' && error.includes('SmartHire.exe --server'));

  const message = typeof error === 'string' ? error : error?.message || 'An unexpected error occurred.';

  return (
    <div style={{
      background: 'rgba(239, 68, 68, 0.08)',
      border: '1px solid rgba(239, 68, 68, 0.25)',
      borderRadius: 'var(--radius-lg)',
      padding: '1.5rem',
      margin: '1.5rem 0',
      display: 'flex',
      flexDirection: 'column',
      gap: '0.75rem',
    }}>
      <div style={{ display: 'flex', alignItems: 'center', gap: '0.75rem', color: '#fca5a5' }}>
        <AlertCircleIcon size={20} color="#ef4444" />
        <h4 style={{ margin: 0, fontSize: '1.05rem', color: '#fca5a5', fontWeight: 600 }}>
          {isServerDown ? 'Server Connection Failed' : 'Notice'}
        </h4>
      </div>
      <p style={{ margin: 0, color: 'var(--text-muted)', fontSize: '0.925rem' }}>
        {message}
      </p>

      {isServerDown && (
        <div style={{
          background: 'rgba(15, 23, 42, 0.8)',
          border: '1px solid var(--border-subtle)',
          borderRadius: 'var(--radius-md)',
          padding: '0.85rem 1rem',
          fontFamily: 'var(--font-mono)',
          fontSize: '0.875rem',
          color: '#a5b4fc',
          marginTop: '0.5rem',
          display: 'flex',
          flexDirection: 'column',
          gap: '0.35rem'
        }}>
          <span style={{ display: 'flex', alignItems: 'center', gap: '0.4rem', color: '#cbd5e1' }}>
            <InfoIcon size={14} color="#60a5fa" /> Please start the backend server in a terminal:
          </span>
          <strong style={{ color: '#fff' }}>./build/SmartHire.exe --server</strong>
        </div>
      )}

      {onRetry && (
        <div style={{ marginTop: '0.5rem' }}>
          <button className="btn btn-sm btn-secondary" onClick={onRetry} style={{ display: 'inline-flex', alignItems: 'center', gap: '0.4rem' }}>
            <RotateCcwIcon size={14} />
            <span>Try Again</span>
          </button>
        </div>
      )}
    </div>
  );
};

export default ErrorMessage;
