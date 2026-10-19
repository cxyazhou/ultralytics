import os
import random
import shutil


def move_samples(source_dir, test_img_dir, test_labels_dir, sample_rate=0.1):
    """
    从每个图片文件夹随机抽取样本及其标签.

    :param source_dir: 包含图片文件夹和标签文件夹的根目录
    :param test_img_dir: 测试图片目标目录
    :param test_labels_dir: 测试标签目标目录
    :param sample_rate: 抽样比例 (0-1)
    """
    # 创建目标目录
    os.makedirs(test_img_dir, exist_ok=True)
    os.makedirs(test_labels_dir, exist_ok=True)

    # 遍历源目录下的所有子目录
    for dir_name in os.listdir(source_dir):
        img_dir = os.path.join(source_dir, dir_name)

        # 跳过非目录文件、标签目录和目标目录
        if not os.path.isdir(img_dir) or dir_name.endswith("_labels") or dir_name in [test_img_dir, test_labels_dir]:
            continue

        # 获取对应的标签目录
        labels_dir = os.path.join(source_dir, f"{dir_name}_labels")
        if not os.path.exists(labels_dir):
            print(f"警告: 找不到标签目录 {labels_dir}，跳过图片目录 {img_dir}")
            continue

        # 获取所有JPG文件
        jpg_files = [f for f in os.listdir(img_dir) if f.lower().endswith(".jpg")]
        if not jpg_files:
            print(f"警告: 图片目录 {img_dir} 中没有JPG文件")
            continue

        # 计算抽样数量（至少1个）
        sample_count = max(1, int(len(jpg_files) * sample_rate))
        sampled_files = random.sample(jpg_files, sample_count)

        print(f"从 {dir_name} 抽取 {sample_count}/{len(jpg_files)} 个文件")

        # 移动图片和对应的标签
        for jpg_file in sampled_files:
            # 图片路径处理
            src_img = os.path.join(img_dir, jpg_file)
            dest_img = os.path.join(test_img_dir, jpg_file)

            # 标签路径处理
            base_name = os.path.splitext(jpg_file)[0]
            label_file = f"{base_name}.txt"
            src_label = os.path.join(labels_dir, label_file)
            dest_label = os.path.join(test_labels_dir, label_file)

            # 检查标签是否存在
            if not os.path.exists(src_label):
                print(f"警告: 标签文件不存在 {src_label}，跳过图片 {src_img}")
                continue

            # 移动文件（避免覆盖）
            if not os.path.exists(dest_img):
                shutil.move(src_img, dest_img)
            else:
                print(f"警告: 目标文件已存在 {dest_img}，跳过移动")

            if not os.path.exists(dest_label):
                shutil.move(src_label, dest_label)
            else:
                print(f"警告: 目标文件已存在 {dest_label}，跳过移动")


if __name__ == "__main__":
    # 配置参数
    SOURCE_DIR = "E:/Light-HaGRID-Yolo"  # 包含图片和标签文件夹的根目录
    TEST_IMG_DIR = "./myTmpImg"  # 测试图片目标目录
    TEST_LABELS_DIR = "./myTmpLabels"  # 测试标签目标目录
    SAMPLE_RATE = 0.01  # 抽样比例 (1%)

    print("开始抽样移动文件...")
    move_samples(SOURCE_DIR, TEST_IMG_DIR, TEST_LABELS_DIR, SAMPLE_RATE)
    print("\n操作完成！")
    print(f"测试图片已保存到: {os.path.abspath(TEST_IMG_DIR)}")
    print(f"测试标签已保存到: {os.path.abspath(TEST_LABELS_DIR)}")
