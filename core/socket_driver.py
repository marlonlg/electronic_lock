import os
import socket


class SocketDriver:
    def __init__(self, socket_path: str = ""):
        self.socket_path = socket_path
        self.sock = None
        self.create_socket()

    def create_socket(self):
        if os.path.exists(self.socket_path):
            os.remove(self.socket_path)
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
        self.sock.bind(self.socket_path)

    def listen(self):
        data, _ = self.sock.recvfrom(1024)
        return data.decode().strip()
    
    def send(self, data: str, target_path: str):
        """
        Envia dados para um socket alvo.

        :param data: Dados a serem enviados.
        :param target_path: Caminho do socket alvo.
        """
        if not os.path.exists(target_path):
            raise FileNotFoundError(f"O socket alvo '{target_path}' não existe.")

        self.sock.sendto(data.encode(), target_path)

    def close(self):
        self.sock.close()
        if os.path.exists(self.socket_path):
            os.remove(self.socket_path)