function update()
   gcs:send_text(0, "Hello Aleks")

   return update, 2000
end

return update, 1000
