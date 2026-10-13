import os
import glob


def delete_json_files_silently(directory):
    """
    静默删除指定目录及其子目录中所有JSON文件
    :param directory: 目标目录路径
    :return: 删除的文件数量
    """
    json_files = glob.glob(os.path.join(directory, '**', '*.json'), recursive=True)

    if not json_files:
        return 0  # 没有找到JSON文件

    deleted_count = 0
    for file_path in json_files:
        try:
            os.remove(file_path)
            deleted_count += 1
        except Exception as e:
            # 静默失败，不显示错误信息
            pass

    return deleted_count


# 使用示例 - 直接执行删除
target_directory = "E:/GuesDet"  # 替换为你的目标目录
deleted_count = delete_json_files_silently(target_directory)
print(f"已静默删除 {deleted_count} 个JSON文件")