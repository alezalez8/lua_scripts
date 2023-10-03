-- Импортируем необходимые библиотеки
import("mavlink")

-- Создаем кастомный идентификатор сообщения
custom_message_id = mavlink.message_id_new("MyCustomMessage")

-- Инициализируем UART 6
uart6 = uart.setup(6, 9600, 8, uart.PARITY_NONE, uart.STOPBITS_1)

-- Создаем функцию для чтения данных с UART 6
function read_data()
  -- Считываем данные с UART 6
  data = uart6.read(1024)

  -- Если данные были получены, то отправляем их на gcs
  if data then
    -- Преобразуем данные в массив
    data = string.split(data, ",")

    -- Создаем сообщение mavlink
    msg = mavlink.Message(custom_message_id)

    -- Заполняем сообщение данными
    for i = 1, #data do
      msg.data[i] = tonumber(data[i])
    end

    -- Отправляем сообщение на gcs
    mavlink.send_message(msg)
  end
end

-- Создаем таймер для запуска функции read_data()
timer = timer.periodic(1000, read_data)
