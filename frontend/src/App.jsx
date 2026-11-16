import React from 'react';
import { Routes, Route, Navigate, Link } from 'react-router-dom';
import Navbar from './components/Navbar';
import ProtectedRoute from './components/ProtectedRoute';

// Public Pages
import Home from './pages/Home';
import Login from './pages/Login';
import Register from './pages/Register';

// Candidate Pages
import CandidateDashboard from './pages/CandidateDashboard';
import Resume from './pages/Resume';
import RealTimeAnalyzer from './pages/RealTimeAnalyzer';
import ResumeAnalysis from './pages/ResumeAnalysis';
import SkillGap from './pages/SkillGap';
import ResumeJobComparisonPage from './pages/ResumeJobComparisonPage';
import CandidateAnalyticsPage from './pages/CandidateAnalyticsPage';
import MarketInsights from './pages/MarketInsights';
import Jobs from './pages/Jobs';
import JobDetails from './pages/JobDetails';
import MatchAnalysis from './pages/MatchAnalysis';
import SavedJobs from './pages/SavedJobs';
import Applications from './pages/Applications';
import Profile from './pages/Profile';

// Recruiter Pages
import RecruiterDashboard from './pages/RecruiterDashboard';
import RecruiterJobs from './pages/RecruiterJobs';
import Applicants from './pages/Applicants';
import RecruiterAnalyticsPage from './pages/RecruiterAnalyticsPage';
import JobAnalyticsPage from './pages/JobAnalyticsPage';
import CandidateComparisonPage from './pages/CandidateComparisonPage';

// Shared Authenticated Pages
import Activity from './pages/Activity';
import Settings from './pages/Settings';

export const App = () => {
  return (
    <Routes>
      {/* ── Public Routes ──────────────────────────── */}
      <Route path="/" element={<Home />} />
        <Route path="/login" element={<Login />} />
        <Route path="/register" element={<Register />} />
        <Route path="/jobs" element={<Jobs />} />
        <Route path="/jobs/:id" element={<JobDetails />} />

        {/* ── Candidate Portal (Role Protected) ──────── */}
        <Route
          path="/candidate"
          element={
            <ProtectedRoute allowedRole="CANDIDATE">
              <CandidateDashboard />
            </ProtectedRoute>
          }
        />
        <Route
          path="/candidate/resume"
          element={
            <ProtectedRoute allowedRole="CANDIDATE">
              <Resume />
            </ProtectedRoute>
          }
        />
        <Route
          path="/candidate/realtime-analyzer"
          element={
            <ProtectedRoute allowedRole="CANDIDATE">
              <RealTimeAnalyzer />
            </ProtectedRoute>
          }
        />
        <Route
          path="/candidate/resume-analysis"
          element={
            <ProtectedRoute allowedRole="CANDIDATE">
              <ResumeAnalysis />
            </ProtectedRoute>
          }
        />
        <Route
          path="/candidate/skill-gap"
          element={
            <ProtectedRoute allowedRole="CANDIDATE">
              <SkillGap />
            </ProtectedRoute>
          }
        />
        <Route
          path="/candidate/compare"
          element={
            <ProtectedRoute allowedRole="CANDIDATE">
              <ResumeJobComparisonPage />
            </ProtectedRoute>
          }
        />
        <Route
          path="/candidate/analytics"
          element={
            <ProtectedRoute allowedRole="CANDIDATE">
              <CandidateAnalyticsPage />
            </ProtectedRoute>
          }
        />
        <Route
          path="/candidate/market-insights"
          element={
            <ProtectedRoute allowedRole="CANDIDATE">
              <MarketInsights />
            </ProtectedRoute>
          }
        />
        <Route
          path="/candidate/jobs"
          element={
            <ProtectedRoute allowedRole="CANDIDATE">
              <Jobs />
            </ProtectedRoute>
          }
        />
        <Route
          path="/candidate/jobs/:id"
          element={
            <ProtectedRoute allowedRole="CANDIDATE">
              <JobDetails />
            </ProtectedRoute>
          }
        />
        <Route
          path="/candidate/matches/:id"
          element={
            <ProtectedRoute allowedRole="CANDIDATE">
              <MatchAnalysis />
            </ProtectedRoute>
          }
        />
        <Route
          path="/candidate/saved-jobs"
          element={
            <ProtectedRoute allowedRole="CANDIDATE">
              <SavedJobs />
            </ProtectedRoute>
          }
        />
        <Route
          path="/candidate/applications"
          element={
            <ProtectedRoute allowedRole="CANDIDATE">
              <Applications />
            </ProtectedRoute>
          }
        />
        <Route
          path="/candidate/profile"
          element={
            <ProtectedRoute allowedRole="CANDIDATE">
              <Profile />
            </ProtectedRoute>
          }
        />
        <Route
          path="/candidate/activity"
          element={
            <ProtectedRoute allowedRole="CANDIDATE">
              <Activity />
            </ProtectedRoute>
          }
        />
        <Route
          path="/candidate/settings"
          element={
            <ProtectedRoute allowedRole="CANDIDATE">
              <Settings />
            </ProtectedRoute>
          }
        />

        {/* ── Recruiter Portal (Role Protected) ──────── */}
        <Route
          path="/recruiter"
          element={
            <ProtectedRoute allowedRole="RECRUITER">
              <RecruiterDashboard />
            </ProtectedRoute>
          }
        />
        <Route
          path="/recruiter/analytics"
          element={
            <ProtectedRoute allowedRole="RECRUITER">
              <RecruiterAnalyticsPage />
            </ProtectedRoute>
          }
        />
        <Route
          path="/recruiter/jobs/:id/analytics"
          element={
            <ProtectedRoute allowedRole="RECRUITER">
              <JobAnalyticsPage />
            </ProtectedRoute>
          }
        />
        <Route
          path="/recruiter/jobs/:id/compare"
          element={
            <ProtectedRoute allowedRole="RECRUITER">
              <CandidateComparisonPage />
            </ProtectedRoute>
          }
        />
        <Route
          path="/recruiter/jobs"
          element={
            <ProtectedRoute allowedRole="RECRUITER">
              <RecruiterJobs />
            </ProtectedRoute>
          }
        />
        <Route
          path="/recruiter/jobs/new"
          element={
            <ProtectedRoute allowedRole="RECRUITER">
              <RecruiterJobs />
            </ProtectedRoute>
          }
        />
        <Route
          path="/recruiter/jobs/:id"
          element={
            <ProtectedRoute allowedRole="RECRUITER">
              <RecruiterJobs />
            </ProtectedRoute>
          }
        />
        <Route
          path="/recruiter/jobs/:id/applicants"
          element={
            <ProtectedRoute allowedRole="RECRUITER">
              <Applicants />
            </ProtectedRoute>
          }
        />
        <Route
          path="/recruiter/applicants"
          element={
            <ProtectedRoute allowedRole="RECRUITER">
              <Applicants />
            </ProtectedRoute>
          }
        />
        <Route
          path="/recruiter/activity"
          element={
            <ProtectedRoute allowedRole="RECRUITER">
              <Activity />
            </ProtectedRoute>
          }
        />
        <Route
          path="/recruiter/settings"
          element={
            <ProtectedRoute allowedRole="RECRUITER">
              <Settings />
            </ProtectedRoute>
          }
        />

        {/* ── 404 Fallback ───────────────────────────── */}
        <Route
          path="*"
          element={
            <div style={{ textAlign: 'center', padding: '6rem 2rem' }}>
              <h1 style={{ fontSize: '4rem', fontWeight: 800, color: 'var(--primary)' }}>404</h1>
              <h2 style={{ fontSize: '1.5rem', marginBottom: '1rem' }}>Page Not Found</h2>
              <p style={{ color: 'var(--text-muted)', marginBottom: '2rem' }}>
                The requested URL does not exist on SmartHire.
              </p>
              <Link to="/" className="btn btn-primary">
                Return to Home
              </Link>
            </div>
          }
        />
      </Routes>
  );
};

export default App;
