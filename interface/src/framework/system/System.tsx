import { type FC, useContext } from 'react';
import { Navigate, Route, Routes } from 'react-router-dom';

import { Tab } from '@mui/material';

import { RequireAdmin, RouterTabs, useLayoutTitle, useRouterTab } from 'components';
import { AuthenticatedContext } from 'contexts/authentication';
import { useI18nContext } from 'i18n/i18n-react';
import SystemActivity from 'project/SystemActivity';

import SystemLog from './SystemLog';
import SystemStatus from './SystemStatus';

const System: FC = () => {
  const { LL } = useI18nContext();

  useLayoutTitle(LL.SYSTEM(0));

  const { routerTab } = useRouterTab();
  const { me } = useContext(AuthenticatedContext);

  return (
    <>
      <RouterTabs value={routerTab}>
        <Tab value="status" label={LL.STATUS_OF('')} />
        <Tab value="activity" label={LL.ACTIVITY()} />
        <Tab
          disabled={!me.admin}
          value="log"
          label={me.admin ? LL.LOG_OF('') : ''}
        />
      </RouterTabs>
      <Routes>
        <Route path="status" element={<SystemStatus />} />
        <Route path="activity" element={<SystemActivity />} />
        <Route
          path="log"
          element={
            <RequireAdmin>
              <SystemLog />
            </RequireAdmin>
          }
        />
        <Route path="*" element={<Navigate replace to="status" />} />
      </Routes>
    </>
  );
};

export default System;
