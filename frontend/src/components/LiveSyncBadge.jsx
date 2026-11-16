import React, { useState, useEffect } from 'react';
import { RotateCcwIcon, ZapIcon } from './Icons';

export const LiveSyncBadge = ({
  lastSynced,
  isSyncing,
  syncNow,
  isLive = true,
  setIsLive,
  label = 'Real-Time Sync',
}) => {
  const [secondsAgo, setSecondsAgo] = useState(0);

  useEffect(() => {
    const updateTime = () => {
      if (!lastSynced) return;
      const diff = Math.floor((new Date() - new Date(lastSynced)) / 1000);
      setSecondsAgo(Math.max(0, diff));
    };
    updateTime();
    const interval = setInterval(updateTime, 1000);
    return () => clearInterval(interval);
  }, [lastSynced]);

  const timeText =
    secondsAgo < 3
      ? 'just now'
      : secondsAgo < 60
      ? `${secondsAgo}s ago`
      : `${Math.floor(secondsAgo / 60)}m ago`;

  return (
    <div
      style={{
        display: 'inline-flex',
        alignItems: 'center',
        gap: '0.5rem',
        padding: '0.35rem 0.75rem',
        borderRadius: '9999px',
        backgroundColor: isLive
          ? 'rgba(16, 185, 129, 0.1)'
          : 'rgba(148, 163, 184, 0.1)',
        border: `1px solid ${
          isLive ? 'rgba(16, 185, 129, 0.3)' : 'rgba(148, 163, 184, 0.25)'
        }`,
        fontSize: '0.78rem',
        color: isLive ? '#10b981' : 'var(--text-muted)',
        userSelect: 'none',
      }}
    >
      <span
        style={{
          display: 'inline-block',
          width: '8px',
          height: '8px',
          borderRadius: '50%',
          backgroundColor: isLive ? '#10b981' : '#94a3b8',
          boxShadow: isLive ? '0 0 8px #10b981' : 'none',
          animation: isLive ? 'pulse-live 2s infinite ease-in-out' : 'none',
        }}
      />
      <span style={{ fontWeight: 600, letterSpacing: '0.02em' }}>
        {label}: {isLive ? 'LIVE' : 'PAUSED'}
      </span>
      <span
        style={{
          color: 'var(--text-muted)',
          fontSize: '0.74rem',
          borderLeft: '1px solid var(--border-subtle)',
          paddingLeft: '0.45rem',
        }}
      >
        {isSyncing ? 'Syncing...' : timeText}
      </span>

      {syncNow && (
        <button
          onClick={syncNow}
          title="Force sync now"
          disabled={isSyncing}
          style={{
            background: 'none',
            border: 'none',
            cursor: 'pointer',
            padding: '2px',
            color: 'inherit',
            display: 'inline-flex',
            alignItems: 'center',
            opacity: isSyncing ? 0.5 : 0.8,
            transition: 'transform 0.2s ease',
          }}
          onMouseEnter={(e) => (e.currentTarget.style.opacity = '1')}
          onMouseLeave={(e) => (e.currentTarget.style.opacity = '0.8')}
        >
          <RotateCcwIcon
            size={13}
            className={isSyncing ? 'smart-icon-spin' : ''}
          />
        </button>
      )}

      {setIsLive && (
        <button
          onClick={() => setIsLive(!isLive)}
          title={isLive ? 'Pause real-time updates' : 'Resume real-time updates'}
          style={{
            background: 'none',
            border: 'none',
            cursor: 'pointer',
            fontSize: '0.72rem',
            color: 'var(--text-muted)',
            textDecoration: 'underline',
            padding: 0,
            marginLeft: '0.2rem',
          }}
        >
          {isLive ? 'Pause' : 'Resume'}
        </button>
      )}

      <style>{`
        @keyframes pulse-live {
          0% { transform: scale(0.95); opacity: 0.7; }
          50% { transform: scale(1.2); opacity: 1; }
          100% { transform: scale(0.95); opacity: 0.7; }
        }
        .smart-icon-spin {
          animation: spin-live 1s linear infinite;
        }
        @keyframes spin-live {
          from { transform: rotate(0deg); }
          to { transform: rotate(360deg); }
        }
      `}</style>
    </div>
  );
};

export default LiveSyncBadge;

