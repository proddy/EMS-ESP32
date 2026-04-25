import type { LogSettings, SystemStatus } from 'types';

import { DOCS_BASE_URL, alovaInstance } from './endpoints';

// systemStatus - also used to ping in System Monitor for pinging
export const readSystemStatus = () =>
  alovaInstance.Get<SystemStatus>('/rest/systemStatus');

// SystemLog
export const readLogSettings = () =>
  alovaInstance.Get<LogSettings>('/rest/logSettings');
export const updateLogSettings = (data: LogSettings) =>
  alovaInstance.Post('/rest/logSettings', data);
export const fetchLogES = () => alovaInstance.Get('/es/log');

// get versions from emsesp.org/versions.json
// uses native fetch (no custom headers) to keep this as a "simple" CORS
export const getVersions = async <T = unknown>(): Promise<T> => {
  const res = await fetch(`${DOCS_BASE_URL}/versions.json`);
  if (!res.ok) throw new Error(res.statusText);
  return res.json() as Promise<T>;
};

const UPLOAD_TIMEOUT = 60000; // 1 minute

export const uploadFile = (file: File) => {
  const formData = new FormData();
  formData.append('file', file);
  return alovaInstance.Post('/rest/uploadFile', formData, {
    timeout: UPLOAD_TIMEOUT
  });
};
