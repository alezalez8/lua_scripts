local port = serial:find_serial(0)

if not port or baud == 0 then
    gcs:send_text(0, "No Scripting Serial Port")
    return
end

port:begin(57600)
port:set_flow_control(0)

local MAX_BUFFER = 10
local if_request = true
local lat, lng, satt


function getFromLogger()
    while port:available() > 0 do
        local passw = port:read()
        gcs:send_text(0, passw)
        if passw == 53 then
            if_request = true
        else
            if_request = false
        end
    end
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
    if not port then
        gcs:send_text(0, "no Scripting Serial Port")
        return update, 100
    end

    if port:available() > 0 then
        local nothing = port:read()

        if (arming:is_armed()) then
            lat, lng, satt = getGPS()
        else
            lat = 0
            lng = 0
            satt = 0
        end



        local lidar = rangefinder:distance_cm_orient(25)
        local battery_level = battery:voltage(0)
        local gpsok = 0

        local battery1 = string.format("%.2f", battery_level)

        local dataString = "b " ..
            lat ..
            " " ..
            lng .. " " .. lidar .. " " .. battery1 .. " s " .. satt .. " d @"


        local asciiCodes = {}
        for i = 1, #dataString do
            local asciiCode = string.byte(dataString, i)
            port:write(asciiCode)
        end
    end



    return update, 500
end

return update, 1000
