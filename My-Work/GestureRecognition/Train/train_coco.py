from ultralytics import YOLO

# 加载预训练模型
model = YOLO('yolov8n.pt')

# 开始训练，指定使用 CPU
results = model.train(
    data='coco8.yaml',   # 数据集配置文件
    epochs=100,          # 训练周期数
    imgsz=320,           # 输入图像大小
    device='cpu'         #
)
