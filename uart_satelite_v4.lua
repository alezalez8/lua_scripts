local port = serial:find_serial(0)
port:begin(19200)
port:set_flow_control(0)
local MAX_BUFFER = 20


function getBuffer()
   local buffer = ''
   while port:available() > 0 do
      local currentChar = port:read()
      if (currentChar ~= '/n') then
         buffer = buffer .. string.char(currentChar)

         if buffer:len() >= MAX_BUFFER then
            break
         end
      end
   end
   return buffer
end

function getGPS()
   local coord = {}
   local gps_position = gps:location(gps:primary_sensor())
   local lat = gps_position:lat()
   local lng = gps_position:lng()
   if lat == 0 or lng == 0 then
      lat = 0
      lng = 0
   end
   coord[1] = tostring(lat)
   coord[2] = tostring(lng)
   return coord[1], coord[2]
end

function splitNumberData(number)
   if number == nil or #number == 0 then
      return 1
   end
   local numberString = tostring(number)
   if #numberString > 10 then
      local levels = numberString:sub(1, 10)

      return levels
   else
      return numberString
   end
end

-- Дополняем строки до 10 символов
function padString(inputString, padChar, targetLength)
   local currentLength = #inputString
   if currentLength >= targetLength then
       return inputString
   else
       local padding = string.rep(padChar, targetLength - currentLength)
       return inputString .. padding
   end
end

function update()
   port:write(53)
   local myUARTData = getBuffer()
   local lat, lng = getGPS()
   local levels = splitNumberData(myUARTData)

   -- Дополняем строки до 10 символов
   lat = padString(lat, "0", 10)
   lng = padString(lng, "0", 10)
   levels = padString(levels, "1", 10)

   gcs:send_named_float(lat, 1)
   gcs:send_named_float(lng, 2)
   gcs:send_named_float(levels, 3)


   return update, 200
end

return update, 1000
