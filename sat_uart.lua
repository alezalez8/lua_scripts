local port = serial:find_serial(0)
port:begin(9200)
port:set_flow_control(0)


local step = 0
local amountOfData = 10
local myData = ''
local currentData = "Port not found"
local lat
local lng


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
   if port then
      for i = 1, amountOfData do
         myData = myData .. getData()
      end
   end

   local gps_position = gps:location(gps:primary_sensor())
   lat = gps_position:lat()
   lng = gps_position:lng()
   myData = myData .. " " .. lat .. " " .. lng
   return myData
end

function update()
   gcs:send_text(0, getAllData())
   -- MAVLINK_MSG_ID_NAMED_VALUE_FLOAT
   --gcs:send_named_float("myMineMessage", read)
   --gcs:send_named_float("myMineMessage______", 33)

   -- Named float values from Lua show up as MAV_XXXX

   return update, 200
end

return update, 1000
