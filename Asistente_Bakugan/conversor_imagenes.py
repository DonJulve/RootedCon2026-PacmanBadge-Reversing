import os
from PIL import Image

def generate_header(img_paths, var_names, out_path, size):
    width, height = size
    with open(out_path, "w") as f:
        f.write("#include <pgmspace.h>\n\n")
        
        valid_vars = []
        for img_path, var_name in zip(img_paths, var_names):
            if not os.path.exists(img_path):
                print(f"El archivo {img_path} no existe. Saltando...")
                # Escribir array vacío para que el código compile
                f.write(f"const uint16_t {var_name}[{width*height}] PROGMEM = {{0}};\n\n")
                valid_vars.append(var_name)
                continue
                
            try:
                img = Image.open(img_path).convert("RGBA")
                canvas = Image.new("RGB", size, (0, 0, 0))
                img.thumbnail(size, Image.Resampling.LANCZOS)
                
                x = (width - img.width) // 2
                y = (height - img.height) // 2
                
                if 'A' in img.getbands():
                    canvas.paste(img, (x, y), img.split()[3])
                else:
                    canvas.paste(img, (x, y))
                
                f.write(f"const uint16_t {var_name}[{width*height}] PROGMEM = {{\n")
                for py in range(height):
                    for px in range(width):
                        r, g, b = canvas.getpixel((px, py))
                        rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                        f.write(f"0x{rgb565:04X}, ")
                    f.write("\n")
                f.write("};\n\n")
                valid_vars.append(var_name)
                print(f"Procesado {img_path} a {size[0]}x{size[1]}")
            except Exception as e:
                print(f"Error al convertir {img_path}: {e}")
                f.write(f"const uint16_t {var_name}[{width*height}] PROGMEM = {{0}};\n\n")
                valid_vars.append(var_name)
        
        # Array of pointers
        array_name = out_path.replace('.h', '_images')
        f.write(f"const uint16_t* const {array_name}[{len(valid_vars)}] = {{\n")
        for v in valid_vars:
            f.write(f"  {v},\n")
        f.write("};\n")
        print(f"Generado exitosamente: {out_path}")

def convert_to_h(img_path, var_name, out_path, size=(128, 128)):
    generate_header([img_path], [var_name], out_path, size)

if __name__ == "__main__":
    print("Iniciando conversor de imagenes para ESP32 Bakugan...")
    # Logos y Rueda
    convert_to_h("logo.png", "bakugan_logo", "logo.h", (128, 128))
    convert_to_h("wheel.png", "bakugan_wheel", "wheel.h", (128, 128))
    
    # Atributos: Pyrus, Aquos, Subterra, Haos, Darkus, Ventus
    attr_images = ["Pyrus.png", "Aquos.png", "Subterra.png", "Haos.png", "Darkus.png", "Ventus.png"]
    attr_names_small = [f"attr_{n.split('.')[0].lower()}_small" for n in attr_images]
    attr_names_large = [f"attr_{n.split('.')[0].lower()}_large" for n in attr_images]
    
    generate_header(attr_images, attr_names_small, "attr_small.h", (10, 10))
    generate_header(attr_images, attr_names_large, "attr_large.h", (40, 40))
    
    print("¡Proceso finalizado!")
