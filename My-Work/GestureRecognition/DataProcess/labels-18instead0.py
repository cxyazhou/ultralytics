import os
import glob
import argparse


def process_labels(input_dir, output_dir=None):
    """
    处理 YOLO 格式的标签文件：
    - 删除类别 ID 为 0 的所有行
    - 将类别 ID 18 替换为 0
    - 保持其他行不变

    参数:
        input_dir: 包含原始标签文件的目录
        output_dir: 处理后的文件输出目录（默认覆盖原文件）
    """
    # 设置输出目录（如果未指定则覆盖原文件）
    if output_dir is None:
        output_dir = input_dir
    else:
        os.makedirs(output_dir, exist_ok=True)

    # 统计处理结果
    total_files = 0
    total_lines = 0
    deleted_lines = 0
    modified_lines = 0

    # 获取所有txt文件
    txt_files = glob.glob(os.path.join(input_dir, '*.txt'))

    for file_path in txt_files:
        total_files += 1
        output_path = os.path.join(output_dir, os.path.basename(file_path))

        # 读取文件内容
        with open(file_path, 'r') as f:
            lines = f.readlines()

        # 处理每一行
        new_lines = []
        for line in lines:
            total_lines += 1
            parts = line.strip().split()

            # 跳过空行
            if not parts:
                continue

            # 获取类别ID
            try:
                class_id = int(parts[0])
            except ValueError:
                # 如果无法转换为整数，保留原行
                new_lines.append(line)
                continue

            # 处理逻辑
            if class_id == 0:
                # 删除类别0的行
                deleted_lines += 1
                continue
            elif class_id == 18:
                # 将类别18改为0
                parts[0] = '0'
                modified_lines += 1
                new_lines.append(' '.join(parts) + '\n')
            else:
                # 保留其他行
                new_lines.append(line)

        # 写入处理后的文件
        with open(output_path, 'w') as f:
            f.writelines(new_lines)

    # 打印统计信息
    print(f"\n处理完成!")
    print(f"处理文件数量: {total_files}")
    print(f"总行数: {total_lines}")
    print(f"删除行数 (类别0): {deleted_lines}")
    print(f"修改行数 (类别18→0): {modified_lines}")
    print(f"保留行数: {total_lines - deleted_lines - modified_lines}")
    print(f"输出目录: {os.path.abspath(output_dir)}")


if __name__ == "__main__":
    # 创建命令行参数解析
    parser = argparse.ArgumentParser(
        description='YOLO标签处理工具: 删除类别0行，将类别18改为0',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        '--input',
        type=str,
        default='labels',
        help='包含原始标签文件的目录'
    )
    parser.add_argument(
        '--output',
        type=str,
        default=None,
        help='处理后的文件输出目录 (默认覆盖原文件)'
    )

    args = parser.parse_args()

    # 运行处理函数
    process_labels(args.input, args.output)