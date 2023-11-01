-- Lua script to write and read from a serial

local port = serial:find_serial(0)

port:begin(9200)

-- desc
---@param flow_control_setting integer
---| '0' # disabled
---| '1' # enabled
---| '2' # auto
port:set_flow_control(0)

local step = 0

local function bytes_to_str(frame, num_chars)
  local str = ''
  if num_chars > #frame then num_chars = #frame end
  for i = 1, num_chars do
      str = str .. string.char(frame[i])
  end
  return str
end


function spit ()
  if port:available() > 0 then
    read = port:read()
    --gcs:send_text(0, read .. " = " .. step)
    gcs:send_text(0, string.char(read) .. " = " .. step)

    -- MAVLINK_MSG_ID_NAMED_VALUE_FLOAT
    gcs:send_named_float("myMineMessage", read) 
    gcs:send_named_float("myMineMessage______", 33)

    -- Named float values from Lua show up as MAV_XXXX
     

  end
  if step >= 10 then
    step = 0
  else
    step = step + 1
  end
  port:write(step)
  return spit, 10
end

return spit, 1000
-- Returns the UART instance that allows connections from scripts (those with SERIALx_PROTOCOL = 28`).
-- For instance = 0, returns first such UART, second for instance = 1, and so on.
-- If such an instance is not found, returns nil.
---@param instance integer -- the 0-based index of the UART instance to return.
---@return AP_HAL__UARTDriver_ud -- the requested UART instance available for scripting, or nil if --none.