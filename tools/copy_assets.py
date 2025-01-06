import os
import shutil
import sys

from PIL import Image


def convert_png_to_abgr(source_path, dest_path):
    with Image.open(source_path) as img:
        if img.mode != 'RGBA':
            img = img.convert('RGBA')

        r, g, b, a = img.split()
        converted = Image.merge('RGBA', (a, b, g, r))
        converted.save(dest_path, 'PNG')

def copy_assets(source_dir, dest_dir):
    if not os.path.exists(source_dir):
        print(f"Error: Source directory '{source_dir}' does not exist")
        return 1

    if os.path.exists(dest_dir):
        shutil.rmtree(dest_dir)

    os.makedirs(dest_dir)

    for root, dirs, files in os.walk(source_dir):
        rel_path = os.path.relpath(root, source_dir)
        dest_root = os.path.join(dest_dir, rel_path)

        for dir_name in dirs:
            os.makedirs(os.path.join(dest_root, dir_name), exist_ok=True)

        for file_name in files:
            source_file = os.path.join(root, file_name)
            dest_file = os.path.join(dest_root, file_name)

            if file_name.lower().endswith('.png'):
                convert_png_to_abgr(source_file, dest_file)
            else:
                shutil.copy2(source_file, dest_file)

    print(f"Assets copied from '{source_dir}' to '{dest_dir}'")
    return 0

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python copy_assets.py <source_dir> <dest_dir>")
        sys.exit(1)

    source_dir = sys.argv[1]
    dest_dir = sys.argv[2]
    sys.exit(copy_assets(source_dir, dest_dir))