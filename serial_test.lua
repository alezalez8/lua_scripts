local port = serial:find_serial(0)
port:begin(9200)
port:set_flow_control(0)

local step = 0
local invStep = 100
local RUN_INTERVAL_MS = 10 -- 10ms == 100 Hz



function spit()
  --if port:available() > 0 then
  --read = port:read()

  gcs:send_named_float("latit", (invStep - step + 100))
  gcs:send_named_float("longtid", 987654321)
  gcs:send_named_float("counter", step)
  gcs:send_named_float("MY_DATA", invStep)
  step = step + 1
  invStep = invStep - 1
  if step > 100 then
    step = 0
  end
  if invStep == 0 then
    invStep = 100
  end

  return spit, RUN_INTERVAL_MS

  -- end
end

return spit, 1000
