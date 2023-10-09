local FRAME_SIZE     =   21
local UPDATE_RATE_HZ =    2
local BAUD_RATE      = 9600
local FLOAT_PREFIX   = 'RNG'
local DEBUG_PREFIX   = 'BAD_F'  -- comment this line to suppress bad frame data

local MAV_SEVERITY_INFO = 6

local port = assert(serial:find_serial(0),"Could not find scripting serial port!")
port:begin(BAUD_RATE)
port:set_flow_control(0)

local function valid_frame(frame)
    if #frame ~= FRAME_SIZE then return false end  -- check frame size (in bytes)
    if frame[6] ~= 0x01 then return false end      -- check range string separator
    local checksum = 0
    for i = 1, #frame - 1 do
        checksum = (checksum + frame[i]) % 256
    end
    if checksum ~= frame[#frame] then return false end  -- validate checksum
    return true
end

local function bytes_to_str(frame, num_chars)
    local str = ''
    if num_chars > #frame then num_chars = #frame end
    for i = 1, num_chars do
        str = str .. string.char(frame[i])
    end
    return str
end

local run_interval_ms = math.floor(500 / UPDATE_RATE_HZ)  -- twice the rate of incoming data
local bad_frames = 0
function update()

    -- read serial stream into buffer table
    local buffer = {}
    local bytes = port:available()
    while bytes > 0 do
        buffer[#buffer + 1]= port:read()
        bytes = bytes - 1
    end

    -- validate incoming data as a sonar frame
    if #buffer == 0 then return update, run_interval_ms end
    if not valid_frame(buffer) then   -- discard frame
        if DEBUG_PREFIX then
            bad_frames = bad_frames + 1
            gcs:send_named_float(DEBUG_PREFIX, bad_frames)
        end
        return update, math.floor(run_interval_ms / 2)  -- offset run schedule to avoid data sync issues
    end

    -- format frame data as a floating point number
    local range = tonumber(bytes_to_str(buffer, 5))
    gcs:send_named_float(FLOAT_PREFIX, range)
    return update, run_interval_ms
end

gcs:send_text(MAV_SEVERITY_INFO, 'UART Sonar Script Active')

return update()
