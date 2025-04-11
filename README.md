#  **Fruit and vegetable visual recognition system based on Raspberry Pi** 

## Project Overview
This is an intelligent refrigerator system that identifies food types and freshness through visual recognition, and records food storage time to optimize storage and reduce food waste.

---

## 📌 INTRODUCTION
### **-Background** 
In our daily life,improper storage and forgotten food often cause uncertainty about consumption order, leading to spoilage and waste.So we came up with the idea of designing a smart refrigerator system to optimize storage and reduce food waste.

### **-Device**  
💡 **Based on Raspberry Pi 5 and requires an external camera.**

---

## 🔥 **Main function**
### ✅ **Automatic identification of stored ingredients**
When the **refrigerator door opens**, the camera activates to detect existing and new food.
### ✅ **Intelligent storage management**
The system **calculates shelf life** and displays food status, alerting users to expiring items.
### ✅ **Efficient operation mode**
The **camera is activated** to detect only when the refrigerator door is opened, saving energy.

---

## 🧩 Environment Setup

### 🛠️ Requirements

- Qt 5  
- OpenCV  
- Ultralytics YOLOv8  
- libgpio 

---

### 1.  Install Python Packages (for YOLOv8)

 ```bash 
pip install ultralytics opencv-python PyQt5 
 ```

### 2.  Install Ultralytics YOLOv8 (optional from source)

```bash 
git clone https://github.com/ultralytics/ultralytics.git
cd ultralytics
pip install -e . 
```

### 3. Install OpenCV (C++ version)

```bash 
sudo apt update
sudo apt install libopencv-dev 
``` 

### 4. Install Qt 5

```bash 
sudo apt update
sudo apt install qt5-default qtcreator -y 
``` 

### 5. Install libgpio

 ```bash 
sudo apt update
sudo apt install libgpiod-dev gpiod 
```



