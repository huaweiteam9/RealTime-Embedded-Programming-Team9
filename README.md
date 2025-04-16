#  **Fruit and vegetable visual recognition system based on Raspberry Pi** 

## Project Overview
This is an intelligent refrigerator system that identifies food types and freshness through visual recognition, and records food storage time to optimize storage and reduce food waste.


---

## 📌 INTRODUCTION
### **-Background** 
In our daily life,improper storage and forgotten food often cause uncertainty about consumption order, leading to spoilage and waste.So we came up with the idea of designing a smart refrigerator system to optimize storage and reduce food waste.

### **- Device**

<p align="center">
  <img src="images/Raspberry.png" width="250"/><br>
  <strong>Raspberry Pi 5</strong>
</p>

<p align="center">
  <img src="images/camera.jpg" width="250"/><br>
  <strong>External Raspberry Camera</strong>
</p>

<p align="center">
  <img src="images/switch.jpg" width="250"/><br>
  <strong>Switch (KW11-B, 20×10×6.5mm)</strong>
</p>

<p align="center">
  <img src="images/sensor.jpg" width="250"/><br>
  <strong>Temperature and Humidity Sensor(DHT11)</strong>
</p>

---

## 🔥 **Main function**

### ✅ **Automatic identification of stored ingredients**  
When the **refrigerator door opens**, the camera automatically activates to detect both existing and newly added food.

### ✅ **Intelligent storage management**  
When the **refrigerator door closes**, the system records the recognition results, **calculates shelf life**, displays food status in real time, and alerts users to expiring items.

### ✅ **Temperature and humidity monitoring**  
The refrigerator continuously monitors **temperature and humidity** to ensure optimal operating conditions.

---

## 🧩 Environment Setup

### 🛠️ Requirements

- Qt 5  
- OpenCV  
- Ultralytics YOLOv5  
- libgpio 
- cmake

---

### 1.  Install Python Packages (for YOLOv5)

 ```bash 
pip install ultralytics opencv-python PyQt5 
 ```

### 2.  Install Ultralytics YOLOv5 (optional from source)

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

### 6. Install CMake

 ```bash 
sudo apt update
sudo apt install cmake 
```
---
## 🧩 Build & Run

### Build：

 ```bash 
mkdir build
cd build
cmake ..
make
```
### Run：

 ```bash 
./fridgemanager
```
## 🧩 Display

### When the fruit appears under the camera, the detection results will be displayed in real time.

<p align="center">
  <img src="images/apple.png" width="250"/><br>
  <strong>Detection Result</strong>
</p>

### When the switch is pressed (that is, when the refrigerator door is closed), the second interface will be displayed, showing the type of fruit identified, the date it was placed in the refrigerator, and the expiration date.

<p align="center">
  <img src="images/apple_PAGE2.png" width="250"/><br>
  <strong>Detection Result2</strong>
</p>

---

## 🧩 Social Media

### Here is our tiktok link, where we documented the process of completing the entire project.

[tiktok_link_foliage](https://www.tiktok.com/@foliage419?_t=ZN-8vaBg5RbNJa&_r=1)  

---

## 🧊 Minimum Shelf Life of Fruits and Vegetables in Refrigerator (~4°C)

| Item                  | Min. Shelf Life (days) | Recommended for Refrigeration | Reference |
|-----------------------|------------------------|-------------------------------|-----------|
| Apple                 | 28                     | ✅ Yes                         | [1]       |
| Cabbage               | 28                     | ✅ Yes                         | [2]       |
| Carrot                | 28                     | ✅ Yes                         | [3]       |
| Grape                 | 7                      | ✅ Yes                         | [4]       |
| Lemon                 | 28                     | ✅ Yes                         | [5]       |
| Mango (ripe only)     | 5                      | ⚠️ Yes (only after ripening)  | [6]       |
| Napa cabbage          | 56                     | ✅ Yes                         | [7]       |
| Peach (ripe only)     | 3                      | ✅ Yes (only after ripening)  | [8]       |
| Bell/Chili Pepper     | 14                     | ✅ Yes                         | [9]       |
| Radish                | 21                     | ✅ Yes                         | [10]      |

## 📚 References

1. [UC Davis – Apple Storage Guidelines](http://postharvest.ucdavis.edu/Commodity_Resources/Fact_Sheets/)  
2. [USDA Handbook 66 – Cabbage](https://ucanr.edu/datastoreFiles/234-1507.pdf)  
3. [FAO Agricultural Services Bulletin – Carrot](http://www.fao.org/3/y5431e/y5431e05.htm)  
4. [UC Davis Postharvest Guidelines – Grapes](http://postharvest.ucdavis.edu)  
5. [UF IFAS Extension – Citrus Storage](https://edis.ifas.ufl.edu/pdf/HS/HS132300.pdf)  
6. [FAO – Mango Maturity and Ripening](http://www.fao.org/3/y4358e/y4358e05.htm)  
7. [Postharvest Biology of Chinese Cabbage (2019)](https://doi.org/10.1007/s13197-019-03747-x)  
8. [UC Davis – Peach Postharvest Handling](http://postharvest.ucdavis.edu)  
9. [FAO – Storage of Fresh Peppers](http://www.fao.org/3/y4358e/y4358e07.htm)  
10. [UC Davis – Radish Storage Fact Sheet](http://postharvest.ucdavis.edu/Commodity_Resources/Fact_Sheets/)




[1]L. Panoff, “How Long Do Apples Last?,” Healthline, Feb. 05, 2020. https://www.healthline.com/nutrition/how-long-do-apples-last#shelf-life (accessed Apr. 16, 2025).
[2]“Storage Life of Vegetables,” SDSU Extension, Nov. 2021. https://extension.sdstate.edu/storage-life-vegetables#:~:text=Beets%20Cool%20and%20Humid%C2%A0%2032,1%20week (accessed Apr. 16, 2025).
[3]N. Sharma, “How to Store Carrots So They Last for Months,” Food52, Feb. 26, 2021. https://food52.com/blog/26003-how-to-store-carrots?srsltid=AfmBOoqirJHQLkB7ObgdbNIswDIwoUN3hbny6EpOOGWTor-JIzXSoQE1#:~:text=What%E2%80%99s%20the%20Best%20Way%20to,Store%20Carrots (accessed Apr. 16, 2025).
[4]https://www.facebook.com/allrecipes, “How to Store Grapes to Keep Them Fresh,” Allrecipes, 2021. https://www.allrecipes.com/article/how-to-store-grapes/#:~:text=It%20depends%20on%20how%20you,lose%20their%20crispness%20more%20quickly (accessed Apr. 16, 2025).
[5]A. Skaria, “How Long Do Lemons Last? A Guide for Long-Lasting Lemons & Juice,” US Citrus, Aug. 31, 2020. https://uscitrus.com/blogs/citrus-delight-blog/how-long-do-lemons-last-a-guide-for-long-lasting-lemons-juice?srsltid=AfmBOoqd1rX1BJA1IGgzbGWDatxAJEgwbe9LKhW_RmtWSV7_51fNY2lI#:~:text=The%20life%20of%20a%20lemon,last%20for%20a%20few%20days (accessed Apr. 16, 2025).
[6]https://www.facebook.com/marthastewart, “How to Store Mangoes the Right Way, According to Fruit Pros,” Martha Stewart, 2024. https://www.marthastewart.com/how-to-store-mangoes-8644687#:~:text=Our%20experts%20agree%20that%20you,counter%20is%20perfect%2C%20he%20says (accessed Apr. 16, 2025).
[7]“Bulletin #4135, Storage Conditions: Fruits and Vegetables - Cooperative Extension Publications - University of Maine Cooperative Extension,” Cooperative Extension Publications, May 23, 2024. https://extension.umaine.edu/publications/4135e/#:~:text=Cabbage%2C%20Chinese%2032%2095%E2%80%93100%202%E2%80%933,5 (accessed Apr. 16, 2025).
[8]https://www.facebook.com/allrecipes, “How to Store Peaches So They Last Longer (And Taste Their Best),” Allrecipes, 2023. https://www.allrecipes.com/how-to-store-fresh-peaches-7494995#:~:text=plastic%20bag%2C%20says%20Jenny%20Friedman%2C,the%20produce%20drawer%2C%E2%80%9D%20she%20explains (accessed Apr. 16, 2025).
[9]“Storage Life of Vegetables,” SDSU Extension, Nov. 2021. https://extension.sdstate.edu/storage-life-vegetables#:~:text=Peas%20Cool%20and%20Humid%C2%A0%2032,2%20weeks (accessed Apr. 16, 2025).
[10]“Storage Life of Vegetables,” SDSU Extension, Nov. 2021. https://extension.sdstate.edu/storage-life-vegetables#:~:text=Potatoes%20Humid%2045%E2%84%89%2095%25%202,2%20weeks (accessed Apr. 16, 2025).


