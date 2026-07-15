import os
from PIL import Image

def convert_to_h(img_path, var_name, out_path):
    if not os.path.exists(img_path):
        print(f"El archivo {img_path} no existe. Saltando...")
        return
        
    try:
        img = Image.open(img_path).convert("RGBA")
        canvas = Image.new("RGB", (128, 128), (0, 0, 0))
        img.thumbnail((128, 128), Image.Resampling.LANCZOS)
        
        x = (128 - img.width) // 2
        y = (128 - img.height) // 2
        
        if 'A' in img.getbands():
            canvas.paste(img, (x, y), img.split()[3])
        else:
            canvas.paste(img, (x, y))
        
        with open(out_path, "w") as f:
            f.write("#include <pgmspace.h>\n\n")
            f.write(f"const uint16_t {var_name}[16384] PROGMEM = {{\n")
            for py in range(128):
                for px in range(128):
                    r, g, b = canvas.getpixel((px, py))
                    rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                    f.write(f"0x{rgb565:04X}, ")
                f.write("\n")
            f.write("};\n")
        print(f"Generado exitosamente: {out_path} desde {img_path}")
    except Exception as e:
        print(f"Error al convertir {img_path}: {e}")

if __name__ == "__main__":
    print("Iniciando conversor de imagenes para ESP32 Bakugan...")
    convert_to_h("logo.png", "bakugan_logo", "logo.h")
    convert_to_h("wheel.png", "bakugan_wheel", "wheel.h")
    print("¡Proceso finalizado!")
