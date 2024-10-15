import type { Translation } from '../i18n-types';

const cz: Translation = {
  LANGUAGE: 'Jazyk',
  RETRY: 'Zkusit znovu',
  LOADING: 'Načítání',
  IS_REQUIRED: '{0} je vyžadováno',
  SIGN_IN: 'Přihlásit se',
  SIGN_OUT: 'Odhlásit se',
  USERNAME: 'Uživatelské jméno',
  PASSWORD: 'Heslo',
  SU_PASSWORD: 'Heslo superuživatele',
  SETTINGS_OF: '{0} Nastavení ',
  HELP: 'Nápověda',
  LOGGED_IN: 'Přihlášen jako {name}',
  PLEASE_SIGNIN: 'Pro pokračování se prosím přihlaste',
  UPLOAD_SUCCESSFUL: 'Nahrání bylo úspěšné',
  DOWNLOAD_SUCCESSFUL: 'Stažení bylo úspěšné',
  INVALID_LOGIN: 'Neplatné přihlašovací údaje',
  NETWORK: 'Síť',
  SECURITY: 'Zabezpečení',
  ONOFF_CAP: 'ZAP/VYP',
  ONOFF: 'zap/vyp',
  TYPE: 'Typ',
  DESCRIPTION: 'Popis',
  ENTITIES: 'entity',
  REFRESH: 'Obnovit',
  EXPORT: 'Exportovat',
  FAVORITES: "Oblíbené",
  DEVICE_DETAILS: 'Podrobnosti zařízení',
  ID_OF: '{0} ID',
  DEVICE: 'Zařízení',
  PRODUCT: 'Produkt',
  VERSION: 'Verze',
  BRAND: 'Značka',
  ENTITY_NAME: 'Název entity',
  VALUE: '{{value|Value}}',
  DEVICES: 'Zařízení',
  SENSORS: 'Senzory',
  RUN_COMMAND: 'Zavolat příkaz',
  CHANGE_VALUE: 'Změnit hodnotu',
  CANCEL: 'Zrušit',
  RESET: 'Resetovat',
  APPLY_CHANGES: 'Použít změny ({0})',
  UPDATE: 'Aktualizovat',
  EXECUTE: 'Provést',
  REMOVE: 'Odebrat',
  PROBLEM_UPDATING: 'Problém s aktualizací',
  PROBLEM_LOADING: 'Problém s načítáním',
  ANALOG_SENSOR: 'Analogový senzor',
  ANALOG_SENSORS: 'Analogové senzory',
  SETTINGS: 'Nastavení',
  UPDATED_OF: '{0} Aktualizováno',
  UPDATE_OF: '{0} Aktualizace',
  REMOVED_OF: '{0} Odebráno',
  DELETION_OF: '{0} Smazání',
  OFFSET: 'Offset',
  FACTOR: 'Faktor',
  FREQ: 'Frekvence',
  DUTY_CYCLE: 'Pracovní cyklus',
  UNIT: 'Jednotka',
  STARTVALUE: 'Počáteční hodnota',
  WARN_GPIO: 'Upozornění: buďte opatrní při přiřazování GPIO!',
  EDIT: 'Upravit',
  SENSOR: 'Senzor',
  TEMP_SENSOR: 'Teplotní senzor',
  TEMP_SENSORS: 'Teplotní senzory',
  WRITE_CMD_SENT: 'Zapisovací příkaz odeslán',
  EMS_BUS_WARNING: 'EMS sběrnice je odpojena. Pokud toto varování přetrvává i po několika sekundách, zkontrolujte nastavení a profil desky',
  EMS_BUS_SCANNING: 'Prohledávání zařízení na EMS sběrnici...',
  CONNECTED: 'Připojeno',
  TX_ISSUES: 'Problémy s Tx - zkontrolujte režim Tx',
  DISCONNECTED: 'Odpojeno',
  EMS_SCAN: 'Opravdu chcete zahájit úplné skenování zařízení na EMS sběrnici?',
  DATA_TRAFFIC: 'Datový provoz',
  EMS_DEVICE: 'EMS zařízení',
  SUCCESS: 'ÚSPĚCH',
  FAIL: 'SELHÁNÍ',
  QUALITY: 'KVALITA',
  SCAN: 'Skenovat',
  STATUS_NAMES: [
    'Přijaté EMS telegramy (Rx)',
    'Čtení EMS (Tx)',
    'Zápisy EMS (Tx)',
    'Čtení teplotních senzorů',
    'Čtení analogových senzorů',
    'Publikace MQTT',
    'API volání',
    'Syslog zprávy'
  ],
  NUM_DAYS: '{num} d{{ní|en|ní|ní|ní|ní}}',
  NUM_SECONDS: '{num} sek{{und|unda|undy|undy|und|und}}',
  NUM_HOURS: '{num} hod{{in|ina|iny|iny|in|in}}',
  NUM_MINUTES: '{num} minu{{t|ta|ty|ty|t|t}}',
  APPLICATION: 'Aplikace',
  CUSTOMIZATIONS: 'Přizpůsobení',
  APPLICATION_RESTARTING: 'EMS-ESP se restartuje',
  BOARD_PROFILE: 'Profil desky',
  CUSTOM: 'Vlastní',
  GPIO_OF: 'GPIO {0}',
  BUTTON: 'Tlačítko',
  TEMPERATURE: 'Teplota',
  PHY_TYPE: 'Typ Eth PHY',
  DISABLED: 'zakázáno',
  TX_MODE: 'EMS Tx režim',
  HARDWARE: 'Hardware',
  EMS_BUS: '{{BUS|EMS BUS}}',
  GENERAL_OPTIONS: 'Obecné možnosti',
  LANGUAGE_ENTITIES: 'Jazyk (pro entity zařízení)',
  HIDE_LED: 'Skrýt LED',
  ENABLE_TELNET: 'Povolit Telnet konzoli',
  ENABLE_ANALOG: 'Povolit analogové senzory',
  CONVERT_FAHRENHEIT: 'Převést hodnoty teploty na Fahrenheit',
  BYPASS_TOKEN: 'Obejít autorizaci tokenu při API voláních',
  READONLY: 'Povolit režim jen pro čtení (blokuje všechny odchozí příkazy EMS Tx Write)',
  UNDERCLOCK_CPU: 'Snížit takt CPU',
  REMOTE_TIMEOUT: 'Časový limit vzdáleného připojení',
  REMOTE_TIMEOUT_EN: 'Zakázat vzdálený přístup při chybějící teplotě v místnosti',
  HEATINGOFF: 'Spustit kotel s vynuceným vypnutým topením',
  MIN_DURATION: 'Čekací doba',
  ENABLE_SHOWER_TIMER: 'Povolit časovač sprchy',
  ENABLE_SHOWER_ALERT: 'Povolit upozornění na sprchu',
  TRIGGER_TIME: 'Čas spuštění',
  COLD_SHOT_DURATION: 'Délka studeného výstřiku',
  FORMATTING_OPTIONS: 'Možnosti formátování',
  BOOLEAN_FORMAT_DASHBOARD: 'Formát booleovské hodnoty na webu',
  BOOLEAN_FORMAT_API: 'Formát booleovské hodnoty v API/MQTT',
  ENUM_FORMAT: 'Formát enum v API/MQTT',
  INDEX: 'Index',
  ENABLE_PARASITE: 'Povolit 1-Wire parazitní napájení',
  LOGGING: 'Protokolování',
  LOG_HEX: 'Protokolovat EMS telegramy v hexadecimálním formátu',
  ENABLE_SYSLOG: 'Povolit Syslog',
  LOG_LEVEL: 'Úroveň protokolování',
  MARK_INTERVAL: 'Interval označení',
  SECONDS: 'sekundy',
  MINUTES: 'minuty',
  HOURS: 'hodiny',
  RESTART: 'Restart',
  RESTART_TEXT: 'EMS-ESP musí být restartován, aby se změny systému projevily',
  RESTART_CONFIRM: 'Opravdu chcete restartovat EMS-ESP?',
  COMMAND: 'Příkaz',
  CUSTOMIZATIONS_RESTART: 'Všechna přizpůsobení byla odstraněna. Restartování...',
  CUSTOMIZATIONS_FULL: 'Vybrané entity překročily limit. Uložte je po částech',
  CUSTOMIZATIONS_SAVED: 'Přizpůsobení uloženo',
  CUSTOMIZATIONS_HELP_1: 'Vyberte zařízení a přizpůsobte možnosti entit nebo klikněte pro přejmenování',
  CUSTOMIZATIONS_HELP_2: 'označit jako oblíbené',
  CUSTOMIZATIONS_HELP_3: 'zakázat akci zápisu',
  CUSTOMIZATIONS_HELP_4: 'vyloučit z MQTT a API',
  CUSTOMIZATIONS_HELP_5: 'skrýt z Zařízení',
  CUSTOMIZATIONS_HELP_6: 'odstranit z paměti',
  SELECT_DEVICE: 'Vyberte zařízení',
  SET_ALL: 'nastavit vše',
  OPTIONS: 'Možnosti',
  NAME: 'Název',
  CUSTOMIZATIONS_RESET: 'Opravdu chcete odstranit všechna přizpůsobení včetně vlastních nastavení teplotních a analogových senzorů?',
  SUPPORT_INFORMATION: 'Podpora',
  HELP_INFORMATION_1: 'Navštivte online wiki pro pokyny, jak konfigurovat EMS-ESP',
  HELP_INFORMATION_2: 'Pro živý chat se komunitou se připojte k našemu serveru Discord',
  HELP_INFORMATION_3: 'Chcete-li požádat o funkci nebo nahlásit chybu',
  HELP_INFORMATION_4: 'Stáhněte a připojte informace o podpoře pro rychlejší odezvu při hlášení problému',
  HELP_INFORMATION_5: 'Pro pomoc a dotazy kontaktujte svého instalatéra.',
  UPLOAD: 'Nahrát',
  DOWNLOAD: '{{S|s|s}}táhnout',
  INSTALL: 'Instalovat {0}',
  ABORTED: 'přerušeno',
  FAILED: 'neúspěšné',
  SUCCESSFUL: 'úspěšné',
  SYSTEM: 'Systém',
  LOG_OF: '{0} Záznam',
  STATUS_OF: '{0} Stav',
  DOWNLOAD_UPLOAD: 'Stáhnout/Nahrát',
  CLOSE: 'Zavřít',
  USE: 'Použít',
  FACTORY_RESET: 'Obnovení továrního nastavení',
  SYSTEM_FACTORY_TEXT: 'Zařízení bylo obnoveno do továrního nastavení a nyní se restartuje',
  SYSTEM_FACTORY_TEXT_DIALOG: 'Opravdu chcete resetovat EMS-ESP do továrního nastavení?',
  STABLE: 'Stabilní',
  DEVELOPMENT: 'Vývojová verze',
  RELEASE_NOTES: 'poznámky k vydání',
  EMS_ESP_VER: 'Verze firmwaru',
  UPTIME: 'Doba provozu systému',
  FREE_MEMORY: 'Volná paměť',
  PSRAM: 'PSRAM (Velikost / Volná)',
  FLASH: 'Flash čip (Velikost, Rychlost)',
  APPSIZE: 'Aplikace (Partition: Použito / Volné)',
  FILESYSTEM: 'Souborový systém (Použito / Volné)',
  BUFFER_SIZE: 'Maximální velikost bufferu',
  COMPACT: 'Kompaktní',
  DOWNLOAD_SETTINGS_TEXT: 'Vytvořte zálohu svého nastavení a konfigurace',
  UPLOAD_TEXT: 'Nahrajte nový soubor firmwaru (.bin) nebo záložní soubor (.json)',
  UPLOAD_DROP_TEXT: 'Přetáhněte soubor nebo klikněte sem',
  ERROR: 'Neočekávaná chyba, zkuste to prosím znovu',
  TIME_SET: 'Čas nastaven',
  MANAGE_USERS: 'Spravovat uživatele',
  IS_ADMIN: 'je Admin',
  USER_WARNING: 'Musí být nakonfigurován alespoň jeden uživatel s oprávněním admin',
  ADD: 'Přidat',
  ACCESS_TOKEN_FOR: 'Přístupový token pro',
  ACCESS_TOKEN_TEXT: 'Níže uvedený token se používá s REST API voláními, která vyžadují autorizaci. Lze jej předat buď jako Bearer token v hlavičce Authorization, nebo v parametru URL access_token.',
  GENERATING_TOKEN: 'Generování tokenu',
  USER: 'Uživatel',
  MODIFY: 'Upravit',
  SU_TEXT: 'Heslo superuživatele (su) se používá k podepisování autentizačních tokenů a také k povolení administrátorských práv v konzole.',
  NOT_ENABLED: 'Nepovoleno',
  ERRORS_OF: '{0} Chyby',
  DISCONNECT_REASON: 'Důvod odpojení',
  ENABLE_MQTT: 'Povolit MQTT',
  BROKER: 'Broker',
  CLIENT: 'Klient',
  BASE_TOPIC: 'Základní téma',
  OPTIONAL: 'volitelné',
  FORMATTING: 'Formátování',
  MQTT_FORMAT: 'Formát Téma/Payload',
  MQTT_NEST_1: 'Vnořené do jednoho tématu',
  MQTT_NEST_2: 'Jako samostatná témata',
  MQTT_RESPONSE: 'Publikovat výstup příkazů do tématu `response`',
  MQTT_PUBLISH_TEXT_1: 'Publikovat témata jednotlivých hodnot při změně',
  MQTT_PUBLISH_TEXT_2: 'Publikovat do příkazových témat (ioBroker)',
  MQTT_PUBLISH_TEXT_3: 'Povolit MQTT Discovery',
  MQTT_PUBLISH_TEXT_4: 'Prefix pro Discovery témata',
  MQTT_PUBLISH_TEXT_5: 'Typ Discovery',
  MQTT_PUBLISH_INTERVALS: 'Intervaly publikování',
  MQTT_INT_BOILER: 'Kotly a tepelná čerpadla',
  MQTT_INT_THERMOSTATS: 'Termostaty',
  MQTT_INT_SOLAR: 'Solární moduly',
  MQTT_INT_MIXER: 'Míchací moduly',
  MQTT_INT_WATER: 'Vodní moduly',
  MQTT_QUEUE: 'MQTT fronta',
  DEFAULT: 'Výchozí',
  MQTT_ENTITY_FORMAT: 'Formát ID entity',
  MQTT_ENTITY_FORMAT_0: 'Jediná instance, dlouhý název (v3.4)',
  MQTT_ENTITY_FORMAT_1: 'Jediná instance, krátký název',
  MQTT_ENTITY_FORMAT_2: 'Více instancí, krátký název',
  MQTT_CLEAN_SESSION: 'Nastavit čistou relaci',
  MQTT_RETAIN_FLAG: 'Vždy nastavit příznak Retain',
  INACTIVE: 'Neaktivní',
  ACTIVE: 'Aktivní',
  UNKNOWN: 'Neznámé',
  SET_TIME: 'Nastavit čas',
  SET_TIME_TEXT: 'Zadejte místní datum a čas pro nastavení času',
  LOCAL_TIME: 'Místní čas',
  UTC_TIME: 'Čas UTC',
  ENABLE_NTP: 'Povolit NTP',
  NTP_SERVER: 'NTP server',
  TIME_ZONE: 'Časová zóna',
  ACCESS_POINT: 'Přístupový bod',
  AP_PROVIDE: 'Povolit přístupový bod',
  AP_PROVIDE_TEXT_1: 'Vždy',
  AP_PROVIDE_TEXT_2: 'Když je WiFi odpojena',
  AP_PROVIDE_TEXT_3: 'Nikdy',
  AP_PREFERRED_CHANNEL: 'Preferovaný kanál',
  AP_HIDE_SSID: 'Skrýt SSID',
  AP_CLIENTS: 'Klienti AP',
  AP_MAX_CLIENTS: 'Maximální počet klientů',
  AP_LOCAL_IP: 'Místní IP',
  NETWORK_SCAN: 'Skenovat WiFi sítě',
  IDLE: 'Nečinný',
  LOST: 'Ztraceno',
  SCANNING: 'Skenování',
  SCAN_AGAIN: 'Skenovat znovu',
  NETWORK_SCANNER: 'Síťový skener',
  NETWORK_NO_WIFI: 'Nenalezeny žádné WiFi sítě',
  NETWORK_BLANK_SSID: 'nechte prázdné pro deaktivaci WiFi a povolení ETH',
  NETWORK_BLANK_BSSID: 'nechte prázdné pro použití pouze SSID',
  TX_POWER: 'Vysílací výkon',
  HOSTNAME: 'Název hostitele',
  NETWORK_DISABLE_SLEEP: 'Zakázat režim spánku WiFi',
  NETWORK_LOW_BAND: 'Použít nižší šířku pásma WiFi',
  NETWORK_USE_DNS: 'Povolit mDNS službu',
  NETWORK_ENABLE_CORS: 'Povolit CORS',
  NETWORK_CORS_ORIGIN: 'Původ CORS',
  NETWORK_FIXED_IP: 'Použít pevnou IP adresu',
  NETWORK_GATEWAY: 'Brána',
  NETWORK_SUBNET: 'Maska podsítě',
  NETWORK_DNS: 'DNS servery',
  ADDRESS_OF: '{0} Adresa',
  ADMINISTRATOR: 'Administrátor',
  GUEST: 'Host',
  NEW: 'Nový',
  NEW_NAME_OF: '{0} Nový název',
  ENTITY: 'entita',
  MIN: 'min',
  MAX: 'max',
  BLOCK_NAVIGATE_1: 'Máte neuložené změny',
  BLOCK_NAVIGATE_2: 'Pokud přejdete na jinou stránku, vaše neuložené změny budou ztraceny. Opravdu chcete opustit tuto stránku?',
  STAY: 'Zůstat',
  LEAVE: 'Odejít',
  SCHEDULER: 'Plánovač',
  SCHEDULER_HELP_1: 'Automatizujte příkazy přidáním naplánovaných událostí níže. Nastavte jedinečný název pro povolení/zakázání aktivace přes API/MQTT.',
  SCHEDULER_HELP_2: 'Použijte 00:00 pro spuštění při startu',
  SCHEDULE: 'Harmonogram',
  TIME: 'Čas',
  TIMER: 'Časovač',
  ONCHANGE: 'Při změně',
  CONDITION: 'Podmínka',
  IMMEDIATE: 'Ihned',
  SCHEDULE_UPDATED: 'Harmonogram aktualizován',
  SCHEDULE_TIMER_1: 'při startu',
  SCHEDULE_TIMER_2: 'každou minutu',
  SCHEDULE_TIMER_3: 'každou hodinu',
  CUSTOM_ENTITIES: 'Vlastní entity',
  ENTITIES_HELP_1: 'Definujte vlastní EMS entity nebo dynamické uživatelské proměnné',
  ENTITIES_UPDATED: 'Entity aktualizovány',
  WRITEABLE: 'Zapisovatelné',
  SHOWING: 'Zobrazuje se',
  SEARCH: 'Hledat',
  CERT: 'TLS kořenový certifikát (nechte prázdné pro nezabezpečené připojení)',
  ENABLE_TLS: 'Povolit TLS',
  ON: 'Zapnuto',
  OFF: 'Vypnuto',
  POLARITY: 'Polarita',
  ACTIVEHIGH: 'Aktivní vysoká',
  ACTIVELOW: 'Aktivní nízká',
  UNCHANGED: 'Beze změny',
  ALWAYS: 'Vždy',
  ACTIVITY: 'Aktivita',
  CONFIGURE: '{0} Konfigurovat',
  SYSTEM_MEMORY: 'Paměť systému',
  APPLICATION_SETTINGS_1: 'Upravit nastavení aplikace EMS-ESP',
  SECURITY_1: 'Přidat nebo odebrat uživatele',
  DOWNLOAD_UPLOAD_1: 'Stáhnout a nahrát nastavení a firmware',
  MODULES: 'Moduly',
  MODULES_1: 'Aktivovat nebo deaktivovat externí moduly',
  MODULES_UPDATED: 'Moduly aktualizovány',
  MODULES_DESCRIPTION: 'Klikněte na modul pro aktivaci nebo deaktivaci modulů knihovny EMS-ESP',
  MODULES_NONE: 'Nenalezeny žádné externí moduly',
  RENAME: 'Přejmenovat',
  ENABLE_MODBUS: 'Povolit Modbus',
  VIEW_LOG: 'Zobrazit záznam pro diagnostiku problémů',
  UPLOAD_DRAG: 'přetáhněte soubor sem nebo klikněte pro výběr',
  SERVICES: 'Služby',
  ALLVALUES: 'Všechny hodnoty',
  SPECIAL_FUNCTIONS: 'Speciální funkce',
  WAIT_FIRMWARE: 'Firmware se nahrává a instaluje',
  INSTALL_VERSION: 'Tímto se nainstaluje verze {0}. Jste si jistí?',
  SWITCH_DEV: 'přepnout na vývojovou verzi',
  UPGRADE_AVAILABLE: 'Je k dispozici aktualizace firmwaru!',
  LATEST_VERSION: 'Používáte nejnovější verzi firmwaru.',
  PLEASE_WAIT: 'Prosím čekejte',
  RESTARTING_PRE: 'Inicializace',
  RESTARTING_POST: 'Příprava',
  AUTO_SCROLL: 'Automatické rolování',
  DASHBOARD: 'Dashboard',
  NO_DATA: 'Žádná data nejsou k dispozici',
  DASHBOARD_1: 'Přizpůsobte si dashboard označením EMS entit jako Oblíbené pomocí modulu Přizpůsobení.',
};

export default cz;
