# Цвета

Варианты установки цвета. Постепенно буду переводить на эту систему все команды где задается цвет (кроме цвета модели и рейла).

Все символы регистронезависимы.

## Текст

    white, black, red, green, blue, yellow, magenta, cyan, white, grey

## Один символ

    0-9 - цвета как у текста

## HEX 

24 bit:

    RGB
    #RGB
    xRGB
    0xRGB

48 bit:

    RRGGBBB
    #RRGGBB
    xRRGGBB
    0xRRGGBB

Не значащие нули можно отбрасывать. Например x1122 будет прочитано как x001122

При этом не стоит путать:

- 0xff это 0x0ff а не 0x0000ff
- 5 это 5 а не 0x005


# Новые команды для прицела
---

    cg_crosshairDecor 0..N

Рисует декор вокруг прицела.

`0` отключено

`1..N` номер декора

---

    cg_crosshairRotate45 0/1 cg_crosshairDecorRotate45 0/1

Поворачивает прицел или декор на 45 градусов.

`0` отключено

`1` включено


---

    cg_crosshairAction 0..? cg_crosshairDecorAction 0..N


`0` отключено

`1` Пульсирует попадании по цели

`2` Невидим, но появляется при попадании по цели

`3` Меняет цвет при попадании по цели


---

    cg_crosshairActionColor <color> cg_crosshairDecorActionColor <color>

Цвет действия прицела или декора при action = 3.

---

    cg_crosshairOpaque 0..1 cg_crosshairDecorOpaque 0..1

Прозрачность прицела или декора.


---

    ch_crosshairColor <color> cg_crosshairColor <color> cg_crosshairDecorColor <color>

Цвет действия прицела или декора.



