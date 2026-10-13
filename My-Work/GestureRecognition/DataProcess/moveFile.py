import os
import shutil


def move_files_to_parent():
    # 获取当前工作目录
    current_dir = "E:/Light-HaGRID-Yolo/labels"
    print(f"当前目录: {current_dir}")

    # 遍历当前目录下的所有项目
    for item in os.listdir(current_dir):
        item_path = os.path.join(current_dir, item)

        # 只处理文件夹（跳过文件）
        if os.path.isdir(item_path):
            print(f"\n处理文件夹: {item}")
            file_count = 0

            # 遍历文件夹中的所有文件
            for root, _, files in os.walk(item_path):
                for filename in files:
                    file_path = os.path.join(root, filename)

                    # 构造目标路径（当前目录）
                    dest_path = os.path.join(current_dir, filename)

                    # 处理文件名冲突
                    counter = 1
                    while os.path.exists(dest_path):
                        name, ext = os.path.splitext(filename)
                        dest_path = os.path.join(current_dir, f"{name}_{counter}{ext}")
                        counter += 1

                    # 移动文件
                    try:
                        shutil.move(file_path, dest_path)
                        print(f"移动: {filename} -> {os.path.basename(dest_path)}")
                        file_count += 1
                    except Exception as e:
                        print(f"移动 {filename} 失败: {e}")

            print(f"已移动 {file_count} 个文件")

    print("\n操作完成！所有子文件夹中的文件已移动到当前目录")

#将当前目录下所有子文件夹中的文件移动到当前目录（父目录）中，并保留原始文件名
if __name__ == "__main__":
    move_files_to_parent()