#https://twgo.io/xpuka
#測試影片：https://twgo.io/vlpwc
#tw live可搜尋台灣即時影像：https://trafficvideo2.tainan.gov.tw/b596d902

from ultralytics import YOLO
import cv2,time
#設定視窗名稱及型態
cv2.namedWindow('YOLOv8', cv2.WINDOW_NORMAL)

target='Counter.mp4' #'city.mp4'
model = YOLO('yolov8m.pt')  # n,s,m,l,x 五種大小

names=model.names
print(names)

cap=cv2.VideoCapture(target)


while 1:
    st=time.time()  
    r,frame = cap.read()
    if r==False:
        break
    results = model(frame,verbose=False) #verbose不要廢話

    person_total = 0

    for box in results[0].boxes.data:
        
        x1 = int(box[0]) #左
        y1 = int(box[1]) #上
        x2 = int(box[2]) #右
        y2 = int(box[3]) #下
        r = round(float(box[4]),2) #信任度
        name = names[int(box[5])]
        
        if (name == 'person'):
            cv2.rectangle(frame,(x1,y1),(x2,y2),(0,0,255),2)
            cv2.putText(frame,name,(x1,y1),2,cv2.FONT_HERSHEY_PLAIN,(0,255,0),2)
            person_total = int(person_total + 1)
            # print("Person: " , person_total)

                

    #顯示車輛數
    cv2.putText(frame,'Person=' + str(person_total), (20, 120),cv2.FONT_HERSHEY_PLAIN,5,(255,0,0),4)

    #            圖         文字內容        座標            字型             大小    顏色BGR   粗細   樣式(實心)

    # frame= results[0].plot()
    et=time.time()
   
    FPS=round((1/(et-st)),1) #評估時間

    cv2.putText(frame, 'FPS=' + str(FPS), (20, 50), cv2.FONT_HERSHEY_PLAIN, 5, (0, 255, 255), 4, cv2.LINE_AA)
    cv2.imshow('YOLOv8', frame)
    key=cv2.waitKey(1)
    if key==27:
        break



