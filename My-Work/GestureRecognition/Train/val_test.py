from ultralytics import YOLO

def validate_model():
    # 加载预训练模型
    model = YOLO('best.pt')
    
    # 执行验证
    results = model.val(
        data='coco8.yaml',     # 数据集配置文件路径
        split='test',          # 使用测试集（YOLOv8会自动处理）
        imgsz=320,             # 图像尺寸
        conf=0.25,             # 置信度阈值
        iou=0.7,               # IoU阈值
        name='yolov8_test',    # 结果保存名称
        save_json=True         # 保存JSON格式结果
    )

if __name__ == '__main__':
    validate_model()