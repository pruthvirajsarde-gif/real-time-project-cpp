import React from 'react';
import { CheckIcon, XIcon, ChevronRightIcon } from './Icons';

export const SkillBadge = ({ name, type = 'default' }) => {
  let badgeStyle = {
    background: 'rgba(99, 102, 241, 0.12)',
    color: '#a5b4fc',
    border: '1px solid rgba(99, 102, 241, 0.25)',
  };

  let icon = null;

  if (type === 'exact' || type === 'matched') {
    badgeStyle = {
      background: 'rgba(16, 185, 129, 0.15)',
      color: '#6ee7b7',
      border: '1px solid rgba(16, 185, 129, 0.35)',
    };
    icon = <CheckIcon size={11} />;
  } else if (type === 'missing-required') {
    badgeStyle = {
      background: 'rgba(239, 68, 68, 0.15)',
      color: '#fca5a5',
      border: '1px solid rgba(239, 68, 68, 0.35)',
    };
    icon = <XIcon size={11} />;
  } else if (type === 'missing-preferred') {
    badgeStyle = {
      background: 'rgba(245, 158, 11, 0.15)',
      color: '#fcd34d',
      border: '1px solid rgba(245, 158, 11, 0.35)',
    };
    icon = <ChevronRightIcon size={11} />;
  }

  return (
    <span
      style={{
        display: 'inline-flex',
        alignItems: 'center',
        gap: '0.3rem',
        padding: '0.28rem 0.65rem',
        borderRadius: 'var(--radius-full)',
        fontSize: '0.8rem',
        fontWeight: 600,
        margin: '0.2rem',
        ...badgeStyle,
      }}
    >
      {icon}
      <span>{name}</span>
    </span>
  );
};

export default SkillBadge;
