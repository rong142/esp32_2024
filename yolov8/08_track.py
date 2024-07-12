#https://twgo.io/dgnab
import cv2, numpy as np
from ultralytics import YOLO
#路徑字典：物件編號+移動路徑座標
from collections import defaultdict
track_history = defaultdict(lambda: [])

model = YOLO("yolov8m.pt")
names = model.names

cv2.namedWindow('YOLOv8', cv2.WINDOW_NORMAL)
target = "Counter.mp4"
cap = cv2.VideoCapture(target)

while cap.isOpened():
    r, frame = cap.read()
    #成功讀取影像
    if r==False:
        break
        #       啟動辨識+追蹤      持續          不顯示結果
    results = model.track(frame, persist=True, verbose=False)

    #至少有一個物件
    if results[0].boxes.id is not None:      

        for box in results[0].boxes.data:
            x1 = int(box[0]) #左
            y1 = int(box[1]) #上
            x2 = int(box[2]) #右
            y2 = int(box[3]) #下
            trackid = int(box[4]) #每個物件的唯一追蹤編號
            r = round(float(box[5]),2) #信任度
            n = names[int(box[6])] #名字
           
            #只追蹤人(ClassID=0)
            if n in ['person']:

                #劃出box
                cv2.rectangle(frame,(int(box[0]), int(box[1])), (int(box[2]),int(box[3])), (0, 255, 0),5)

                #找出物件的移動軌跡座標
                track = track_history[trackid]

                #加一個點，物件的中心點為軌跡座標
                track.append((int((x1 + x2) / 2), int((y1 + y2) / 2)))

                #只追蹤30個點，約一秒，以fps=30
                if len(track) > 100:
                    track.pop(0) #刪除最早的那個軌跡

                # 軌跡List轉換成陣列                      重分配陣列(-1：由系統指定)
                points = np.array(track, dtype=np.int32).reshape((-1, 1, 2))
                #現在位置     倒數第一個軌跡座標,半徑,顏色,   填滿(正:外框寬度,負:填滿)
                cv2.circle(frame, (track[-1]), 7, (0,0,255), -1)
                #移動路徑                        不封閉:頭尾相接     顏色            寬度
                cv2.polylines(frame, [points], isClosed=False, color=(0,0,255), thickness=2)

    cv2.imshow('YOLOv8',frame)

    if cv2.waitKey(1) == 27: #ESC退出
        break


