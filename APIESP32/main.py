from flask import Flask
import time, threading
from flask import request, jsonify
import mysql.connector
from mysql.connector import Error
app = Flask(__name__)

# Переменные для контроля времени последнего запроса
last_request_time = None
timer_active = True
time_sleep = 60 # если не было запросов через 60 секунд, значит esp32 спит
# Конфигурация базы данных
db_config = {
    'host': '127.127.126.25',
    'user': 'root',
    'password': '',
    'database': 'ESP32'
}
def check_requests_timeout():
    global last_request_time, timer_active
    while timer_active:
        if last_request_time is not None and (time.time() - last_request_time) > time_sleep:  # 60 секунд = 1 минута
            print("⚠️ Не было запросов к /api в течение 1 минуты!")
            last_request_time = None  # Сброс, чтобы не спамить в консоль
            try:
                # Подключаемся к MySQL
                connection = mysql.connector.connect(**db_config)
                cursor = connection.cursor()

                # Выполняем SQL запрос
                query = "INSERT INTO data (status) VALUES (%s)"
                cursor.execute(query, (0))
                connection.commit()

                return jsonify({'status': 'success'})

            except Error as e:
                return jsonify({'status': 'error', 'message': str(e)}), 500

            finally:
                if 'connection' in locals() and connection.is_connected():
                    cursor.close()
                    connection.close()
        time.sleep(1)  # Проверяем каждую секунду

# Запускаем фоновый поток для проверки таймаута
thread = threading.Thread(target=check_requests_timeout)
thread.daemon = True
thread.start()

@app.route('/api', methods=['POST'])
def api():
    global last_request_time
    last_request_time = time.time()  # Обновляем время последнего запроса
    try:
        # Получаем JSON данные из запроса
        data = request.get_json()

        # Проверяем обязательные поля
        if not data or 'status' not in data or 'level_garbage' not in data or 'level_energy' not in data:
            return jsonify({'status': 'error', 'message': 'Missing required fields'}), 400

        # Подключаемся к MySQL
        connection = mysql.connector.connect(**db_config)
        cursor = connection.cursor()
        if data["clear"] is True:
            query = "INSERT INTO data (status, level_garbage, level_energy) VALUES (%s, %s, %s)"
            cursor.execute(query, (data['status'], data['level_garbage'], data['level_energy']))
        # Выполняем SQL запрос
        else:
            query = "INSERT INTO data (status, level_garbage, level_energy) VALUES (%s, %s, %s)"
            cursor.execute(query, (data['status'], data['level_garbage'], data['level_energy']))
        connection.commit()

        return jsonify({'status': 'success'})

    except Error as e:
        return jsonify({'status': 'error', 'message': str(e)}), 500

    finally:
        if 'connection' in locals() and connection.is_connected():
            cursor.close()
            connection.close()

if __name__ == '__main__':
    try:
        app.run(debug=True)
    except KeyboardInterrupt:
        timer_active = False  # Останавливаем поток при завершении программы
        thread.join()