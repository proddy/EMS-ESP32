import type { Translation } from '../i18n-types';
/* prettier-ignore */
/* eslint-disable */

const sk: Translation = {
  LANGUAGE: 'Jazyk',
  RETRY: 'Opakovať',
  LOADING: 'Načítanie',
  IS_REQUIRED: '{0} je požadovaných',
  SIGN_IN: 'Prihlásiť sa',
  SIGN_OUT: 'Odhlásiť sa',
  USERNAME: 'Užívateľské meno',
  PASSWORD: 'Heslo',
  SU_PASSWORD: 'su heslo',
  SETTINGS_OF: '{0} Nastavenia',
  HELP_OF: '{0} Pomoc',
  LOGGED_IN: 'Prihlásený ako {name}',
  PLEASE_SIGNIN: 'Ak chcete pokračovať, prihláste sa',
  UPLOAD_SUCCESSFUL: 'Nahratie úspešné',
  DOWNLOAD_SUCCESSFUL: 'Stiahnutie úspešné',
  INVALID_LOGIN: 'Nesprávne prihlasovacie údaje',
  NETWORK: 'Sieť',
  SECURITY: 'Zabezpečenie',
  ONOFF_CAP: 'ZAP/VYP',
  ONOFF: 'zap/vyp',
  TYPE: 'Typ',
  DESCRIPTION: 'Popis',
  ENTITIES: 'Entity',
  REFRESH: 'Obnoviť',
  EXPORT: 'Export',
  DEVICE_DETAILS: 'Detaily zariadenia',
  ID_OF: '{0} ID',
  DEVICE: 'Zariadenie',
  PRODUCT: 'Produkt',
  VERSION: 'Verzia',
  BRAND: 'Značka',
  ENTITY_NAME: 'Názov entity',
  VALUE: '{{Hodnota|hodnota}}',
  DEVICES: 'Zariadenia',
  SENSORS: 'Snímače',
  RUN_COMMAND: 'Volať príkaz',
  CHANGE_VALUE: 'Zmena hodnoty',
  CANCEL: 'Zrušiť',
  RESET: 'Reset',
  APPLY_CHANGES: 'Aplikovať zmeny ({0})',
  UPDATE: 'Aktualizovať',
  EXECUTE: 'Spustiť',
  REMOVE: 'Odstrániť',
  PROBLEM_UPDATING: 'Problém s aktualizáciou',
  PROBLEM_LOADING: 'Problém s načítaním',
  ANALOG_SENSOR: 'Analógový snímač',
  ANALOG_SENSORS: 'Analógové snímače',
  SETTINGS: 'Nastavenia',
  UPDATED_OF: '{0} aktualizovaných',
  UPDATE_OF: '{0} aktualizované',
  REMOVED_OF: '{0} odstránených',
  DELETION_OF: '{0} zmazaných',
  OFFSET: 'Ofset',
  FACTOR: 'Faktor',
  FREQ: 'Frekvencia',
  DUTY_CYCLE: 'Duty Cycle',
  UNIT: 'UoM',
  STARTVALUE: 'Počiatočná hodnota',
  WARN_GPIO: 'Upozornenie: Buďte opatrní pri priraďovaní GPIO!',
  EDIT: 'Editovať',
  SENSOR: 'Snímač',
  TEMP_SENSOR: 'Snímač teploty',
  TEMP_SENSORS: 'Snímače teploty',
  WRITE_CMD_SENT: 'Príkaz zápisu bol odoslaný',
  EMS_BUS_WARNING: 'Zbernica EMS odpojená. Ak toto upozornenie pretrváva aj po niekoľkých sekundách, skontrolujte nastavenia a profil dosky',
  EMS_BUS_SCANNING: 'Zisťovanie EMS zariadení...',
  CONNECTED: 'Pripojené',
  TX_ISSUES: 'Problémy s Tx – skontrolujte Tx režim',
  DISCONNECTED: 'Odpojené',
  EMS_SCAN: 'Naozaj chcete spustiť úplnú kontrolu zariadenia zbernice EMS?',
  EMS_BUS_STATUS: 'Stav zbernice EMS',
  ACTIVE_DEVICES: 'Aktívne zariadenia a snímače',
  EMS_DEVICE: 'EMS zariadenie',
  SUCCESS: 'ÚSPEŠNÉ',
  FAIL: 'ZLÝHANIE',
  QUALITY: 'KVALITA',
  SCAN_DEVICES: 'Scan pre nové zariadenia',
  SCAN: 'Scan',
  STATUS_NAMES: [
    'EMS Telegramy prijaté (Rx)',
    'EMS Čítania (Tx)',
    'EMS Zápisy (Tx)',
    'Čítanie snímačov teploty',
    'Čítanie analógových snímačov',
    'MQTT Publikovanie',
    'Externé API volania',
    'Syslog správy'
  ],
  NUM_DEVICES: '{num} Zariaden{{í|ie|ia|ia|í|í}}',
  NUM_TEMP_SENSORS: '{num} Teplotn{{ých|ý|é|é|ých|ých}} sníma{{čov|č|če|če|čov|čov}}',
  NUM_ANALOG_SENSORS: '{num} Analogov{{ých|ý|é|é|ých|ých}} sníma{{čov|č|če|če|čov|čov}}',
  NUM_DAYS: '{num} d{{ní|eň|ní|ní|ní|ní}}',
  NUM_SECONDS: '{num} sek{{únd|unda|undy|undy|únd|únd}}',
  NUM_HOURS: '{num} hod{{ín|ina|iny|iny|ín|ín}}',
  NUM_MINUTES: '{num} minú{{t|ta|ty|ty|t|t}}',
  APPLICATION_SETTINGS: 'Nastavenia aplikácie',
  CUSTOMIZATIONS: 'Prispôsobenia',
  APPLICATION_RESTARTING: 'EMS-ESP sa reštartuje',
  INTERFACE_BOARD_PROFILE: 'Profil dosky rozhrania',
  BOARD_PROFILE_TEXT: 'Vyberte vopred nakonfigurovaný profil dosky rozhrania zo zoznamu nižšie, alebo vyberte možnosť Vlastné a nakonfigurujte svoje vlastné hardvérové nastavenia',
  BOARD_PROFILE: 'Profil dosky',
  CUSTOM: 'Vlastné',
  GPIO_OF: '{0} GPIO',
  BUTTON: 'Tlačidlo',
  TEMPERATURE: 'Teplota',
  PHY_TYPE: 'Eth PHY Typ',
  DISABLED: 'zakázané',
  TX_MODE: 'Tx režim',
  HARDWARE: 'Hardware',
  EMS_BUS: '{{BUS|EMS BUS}}',
  GENERAL_OPTIONS: 'Všeobecné možnosti',
  LANGUAGE_ENTITIES: 'Jazyk (pre entity zariadenia)',
  HIDE_LED: 'Skryť LED',
  ENABLE_TELNET: 'Povoliť Telnet konzolu',
  ENABLE_ANALOG: 'Povoliť analógové snímače',
  CONVERT_FAHRENHEIT: 'Previesť hodnoty teploty na °F',
  BYPASS_TOKEN: 'Vynechajte autorizáciu prístupového tokenu pri volaniach API',
  READONLY: 'Povoliť režim len na čítanie (blokuje všetky odchádzajúce príkazy EMS Tx Write)',
  UNDERCLOCK_CPU: 'Podtaktovanie rýchlosti procesora',
  HEATINGOFF: 'Spustiť kotol s vynúteným vykurovaním',

  ENABLE_SHOWER_TIMER: 'Povoliť časovač sprchovania',
  ENABLE_SHOWER_ALERT: 'Povoliť upozornenie na sprchu',
  TRIGGER_TIME: 'Čas spustenia',
  COLD_SHOT_DURATION: 'Trvanie studeného záberu',
  FORMATTING_OPTIONS: 'Možnosti formátovania',
  BOOLEAN_FORMAT_DASHBOARD: 'Panel Boolean formát',
  BOOLEAN_FORMAT_API: 'Boolean formát API/MQTT',
  ENUM_FORMAT: 'Enum formát API/MQTT',
  INDEX: 'Index',
  ENABLE_PARASITE: 'Povoliť parazité napájanie DS18B20',
  LOGGING: 'Logovanie',
  LOG_HEX: 'Záznam telegramov EMS v hexadecimálnej sústave',
  ENABLE_SYSLOG: 'Povoliť Syslog',
  LOG_LEVEL: 'Log úroveň',
  MARK_INTERVAL: 'Označenie intervalu',
  SECONDS: 'sekundy',
  MINUTES: 'minúty',
  HOURS: 'hodiny',
  RESTART: 'Reštart',
  RESTART_TEXT: 'EMS-ESP sa musí reštartovať, aby sa použili zmenené systémové nastavenia',
  RESTART_CONFIRM: 'Ste si istí, že chcete reštartovať EMS-ESP?',
  COMMAND: 'Príkaz',
  CUSTOMIZATIONS_RESTART: 'Ste si istí, že chcete reštartovať EMS-ESP?',
  CUSTOMIZATIONS_FULL: 'Vybrané subjekty prekročili limit. Prosím, ukladajte v dávkach',
  CUSTOMIZATIONS_SAVED: 'Uložené prispôsobenia',
  CUSTOMIZATIONS_HELP_1: 'Vyberte zariadenie a prispôsobte možnosti entít alebo kliknutím premenujte',
  CUSTOMIZATIONS_HELP_2: 'označiť ako obľúbené',
  CUSTOMIZATIONS_HELP_3: 'zakázať akciu zápisu',
  CUSTOMIZATIONS_HELP_4: 'vylúčiť z MQTT a API',
  CUSTOMIZATIONS_HELP_5: 'skryť z panela',
  CUSTOMIZATIONS_HELP_6: 'odstrániť z pamäte',
  SELECT_DEVICE: 'Zvoliť zariadenie',
  SET_ALL: 'nastaviť všetko',
  OPTIONS: 'Možnosti',
  NAME: 'Názov',
  CUSTOMIZATIONS_RESET: 'Naozaj chcete odstrániť všetky prispôsobenia vrátane vlastných nastavení snímačov teploty a analógových snímačov?',
  SUPPORT_INFORMATION: 'Informácie pre podporu',
  HELP_INFORMATION_1: 'Navštívte online wiki, kde nájdete pokyny na konfiguráciu EMS-ESP',
  HELP_INFORMATION_2: 'Pre živý komunitný chat sa pripojte na náš Discord server',
  HELP_INFORMATION_3: 'Ak chcete požiadať o funkciu alebo nahlásiť chybu',
  HELP_INFORMATION_4: 'nezabudnite si stiahnuť a pripojiť informácie o vašom systéme, aby ste mohli rýchlejšie reagovať pri nahlasovaní problému',
  HELP_INFORMATION_5: 'EMS-ESP je bezplatný a open source projekt. Podporte jeho budúci vývoj tým, že mu dáte hviezdičku na Github!',
  UPLOAD: 'Nahrať',
  DOWNLOAD: '{{S|s|s}}tiahnuť',
  ABORTED: 'zrušené',
  FAILED: 'chybné',
  SUCCESSFUL: 'úspešné',
  SYSTEM: 'Systém',
  LOG_OF: '{0} Log',
  STATUS_OF: '{0} Stav',
  UPLOAD_DOWNLOAD: 'Nahrať/Stiahnuť',
  VERSION_ON: 'Momentálne nainštalovaná verzia: ',
  CLOSE: 'Zatvoriť',
  USE: 'Použiť',
  FACTORY_RESET: 'Továrenské nastavenia',
  SYSTEM_FACTORY_TEXT: 'Zariadenie bolo obnovené z výroby a teraz sa reštartuje',
  SYSTEM_FACTORY_TEXT_DIALOG: 'Naozaj chcete resetovať EMS-ESP na predvolené výrobné nastavenia?',
  THE_LATEST: 'Posledná',
  OFFICIAL: 'officiálna',
  DEVELOPMENT: 'vývojárska',
  RELEASE_IS: 'verzia je',
  RELEASE_NOTES: 'poznámky k verzii',
  EMS_ESP_VER: 'EMS-ESP verzia',
  UPTIME: 'Beh systému',
  HEAP: 'Zásobník (voľné / max pridelenie)',
  PSRAM: 'PSRAM (Veľkosť / Voľné)',
  FLASH: 'Flash chip (Veľkosť / Rýchlosť)',
  APPSIZE: 'Applikácia (Oddiel: Použité / Voľné)',
  FILESYSTEM: 'Súborový systém (Použité / Voľné)',
  BUFFER_SIZE: 'Buffer-max.veľkosť',
  COMPACT: 'Kompaktné',
  ENABLE_OTA: 'Povoliť OTA aktualizácie',
  DOWNLOAD_CUSTOMIZATION_TEXT: 'Stiahnutie prispôsobení entity',
  DOWNLOAD_SCHEDULE_TEXT: 'Stiahnutie plánovača udalostí',
  DOWNLOAD_SETTINGS_TEXT: 'Stiahnite si nastavenia aplikácie. Pri zdieľaní nastavení buďte opatrní, pretože tento súbor obsahuje heslá a iné citlivé systémové informácie.',
  UPLOAD_TEXT: 'Najskôr nahrajte nový súbor firmvéru (.bin), nastavenia alebo prispôsobenia (.json), pre voliteľné overenie nahrajte súbor (.md5)',
  UPLOADING: 'Nahrávanie',
  UPLOAD_DROP_TEXT: 'Potiahnúť a pripnúť súbor alebo kliknúť sem',
  ERROR: 'Neočakávaná chyba, prosím skúste to znova',
  TIME_SET: 'Nastavený čas',
  MANAGE_USERS: 'Správa používateľov',
  IS_ADMIN: 'je Admin',
  USER_WARNING: 'Musíte mať nakonfigurovaného aspoň jedného používateľa administrátora',
  ADD: 'Pridať',
  ACCESS_TOKEN_FOR: 'Prístupový token pre',
  ACCESS_TOKEN_TEXT: 'Nižšie uvedený token sa používa pri volaniach REST API, ktoré vyžadujú autorizáciu. Môže byť odovzdaný buď ako token Bearer v hlavičke Authorization (Autorizácia), alebo v parametri dotazu URL access_token.',
  GENERATING_TOKEN: 'Generovanie tokenu',
  USER: 'Užívateľ',
  MODIFY: 'Upraviť',
  SU_TEXT: 'Heslo su (superužívateľ) sa používa na podpisovanie autentifikačných tokenov a tiež na povolenie oprávnení správcu v rámci konzoly.',
  NOT_ENABLED: 'Nie je povolené',
  ERRORS_OF: '{0} errory',
  DISCONNECT_REASON: 'Dôvod odpojenia',
  ENABLE_MQTT: 'Povoliť MQTT',
  BROKER: 'Broker',
  CLIENT: 'Klient',
  BASE_TOPIC: 'Base',
  OPTIONAL: 'voliteľné',
  FORMATTING: 'Formátovanie',
  MQTT_FORMAT: 'Formát témy/záťaže',
  MQTT_NEST_1: 'Vnorené do jednej témy',
  MQTT_NEST_2: 'Ako jednotlivé témy',
  MQTT_RESPONSE: 'Publikovanie výstupu príkazu do témy `response`',
  MQTT_PUBLISH_TEXT_1: 'Zverejňovanie tém jednotlivých hodnôt pri zmene',
  MQTT_PUBLISH_TEXT_2: 'Publikovanie do tém príkazov (ioBroker)',
  MQTT_PUBLISH_TEXT_3: 'Povolenie zisťovania MQTT',
  MQTT_PUBLISH_TEXT_4: 'Predpona tém Discovery',
  MQTT_PUBLISH_TEXT_5: 'Typ zistenia',
  MQTT_PUBLISH_INTERVALS: 'Intervaly zverejňovania',
  MQTT_INT_BOILER: 'Kotly a tepelné čerpadlá',
  MQTT_INT_THERMOSTATS: 'Termostaty',
  MQTT_INT_SOLAR: 'Solárne moduly',
  MQTT_INT_MIXER: 'Zmiešavacie moduley',
  MQTT_INT_WATER: 'Voda moduley',
  MQTT_QUEUE: 'Fronta MQTT',
  DEFAULT: 'Predvolené',
  MQTT_ENTITY_FORMAT: 'ID formát entity',
  MQTT_ENTITY_FORMAT_0: 'Jedna inštancia, dlhý názov (v3.4)',
  MQTT_ENTITY_FORMAT_1: 'Jedna inštancia, krátky názov',
  MQTT_ENTITY_FORMAT_2: 'Viacero inštancií, krátky názov',
  MQTT_CLEAN_SESSION: 'Nastavenie čistej relácie',
  MQTT_RETAIN_FLAG: 'Vždy nastaviť príznak Retain',
  INACTIVE: 'Neaktívne',
  ACTIVE: 'Aktívne',
  UNKNOWN: 'Neznáme',
  SET_TIME: 'Nastavený čas',
  SET_TIME_TEXT: 'Na nastavenie času zadajte miestny dátum a čas nižšie',
  LOCAL_TIME: 'Lokálny čas',
  UTC_TIME: 'UTC čas',
  ENABLE_NTP: 'Povoliť NTP',
  NTP_SERVER: 'NTP Server',
  TIME_ZONE: 'Časová zóna',
  ACCESS_POINT: 'Prístupový bod',
  AP_PROVIDE: 'Povoliť prístupový bod',
  AP_PROVIDE_TEXT_1: 'vždy',
  AP_PROVIDE_TEXT_2: 'keď je WiFi odpojená',
  AP_PROVIDE_TEXT_3: 'nikdy',
  AP_PREFERRED_CHANNEL: 'Preferovaný kanál',
  AP_HIDE_SSID: 'Skryť SSID',
  AP_CLIENTS: 'AP klienti',
  AP_MAX_CLIENTS: 'Max klientov',
  AP_LOCAL_IP: 'Lokálna IP',
  NETWORK_SCAN: 'Scan WiFi siete',
  IDLE: 'Nečinné',
  LOST: 'Stratené',
  SCANNING: 'Scanovanie',
  SCAN_AGAIN: 'Scanovať znova',
  NETWORK_SCANNER: 'Sieťový scanner',
  NETWORK_NO_WIFI: 'WiFi siete nenájdené',
  NETWORK_BLANK_SSID: 'nechajte prázdne, ak chcete zakázať WiFi a povoliť ETH',
  NETWORK_BLANK_BSSID: 'ponechajte prázdne, ak chcete používať iba SSID',
  TX_POWER: 'Tx výkon',
  HOSTNAME: 'Hostname',
  NETWORK_DISABLE_SLEEP: 'Zakázanie režimu spánku WiFi',
  NETWORK_LOW_BAND: 'Používanie menšej šírky pásma WiFi',
  NETWORK_USE_DNS: 'Povoliť mDNS službu',
  NETWORK_ENABLE_CORS: 'Povoliť CORS',
  NETWORK_CORS_ORIGIN: 'CORS origin',
  NETWORK_ENABLE_IPV6: 'Povoliť podporu IPv6',
  NETWORK_FIXED_IP: 'Použiť fixnú IP adresu',
  NETWORK_GATEWAY: 'Brána',
  NETWORK_SUBNET: 'Maska podsiete',
  NETWORK_DNS: 'DNS servery',
  ADDRESS_OF: '{0} adresa',
  ADMIN: 'Admin',
  GUEST: 'Hosť',
  NEW: 'Nová',
  NEW_NAME_OF: 'Nový názov {0}',
  ENTITY: 'entita',
  MIN: 'min',
  MAX: 'max',
  BLOCK_NAVIGATE_1: 'Máte neuložené zmeny',
  BLOCK_NAVIGATE_2: 'Ak prejdete na inú stránku, neuložené zmeny sa stratia. Ste si istí, že chcete opustiť túto stránku?',
  STAY: 'Zostať',
  LEAVE: 'Opustiť',
  SCHEDULER: 'Plánovač',
  SCHEDULER_HELP_1: 'Automatizujte príkazy pridaním naplánovaných udalostí nižšie. Nastavte jedinečné meno na aktiváciu/deaktiváciu cez API/MQTT.',
  SCHEDULER_HELP_2: 'Použite 00:00 na jednorazové spustenie pri štarte',
  SCHEDULE: 'Plánovač',
  TIME: 'Čas',
  TIMER: 'Časovač',
  SCHEDULE_UPDATED: 'Plánovanie aktualizované',
  SCHEDULE_TIMER_1: 'pri spustení',
  SCHEDULE_TIMER_2: 'každú minútu',
  SCHEDULE_TIMER_3: 'každú hodinu',
  CUSTOM_ENTITIES: 'Vlastné entity',
  ENTITIES_HELP_1: 'Získavanie vlastných entít zo zbernice EMS',
  ENTITIES_UPDATED: 'Aktualizované entity',
  WRITEABLE: 'Zapísateľný',
  SHOWING: 'Zobrazenie',
  SEARCH: 'Vyhľadať',
  CERT: 'Koreňový certifikát TLS (ak chcete vypnúť TLS, nechajte prázdne)',
  ENABLE_TLS: 'Povoliť TLS',
  ON: 'Zap',
  OFF: 'Vyp',
  POLARITY: 'Polarita',
  ACTIVEHIGH: 'Aktívny Vysoký',
  ACTIVELOW: 'Aktívny Nízky',
  UNCHANGED: 'Nezmenené',
  ALWAYS: 'Vždy',
  ACTIVITY: 'Aktivita', 
  CONFIGURE: 'Konfiguracia {0}'
};

export default sk;
