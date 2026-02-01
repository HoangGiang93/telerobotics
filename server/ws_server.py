#!/usr/bin/env python3
import argparse, asyncio, os, signal
import websockets
from websockets.exceptions import ConnectionClosed

HOST, PORT = "0.0.0.0", 7575

class WSServer:
    def __init__(self, client_id, stop_event: asyncio.Event):
        self.client_id = client_id
        self.stop_event = stop_event
        base = os.path.dirname(os.path.abspath(__file__))
        self.sdp_dir = os.path.join(base, "..", "SDP", client_id)
        self.offer_path = os.path.join(self.sdp_dir, "offer.txt")
        self.answer_path = os.path.join(self.sdp_dir, "answer.txt")

    async def wait_file(self, path, dt=0.1):
        while not os.path.exists(path):
            if self.stop_event.is_set():
                raise asyncio.CancelledError()
            await asyncio.sleep(dt)

    async def handler(self, ws):
        try:
            # allow Ctrl+C to break a hanging recv()
            async def recv():
                return await ws.recv()

            first = await asyncio.wait_for(recv(), timeout=None)
            if first != self.client_id:
                await ws.send("ERR")
                return
            await ws.send("OK")

            await self.wait_file(self.offer_path)
            await ws.send(open(self.offer_path, "r", encoding="utf-8").read())

            answer = await asyncio.wait_for(recv(), timeout=None)
            os.makedirs(self.sdp_dir, exist_ok=True)
            open(self.answer_path, "w", encoding="utf-8").write(answer)

            await ws.send("ANSWER_SAVED")
            print(f"[server] saved {self.answer_path}")

        except (ConnectionClosed, asyncio.CancelledError):
            pass
        finally:
            try:
                await ws.close()
            except Exception:
                pass

async def main():
    p = argparse.ArgumentParser()
    p.add_argument("client_id", nargs="?", default="client_1")
    args = p.parse_args()

    stop_event = asyncio.Event()
    loop = asyncio.get_running_loop()

    def _stop(*_):
        stop_event.set()

    for s in (signal.SIGINT, signal.SIGTERM):
        try:
            loop.add_signal_handler(s, _stop)
        except NotImplementedError:
            signal.signal(s, lambda *_: _stop())

    server = WSServer(args.client_id, stop_event)

    async with websockets.serve(server.handler, HOST, PORT):
        print(f"[server] ws://{HOST}:{PORT}, client='{server.client_id}'")
        await stop_event.wait()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        pass
