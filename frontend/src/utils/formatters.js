export const formatDate = (dateStr) => {
  if (!dateStr) return 'N/A';
  try {
    const d = new Date(dateStr);
    if (isNaN(d.getTime())) return dateStr;
    return d.toLocaleDateString('en-US', {
      year: 'numeric',
      month: 'short',
      day: 'numeric',
    });
  } catch (err) {
    return dateStr;
  }
};

export const formatScore = (val) => {
  if (val === null || val === undefined || isNaN(val)) return '0.0%';
  return `${Number(val).toFixed(1)}%`;
};

export const getScoreColor = (score) => {
  const num = Number(score) || 0;
  if (num >= 80) return 'var(--success)';
  if (num >= 60) return 'var(--info)';
  if (num >= 40) return 'var(--warning)';
  return 'var(--danger)';
};

export const getScoreBadgeClass = (score) => {
  const num = Number(score) || 0;
  if (num >= 80) return 'badge-success';
  if (num >= 60) return 'badge-info';
  if (num >= 40) return 'badge-warning';
  return 'badge-danger';
};

export const getStatusBadgeClass = (status) => {
  switch (status?.toUpperCase()) {
    case 'APPLIED':
      return 'badge-info';
    case 'UNDER_REVIEW':
      return 'badge-primary';
    case 'SHORTLISTED':
      return 'badge-warning';
    case 'INTERVIEW':
      return 'badge-info';
    case 'SELECTED':
      return 'badge-success';
    case 'REJECTED':
      return 'badge-danger';
    case 'WITHDRAWN':
      return 'badge-neutral';
    default:
      return 'badge-neutral';
  }
};
