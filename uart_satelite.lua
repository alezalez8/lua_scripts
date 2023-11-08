local port = serial:find_serial(0)
port:begin(57600)
port:set_flow_control(0)


local amountOfData = 5
local myData = ""
local currentData = "Port not found"
local lat
local lng
local MAX_BUFFER = 9


---------------------------------Yuriy_Rage-----------------------------------
function getBuffer()
   local buffer = ''
   while port:available() > 0 do
      buffer = buffer .. string.char(port:read())
      if buffer:len() >= MAX_BUFFER then
         break
      end
      return buffer
   end

   -- if buffer:len() > 0 then
   --    -- insert error checking logic here
   --    local val = tonumber(buffer)
   --    if val then -- tonumber returns nil on error, which evaluates to false
   --       gcs:send_named_float('VAL', val)
   --    end
   -- end
   return '1111111111'
end

--------------------------------------------------------------------


-- get one char from UART3. Data comes every 10 ms
function getData()
   if port:available() > 0 then
      currentData = string.char(port:read())
      return currentData
   else
      return getData(), 2
   end
end

--  get data from GPS
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
   --return "454545450", "676767670"
end

-- Function to split a number into two parts
function splitNumber(number)
   local numberString = number
   if numberString ~= nil then
      -- Check if the number has more than 4 digits
      if #numberString > 4 then
         local firstPart = tonumber(numberString:sub(1, 5))
         local secondPart = tonumber(numberString:sub(6))

         return firstPart, secondPart
      else
         -- If the number has 4 or fewer digits, return the whole number as the first part
         return number, 0
      end
   else
      return 1
   end
end

-- this function calls data from UART and GPS data every 100 ms and send to GS
function update()
   local myUARTData = getBuffer()
   local lat, lng = getGPS()
   local latHigh, latLow = splitNumber(lat)
   local lngHigh, lngLow = splitNumber(lng)
   local dataPartOne, dataPartTwo = splitNumber(myUARTData)

   gcs:send_named_float("LAT_H", latHigh)
   gcs:send_named_float("LAT_L", latLow)
   gcs:send_named_float("LON_H", lngHigh)
   gcs:send_named_float("LON_L", lngLow)
   gcs:send_named_float("DAT_H", dataPartOne)
   gcs:send_named_float("DAT_L", dataPartTwo)


   return update, 200
end

return update, 1000
