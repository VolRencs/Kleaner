// Live version + release link from GitHub (fallback stays if the API is unreachable)
const REPO = 'VolRencs/Kleaner';
async function loadVersion() {
  const els = document.querySelectorAll('[data-kleaner-version]');
  try {
    const res = await fetch('https://api.github.com/repos/' + REPO + '/releases/latest', {
      headers: { Accept: 'application/vnd.github+json' }
    });
    if (!res.ok) throw new Error('HTTP ' + res.status);
    const data = await res.json();
    const tag = String(data.tag_name || '').trim();
    if (tag) els.forEach(el => { el.textContent = tag.startsWith('v') ? tag : 'v' + tag; });
  } catch (err) { /* keep fallback version */ }
}
const idle = window.requestIdleCallback || (fn => setTimeout(fn, 1200));
idle(loadVersion);

// RU / EN language switch for the whole site
const I18N = {
  ru: {
    'doc.title': 'Kleaner — оптимизатор и монитор системы Linux',
    'hero.title': 'Мониторинг, службы и очистка <span class="blue">в одном приложении.</span>',
    'hero.desc': 'Kleaner показывает загрузку CPU, памяти, дисков и сети, управляет службами systemd и автозапуском, очищает кэши и журналы. Приложение собрано на Qt 6 и KDE Frameworks 6 для Arch Linux.',
    'hero.install': 'Установка', 'hero.forArch': 'для Arch Linux',
    'hero.github': 'Исходный код',
    'hero.release': 'Последний релиз:',
    'feat.title': 'Возможности',
    'feat.sub': 'Девять страниц: мониторинг, процессы, службы, автозапуск, очистка, hosts и настройки.',
    'f1t': 'Мониторинг системы',
    'f1d': 'CPU (общая и по ядрам), память, диск и сеть с обновлением раз в секунду.',
    'f2t': 'История метрик',
    'f2d': 'Графики за 60 секунд по нагрузке, памяти, диску и сети.',
    'f3t': 'Процессы',
    'f3d': 'Сортировка по столбцам, поиск по имени, пользователю и PID, завершение через SIGTERM и SIGKILL.',
    'f4t': 'Службы systemd',
    'f4d': 'Запуск, остановка, перезапуск и автозапуск юнитов через D-Bus и polkit.',
    'f5t': 'Автозапуск',
    'f5d': 'Записи из ~/.config/autostart и /etc/xdg/autostart: включение, отключение и правка.',
    'f6t': 'Очистка системы',
    'f6d': 'Корзина, кэши, /tmp, /var/log, systemd journal, кэш pacman и пакеты-сироты.',
    'f7t': '/etc/hosts',
    'f7d': 'Таблица записей с проверкой IPv4/IPv6 и атомарной записью через KAuth.',
    'f8t': 'Языки и оформление',
    'f8d': 'Русский и украинский интерфейс, смена языка без перезапуска, тёмная тема.',
    'gal.eyebrow': 'СТРАНИЦЫ',
    'gal.title': 'Страницы приложения',
    'gal.sub': 'Скриншоты из обычной сессии: интерфейс и элементы управления.',
    'dash.t': 'Обзор',
    'dash.d': 'Три кольцевых индикатора (CPU, память, корневой раздел) и таблица характеристик: хост, дистрибутив, ядро, модель CPU, пользователь и время работы. Ниже — накопители с занятым местом.',
    'dash.a': 'Кольцевые индикаторы: CPU, память, корневой раздел',
    'dash.b': 'Характеристики: хост, дистрибутив, ядро, CPU, пользователь, uptime',
    'dash.c': 'Накопители: точка монтирования, файловая система, занято/всего',
    'res.t': 'Ресурсы',
    'res.d': 'Пять общих графиков с историей за 60 секунд и раскладка нагрузки по ядрам. Данные накапливаются, пока приложение запущено, и не сбрасываются при переходе между страницами.',
    'res.a': 'CPU: загрузка и средняя нагрузка за 1/5/15 минут',
    'res.b': 'Память и swap',
    'res.c': 'Диск: чтение и запись; сеть: приём и передача',
    'proc.t': 'Процессы',
    'proc.d': 'Таблица процессов: сортировка щелчком по столбцу, поиск по имени, пользователю и PID. Список обновляется раз в 2 секунды, позиция прокрутки и выделенная строка сохраняются.',
    'proc.a': 'Столбцы: имя, PID, пользователь, CPU, память, RSS, состояние, команда',
    'proc.b': 'Завершение (SIGTERM) и принудительное завершение (SIGKILL)',
    'proc.c': 'Обновление по кнопке или автоматически раз в 2 секунды',
    'svc.t': 'Службы',
    'svc.d': 'Юниты systemd с описанием, состоянием и переключателем автозапуска. Запуск, остановка и перезапуск идут через D-Bus; пароль запрашивает polkit.',
    'svc.a': 'Запуск, остановка и перезапуск юнитов',
    'svc.b': 'Включение и отключение автозапуска (enable/disable)',
    'svc.c': 'Сортировка по имени, состоянию и автозапуску',
    'start.t': 'Автозапуск',
    'start.d': 'Записи из ~/.config/autostart и /etc/xdg/autostart. Системные записи при отключении копируются в пользовательский каталог.',
    'start.a': 'Включение и отключение записей',
    'start.b': 'Добавление, редактирование и удаление .desktop',
    'start.c': 'Поля: имя, команда, комментарий, значок',
    'clean.t': 'Очистка системы',
    'clean.d': 'Семь категорий с подсчётом размера. Отмечаете нужные пункты — приложение показывает итоговый объём и удаляет выбранное; выбор сохраняется между запусками.',
    'clean.a': 'Корзина, кэши приложений, отчёты о сбоях',
    'clean.b': 'Системные журналы: ротация и полная очистка архива journal',
    'clean.c': 'Кэш pacman и удаление пакетов-сирот',
    'hosts.t': 'Хосты',
    'hosts.d': 'Записи /etc/hosts в виде таблицы: адрес и имена хостов. Перед сохранением проверяются IPv4/IPv6 и имена, запись выполняется через KAuth.',
    'hosts.a': 'Просмотр, добавление, правка и удаление записей',
    'hosts.b': 'Проверка IPv4/IPv6 и имён хостов',
    'hosts.c': 'Сохранение через KAuth с подтверждением',
    'set.t': 'Настройки',
    'set.d': 'Стартовая страница, поведение при закрытии (спросить, свернуть в трей, выйти), значок в трее и язык интерфейса. Настройки хранятся в ~/.config/kleanerrc.',
    'set.a': 'Стартовая страница приложения',
    'set.b': 'Поведение при закрытии и значок в трее',
    'set.c': 'Язык интерфейса: системный, русский, украинский и другие',
    'about.t': 'О программе',
    'about.d': 'Версия, лицензия, автор и ссылки: сайт, репозиторий и трекер.',
    'about.a': 'Версия и лицензия GPL-3.0',
    'about.b': 'Ссылки на сайт, GitHub и трекер',
    'about.c': 'Stacer указан как исходный проект',
    'show.eyebrow': 'ТЕХНИЧЕСКИЕ ДЕТАЛИ',
    'show.title': 'Qt 6, KDE Frameworks 6 <span class="blue">и Wayland.</span>',
    'show.desc': 'Интерфейс — Qt Quick Controls с собственным тёмным стилем. Службы и процессы работают через системный D-Bus, очистка и /etc/hosts — через KAuth-helper с подтверждением polkit.',
    'c1': 'Одно окно и один процесс, без встроенного терминала',
    'c2': 'Systemd — через D-Bus, файловые операции — через KAuth',
    'c3': 'Arch Linux: сборка через PKGBUILD или CMake',
    'show.tech': 'Технологии:',
    'foot.rel': 'Релизы', 'foot.src': 'Исходный код', 'foot.iss': 'Сообщить об ошибке',
    'foot.right': 'Открытый код &nbsp;•&nbsp; Лицензия GPL-3.0 &nbsp;•&nbsp; Форк Stacer'
  },
  en: {
    'doc.title': 'Kleaner — Linux System Optimizer',
    'hero.title': 'Monitoring, services and cleanup <span class="blue">in a single app.</span>',
    'hero.desc': 'Kleaner shows CPU, memory, disk and network usage, manages systemd services and startup entries, and cleans caches and logs. Built with Qt 6 and KDE Frameworks 6 for Arch Linux.',
    'hero.install': 'Install', 'hero.forArch': 'for Arch Linux',
    'hero.github': 'Source code',
    'hero.release': 'Latest release:',
    'feat.title': 'Features',
    'feat.sub': 'Nine pages: monitoring, processes, services, startup, cleanup, hosts and settings.',
    'f1t': 'System monitoring',
    'f1d': 'CPU (total and per core), memory, disk and network, refreshed every second.',
    'f2t': 'Metric history',
    'f2d': '60-second charts for load, memory, disk and network.',
    'f3t': 'Processes',
    'f3d': 'Sort by columns, search by name, user or PID, terminate with SIGTERM or SIGKILL.',
    'f4t': 'systemd services',
    'f4d': 'Start, stop, restart and enable units over D-Bus and polkit.',
    'f5t': 'Startup apps',
    'f5d': 'Entries from ~/.config/autostart and /etc/xdg/autostart: toggle and edit.',
    'f6t': 'System cleaner',
    'f6d': 'Trash, caches, /tmp, /var/log, systemd journal, pacman cache and orphan packages.',
    'f7t': '/etc/hosts',
    'f7d': 'Entry table with IPv4/IPv6 validation and an atomic write through KAuth.',
    'f8t': 'Languages and appearance',
    'f8d': 'Russian and Ukrainian interfaces, runtime language switching, dark theme.',
    'gal.eyebrow': 'PAGES',
    'gal.title': 'Application pages',
    'gal.sub': 'Screenshots from a regular session: the interface and its controls.',
    'dash.t': 'Overview',
    'dash.d': 'Three ring gauges (CPU, memory, root volume) and a spec table: host, distribution, kernel, CPU model, user and uptime. Below — mounted volumes with used space.',
    'dash.a': 'Ring gauges: CPU, memory, root volume',
    'dash.b': 'Spec table: host, distribution, kernel, CPU, user, uptime',
    'dash.c': 'Volumes: mount point, filesystem, used/total',
    'res.t': 'Resources',
    'res.d': 'Five overview charts with 60 seconds of history plus a per-core breakdown. Data accumulates while the app is running and is not reset when you switch pages.',
    'res.a': 'CPU: usage and 1/5/15-minute load average',
    'res.b': 'Memory and swap',
    'res.c': 'Disk: read and write; network: receive and transmit',
    'proc.t': 'Processes',
    'proc.d': 'Process table with click-to-sort columns and search by name, user or PID. The list refreshes every 2 seconds while the scroll position and selected row stay put.',
    'proc.a': 'Columns: name, PID, user, CPU, memory, RSS, state, command',
    'proc.b': 'Terminate (SIGTERM) and force-kill (SIGKILL)',
    'proc.c': 'Refresh by button or automatically every 2 seconds',
    'svc.t': 'Services',
    'svc.d': 'systemd units with description, state and an autostart switch. Start, stop and restart go over D-Bus; polkit asks for a password when needed.',
    'svc.a': 'Start, stop and restart units',
    'svc.b': 'Enable and disable autostart',
    'svc.c': 'Sort by name, state or autostart',
    'start.t': 'Startup apps',
    'start.d': 'Entries from ~/.config/autostart and /etc/xdg/autostart. Disabling a system entry copies it into your user directory.',
    'start.a': 'Enable and disable entries',
    'start.b': 'Add, edit and remove .desktop files',
    'start.c': 'Fields: name, command, comment, icon',
    'clean.t': 'System cleaner',
    'clean.d': 'Seven categories with size calculation. Tick the items you need, see the total size and remove them; selections persist across restarts.',
    'clean.a': 'Trash, application caches, crash reports',
    'clean.b': 'System logs: rotation and full journal archive cleanup',
    'clean.c': 'pacman cache and orphan package removal',
    'hosts.t': 'Hosts',
    'hosts.d': 'A table of /etc/hosts entries: address and hostnames. IPv4/IPv6 and names are validated before saving; the write goes through KAuth.',
    'hosts.a': 'View, add, edit and delete entries',
    'hosts.b': 'IPv4/IPv6 and hostname validation',
    'hosts.c': 'Save through KAuth with confirmation',
    'set.t': 'Settings',
    'set.d': 'Startup page, close behavior (ask, minimize to tray, quit), tray icon and interface language. Settings are stored in ~/.config/kleanerrc.',
    'set.a': 'Application startup page',
    'set.b': 'Close behavior and tray icon',
    'set.c': 'Interface language: system, Russian, Ukrainian and more',
    'about.t': 'About',
    'about.d': 'Version, license, author and links: website, repository and issue tracker.',
    'about.a': 'Version and GPL-3.0 license',
    'about.b': 'Website, GitHub and issue tracker links',
    'about.c': 'Stacer credited as the original project',
    'show.eyebrow': 'IMPLEMENTATION',
    'show.title': 'Qt 6, KDE Frameworks 6 <span class="blue">and Wayland.</span>',
    'show.desc': 'The interface is Qt Quick Controls with its own dark theme. Services and processes use the system D-Bus, while cleanup and /etc/hosts go through the KAuth helper with polkit confirmation.',
    'c1': 'One window and one process, no embedded terminal',
    'c2': 'systemd over D-Bus, file operations through KAuth',
    'c3': 'Arch Linux: build with PKGBUILD or CMake',
    'show.tech': 'Technologies:',
    'foot.rel': 'Releases', 'foot.src': 'Source code', 'foot.iss': 'Report an issue',
    'foot.right': 'Open source &nbsp;•&nbsp; GPL-3.0 License &nbsp;•&nbsp; Fork of Stacer'
  }
};
let lang = 'ru';
try {
  const saved = localStorage.getItem('kleaner-lang');
  if (saved === 'en' || saved === 'ru') lang = saved;
  else lang = (navigator.language || 'ru').toLowerCase().startsWith('en') ? 'en' : 'ru';
} catch (e) { /* private mode */ }
const reduceMotion = window.matchMedia('(prefers-reduced-motion: reduce)');
let firstPaint = true;
function applyLang() {
  try { localStorage.setItem('kleaner-lang', lang); } catch (e) {}
  document.documentElement.lang = lang;
  document.title = I18N[lang]['doc.title'];
  document.querySelectorAll('[data-i18n]').forEach(el => {
    const v = I18N[lang][el.dataset.i18n];
    if (v == null) return;
    if (el.hasAttribute('data-i18n-html')) el.innerHTML = v;
    else el.textContent = v;
  });
  document.querySelectorAll('.langseg button').forEach(b => b.classList.toggle('on', b.dataset.lang === lang));
}
function setLang(l) {
  lang = l === 'en' ? 'en' : 'ru';
  const useVT = !firstPaint && !reduceMotion.matches && typeof document.startViewTransition === 'function';
  firstPaint = false;
  useVT ? document.startViewTransition(applyLang) : applyLang();
}
document.querySelectorAll('.langseg button').forEach(b => b.addEventListener('click', () => setLang(b.dataset.lang)));
setLang(lang);

// Scroll to top for logo buttons
document.querySelectorAll('[data-scroll-top]').forEach(b =>
  b.addEventListener('click', () => {
    window.scrollTo({ top: 0, behavior: reduceMotion.matches ? 'auto' : 'smooth' });
  })
);

// Reveal fallback: native scroll-driven animations handle this where supported
(function reveal() {
  if (reduceMotion.matches) return;
  if (CSS.supports && CSS.supports('animation-timeline: view()')) return;
  const els = document.querySelectorAll('.features .f, .page-row, .show-text, .show-art, .foot-top > div');
  if (!('IntersectionObserver' in window) || !els.length) return;
  els.forEach((el, k) => {
    el.classList.add('rv');
    if (el.matches('.features .f')) el.style.transitionDelay = (k % 8) * 45 + 'ms';
  });
  const io = new IntersectionObserver(entries => {
    entries.forEach(e => {
      if (!e.isIntersecting) return;
      e.target.classList.add('vis');
      io.unobserve(e.target);
      setTimeout(() => { e.target.style.transitionDelay = ''; }, 700);
    });
  }, { threshold: 0.12 });
  els.forEach(el => io.observe(el));
})();
