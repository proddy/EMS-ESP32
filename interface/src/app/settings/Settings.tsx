import { useContext, useState } from 'react';
import { toast } from 'react-toastify';

import AccessTimeIcon from '@mui/icons-material/AccessTime';
import BuildIcon from '@mui/icons-material/Build';
import CancelIcon from '@mui/icons-material/Cancel';
import DeviceHubIcon from '@mui/icons-material/DeviceHub';
import ImportExportIcon from '@mui/icons-material/ImportExport';
import LockIcon from '@mui/icons-material/Lock';
import PowerSettingsNewIcon from '@mui/icons-material/PowerSettingsNew';
import SettingsBackupRestoreIcon from '@mui/icons-material/SettingsBackupRestore';
import SettingsEthernetIcon from '@mui/icons-material/SettingsEthernet';
import SettingsInputAntennaIcon from '@mui/icons-material/SettingsInputAntenna';
import TuneIcon from '@mui/icons-material/Tune';
import ViewModuleIcon from '@mui/icons-material/ViewModule';
import {
  Box,
  Button,
  Dialog,
  DialogActions,
  DialogContent,
  DialogTitle,
  Divider,
  List
} from '@mui/material';

import { API } from 'api/app';

import { dialogStyle } from 'CustomTheme';
import { useRequest } from 'alova/client';
import type { APIcall } from 'app/main/types';
import { SectionContent, useLayoutTitle } from 'components';
import ListMenuItem from 'components/layout/ListMenuItem';
import { AuthenticatedContext } from 'contexts/authentication';
import { useI18nContext } from 'i18n/i18n-react';

import SystemMonitor from '../status/SystemMonitor';

const Settings = () => {
  const { LL } = useI18nContext();
  const { versions } = useContext(AuthenticatedContext);
  useLayoutTitle(LL.SETTINGS(0));

  const upgradeAvailable = versions?.current?.upgradeable ?? false;
  const firmwareText = versions?.current?.version
    ? `v${versions.current.version}${upgradeAvailable ? ` (${LL.UPDATE_AVAILABLE()})` : ''}`
    : '';

  const [confirmFactoryReset, setConfirmFactoryReset] = useState(false);
  const [confirmRestart, setConfirmRestart] = useState(false);
  const [restarting, setRestarting] = useState<boolean>();

  const { send: sendAPI } = useRequest((data: APIcall) => API(data), {
    immediate: false
  });

  const doFormat = async () => {
    await sendAPI({ device: 'system', cmd: 'format', id: 0 }).then(() => {
      setRestarting(true);
      setConfirmFactoryReset(false);
    });
  };

  const doRestart = async () => {
    setConfirmRestart(false);
    setRestarting(true);
    await sendAPI({ device: 'system', cmd: 'restart', id: 0 }).catch(
      (error: Error) => {
        toast.error(error.message);
      }
    );
  };

  const handleFactoryResetClose = () => {
    setConfirmFactoryReset(false);
  };

  const handleFactoryResetClick = () => {
    setConfirmFactoryReset(true);
  };

  const handleRestartClose = () => {
    setConfirmRestart(false);
  };

  const handleRestartClick = () => {
    setConfirmRestart(true);
  };

  if (restarting) {
    return <SystemMonitor />;
  }

  return (
    <SectionContent>
      <List>
        <ListMenuItem
          icon={BuildIcon}
          bgcolor="#72caf9"
          label="EMS-ESP Firmware"
          text={firmwareText}
          to="/settings/version"
          badge={upgradeAvailable}
        />

        <ListMenuItem
          icon={TuneIcon}
          bgcolor="#134ba2"
          label={LL.APPLICATION()}
          text={LL.APPLICATION_SETTINGS_1()}
          to="application"
        />

        <ListMenuItem
          icon={SettingsEthernetIcon}
          bgcolor="#40828f"
          label={LL.NETWORK(0)}
          text={LL.CONFIGURE(LL.SETTINGS_OF(LL.NETWORK(1)))}
          to="network"
        />

        <ListMenuItem
          icon={SettingsInputAntennaIcon}
          bgcolor="#5f9a5f"
          label={LL.ACCESS_POINT(0)}
          text={LL.CONFIGURE(LL.ACCESS_POINT(1))}
          to="ap"
        />

        <ListMenuItem
          icon={AccessTimeIcon}
          bgcolor="#c5572c"
          label="NTP"
          text={LL.CONFIGURE(LL.LOCAL_TIME(1))}
          to="ntp"
        />

        <ListMenuItem
          icon={DeviceHubIcon}
          bgcolor="#68374d"
          label="MQTT"
          text={LL.CONFIGURE('MQTT')}
          to="mqtt"
        />

        <ListMenuItem
          icon={LockIcon}
          label={LL.SECURITY(0)}
          text={LL.SECURITY_1()}
          to="security"
        />

        <ListMenuItem
          icon={ViewModuleIcon}
          bgcolor="#efc34b"
          label={LL.MODULES()}
          text={LL.MODULES_1()}
          to="modules"
        />

        <ListMenuItem
          icon={ImportExportIcon}
          bgcolor="#5d89f7"
          label={LL.DOWNLOAD_UPLOAD()}
          text={LL.DOWNLOAD_UPLOAD_1()}
          to="downloadUpload"
        />
      </List>

      <Dialog
        sx={dialogStyle}
        open={confirmFactoryReset}
        onClose={handleFactoryResetClose}
      >
        <DialogTitle>{LL.FACTORY_RESET()}</DialogTitle>
        <DialogContent dividers>{LL.SYSTEM_FACTORY_TEXT_DIALOG()}</DialogContent>
        <DialogActions>
          <Button
            startIcon={<CancelIcon />}
            variant="outlined"
            onClick={handleFactoryResetClose}
            color="secondary"
          >
            {LL.CANCEL()}
          </Button>
          <Button
            startIcon={<SettingsBackupRestoreIcon />}
            variant="outlined"
            onClick={doFormat}
            color="error"
          >
            {LL.FACTORY_RESET()}
          </Button>
        </DialogActions>
      </Dialog>

      <Dialog sx={dialogStyle} open={confirmRestart} onClose={handleRestartClose}>
        <DialogTitle>{LL.RESTART()}</DialogTitle>
        <DialogContent dividers>{LL.RESTART_CONFIRM()}</DialogContent>
        <DialogActions>
          <Button
            startIcon={<CancelIcon />}
            variant="outlined"
            onClick={handleRestartClose}
            color="secondary"
          >
            {LL.CANCEL()}
          </Button>
          <Button
            startIcon={<PowerSettingsNewIcon />}
            variant="outlined"
            onClick={doRestart}
            color="error"
          >
            {LL.RESTART()}
          </Button>
        </DialogActions>
      </Dialog>

      <Divider />

      <Box
        sx={{
          mt: 2,
          display: 'flex',
          justifyContent: 'flex-end',
          flexWrap: 'nowrap',
          whiteSpace: 'nowrap',
          gap: 1
        }}
      >
        <Button
          startIcon={<PowerSettingsNewIcon />}
          variant="outlined"
          onClick={handleRestartClick}
          color="error"
        >
          {LL.RESTART()}
        </Button>
        <Button
          startIcon={<SettingsBackupRestoreIcon />}
          variant="outlined"
          onClick={handleFactoryResetClick}
          color="error"
        >
          {LL.FACTORY_RESET()}
        </Button>
      </Box>
    </SectionContent>
  );
};

export default Settings;
