#!/usr/bin/env python3
import spidev
import time
import logging

SPI_BUS = 0
SPI_DEV = 0
SPI_SPEED = 500_000

LOG_LEVEL = logging.INFO
# Para debug completo:
# LOG_LEVEL = logging.DEBUG

logging.basicConfig(
    level=LOG_LEVEL,
    format="%(asctime)s [%(levelname)s] %(message)s",
)

# MFRC522 commands
PCD_IDLE        = 0x00
PCD_CALCCRC     = 0x03
PCD_TRANSCEIVE  = 0x0C
PCD_SOFTRESET   = 0x0F

# PICC commands
PICC_REQA       = 0x26

# Cascade levels
PICC_ANTICOLL_CL1 = 0x93
PICC_ANTICOLL_CL2 = 0x95
PICC_SELECT_NV    = 0x70

CASCADE_TAG = 0x88

# Registers
CommandReg      = 0x01
ComIEnReg       = 0x02
ComIrqReg       = 0x04
DivIrqReg       = 0x05
ErrorReg        = 0x06
Status2Reg      = 0x08
FIFODataReg     = 0x09
FIFOLevelReg    = 0x0A
BitFramingReg   = 0x0D
ModeReg         = 0x11
TxControlReg    = 0x14
TxASKReg        = 0x15
TModeReg        = 0x2A
TPrescalerReg   = 0x2B
TReloadRegH     = 0x2C
TReloadRegL     = 0x2D
CRCResultRegH   = 0x21
CRCResultRegL   = 0x22
VersionReg      = 0x37


def hex_list(data):
    return " ".join(f"{b:02X}" for b in data)


class MFRC522:
    def __init__(self, bus=0, dev=0, speed=500_000):
        logging.debug("Abrindo SPI bus=%d dev=%d speed=%d Hz", bus, dev, speed)

        self.spi = spidev.SpiDev()
        self.spi.open(bus, dev)
        self.spi.max_speed_hz = speed
        self.spi.mode = 0
        self.spi.bits_per_word = 8

        self.reset()
        self.init()

    def close(self):
        logging.debug("Fechando SPI")
        self.spi.close()

    def write_reg(self, reg, value):
        addr = (reg << 1) & 0x7E
        self.spi.xfer2([addr, value & 0xFF])
        logging.debug("WRITE reg=0x%02X value=0x%02X", reg, value & 0xFF)

    def read_reg(self, reg):
        addr = ((reg << 1) & 0x7E) | 0x80
        value = self.spi.xfer2([addr, 0x00])[1]
        logging.debug("READ  reg=0x%02X value=0x%02X", reg, value)
        return value

    def set_bit_mask(self, reg, mask):
        self.write_reg(reg, self.read_reg(reg) | mask)

    def clear_bit_mask(self, reg, mask):
        self.write_reg(reg, self.read_reg(reg) & (~mask & 0xFF))

    def reset(self):
        logging.debug("Resetando MFRC522")
        self.write_reg(CommandReg, PCD_SOFTRESET)
        time.sleep(0.05)

    def antenna_on(self):
        value = self.read_reg(TxControlReg)

        if not (value & 0x03):
            logging.debug("Ativando antena")
            self.set_bit_mask(TxControlReg, 0x03)
        else:
            logging.debug("Antena já estava ativa")

    def init(self):
        logging.debug("Inicializando registradores do MFRC522")

        self.write_reg(CommandReg, PCD_IDLE)

        self.write_reg(TModeReg, 0x8D)
        self.write_reg(TPrescalerReg, 0x3E)
        self.write_reg(TReloadRegL, 30)
        self.write_reg(TReloadRegH, 0)

        self.write_reg(TxASKReg, 0x40)
        self.write_reg(ModeReg, 0x3D)

        self.antenna_on()

        version = self.read_reg(VersionReg)
        logging.debug("VersionReg = 0x%02X", version)

        if version not in (0x91, 0x92):
            logging.warning("MFRC522 não respondeu como esperado")

    def transceive(self, data, valid_bits=0):
        logging.debug("TRANSCEIVE TX: %s valid_bits=%d", hex_list(data), valid_bits)

        self.write_reg(CommandReg, PCD_IDLE)
        self.write_reg(ComIrqReg, 0x7F)
        self.set_bit_mask(FIFOLevelReg, 0x80)

        for byte in data:
            self.write_reg(FIFODataReg, byte)

        self.write_reg(BitFramingReg, valid_bits)
        self.write_reg(CommandReg, PCD_TRANSCEIVE)
        self.set_bit_mask(BitFramingReg, 0x80)

        timeout = 2000

        while timeout:
            irq = self.read_reg(ComIrqReg)

            if irq & 0x30:
                break

            timeout -= 1

        self.clear_bit_mask(BitFramingReg, 0x80)

        if timeout == 0:
            logging.debug("TRANSCEIVE timeout")
            return None

        error = self.read_reg(ErrorReg)

        if error & 0x1B:
            logging.debug("TRANSCEIVE erro ErrorReg=0x%02X", error)
            return None

        length = self.read_reg(FIFOLevelReg)
        result = []

        for _ in range(length):
            result.append(self.read_reg(FIFODataReg))

        logging.debug("TRANSCEIVE RX: %s", hex_list(result))

        return result

    def calculate_crc(self, data):
        logging.debug("Calculando CRC para: %s", hex_list(data))

        self.clear_bit_mask(DivIrqReg, 0x04)
        self.set_bit_mask(FIFOLevelReg, 0x80)

        for byte in data:
            self.write_reg(FIFODataReg, byte)

        self.write_reg(CommandReg, PCD_CALCCRC)

        timeout = 255

        while timeout:
            if self.read_reg(DivIrqReg) & 0x04:
                break

            timeout -= 1

        if timeout == 0:
            logging.warning("Timeout calculando CRC")
            return None

        crc_l = self.read_reg(CRCResultRegL)
        crc_h = self.read_reg(CRCResultRegH)

        logging.debug("CRC = %02X %02X", crc_l, crc_h)

        return [crc_l, crc_h]

    def request(self):
        logging.debug("Enviando REQA")

        self.write_reg(BitFramingReg, 0x07)
        result = self.transceive([PICC_REQA], valid_bits=0x07)

        if result is None:
            logging.debug("Nenhum cartão detectado")
        else:
            logging.debug("Cartão detectado. ATQA: %s", hex_list(result))

        return result

    def anticoll(self, cascade_level):
        logging.debug("Executando ANTICOLL CL%d", 1 if cascade_level == PICC_ANTICOLL_CL1 else 2)

        self.write_reg(BitFramingReg, 0x00)

        result = self.transceive([cascade_level, 0x20])

        if result is None:
            logging.warning("ANTICOLL sem resposta")
            return None

        if len(result) < 5:
            logging.warning("ANTICOLL resposta curta: %s", hex_list(result))
            return None

        uid_part = result[:4]
        bcc = result[4]

        calc_bcc = uid_part[0] ^ uid_part[1] ^ uid_part[2] ^ uid_part[3]

        logging.debug(
            "ANTICOLL RX: %s | BCC=0x%02X calc=0x%02X",
            hex_list(result),
            bcc,
            calc_bcc,
        )

        if calc_bcc != bcc:
            logging.warning("BCC inválido no ANTICOLL")
            return None

        return uid_part

    def select_tag(self, cascade_level, uid_part):
        logging.debug(
            "Executando SELECT CL%d UID_PART=%s",
            1 if cascade_level == PICC_ANTICOLL_CL1 else 2,
            hex_list(uid_part),
        )

        bcc = uid_part[0] ^ uid_part[1] ^ uid_part[2] ^ uid_part[3]

        frame = [
            cascade_level,
            PICC_SELECT_NV,
            uid_part[0],
            uid_part[1],
            uid_part[2],
            uid_part[3],
            bcc,
        ]

        crc = self.calculate_crc(frame)

        if crc is None:
            return None

        frame.extend(crc)

        result = self.transceive(frame)

        if result is None:
            logging.warning("SELECT sem resposta")
            return None

        if len(result) < 1:
            logging.warning("SELECT resposta vazia")
            return None

        sak = result[0]

        logging.debug("SELECT SAK=0x%02X", sak)

        return sak

    def read_uid(self):
        if self.request() is None:
            return None

        uid_cl1 = self.anticoll(PICC_ANTICOLL_CL1)

        if uid_cl1 is None:
            return None

        sak1 = self.select_tag(PICC_ANTICOLL_CL1, uid_cl1)

        if sak1 is None:
            return None

        if uid_cl1[0] == CASCADE_TAG:
            logging.debug("UID longo detectado: Cascade Tag 0x88 presente")

            uid_cl2 = self.anticoll(PICC_ANTICOLL_CL2)

            if uid_cl2 is None:
                return None

            sak2 = self.select_tag(PICC_ANTICOLL_CL2, uid_cl2)

            if sak2 is None:
                return None

            uid = uid_cl1[1:] + uid_cl2

            logging.debug("UID 7 bytes lido: %s", hex_list(uid))

            return uid

        uid = uid_cl1

        logging.debug("UID 4 bytes lido: %s", hex_list(uid))

        return uid


def main():
    reader = MFRC522(SPI_BUS, SPI_DEV, SPI_SPEED)

    last_uid = None

    try:
        while True:
            uid = reader.read_uid()

            if uid:
                uid_str = hex_list(uid)

                if uid != last_uid:
                    logging.debug("UID: %s", uid_str)

                last_uid = uid
            else:
                last_uid = None

            time.sleep(0.1)

    except KeyboardInterrupt:
        logging.debug("\nEncerrado.")

    finally:
        reader.close()


if __name__ == "__main__":
    main()
