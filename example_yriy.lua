local RUN_INTERVAL_MS = 50 -- 50ms == 20 Hz
local MAX_BUFFER = 128     -- prevent Lua timeout

local port = serial:find_serial(0)
local bad_frames = 0

port:begin(uint32_t(115200))
port:set_flow_control(0)

function update()
   local bytes = {}
   while port:available() > 0 do
      bytes[#bytes + 1] = port:read()
      if #bytes > MAX_BUFFER then
         return update, RUN_INTERVAL_MS
      end
   end

   if #bytes ~= 4 then
      return update, RUN_INTERVAL_MS
   end

   if bytes[1] ~= 0xFF or bytes[4] ~= bytes[2] ~ bytes[3] then
      bad_frames = bad_frames + 1
      gcs:send_named_float('BAD_FR', bad_frames)
      return update, RUN_INTERVAL_MS
   end

   gcs:send_named_float('VAL', bytes[2] << 8 | bytes[3])
   gcs:send_named_float('BAD_FR', bad_frames)

   return update, RUN_INTERVAL_MS
end

return update()
