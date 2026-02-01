#!/usr/bin/env python3
import argparse, asyncio, os, ssl, signal
from websockets import connect
from websockets.exceptions import ConnectionClosed

URL = "wss://ws.multiverse-framework.com"

class WSClient:
    def __init__(self, client_id: str, stop_event: asyncio.Event):
        self.client_id = client_id
        self.stop_event = stop_event
        base = os.path.dirname(os.path.abspath(__file__))
        self.sdp_dir = os.path.join(base, "..", "SDP", client_id)
        self.offer_path = os.path.join(self.sdp_dir, "offer.txt")
        self.answer_path = os.path.join(self.sdp_dir, "answer.txt")

    async def wait_file(self, path: str, dt: float = 0.1):
        while not os.path.exists(path):
            if self.stop_event.is_set():
                raise asyncio.CancelledError()
            await asyncio.sleep(dt)

    async def run(self):
        ssl_ctx = ssl.create_default_context()
        ws = None
        try:
            ws = await connect(URL, ssl=ssl_ctx)
            print("[client] connected")

            await ws.send(self.client_id)
            print(f"[client] sent: {self.client_id!r}")

            reply = await ws.recv()
            print(f"[client] recv: {reply!r}")
            if reply != "OK":
                return

            offer = await ws.recv()
            os.makedirs(self.sdp_dir, exist_ok=True)
            open(self.offer_path, "w", encoding="utf-8").write(offer)
            print(f"[client] saved {self.offer_path}")

            print(f"[client] waiting for {self.answer_path} ...")
            await self.wait_file(self.answer_path)

            answer = open(self.answer_path, "r", encoding="utf-8").read()
            await ws.send(answer)
            print("[client] sent answer.txt to server")

            try:
                ack = await ws.recv()
                print(f"[client] recv: {ack!r}")
            except Exception:
                pass

        except (ConnectionClosed, asyncio.CancelledError):
            pass
        finally:
            if ws is not None:
                try:
                    await ws.close()
                except Exception:
                    pass
            print("[client] done")

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

    await WSClient(args.client_id, stop_event).run()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        pass
