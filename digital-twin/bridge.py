import serial
import asyncio
import websockets

SERIAL_PORT = 'COM10'
BAUD_RATE = 115200

connected_clients = set()

async def broadcast(data):
    if connected_clients:
        await asyncio.gather(*[c.send(data) for c in connected_clients])

async def read_serial():
    loop = asyncio.get_event_loop()
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0)
    print("Serial connected on COM10...")
    buf = ""
    while True:
        data = await loop.run_in_executor(None, ser.read, ser.in_waiting or 1)
        if data:
            buf += data.decode('utf-8', errors='ignore')
            while '\n' in buf:
                line, buf = buf.split('\n', 1)
                line = line.strip()
                if ',' in line:
                    await broadcast(line)
        await asyncio.sleep(0.005)  # 5ms instead of 50ms

async def handler(websocket):
    connected_clients.add(websocket)
    try:
        await websocket.wait_closed()
    finally:
        connected_clients.remove(websocket)

async def main():
    async with websockets.serve(handler, "localhost", 8765):
        print("WebSocket running on ws://localhost:8765")
        await asyncio.gather(read_serial())

asyncio.run(main())