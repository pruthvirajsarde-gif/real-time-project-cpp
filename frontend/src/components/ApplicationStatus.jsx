import React from 'react';
import { getStatusBadgeClass } from '../utils/formatters';
import {
  SendIcon,
  SearchIcon,
  StarIcon,
  ClockIcon,
  CheckIcon,
  XIcon,
  RotateCcwIcon
} from './Icons';

export const ApplicationStatus = ({ status = 'APPLIED' }) => {
  const badgeClass = getStatusBadgeClass(status);

  const renderStatusIcon = (st) => {
    const s = 12;
    switch (st?.toUpperCase()) {
      case 'APPLIED':
        return <SendIcon size={s} />;
      case 'UNDER_REVIEW':
        return <SearchIcon size={s} />;
      case 'SHORTLISTED':
        return <StarIcon size={s} />;
      case 'INTERVIEW':
        return <ClockIcon size={s} />;
      case 'SELECTED':
        return <CheckIcon size={s} />;
      case 'REJECTED':
        return <XIcon size={s} />;
      case 'WITHDRAWN':
        return <RotateCcwIcon size={s} />;
      default:
        return <span style={{ fontSize: '0.65rem' }}>•</span>;
    }
  };

  const formatText = (st) => {
    return (st || 'UNKNOWN').replace('_', ' ');
  };

  return (
    <span className={`badge ${badgeClass}`} style={{ display: 'inline-flex', alignItems: 'center', gap: '0.35rem' }}>
      {renderStatusIcon(status)}
      <span>{formatText(status)}</span>
    </span>
  );
};

export default ApplicationStatus;
