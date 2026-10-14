from ultralytics import YOLO

# 加载预训练模型
model = YOLO('yolov8n.pt')

# 开始训练，指定使用 CPU
results = model.train(
    data='coco8.yaml',   # 数据集配置文件
    epochs=100,          # 训练周期数
    imgsz=320,           # 输入图像大小
    device='cpu',        # cpu训练
    batch=64,            # 从 16 改为 64
    optimizer="SGD",     # 关掉 auto，让 lr0 生效
    lr0=0.04,            # 从 0.01 修改为 0.04
    patience=10,         # 但连续 10 轮没提升就提前停
)
