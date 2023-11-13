local port = serial:find_serial(0)
port:begin(57600)
port:set_flow_control(0)
local MAX_BUFFER = 10


function getBuffer()
    local buffer = ''
    while port:available() > 0 do
        buffer = buffer .. string.char(port:read())

        if buffer:len() >= MAX_BUFFER then
            break
        end
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

function splitNumber(number)
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

    return update, 100
end


return update, 1000




