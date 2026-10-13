from ultralytics import YOLO

# 加载训练好的模型
model = YOLO('best.pt')  # 替换为你的模型路径

# 导出为ONNX格式
model.export(
    format='onnx',
    imgsz=320,             # 输入尺寸（与训练时一致）
    opset=17,              # ONNX算子集版本（推荐12-17）
    nms=True,              # 包含NMS后处理（输出为[1, num_detections, 6]）
    max_det=50,            # 最多输出多少目标
    dynamic=False,         # 是否使用动态维度（True支持可变输入尺寸）
    simplify=True,         # 简化模型（移除冗余节点）
    half=False,            # FP16半精度（GPU加速，可能降低精度）
    device=[0],            # 导出设备（'cpu'或'cuda'）
)