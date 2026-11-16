import { useState, useEffect, useRef, useCallback } from 'react';

/**
 * useLiveSync
 * Periodically executes async fetchFn in background without triggering full-screen loading state.
 * Returns { lastSynced, isSyncing, syncNow, isLive, setIsLive }
 */
export function useLiveSync(fetchFn, intervalMs = 4000, enabled = true) {
  const [lastSynced, setLastSynced] = useState(() => new Date());
  const [isSyncing, setIsSyncing] = useState(false);
  const [isLive, setIsLive] = useState(enabled);
  const fetchRef = useRef(fetchFn);
  fetchRef.current = fetchFn;

  const runSync = useCallback(async () => {
    if (isSyncing) return;
    setIsSyncing(true);
    try {
      if (typeof fetchRef.current === 'function') {
        await fetchRef.current();
      }
      setLastSynced(new Date());
    } catch (e) {
      // background sync failures handled gracefully without crash
    } finally {
      setIsSyncing(false);
    }
  }, [isSyncing]);

  useEffect(() => {
    if (!isLive) return;
    const timer = setInterval(() => {
      runSync();
    }, intervalMs);
    return () => clearInterval(timer);
  }, [isLive, intervalMs, runSync]);

  return {
    lastSynced,
    isSyncing,
    syncNow: runSync,
    isLive,
    setIsLive,
  };
}

export default useLiveSync;

