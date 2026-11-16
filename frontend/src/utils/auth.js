// Browser session storage management for Bearer token and cached user
const TOKEN_KEY = 'smarthire_token';
const USER_KEY = 'smarthire_user';

export const getToken = () => {
  try {
    return sessionStorage.getItem(TOKEN_KEY);
  } catch (err) {
    return null;
  }
};

export const setToken = (token) => {
  try {
    if (token) {
      sessionStorage.setItem(TOKEN_KEY, token);
    } else {
      sessionStorage.removeItem(TOKEN_KEY);
    }
  } catch (err) {
    console.error('Failed to set session token', err);
  }
};

export const removeToken = () => {
  try {
    sessionStorage.removeItem(TOKEN_KEY);
    sessionStorage.removeItem(USER_KEY);
  } catch (err) {
    console.error('Failed to remove token', err);
  }
};

export const getCachedUser = () => {
  try {
    const raw = sessionStorage.getItem(USER_KEY);
    return raw ? JSON.parse(raw) : null;
  } catch (err) {
    return null;
  }
};

export const setCachedUser = (user) => {
  try {
    if (user) {
      sessionStorage.setItem(USER_KEY, JSON.stringify(user));
    } else {
      sessionStorage.removeItem(USER_KEY);
    }
  } catch (err) {}
};

export const removeCachedUser = () => {
  try {
    sessionStorage.removeItem(USER_KEY);
  } catch (err) {}
};

export const clearAuthSession = () => {
  removeToken();
};
