-- Настройки UART
local uart_number = 6 -- Номер UART порта
local baud_rate = 57600 -- Скорость передачи данных

-- Настройки MAVLink
local custom_message_id = 1000 -- Кастомный идентификатор сообщения
local system_id = 1 -- Идентификатор системы
local component_id = 1 -- Идентификатор компонента

-- Функция для отправки сообщения MAVLink
local function send_mavlink_message(data)
    local message = mavlink_msg_custom_message_pack(system_id, component_id, custom_message_id, data)
    mavlink_send_message(message)
end

-- Основной цикл скрипта
while true do
    -- Считываем данные с UART6
    local data_length = uart.available(uart_number)
    gcs:send_text(0, "Hello Aleks")
    if data_length >= 10 and data_length <= 20 then
        local received_data = uart.read(uart_number, data_length)
        
        -- Создаем сообщение в необходимом формате (пример: строка данных через запятую)
        local message_data = table.concat(received_data, ",")
        
        -- Отправляем данные через MAVLink
        send_mavlink_message(message_data)
    end
    
    -- Ждем некоторое время перед следующей итерацией
    delay(100) -- Задержка в миллисекундах (пример: 100 мс)
end
