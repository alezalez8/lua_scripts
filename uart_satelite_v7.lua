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

        local lidar_distance = RangeFinder_State():distance()
        --local levBattery = num_instances()
        --local batt = voltage(levBattery)
        local saa = battery:voltage(battery:num_instances())
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
    --port:write(53)
    --local myUARTData = getBuffer()

    local lat, lng = getGPS()
    local latHigh, latLow = splitNumberGPS(lat)
    local lngHigh, lngLow = splitNumberGPS(lng)

    local lidar = rangefinder:distance_cm_orient(25)
    local battery_level = battery:voltage(0)
    gcs:send_named_float("BATTERY", battery_level)
    gcs:send_named_float("LIDAR", lidar)

    local dataString = "1: " .. lat .. "2: " .. lng .. "3: " .. lidar .. "4: " .. battery_level


    -- local numbers = {}
    -- for number in my_packet:gmatch("%d+%.?%d*") do
    --     table.insert(numbers, tonumber(number))
    -- end

    -- -- Convert floating-point numbers to integers
    -- for i, v in ipairs(numbers) do
    --     numbers[i] = math.floor(v)
    -- end

    -- -- Assuming you have a valid UART port object named 'port', write the numbers to the port
    -- local dataToSend = string.char(table.unpack(numbers))

    local asciiCodes = {}
    for i = 1, #dataString do
        -- table.insert(asciiCodes, string.byte(dataString, i))
        local asciiCode = string.byte(dataString, i)
        port:write(asciiCode)
    end
    -- ++++++++++++++++++++++++++++++++++++
    -- -- Assuming you have a valid UART port object named 'port', write the ASCII codes to the port
    -- local dataToSend = string.char(table.unpack(asciiCodes))

    -- --port:write(dataToSend)
    -- port:write(table.unpack(asciiCodes))
    -- ++++++++++++++++++++++++++++++++++++++++++++++++++++




    -- gcs:send_text('0', "Battery = " .. saa)
    -- gcs:send_text('0', "Lidar = " .. lid)


    gcs:send_named_float("LAT_H", latHigh)
    gcs:send_named_float("LAT_L", latLow)
    gcs:send_named_float("LON_H", lngHigh)
    gcs:send_named_float("LON_L", lngLow)


    return update, 200 -- was 200
end

return update, 1000
