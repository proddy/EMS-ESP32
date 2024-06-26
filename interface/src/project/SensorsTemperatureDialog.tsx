import { useEffect, useState } from 'react';

import CancelIcon from '@mui/icons-material/Cancel';
import WarningIcon from '@mui/icons-material/Warning';
import {
  Box,
  Button,
  Dialog,
  DialogActions,
  DialogContent,
  DialogTitle,
  Grid,
  InputAdornment,
  TextField,
  Typography
} from '@mui/material';

import { dialogStyle } from 'CustomTheme';
import type Schema from 'async-validator';
import type { ValidateFieldsError } from 'async-validator';
import { ValidatedTextField } from 'components';
import { useI18nContext } from 'i18n/i18n-react';
import { numberValue, updateValue } from 'utils';
import { validate } from 'validators';

import type { TemperatureSensor } from './types';

interface SensorsTemperatureDialogProps {
  open: boolean;
  onClose: () => void;
  onSave: (ts: TemperatureSensor) => void;
  selectedItem: TemperatureSensor;
  validator: Schema;
}

const SensorsTemperatureDialog = ({
  open,
  onClose,
  onSave,
  selectedItem,
  validator
}: SensorsTemperatureDialogProps) => {
  const { LL } = useI18nContext();
  const [fieldErrors, setFieldErrors] = useState<ValidateFieldsError>();
  const [editItem, setEditItem] = useState<TemperatureSensor>(selectedItem);
  const updateFormValue = updateValue(setEditItem);

  useEffect(() => {
    if (open) {
      setFieldErrors(undefined);
      setEditItem(selectedItem);
    }
  }, [open, selectedItem]);

  const close = () => {
    onClose();
  };

  const save = async () => {
    try {
      setFieldErrors(undefined);
      await validate(validator, editItem);
      onSave(editItem);
    } catch (error) {
      setFieldErrors(error as ValidateFieldsError);
    }
  };

  return (
    <Dialog sx={dialogStyle} open={open} onClose={close}>
      <DialogTitle>
        {LL.EDIT()}&nbsp;{LL.TEMP_SENSOR()}
      </DialogTitle>
      <DialogContent dividers>
        <Box color="warning.main" p={0} pl={0} pr={0} mt={0} mb={2}>
          <Typography variant="body2">
            {LL.ID_OF(LL.SENSOR(0))}: {editItem.id}
          </Typography>
        </Box>
        <Grid container spacing={1}>
          <Grid item>
            <ValidatedTextField
              fieldErrors={fieldErrors}
              name="n"
              label={LL.NAME(0)}
              value={editItem.n}
              autoFocus
              sx={{ width: '30ch' }}
              onChange={updateFormValue}
            />
          </Grid>
          <Grid item>
            <TextField
              name="o"
              label={LL.OFFSET()}
              value={numberValue(editItem.o)}
              sx={{ width: '12ch' }}
              type="number"
              variant="outlined"
              onChange={updateFormValue}
              inputProps={{ min: '-5', max: '5', step: '0.1' }}
              InputProps={{
                startAdornment: <InputAdornment position="start">°C</InputAdornment>
              }}
            />
          </Grid>
        </Grid>
      </DialogContent>
      <DialogActions>
        <Button
          startIcon={<CancelIcon />}
          variant="outlined"
          onClick={close}
          color="secondary"
        >
          {LL.CANCEL()}
        </Button>
        <Button
          startIcon={<WarningIcon color="warning" />}
          variant="contained"
          onClick={save}
          color="info"
        >
          {LL.UPDATE()}
        </Button>
      </DialogActions>
    </Dialog>
  );
};

export default SensorsTemperatureDialog;
