import os
import shutil
import re

# 定义目标文件夹路径
src_directory = "src"
include_directory = "include"

# 确保目标文件夹存在
os.makedirs(src_directory, exist_ok=True)
os.makedirs(include_directory, exist_ok=True)

def move_files(directory):
    for root, dirs, files in os.walk(directory, topdown=True):
        # 排除src和include文件夹
        dirs[:] = [d for d in dirs if d not in [src_directory, include_directory]]
        for file in files:
            file_path = os.path.join(root, file)
            try:
                if file.endswith('.c') or file.endswith('.cpp'):
                    shutil.move(file_path, src_directory)
                    print(f"Moved {file_path} to {src_directory}")
                elif file.endswith('.h') or file.endswith('.hpp'):
                    shutil.move(file_path, include_directory)
                    print(f"Moved {file_path} to {include_directory}")
            except Exception as e:
                print(f"Error moving file {file_path}: {e}")

# 当前工作目录
current_directory = '.'

# 移动文件
move_files(current_directory)



def modify_includes_in_directory(directory):
    # 定义文件扩展名
    file_extensions = ['.h', '.c', '.cpp']

    # 遍历目录及其子目录
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(tuple(file_extensions)):
                file_path = os.path.join(root, file)

                # 读取文件内容
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()

                # 修改包含指令，去掉路径部分
                modified_content = re.sub(r'#include\s+"[^"]*/([^"]+)"', r'#include "\1"', content)

                # 写回修改后的内容
                with open(file_path, 'w', encoding='utf-8') as f:
                    f.write(modified_content)

                print(f"Modified includes in {file_path}")

# 当前工作目录下的include和src目录
directories_to_modify = ['./include', './src']

for directory in directories_to_modify:
    modify_includes_in_directory(directory)

def remove_empty_folders(path):
    # 遍历当前目录及其子目录
    for root, dirs, files in os.walk(path, topdown=False):
        for dir in dirs:
            dir_path = os.path.join(root, dir)
            # 检查文件夹是否为空
            if not os.listdir(dir_path):
                os.rmdir(dir_path)
                print(f"Deleted empty folder: {dir_path}")

# 当前工作目录
current_directory = '.'

# 删除空文件夹
remove_empty_folders(current_directory)
