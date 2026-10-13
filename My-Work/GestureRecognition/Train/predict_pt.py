from ultralytics import YOLO
model = YOLO('best.pt')

# 自动进行 Letterbox 处理（无需手动操作）
results = model.predict(
    source='input.jpg', 
    imgsz=320,  # 必须与训练尺寸一致！
    save=True    # 保存带结果的图像（可看到灰边）
)