from ultralytics import YOLO
#import torch

# check GPU
#print(torch.cuda.get_device_name(0))
import torch

# 检查是否支持 MPS
if torch.backends.mps.is_available():
    device = torch.device("mps")
    print("Using Metal (MPS) GPU acceleration!")
else:
    device = torch.device("cpu")
    print("Using CPU!")

print(f"PyTorch is using: {device}")
# Load the model.
model = YOLO('yolov8m.pt') # pretrained model 
 
# Training.
results = model.train(
   data='training.yaml',  # .yaml file 
   imgsz=416, # image size
   epochs=50,  # epoch number
   batch=4, # batch size , I normally use 8 or 16 but my GPU gave memory errors, therefore I reduced it to 4 for this time.
   name='train_model_model4', # output folder name, it contains model weights and all of the other things.
    plots=True, # Plots about metrics (precision, recall,f1 score)
 amp=False, # amp=True gives me an error, I don't know why , If it doesn't give you an error, set it to True
)