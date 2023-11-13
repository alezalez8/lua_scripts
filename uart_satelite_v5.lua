-- Создание таблицы с данными (в данном случае, массив чисел)
local data = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } -- Замените ... на остальные значения

local temp = 0
local myCustomMessageID = 200


function convertData(data)
   -- Создание Mavlink-пакета
   local mavlink_msg = {
      target_system = 1,    -- Замените на ваш ID системы
      target_component = 1, -- Замените на ваш ID компонента
      seq = 0,              -- Последовательный номер пакета
      sysid = 1,            -- ID системы отправителя
      compid = 1,           -- ID компонента отправителя
      length = 8 * #data,   -- Длина данных в байтах

      -- Тип сообщения и подтип (пользовательский кастомный тип)
      msgid = 150, -- MAVLINK_MSG_ID_USER (замените на нужный вам ID)

      -- Payload64
      payload64 = {}
   }

   -- Заполнение payload64 данными
   for i = 1, #data do
      mavlink_msg.payload64[i] = data[i]
   end

   -- Преобразование Mavlink-пакета в строку
   local mavlink_str = string.pack("<BBBBIIB", mavlink_msg.sysid, mavlink_msg.compid, mavlink_msg.seq, mavlink_msg.msgid,
      mavlink_msg.target_system, mavlink_msg.target_component, mavlink_msg.length)
   --формат строки (например, "<BBBBIIB") зависит от структуры вашего Mavlink-пакета


   -- Добавление payload64 к строке
   for _, value in ipairs(mavlink_msg.payload64) do
      mavlink_str = mavlink_str .. string.pack("<I", value)
   end
   return mavlink_str
end

function update()
   local myData = convertData(data)
   mavlink:send_chan(0, myCustomMessageID, myData)
   gcs:send_text(0, "All sent  " .. temp)
   temp = temp + 1
   if temp >= 20 then
      temp = 0
   end
   return update, 200
end

return update, 1000
