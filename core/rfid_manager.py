#!/bin/env python3

import RPi.GPIO as GPIO
from mfrc522 import SimpleMFRC522
from typing import Tuple

GPIO.setwarnings(False)


class RfidManager:
    def __init__(self):
        self.rfid_driver = SimpleMFRC522()
    
    def read_data(self) -> Tuple[str, str]:
        return self.rfid_driver.read()

    def write_data(self, data: str):
        return self.rfid_driver.write(data)

