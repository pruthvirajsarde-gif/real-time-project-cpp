import React, { createContext, useContext, useState, useEffect, useCallback } from 'react';
import { getToken, setToken, removeToken, getCachedUser, setCachedUser } from '../utils/auth';
import { authApi } from '../api/authApi';
import { setOnUnauthorizedCallback } from '../api/apiClient';

const AuthContext = createContext(null);

export const AuthProvider = ({ children }) => {
  const [tokenState, setTokenState] = useState(getToken());
  const [user, setUser] = useState(getCachedUser());
  const [loading, setLoading] = useState(true);
  const [authNotice, setAuthNotice] = useState(null);

  const logout = useCallback(async () => {
    try {
      if (getToken()) {
        await authApi.logout();
      }
    } catch (err) {
      // Ignore network errors on logout
    } finally {
      removeToken();
      setTokenState(null);
      setUser(null);
    }
  }, []);

  // Listen for unauthorized 401 events from apiClient
  useEffect(() => {
    setOnUnauthorizedCallback((msg) => {
      setTokenState(null);
      setUser(null);
      setAuthNotice(msg || 'Your session has expired. Please log in again.');
    });
  }, []);

  const refreshUser = useCallback(async () => {
    const currentToken = getToken();
    if (!currentToken) {
      setUser(null);
      setLoading(false);
      return null;
    }

    try {
      const res = await authApi.getMe();
      if (res && res.data) {
        setUser(res.data);
        setCachedUser(res.data);
        return res.data;
      }
    } catch (err) {
      console.warn('Failed to verify session token:', err.message);
      removeToken();
      setTokenState(null);
      setUser(null);
    } finally {
      setLoading(false);
    }
    return null;
  }, []);

  useEffect(() => {
    refreshUser();
  }, [refreshUser]);

  const login = async (username, password) => {
    setAuthNotice(null);
    const res = await authApi.login({ username, password });
    if (res && res.data && res.data.sessionToken) {
      const newToken = res.data.sessionToken;
      setToken(newToken);
      setTokenState(newToken);
      
      // Establish user identity
      const userData = {
        accountId: res.data.accountId,
        username: res.data.username,
        role: res.data.role,
        candidateId: res.data.candidateId,
      };
      setUser(userData);
      setCachedUser(userData);

      // Verify and fetch full account details
      await refreshUser();
      return res.data;
    }
    throw new Error(res?.message || 'Login failed.');
  };

  const registerCandidate = async (formData) => {
    setAuthNotice(null);
    return await authApi.registerCandidate(formData);
  };

  const registerRecruiter = async (formData) => {
    setAuthNotice(null);
    return await authApi.registerRecruiter(formData);
  };

  const clearNotice = () => setAuthNotice(null);

  const value = {
    token: tokenState,
    user,
    role: user?.role || null,
    isAuthenticated: !!tokenState && !!user,
    loading,
    authNotice,
    clearNotice,
    login,
    logout,
    registerCandidate,
    registerRecruiter,
    refreshUser,
  };

  return <AuthContext.Provider value={value}>{children}</AuthContext.Provider>;
};

export const useAuth = () => {
  const context = useContext(AuthContext);
  if (!context) {
    throw new Error('useAuth must be used within an AuthProvider');
  }
  return context;
};

export default AuthContext;
