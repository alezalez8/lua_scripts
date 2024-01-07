local uartspeed = 57600
local port = serial:find_serial(0)
port:begin(uartspeed)
port:set_flow_control(0)
local MAX_BUFFER = 10
local if_request = false


function getFromLogger()
    while port:available() > 0 do
        local passw = port:read()
        if passw == 53 then
            if_request = true
        end
    end
    return if_request
end

function getBuffer()
    local buffer = ''
    while port:available() > 0 do
        local currentChar = port:read()
        --if (currentChar ~= '/n') then
        buffer = buffer .. string.char(currentChar)
        if buffer:len() >= MAX_BUFFER then
            break
        end
        --end
    end
    return buffer
end

function getGPS()
    local coord = {}
    local instGPS = gps:primary_sensor()
    local gps_position = gps:location(instGPS)
    local lat = gps_position:lat()
    local lng = gps_position:lng()
    local numberSattel = gps:num_sats(instGPS)
    if lat == 0 or lng == 0 then
        lat = 0
        lng = 0
    end
    coord[1] = tostring(lat)
    coord[2] = tostring(lng)
    coord[3] = tostring(numberSattel)
    return coord[1], coord[2], coord[3]
end

function splitNumberGPS(number)
    if number == nil then
        return 1, 1
    end
    local numberString = tostring(number)
    if #numberString > 4 then
        local firstPart = tonumber(numberString:sub(1, 5))
        local secondPart = tonumber(numberString:sub(6))
        return firstPart, secondPart
    else
        return tonumber(numberString), 0
    end
end

function forarm()
    if (arming:is_armed()) then

    end
end

function update()
    while getFromLogger do
        local lat, lng = getGPS()
        local latHigh, latLow = splitNumberGPS(lat)
        local lngHigh, lngLow = splitNumberGPS(lng)

        local lidar = rangefinder:distance_cm_orient(25)
        local battery_level = battery:voltage(0)
        local gpsok = 0

        gcs:send_named_float("BATTERY", battery_level)
        gcs:send_named_float("LIDAR", lidar)

        local dataString = "1: " .. lat .. "  2: " .. lng .. "  3: " .. lidar .. "  4: " .. battery_level .. "  end  "


        local asciiCodes = {}
        for i = 1, #dataString do
            local asciiCode = string.byte(dataString, i)
            port:write(asciiCode)
        end


        gcs:send_named_float("LAT_H", latHigh)
        gcs:send_named_float("LAT_L", latLow)
        gcs:send_named_float("LON_H", lngHigh)
        gcs:send_named_float("LON_L", lngLow)

        if_request = false
    end


    return update, 50
end

return update, 1000

-- need add GPS_OK_FIX_3D
