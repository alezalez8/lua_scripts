local port = serial:find_serial(0)
port:begin(38400)
port:set_flow_control(0)


local amountOfData = 10
local myData = ""
local currentData = "Port not found"
local lat
local lng


---------------------------------Yuriy_Rage-----------------------------------
local buffer = ''
while port:available() > 0 do
   buffer = buffer .. string.char(port:read())
   if buffer:len() > MAX_BUFFER then
      return update, RUN_INTERVAL_MS
   end
end

if buffer:len() > 0 then
   -- insert error checking logic here
   local val = tonumber(buffer)
   if val then      -- tonumber returns nil on error, which evaluates to false
      gcs:send_named_float('VAL', val)
   end
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

-- get ten numbers from UART and then get data from GPS
function getAllData()
   local str = " "
   --if port then
   for i = 1, amountOfData do
      myData = myData .. getData() .. str
   end
   --end

   local gps_position = gps:location(gps:primary_sensor())
   lat = gps_position:lat()
   lng = gps_position:lng()
   myData = myData .. " " .. lat .. " " .. lng .. " end"
   return myData
end

-- this function calls data from UART and GPS data every 100 ms and send to GS
function update()
   local myMessage = getAllData()
   -- check data on screen--
   gcs:send_text(0, myMessage)
   return update, 100
end

return update, 1000
