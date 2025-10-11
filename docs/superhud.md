# SuperHUD

Сделан на основе документации к [CPMA SuperHud](https://playmorepromode.com/guides/cpma-super-hud), но отличается от него.

Конфигурация худа сохранена в текстовых файлах, расположенных в каталоге `./osp/hud/`. Файлы должны иметь расширение `.cfg`.

Команды для управления: `cg_shud, ch_file, reloadHUD`, подробнее в `commands.md`.


## Синтаксис

Синтаксис аналогичен синтаксису CPMA:

```

ELEMENT
{
    ELEMENT_COMMAND VALUE #комментарий
    ELEMENT_COMMAND VALUE
}

# комментарий
ELEMENT { ELEMENT_COMMAND VALUE; ELEMENT_COMMAND VALUE; }

```

## Элементы

Элемент - базовая часть худа. Элементы отрисовываются в порядке указанном в таблице. 

| Элемент                       | Краткое описание                                             |
|-------------------------------|--------------------------------------------------------------|
| !DEFAULT                      | Дефолтные команды.                                           |
| PreDecorate                   | Отрисовка разделителей и т.п.                                |
| AmmoMessage                   | Сообщение Low ammo/Out of ammo.                              |
| AttackerIcon                  | Иконка последнего кто вас атаковал                           |
| AttackerName                  | Ник последнего кто вас атаковал                              |
| Chat1 - Chat16                | Строки чата. Сообщение появляется сначала в Chat1.           |
| FlagStatus_NME                | Статус флага вражеской команды.                              |
| FlagStatus_OWN                | Статус флага своей команды.                                  |
| FollowMessage                 | Сообщение "Following PlayerName".                            |
| FPS                           | Счетчик FPS.                                                 |
| FragMessage                   | Сообщение о полученном фраге.                                |
| GameTime                      | Игровое время.                                               |
| ItemPickup                    | Название подобранного итема и игровое время.                 |
| ItemPickupIcon                | Изображение подобранного итема.                              |
| LocalTime                     | Реальное время.                                              |
| Name_NME                      | Имя врага или вражеской команды.                             |
| Name_OWN                      | Свое имя или имя команды.                                    |
| NetGraph                      | Лагометр.                                                    |
| NetGraphPing                  | Ping.                                                        |
| Obituary1 - Obituary8         | Некролог (сообщения о смертях)                               |
| PlayerSpeed                   | Скорость игрока.                                             |
| Powerup1_icon - Powerup8_icon | Иконка поверапа.                                             |
| Powerup1_time - Powerup8_time | Оставшееся время действия поверапа.                          |
| RankMessage                   | Ранг в таблице. Видно при получении фрага.                   |
| Score_Limit                   | Лимит очков.                                                 |
| Score_NME                     | Очки врага/команды или лидирующего игрока.                   |
| Score_OWN                     | Собственные очки или очки своей команды.                     |
| Specmessage                   | Сообщение SPECTATOR.                                         |
| StatusBar_ArmorBar            | Полоска брони.                                               |
| StatusBar_ArmorCount          | Счетчик брони.                                               |
| StatusBar_ArmorIcon           | Иконка брони.                                                |
| StatusBar_AmmoBar             | Полоска количество патронов.                                 |
| StatusBar_AmmoCount           | Счетчик патронов.                                            |
| StatusBar_AmmoIcon            | Иконка патронов.                                             |
| StatusBar_HealthBar           | Полоска здоровья.                                            |
| StatusBar_HealthCount         | Счетчик здоровья.                                            |
| StatusBar_HealthIcon          | Иконка здоровья.                                             |
| TargetName                    | Имя игрока в прицеле.                                        |
| TargetStatus                  | Здоровье/броня тиммейта.                                     |
| TeamCount_NME                 | Количество живых игроков во вражеской команде (только фриз). |
| TeamCount_OWN                 | Количество живых игроков в своей команде (только фриз).      |
| Team1-Team16                  | Тимоверлей.                                                  |
| VoteMessageWorld              | Строка голосования.                                          |
| WeaponList                    | Список доступного оружия.                                    |
| RewardIcons                   | Изображение полученной медали.                               |
| RewardNumbers                 | Счетчик полученных медалей.                                  |
| PostDecorate                  | Отрисовка разделителей и т.п.                                |
| WarmupInfo                    | Обратный отсчет/"waiting for players".                       |
| GameType                      | Режим игры(только во время warmup).                          |
| Location                      | Сообщение с названием локации при ее изменении.              |

## Команды


| Команда   | Значения       | Краткое описание                                          |
|-----------|----------------|-----------------------------------------------------------|
| AlignH    | L/C/R          | Выравнивание по горизонтали лево/центр/право              |
| AlignV    | T/C/B          | Выравнивание по вертикали верх/центр/низ                  |
| BgColor   | r g b a        | Цвет фона.                                                |
| Color     | r g b a        | Базовый цвет.                                             |
| Color2    | r g b a        | Второстепенный цвет.                                      |
| Color     | T              | Цвет команды.                                             |
| Color     | E              | Цвет вражеской команды.                                   |
| Direction | L/R/T/B        | Направление: лево->право/право->лево/верх->низ/низ->верх. |
| Doublebar |                | Двойная полоска здоровья/брони/патронов.                  |
| Fade      | r g b a        | Затухание к этому цвету.                                  |
| Fadedelay | milliseconds   | Скорость затухания.                                       |
| Fill      |                | Заливка цветом BgColor.                                   |
| Font      | name           | Название шрифта.                                          |
| FontSize  | w h            | Размер шрифта ширина и высота.                            |
| Image     | string         | Путь к шейдеру или картинке.                              |
| Monospace |                | Использовать моноширинную печать.                         |
| Rect      | x y w h        | Координаты и размер элемента.                             |
| Text      | string         | Текстовая строка.                                         |
| Textalign | L/C/R          | Выравнивание текста лево/центр/право.                     |
| Textstyle | flags          | Флаги текста. 1 - без тени.                               |
| Time      | milliseconds   | Время отображения на экране.                              |
| Style     | flags          | Стиль элемента - цифра                                    |
| VisFlags  | string         | Изменить дефолтные настройки видимости                    |

## Пояснения к некоторым элементам

### !DEFAULT

Используется для установки дефолтовых значений команд для последующих элементов. Для отмены используйте пустой элемент: `!DEFAULT {}`

### Chat1 - Chat16

Сообщение появляется сначала в Сhat1 и по мере поступления новых сообщений сдвигается в следующие элементы.

### Font

Доступные шрифты:

- id
- idblock
- sansman
- m1rage
- elite_emoji
- diablo
- eternal
- qlnumbers
- elite
- elitebigchars

### WeaponList 

### ObituaryN 

Стили фона для командных режимов:

- style 0 - без фона 
- style 1 - фон в цвет команды
- style 2 - фон в цвет команды только для врагов 
- style 3 - фон в цвет enemy colors (legs)
- style 4 - фон в цвет команды для команды игрока

### StatusBar_HealthBar StatusBar_ArmorBar StatusBar_AmmoBar

- Doublebar - включить двойную полосу
- Direction - Направление полосы

Только для здоровья и брони:
- Style 1 - цвет зависит от хп, 2 - обе полосы в одном месте, цвет задаётся через color и color2,  

### VisFlags 

Варианты: 
- im - видно в режиме intermission
- teamonly - видно в только в командных режимах
- spectator - видно в режиме спектатора
- dead - видно если игрок мертв
- demohide - скрывать в режиме просмотра демо
- scoreshide - скрывать в режиме просмотра счета
- key1show - скрывать если не нажата кнопка +shudkey1
- key2show - скрывать если не нажата кнопка +shudkey2
- key3show - скрывать если не нажата кнопка +shudkey3
- key4show - скрывать если не нажата кнопка +shudkey4

Можно указывать несколько вариантов через пробел:

    visflags teamonly key1show;
