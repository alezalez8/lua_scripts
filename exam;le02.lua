local COPTER_MODE_LOITER = 5

local max_distance = 50

function update()
   if (arming:is_armed()) then
      local current_location = ahrs:get_position()
      local home_location = ahrs:get_home()

      if (home_location and current_location) then
         local distance = current_location:get_distance(home_location)

         gcs:send_text(4, string.format("Distance: %.2f", distance))

         if (distance >= max_distance) then
            vehicle:set_mode(COPTER_MODE_LOITER)
         end
      end
   end
   
   return update, 1000
end

return update, 1000
