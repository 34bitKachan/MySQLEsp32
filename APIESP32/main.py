import time
from flask import Flask
from flask_sock import Sock
import mysql.connector
from mysql.connector import Error
import json
from functools import wraps

app = Flask(__name__)
sock = Sock(app)

db_config = {
    'host': '127.127.126.50',
    'user': 'esp32',
    'password': '1234',
    'database': 'ESP32'
}


def handle_websocket_close(func):
    @wraps(func)
    def wrapper(ws):
        client_ip = ws.environ.get('REMOTE_ADDR', 'unknown')
        print(f"New connection from {client_ip}")

        try:
            return func(ws)
        except (ConnectionError, BrokenPipeError, OSError) as e:
            print(f"WebSocket connection lost with {client_ip}: {e}")
            on_websocket_close(ws, client_ip)
        except Exception as e:
            print(f"Unexpected error with {client_ip}: {e}")
            on_websocket_close(ws, client_ip)
        finally:
            on_websocket_close(ws, client_ip)

    return wrapper


def on_websocket_close(ws, client_ip="unknown"):
    print(f"Connection with {client_ip} closed")
    # Здесь можно добавить очистку ресурсов

def write_to_db(data):
    try:
        connection = mysql.connector.connect(**db_config)
        cursor = connection.cursor()

        query = """INSERT INTO indication 
                   (status, level_garbage, level_energy) 
                   VALUES (%s, %s, %s)"""
        cursor.execute(query, (data['status'], data['level_garbage'], data['level_energy']))
        connection.commit()

        print(f"Data inserted: {data}")

    except Error as e:
        print(f"Database error: {e}")
        # При ошибке записи в БД отправляем статус 0
        error_data = {'status': 0, 'level_garbage': 0, 'level_energy': 0}
        write_to_db(error_data)
    finally:
        if connection.is_connected():
            cursor.close()
            connection.close()

@sock.route('/websocket')
@handle_websocket_close
def websocket(ws):

    while True:
        last_active = time.time()
        message = ws.receive(timeout=5)  # Ожидание сообщения от ESP32

        print("Received: ", message)
        try:
            message = ws.receive()  # Таймаут 5 секунд
            message = json.loads(message)
            # Проверяем наличие всех полей
            if all(key in message for key in ['status', 'level_garbage', 'level_energy']):
                write_to_db(message)
                response = "Database INSERT"  # Формирование ответа
                ws.send(response)  # Отправка ответа обратно на ESP32
            else:
                print("Invalid data format")
                ws.send("Invalid data format")  # Отправка ответа обратно на ESP32

        except Exception as e:
            print(f"Error processing data: {e}")
            ws.send(f"Error processing data: {e}")  # Отправка ответа обратно на ESP32
        except TimeoutError:
            # Проверяем, когда последний раз было активность
            if time.time() - last_active > 30:  # 30 секунд без активности
                raise ConnectionError("Client timeout")
            continue


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)