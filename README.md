[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/de_libs)

[СКАЧАТЬ РЕПОЗИТОРИЙ](https://codeload.github.com/DIY-Elecron1cs/AutoWatering/zip/refs/heads/main)

# AutoWatering
Автополив на arduino.
В репозитории содержится только код. Проект в моём исполнении оказался неудачным с точки зрения механики и схемотехники.

## как настроить
PUMP_PULSE_PERIOD – период пульсации помпы
PUMP_MINUTES – в течении скольки минут будет поддерживаться влажность во время полива
RESET_CLOCK – ставится в 1 для первой прошивки, затем ставится в 0 и прошивается ещё раз
WEEK_DAY – текущий день недели (необязательно), 1 - понедельник и т.д.
DEBUG – ставится в 1 для отладки в мониторе порта

## управление
### переключение между экранами
Переключение между экранами осуществляется поворотом энкодера вправо и влево.
### главный экран
На главном экране отображается текущие параметры: влажность почвы, время, состояние помпы.
### второй экран (настройка таймеров)
- При удержании кнопки энкодера осуществляется переход в режим настройки и обратно.
- В режиме настройки можно менять значения (wateringDay – раз во сколько дней будет полив, wateringTime – в какое вреся поливать) вращением энкодера.
### третий экран (настройка влажности)
- Нажатием на кнопку энкодера осуществляется переход в режим настройки.
- Вращением энкодера настраивается порог влажности.
### четвёртый экран (сохранение настроек)
- Нажатием на кнопку энкодера осуществляется переход от параметра YES к NO, по умолчанию стоит NO.
- При удержании кнопки энкодера происходит сохранение текущих настроек в ПЗУ и возврат к начальному экрану.

## фото
![stand](https://github.com/DIY-Elecron1cs/AutoWatering/blob/main/images/auto-watering-stand.jpg?raw=true)

По вопросам писать в [чат DIY electronics](https://t.me/diy_electronics_chat) или на [почту](mailto:diy-electronics@mail.ru).
