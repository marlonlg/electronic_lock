#!/usr/bin/env python3
import json
# from rfid_manager import RfidManager
from user_records import UserRecords
from socket_driver import SocketDriver


LVGL_READ_SOCKET_PATH = "/tmp/lvgl_read.sock"
LVGL_WRITE_SOCKET_PATH = "/tmp/lvgl_write.sock"


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
        # self.rfid_manager = RfidManager()
        self.user_records = UserRecords()
        self.lvgl_manager = LvglManager()
 
    def lvgl_process(self):
        output = []
        data = self.lvgl_manager.process()
        print(f"data {data}")
        if "add_user" in data["cmd"]:
            self.user_records.insert(data)

        elif "list_users" in data["cmd"]:
            print(2)
            output += self.user_records.get_all()
        
        elif "delete_user" in data["cmd"]:
            print(3)
            self.user_records.delete(data["id"])
        
        return output

    # def rfid_process(self):
    #     id, text = self.rfid_manager.read_data()
    #     print(f"ID: {id}, Text: {text}")
    #     user = self.user_records.fetch_by_rfid(id)
    #     if user:
    #         print(f"Usuário encontrado: {user['usuario']}")
    #     else:
    #         print("Usuário não encontrado.")


    def run(self):
        while True:
            output = self.lvgl_process()
            if output:
                data = json.dumps(output)
                self.lvgl_manager.transfer(data)

    
if __name__ == "__main__":
    core = CoreSystem()
    core.run()
