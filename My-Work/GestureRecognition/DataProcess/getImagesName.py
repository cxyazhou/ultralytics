import os
import sys

# 定义文件夹路径
image_dir = "E:/train/images"  # 图片所在的文件夹
output_file = "./train.txt"  # 输出的文本文件名

# 确保images文件夹存在
if not os.path.exists(image_dir):
    print(f"错误：'{image_dir}' 文件夹不存在！")
    sys.exit(1)

# 收集所有JPG文件路径
image_files = []
for filename in os.listdir(image_dir):
    if filename.lower().endswith((".jpg", ".jpeg")):
        # 格式化为 ./images/xxx.jpg
        image_files.append(f"./images/{filename}")

# 检查是否找到图片
if not image_files:
    print(f"警告：'{image_dir}' 文件夹中没有找到JPG图片！")
else:
    print(f"找到 {len(image_files)} 张JPG图片")

# 写入文本文件
try:
    with open(output_file, "w") as f:
        f.write("\n".join(image_files))
    print(f"成功生成图片列表文件: {output_file}")
except OSError as e:
    print(f"写入文件失败: {e}")
