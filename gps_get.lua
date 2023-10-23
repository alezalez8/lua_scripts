-- Lua script to write and read from a serial

local port = serial:find_serial(0)
--local mavlink = require("mavlink.message")

port:begin(9200)

-- desc
---@param flow_control_setting integer
---| '0' # disabled
---| '1' # enabled
---| '2' # auto
port:set_flow_control(0)

local step = 0





function spit ()

   --local lat, lon, alt = mavlink.vehicle:get_location()
   local mygps = gps:num_sensors();
   local gps_position = gps:location(gps:primary_sensor())
   local lat = gps_position:lat()
   local lng = gps_position:lng()


    --lat = lat/1e7
    --lng = lng/1e7
    
    gcs:send_text(0, "lat" .. "=" .. lat .. "  lon" .. "=" .. lng )
    --gcs:send_text(0, "long" .. " = " .. lng)
    



  if step >= 10 then
    step = 0
  else
    step = step + 1
  end
  --port:write(step)
  return spit, 100
end
 
return spit, 1000
-- Returns the UART instance that allows connections from scripts (those with SERIALx_PROTOCOL = 28`).
-- For instance = 0, returns first such UART, second for instance = 1, and so on.
-- If such an instance is not found, returns nil.
---@param instance integer -- the 0-based index of the UART instance to return.
---@return AP_HAL__UARTDriver_ud -- the requested UART instance available for scripting, or nil if --none.
-- --------------------------------------------------------------------
--gps_position1 = gps:location(0)
--gps_position2 = gps:location(1)  получение от двух жпс