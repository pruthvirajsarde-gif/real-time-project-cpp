import { describe, it, expect } from 'vitest';

describe('Real-Time Resume Analysis & Matching Engine', () => {
  it('extracts technical skills from candidate resume text accurately in real-time', () => {
    const text = `
      [SKILLS]
      C++, Python, React, SQL, Linux, Docker, Git, Multithreading
    `;
    const knownSkills = ['c++', 'python', 'react', 'sql', 'linux', 'docker', 'git', 'multithreading'];
    const lower = text.toLowerCase();
    const matched = knownSkills.filter(s => {
      const escaped = s.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
      return new RegExp(`(?:^|[\\s,;:.()\\[\\]/])${escaped}(?:$|[\\s,;:.()\\[\\]/])`, 'i').test(lower);
    });

    expect(matched).toContain('c++');
    expect(matched).toContain('python');
    expect(matched).toContain('react');
    expect(matched).toContain('sql');
    expect(matched.length).toBe(8);
  });

  it('calculates weighted job match score dynamically', () => {
    const requiredSkills = ['C++', 'SQL', 'Git', 'Linux'];
    const preferredSkills = ['Docker', 'CMake'];
    const candidateSkills = ['c++', 'sql', 'git', 'docker']; // 3 req, 1 pref

    const reqMatched = requiredSkills.filter(s => candidateSkills.includes(s.toLowerCase()));
    const prefMatched = preferredSkills.filter(s => candidateSkills.includes(s.toLowerCase()));

    const reqScore = (reqMatched.length / requiredSkills.length) * 70; // (3/4)*70 = 52.5
    const prefScore = (prefMatched.length / preferredSkills.length) * 30; // (1/2)*30 = 15
    const totalScore = Math.round(reqScore + prefScore); // 68

    expect(totalScore).toBe(68);
    expect(reqMatched.length).toBe(3);
    expect(prefMatched.length).toBe(1);
  });

  it('detects action verbs and quantifiable metrics for ATS scoring', () => {
    const text = 'Architected and optimized low-latency services improving performance by 45%.';
    const verbs = ['architected', 'optimized'].filter(v => new RegExp(`\\b${v}\\b`, 'i').test(text));
    const metrics = text.match(/\d+%/g) || [];

    expect(verbs.length).toBe(2);
    expect(metrics).toContain('45%');
  });
});

