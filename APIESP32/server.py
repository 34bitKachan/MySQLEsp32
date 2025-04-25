from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import HTMLResponse

app = FastAPI()

# Простая HTML-страница для тестирования WebSocket
html = """
<!DOCTYPE html>
<html>
    <head>
        <title>WebSocket Test</title>
    </head>
    <body>
        <h1>WebSocket Test</h1>
        <form action="" onsubmit="sendMessage(event)">
            <input type="text" id="messageText" autocomplete="off"/>
            <button>Send</button>
        </form>
        <ul id='messages'>
        </ul>
        <script>
            const ws = new WebSocket("ws://192.168.0.15:8000/ws");
            ws.onmessage = function(event) {
                const messages = document.getElementById('messages');
                const message = document.createElement('li');
                const content = document.createTextNode(event.data);
                message.appendChild(content);
                messages.appendChild(message);
            };
            function sendMessage(event) {
                const input = document.getElementById("messageText");
                ws.send(input.value);
                input.value = '';
                event.preventDefault();
            }
        </script>
    </body>
</html>
"""

@app.get("/")
async def get():
    return HTMLResponse(html)

# WebSocket endpoint
@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()  # Принимаем соединение
    try:
        while True:
            data = await websocket.receive_text()  # Ждём сообщение от клиента
            await websocket.send_text(f"Message received: {data}")  # Отправляем ответ
    except WebSocketDisconnect:
        print("Client disconnected")