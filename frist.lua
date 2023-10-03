function maa()
   if (arming:is_armed()) then
   gcs:send_text(0, "Hello Aleks")
   end

   return maa, 2000
end

return maa, 1000
