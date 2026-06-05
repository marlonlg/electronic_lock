#!/usr/bin/env python3
from collections import deque
import json
import logging
import threading
import time

from user_records import UserRecords
from socket_driver import SocketDriver


LVGL_READ_SOCKET_PATH = "/tmp/lvgl_read.sock"
LVGL_WRITE_SOCKET_PATH = "/tmp/lvgl_write.sock"
RFID_POLL_INTERVAL_S = 0.1


class LvglManager:
    def __init__(self):
        self.socket_read = SocketDriver(LVGL_READ_SOCKET_PATH)
        self.socket_write = SocketDriver(LVGL_WRITE_SOCKET_PATH)
    
    def listen(self):
        data = self.socket_read.listen()
        return data 
    
    def transfer(self, data):
        self.socket_write.send(data, LVGL_WRITE_SOCKET_PATH)

    def process(self):
        data = {}
        try:
            data = json.loads(self.listen())
        except Exception as e:
            print(e)
        return data

class CoreSystem:
    def __init__(self):
        self.user_records = UserRecords()
        self.lvgl_manager = LvglManager()
        self.main_tag_queue = deque()
        self.register_tag_queue = deque()
        self.queue_lock = threading.Lock()
        self.stop_event = threading.Event()
        self.rfid_thread = threading.Thread(target=self.rfid_process, daemon=True)
        self.rfid_thread.start()

    def clear_tag_queues(self):
        with self.queue_lock:
            self.main_tag_queue.clear()
            self.register_tag_queue.clear()

    def pop_main_tag(self):
        with self.queue_lock:
            if not self.main_tag_queue:
                return None
            return self.main_tag_queue.popleft()

    def pop_register_tag(self):
        with self.queue_lock:
            if not self.register_tag_queue:
                return None
            return self.register_tag_queue.popleft()

    def enqueue_known_tag(self, user):
        with self.queue_lock:
            self.main_tag_queue.append(user)

    def enqueue_unknown_tag(self, tag_rfid):
        with self.queue_lock:
            self.register_tag_queue.append(tag_rfid)
 
    def lvgl_process(self):
        output = []
        data = self.lvgl_manager.process()
        cmd = data.get("cmd")

        if not cmd:
            return output

        if cmd == "read_main_tag":
            user = self.pop_main_tag()
            if user:
                output = {
                    "cmd": "authorized",
                    "usuario": user.get("usuario", ""),
                    "tag_rfid": user.get("tag_rfid", ""),
                    "img_path": user.get("img_path", ""),
                }

        elif cmd == "read_register_tag":
            tag_rfid = self.pop_register_tag()
            if tag_rfid:
                output = {
                    "cmd": "tag_read",
                    "tag_rfid": tag_rfid,
                }

        else:
            self.clear_tag_queues()

        if cmd == "add_user":
            self.user_records.insert(data)

        elif cmd == "list_users":
            output += self.user_records.get_all()
        
        elif cmd == "delete_user":
            self.user_records.delete(data["id"])
        
        return output

    def rfid_process(self):
        try:
            from rfid_reader import MFRC522, SPI_BUS, SPI_DEV, SPI_SPEED, hex_list

            reader = MFRC522(SPI_BUS, SPI_DEV, SPI_SPEED)
            user_records = UserRecords(self.user_records.db_name)
        except Exception as e:
            logging.exception("Falha ao iniciar leitor RFID: %s", e)
            return

        last_uid = None

        try:
            while not self.stop_event.is_set():
                uid = reader.read_uid()

                if uid:
                    tag_rfid = hex_list(uid)

                    if uid != last_uid:
                        user = user_records.fetch_by_rfid(tag_rfid)

                        if user:
                            self.enqueue_known_tag(user)
                        else:
                            self.enqueue_unknown_tag(tag_rfid)

                    last_uid = uid
                else:
                    last_uid = None

                time.sleep(RFID_POLL_INTERVAL_S)

        finally:
            user_records.close()
            reader.close()


    def run(self):
        try:
            while True:
                output = self.lvgl_process()
                if output:
                    data = json.dumps(output)
                    self.lvgl_manager.transfer(data)
        finally:
            self.stop_event.set()
            self.user_records.close()

    
if __name__ == "__main__":
    core = CoreSystem()
    core.run()
