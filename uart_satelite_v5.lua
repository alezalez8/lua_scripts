-- Создание таблицы с данными (в данном случае, массив чисел)
local data = { 1, 2, 3, 4, 5, 6, ... } -- Замените ... на остальные значения

-- Создание Mavlink-пакета
local mavlink_msg = {
   target_system = 1,     -- Замените на ваш ID системы
   target_component = 1,  -- Замените на ваш ID компонента
   seq = 0,               -- Последовательный номер пакета
   sysid = 1,             -- ID системы отправителя
   compid = 1,            -- ID компонента отправителя
   length = 8 * #data,    -- Длина данных в байтах

   -- Тип сообщения и подтип (пользовательский кастомный тип)
   msgid = 150,  -- MAVLINK_MSG_ID_USER (замените на нужный вам ID)

   -- Payload64
   payload64 = {}
}

-- Заполнение payload64 данными
for i = 1, #data do
   mavlink_msg.payload64[i] = data[i]
end

-- Отправка Mavlink-пакета
mavlink.send_chan(0, mavlink_chan, mavlink.MAVLINK_MSG_ID_USER, mavlink_msg)
