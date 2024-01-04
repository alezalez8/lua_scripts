local port = serial:find_serial(0)
port:begin(57600)
port:set_flow_control(0)
local MAX_BUFFER = 10


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

function forarm()
    if (arming:is_armed()) then
        --
        --
        --
        --
        local aaa = ahrs:get_location()
        if (aaa ~= nil) then
            local bbb = aaa:alt()
        end

        local lidar_distance = rangefinder:
    end
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

function splitNumberData(number)
    if number == nil or #number == 0 then
        --return 1, 1, 1, 1
        return 1, 1
    end
    local numberString = tostring(number)
    if #numberString > 4 then
        local firstPart = tonumber(numberString:sub(1, 5))
        local secondPart = tonumber(numberString:sub(6, 10))
        --local thirdPart = tonumber(numberString:sub(11, 15))
        --local fourthdPart = tonumber(numberString:sub(16, 20))

        --return firstPart, secondPart, thirdPart, fourthdPart
        return firstPart, secondPart
    else
        return tonumber(numberString), 0
    end
end

function update()
    port:write(53)
    local myUARTData = getBuffer()
    --gcs:send_text(0, myUARTData)
    local lat, lng = getGPS()
    local latHigh, latLow = splitNumberGPS(lat)
    local lngHigh, lngLow = splitNumberGPS(lng)
    --local dataPartOne, dataPartTwo, dataPartThre, dataPartFour = splitNumberData(myUARTData)
    local dataPartOne, dataPartTwo = splitNumberData(myUARTData)

    gcs:send_named_float("LAT_H", latHigh)
    gcs:send_named_float("LAT_L", latLow)
    gcs:send_named_float("LON_H", lngHigh)
    gcs:send_named_float("LON_L", lngLow)
    --gcs:send_text(0, "DAT_H = " .. dataPartOne)
    gcs:send_named_float("DAT_1", dataPartOne)
    gcs:send_named_float("DAT_2", dataPartTwo)
    --gcs:send_named_float("DAT_3", dataPartThre)
    --gcs:send_named_float("DAT_4", dataPartFour)

    return update, 350 -- was 200
end

return update, 1000
