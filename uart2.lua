function myMessage()

if (arming:is_armed()) then
gcs:send_text(0, "Hello Aleks!!!")
end
if ( not arming:is_armed()) then
   gcs:send_text(0, "Good bye Aleks!!!")
   end

return myMessage, 1000
end
return myMessage, 4000

