local NO_GPS = 1
local GPS_OK_FIX_2D = 2
local GPS_OK_FIX_3D = 3
local GPS_OK_FIX_3D_DGPS = 4
local GPS_OK_FIX_3D_RTK_FLOAT = 5
local GPS_OK_FIX_3D_RTK_FIXED = 6

local PLANE_MODE_FLY_BY_WIRE_A = 5
local PLANE_MODE_AUTO = 10

local mode_default = PLANE_MODE_AUTO
local mode_no_gps = PLANE_MODE_FLY_BY_WIRE_A

local RCMAP_ROLL = rc:get_channel(1)
local RCMAP_PITCH = rc:get_channel(2)
local RCMAP_THROTTLE = rc:get_channel(3)
local RCMAP_YAW = rc:get_channel(4)

function update()
   if (arming:is_armed()) then
      local current_gps = gps:primary_sensor()
      local current_gps_status = gps:status(current_gps)
      local cur_pos = gps:location(current_gps)
      --local ggg = current_gps:
      -- gcs:send_text(4, string.format("Status GPS: %d", current_gps_status))
      -- gcs:send_text(4, string.format("Status GPS: %d", cur_pos))

      if ((current_gps_status < GPS_OK_FIX_2D) or (current_gps_status > GPS_OK_FIX_3D_RTK_FIXED)) then
         vehicle:set_mode(mode_no_gps)

         RCMAP_ROLL:set_override(1500)
         RCMAP_PITCH:set_override(1700)
         RCMAP_THROTTLE:set_override(1800)
         RCMAP_YAW:set_override(1500)
      else
         vehicle:set_mode(mode_default)
      end

      gcs:send_text(4, string.format("Status GPS: %d", current_gps_status))
   end

   return update, 1000
end

return update, 1000
