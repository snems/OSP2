# Изменения в командах

---

    cg_enemyModelColors
    cg_teamModelColors
    cg_playerModelColors

Цвет cответствующей модели. Модель должна быть со скином pm.
Цвета имеют приоритет над командами `color1,color2,cg_enemyColors`

Аргумент: 

`XXX` - голова/туловище/ноги, цвета как в команде color1, например `cg_enemyModelColors 222`

`RRGGBBRRGGBBRRGGBB` голова/туловище/ноги, rgb hex цвета, например `cg_enemyModelColors 00FF0000FF0000FF00`

---

    cg_enemyRailColors
    cg_teamRailColors
    cg_playerRailColors


Цвета рельсы.
Цвета имеют приоритет над командами `color1,color2`

Аргумент: 

`XX` - сердцевина/кольца, цвета как в команде color1, например `cg_enemyRailColors 22`

`RRGGBBRRGGBB` сердцевина/кольца, rgb hex цвета, например `cg_enemyRailColors 00FF0000FF00`

---

    cg_enemyFrozenColor
    cg_teamFrozenColor
    cg_playerFrozenColor

Цвета замороженного игрока.

Аргумент: 

`X` - цвет как в команде color1, например `cg_enemyFrozenColor 2`

`RRGGBB` rgb hex цвет, например `cg_enemyFrozenColor 00FF00`

---

    cg_zoomTime 0-2000

Время изменения зума, мс.

---

    cg_zoomSensitivityAuto 

Режим изменения чувствительности мыши во время зума.

`0` ручной

`1` автоматический

---

    cg_zoomSensitivityMultiplier

Множитель чувствительности во время зума, если установлен ручной режим изменения.

Итоговая чувствительность равна `sensivity` * `cg_zoomSensitivityMultiplier`.

---

    cg_projectileNudgeSvFps 1-1000

Значение `sv_fps` сервера, см `cg_projectileNudge`

---

    cg_projectileNudge 0-1000

Компенсация задержки при отрисовке ракет/плазмы. Отрисовывает летящие снаряды чуть впереди, так, как их видит сервер.

`0` отключено

`1-1000` задержка

Для настройки нужно знать значение `sv_fps` сервера и ваш пинг.

Пример:

Сервер meat.q3msk.ru имеет `sv_fps` равный 70 и ваш пинг 23. Тогда настройки должны быть:

```
\cg_projectileNudgeSvFps 70
\cg_projectileNudge 23
```

---

    cg_delag 0|1|2|4

Если включено, будет отрисовывать выстрелы в предсказанную точку, а не туда, куда указывет прицел. 
Исключительно визуальный эффект, никак не влияет на то, как сервер обрабатывает выстрел.

`0` отключено

`1` LG и RG

`2` только LG 

`4` только RG

---

    cg_spectGlow 0/1

Подсветка игроков в режиме спектатора.

`0` выключено

`1` включено

---

    cg_fragSound 0..N

Звук когда получен фраг.

`0` выключено

`1` встроенный звук

`2` Звуки с номером 2 и выше отсутсвуют, доступны для кастомизации. Файл звука должен иметь путь `sound/feedback/fragSoundN.wav`
где N - соответсвующий номер звука, например: `sound/feedback/fragSound2.wav`.

---

    cg_damageKick 0/1

Тряска камеры при получении урона. Эта команда есть в OSP 1.03, но там она не работала.

`0` выключено

`1` включено

---

    cg_drawTimer 0/1/2

Показывать ли таймер

`0` выключено

`1` включено, верхний правый угол

`2` включено, вверху по центру

---

    cg_lightningHideCrosshair 0/1

Скрывает прицел при использовании шафта.

`0` выключено

`1` включено

---

    cg_lightningSilent 0/1

Отключает звук попадания при использовании шафта.

`0` выключено

`1` включено

---

    cg_altLightning 0-15

Расширенная настройка отрисовки луча шафта.

`0` Не изменилось

`1` Не изменилось

`2` Тонкий луч шафта.

`3-15` Для кастомизации. Используются шейдеры  
`lightningBoltNew3..lightningBoltNew15`/`lightningBoltNewNoPicMip3..lightningBoltNewNoPicMip15`. 
Если шейдера нет, то использется шейдер как в `cg_altLightning 0`.

---

    cg_drawCrosshairGauntlet -1..9
    cg_drawCrosshairMachinegun -1..9
    cg_drawCrosshairShotgun -1..9
    cg_drawCrosshairGrenadeLauncher -1..9
    cg_drawCrosshairRocketLauncher -1..9
    cg_drawCrosshairLightning -1..9
    cg_drawCrosshairRailgun -1..9
    cg_drawCrosshairPlasmagun -1..9
    cg_drawCrosshairBFG -1..9

Индивидуальные прицелы для каждого оружия.

`-1` прицел не будет меняться 

`0-9` номер прицела

---

    cg_crosshairAspectRatioFix 0/1

Коррекция пропорций прицела.

---

    cg_noLeadSounds 0/1

Отключает голосовое оповещение о лидерстве для некомандных режимов.

`0` оповещение есть

`1` оповещения нет

---

    cg_noTeamChatBeep 0/1

Отключает звук при приеме командных сообщений.

`0` звук есть

`1` звука нет

---

    cg_teamRails 0/1

Раскарска следов от реилгана. Значение 2 больше не работает.

`0` обычная раскраска

`1` red/blue 

---

    cg_hideScores 0/1

Отключение таблицы очков для клиента XQ3E, чтоб предотвратить двойную отрисовку.

`0` выключено

`1` включено

---

    cg_deadBodyBlack 0/1

Трупы игроков становятся серыми.

`0` выключено

`1` включено

---

    cg_simpleItems 2

Добавлено значение 2 - визуально увеличивает и немного приподнимает некоторые 2d итемы.

---

    crossnext
    crossprev

Переключает прицел на следующий или предыдущий вариант.

---

    cg_commonChatDisable 0/1

То же самое что cg_teamChatsOnly.

---

    cg_teamChatDisable 0/1

Отключает прием сообщений от игроков команды.

---

    cg_chatDisable 0/1

Отключает прием сообщений от игроков.

---

    cg_shud 0/1

Включает суперхуд.

---

    ch_file

Название конфигурационного файла для суперхуда. Файл должен лежать в каталоге hud и иметь расширение '.cfg'.
Название указывается без пути и расширения. Например для файла 'hud/myconfig.cfg' нужно указать 'myconfig'.

---

    reloadHUD

Перечитать конфигурацию суперхуда.

---

    cg_shudChatOnly 0/1
    cg_shudTeamChatOnly 0/1

Отключает дублирование сообщений в консоли.

---

    clientconfig

Отобразить конфигурацию клиента.

---

    cg_hitsounds 0/1/2

Разный тон хитсаунда в зависимости от количества урона. Есть 4 пороговых значения при которых тон меняется: 25, 50, 75, 100.

`0` Тон не меняется

`1` Чем выше урон, тем ниже тон.

`2` Чем выше урон, тем выше тон.


---

    ch_fragMessage
    cg_fallKick
    cg_smokegrowth_gl
    cg_smokeradius_gl
    cg_smokegrowth_rl
    cg_smokeradius_rl
    cg_smoke_sg

Эти команды были разблокированы для VQ3 режима, сам эффект от команд не менялся.

---

    menu*
    cg_float
    cg_drawDecals
    decal*
    cg_defferPlayers

Не работает и восстанавливать не планируется.


