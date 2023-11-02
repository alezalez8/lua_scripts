local port = serial:find_serial(0)
port:begin(9200)
port:set_flow_control(0)


local step = 0
local amountOfData = 2
local myData = ""
local currentData = "Port not found"
local lat
local lng


local myPacket = {

   { name = "latit",   type = "float",   offset = 0 },
   { name = "longtid", type = "float",   offset = 4 },
    { name = "data",    type = "uint8_t", count = 8 } }
   
local dss = string.pack("I4I4ff", 0xFEFEFE, 0, myPacket[1], myPacket[2])



-- Create the message struct
local message = {}
message.name = "My Custom Message"
message.fields = myPacket


--mavlink.send_chan(self, chan, msgid, message)

function getData()
   if port:available() > 0 then
      currentData = string.char(port:read())
      return currentData
   else
      return getData(), 2
   end
end

function getAllData()
   local str = " "
   --if port then
   for i = 1, amountOfData do
      myData = myData .. getData()
   end
   --end

   local gps_position = gps:location(gps:primary_sensor())
   lat = gps_position:lat()
   lng = gps_position:lng()
   myData = myData .. " " .. lat .. " " .. lng .. " end"
   return myData
end

function update()
   local myMessage = getAllData()
   -- check data on screen--
   gcs:send_text(0, myMessage)
   return update, 200
end

return update, 1000


-- Ваши данные (координаты и массив)
-- local coordinate1 = 123456789                        -- Пример первой координаты (9 знаков)
-- local coordinate2 = 987654321                        -- Пример второй координаты (9 знаков)
-- local integerArray = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } -- Пример массива из 10 целых чисел

-- -- Формат строки для упаковки данных
-- local format = "<I9I9"      -- Два беззнаковых целых числа (9 байт каждое)
-- for i = 1, 10 do
--    format = format .. "I4"  -- 10 целых чисел (4 байта каждое)
-- end

-- -- Упаковка данных в строку
-- local mavlink_msg = string.pack(format, coordinate1, coordinate2, table.unpack(integerArray))

-- Отправка mavlink_msg через MAVLink
-- Пожалуйста, добавьте здесь код для отправки сообщения на наземную станцию через MAVLink
