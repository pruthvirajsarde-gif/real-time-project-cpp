import React, { useState, useEffect } from 'react';
import Sidebar from '../components/Sidebar';
import Loading from '../components/Loading';
import ErrorMessage from '../components/ErrorMessage';
import BarChart from '../components/BarChart';
import { analyticsApi } from '../api/analyticsApi';
import {
  TrendingUpIcon,
  MapPinIcon,
  BriefcaseIcon,
  LayersIcon,
  AlertCircleIcon
} from '../components/Icons';

export const MarketInsights = () => {
  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  const fetchInsights = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await analyticsApi.getCandidateMarketInsights();
      if (res?.data) {
        setData(res.data);
      }
    } catch (err) {
      setError(err);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchInsights();
  }, []);

  const totalJobs = data?.totalJobs ?? data?.totalJobsAnalyzed ?? data?.sampleSize ?? 0;

  const rawSkills = Array.isArray(data?.topSkills) ? data.topSkills : [];
  const topSkillItems = rawSkills.map((s) => {
    const count = s.jobCount ?? s.totalCount ?? s.count ?? 0;
    const pct = s.frequencyPercentage ?? s.percentage ?? s.frequencyPct ?? (totalJobs > 0 ? Math.round((count / totalJobs) * 100) : 0);
    return {
      label: s.skill,
      value: count,
      percentage: pct,
      description: `${count} of ${totalJobs} open positions (${pct}%)`,
    };
  });

  // Safe normalization for jobsByLocation (handles both Array and Object)
  const locationItems = Array.isArray(data?.jobsByLocation)
    ? data.jobsByLocation.map((l) => ({
        label: l.location || 'Remote / Unspecified',
        value: l.jobCount ?? l.count ?? 0,
        percentage: totalJobs > 0 ? Math.round(((l.jobCount ?? l.count ?? 0) / totalJobs) * 100) : 0,
        description: `${l.jobCount ?? l.count ?? 0} open requisitions`,
      }))
    : (data?.jobsByLocation && typeof data.jobsByLocation === 'object')
    ? Object.entries(data.jobsByLocation).map(([loc, count]) => ({
        label: loc || 'Remote / Unspecified',
        value: Number(count) || 0,
        percentage: totalJobs > 0 ? Math.round(((Number(count) || 0) / totalJobs) * 100) : 0,
        description: `${count} open requisitions`,
      }))
    : [];

  // Safe normalization for jobsByEmploymentType (handles both Array and Object)
  const empTypeItems = Array.isArray(data?.jobsByEmploymentType)
    ? data.jobsByEmploymentType.map((e) => ({
        label: e.employmentType || 'Unspecified',
        value: e.jobCount ?? e.count ?? 0,
        percentage: totalJobs > 0 ? Math.round(((e.jobCount ?? e.count ?? 0) / totalJobs) * 100) : 0,
        description: `${e.jobCount ?? e.count ?? 0} open requisitions`,
      }))
    : (data?.jobsByEmploymentType && typeof data.jobsByEmploymentType === 'object')
    ? Object.entries(data.jobsByEmploymentType).map(([emp, count]) => ({
        label: emp || 'Unspecified',
        value: Number(count) || 0,
        percentage: totalJobs > 0 ? Math.round(((Number(count) || 0) / totalJobs) * 100) : 0,
        description: `${count} open requisitions`,
      }))
    : [];

  // Safe normalization for co-occurrences (handles skillCoOccurrences, frequentlyCoOccurringSkills, frequentSkillPairs)
  const rawPairs = data?.skillCoOccurrences || data?.frequentlyCoOccurringSkills || data?.frequentSkillPairs || [];
  const coOccurrences = Array.isArray(rawPairs) ? rawPairs.map((p) => {
    let skillA = p.skillA || '';
    let skillB = p.skillB || '';
    if (!skillA && p.pair) {
      const parts = p.pair.split('+').map((s) => s.trim());
      skillA = parts[0] || '';
      skillB = parts[1] || '';
    }
    return {
      skillA,
      skillB,
      count: p.count ?? p.coOccurrenceCount ?? 0,
    };
  }) : [];

  return (
    <div className="portal-layout">
      <Sidebar type="candidate" />

      <main className="portal-main">
        <div className="portal-content">
          <div className="page-header">
            <div>
              <h1 className="page-title">SmartHire Job Market Insights</h1>
              <p className="page-subtitle">
                Aggregate technical skill demands, geographic distribution, and employment dynamics
              </p>
            </div>
            <div style={{ display: 'flex', alignItems: 'center', gap: '0.75rem' }}>
              <span className="badge badge-primary" style={{ fontSize: '0.8rem', padding: '0.35rem 0.85rem' }}>
                {totalJobs} Active Jobs Analyzed
              </span>
              <button onClick={fetchInsights} className="btn btn-outline btn-sm">
                Refresh
              </button>
            </div>
          </div>

          {/* Academic Transparency Note */}
          <div style={{
            background: 'rgba(99, 102, 241, 0.08)',
            border: '1px solid rgba(99, 102, 241, 0.2)',
            borderRadius: 'var(--radius-md)',
            padding: '1rem 1.25rem',
            marginBottom: '1.75rem',
            display: 'flex',
            alignItems: 'center',
            gap: '0.85rem',
            fontSize: '0.85rem',
            color: 'var(--text-muted)',
          }}>
            <AlertCircleIcon size={20} color="#818cf8" />
            <span>
              <strong style={{ color: 'var(--text-main)' }}>Dataset Scope Notice:</strong> These market insights are calculated directly from active requisitions in the SmartHire repository, providing deterministic statistics rather than external projections.
            </span>
          </div>

          <ErrorMessage error={error} onRetry={fetchInsights} />

          {loading ? (
            <Loading message="Synthesizing multi-job market analytics..." />
          ) : (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '1.75rem' }}>
              {/* Top Skills Demand */}
              <div className="card" style={{ padding: '1.75rem' }}>
                <h3 style={{ fontSize: '1.15rem', fontWeight: 700, marginBottom: '0.4rem', display: 'flex', alignItems: 'center', gap: '0.65rem' }}>
                  <TrendingUpIcon size={20} color="#ec4899" />
                  <span>Most Demanded Technical Skills</span>
                </h3>
                <p style={{ color: 'var(--text-muted)', fontSize: '0.85rem', marginBottom: '1.5rem' }}>
                  Frequency of technical requirements across all open positions with required vs. preferred breakdown.
                </p>

                <BarChart
                  items={topSkillItems}
                  maxVal={topSkillItems[0]?.value || 1}
                  emptyMessage="No job skills indexed yet in the database."
                />
              </div>

              {/* Grid: Locations & Employment Types */}
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(320px, 1fr))', gap: '1.5rem' }}>
                {/* Jobs by Location */}
                <div className="card" style={{ padding: '1.75rem' }}>
                  <h3 style={{ fontSize: '1.15rem', fontWeight: 700, marginBottom: '0.4rem', display: 'flex', alignItems: 'center', gap: '0.65rem' }}>
                    <MapPinIcon size={20} color="#06b6d4" />
                    <span>Jobs by Location</span>
                  </h3>
                  <p style={{ color: 'var(--text-muted)', fontSize: '0.85rem', marginBottom: '1.25rem' }}>
                    Geographic distribution of open opportunities.
                  </p>
                  <BarChart items={locationItems} emptyMessage="No location records found." />
                </div>

                {/* Jobs by Employment Type */}
                <div className="card" style={{ padding: '1.75rem' }}>
                  <h3 style={{ fontSize: '1.15rem', fontWeight: 700, marginBottom: '0.4rem', display: 'flex', alignItems: 'center', gap: '0.65rem' }}>
                    <BriefcaseIcon size={20} color="#3b82f6" />
                    <span>Employment Types</span>
                  </h3>
                  <p style={{ color: 'var(--text-muted)', fontSize: '0.85rem', marginBottom: '1.25rem' }}>
                    Full-time, contract, and remote position distribution.
                  </p>
                  <BarChart items={empTypeItems} emptyMessage="No employment types indexed." />
                </div>
              </div>

              {/* Skill Co-occurrence Pairs */}
              {coOccurrences.length > 0 && (
                <div className="card" style={{ padding: '1.75rem' }}>
                  <h3 style={{ fontSize: '1.15rem', fontWeight: 700, marginBottom: '0.4rem', display: 'flex', alignItems: 'center', gap: '0.65rem' }}>
                    <LayersIcon size={20} color="#818cf8" />
                    <span>Frequently Co-Occurring Skill Sets</span>
                  </h3>
                  <p style={{ color: 'var(--text-muted)', fontSize: '0.85rem', marginBottom: '1.35rem' }}>
                    Technical skills that frequently appear together in the same job requirement specifications. Learning these paired competencies strengthens overall qualification.
                  </p>

                  <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(220px, 1fr))', gap: '0.85rem' }}>
                    {coOccurrences.map((pair, idx) => (
                      <div
                        key={idx}
                        style={{
                          background: 'var(--surface)',
                          border: '1px solid var(--border-card)',
                          borderRadius: 'var(--radius-md)',
                          padding: '0.95rem 1.15rem',
                          display: 'flex',
                          justifyContent: 'space-between',
                          alignItems: 'center',
                        }}
                      >
                        <div style={{ display: 'flex', alignItems: 'center', gap: '0.45rem', fontSize: '0.875rem', fontWeight: 600 }}>
                          <span style={{ color: 'var(--primary)' }}>{pair.skillA}</span>
                          <span style={{ color: 'var(--text-muted)' }}>+</span>
                          <span style={{ color: '#06b6d4' }}>{pair.skillB}</span>
                        </div>
                        <span style={{
                          fontSize: '0.75rem',
                          fontWeight: 700,
                          background: 'rgba(255, 255, 255, 0.06)',
                          padding: '0.2rem 0.55rem',
                          borderRadius: 'var(--radius-sm)',
                          color: 'var(--text-muted)',
                        }}>
                          {pair.count} {pair.count === 1 ? 'job' : 'jobs'}
                        </span>
                      </div>
                    ))}
                  </div>
                </div>
              )}
            </div>
          )}
        </div>
      </main>
    </div>
  );
};

export default MarketInsights;
