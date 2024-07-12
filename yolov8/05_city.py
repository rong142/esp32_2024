
#https://twgo.io/xpuka
#測試影片：https://twgo.io/vlpwc
#tw live可搜尋台灣即時影像：https://trafficvideo2.tainan.gov.tw/b596d902

from ultralytics import YOLO
import cv2,time
#設定視窗名稱及型態
cv2.namedWindow('YOLOv8', cv2.WINDOW_NORMAL) #視窗可縮放

# target='Counter.mp4' #數字:webcam編號，字串:檔案或網址
target = 'https://cctvs.freeway.gov.tw/live-view/mjpg/video.cgi?camera=107'
model = YOLO('yolov8m.pt')  # pretrained YOLOv8m model  n,s,m,l,x五種尺寸大小
#認識的物件名（字典dict：編號可以查詢名稱）
names=model.names

cap=cv2.VideoCapture(target)

while 1:
    try:
        st=time.time()  
        r,frame = cap.read() #r=擷取影像成功了嗎？ frame=影像內容
        results = model(frame) # 用yolo model分析frame，得到結果存在results
        frame= results[0].plot() #把發現的結果繪圖出來覆蓋frame，results[0]=第1個結果=第1張照片
        #計算車輛數 results[0].boxes.data
        carCount=0
        for obj in results[0].boxes.data:
            x1,y1,x2,y2=int(obj[0]),int(obj[1]),int(obj[2]),int(obj[3])
            r=round(float(obj[4]),2)
            n=names[int(obj[5])]
            if n=='car':
                carCount=carCount+1
            print(x1,y1,x2,y2,r,n)
        print('carCount=',carCount)        
       
        et=time.time()
       
        FPS=round((1/(et-st)),1)
        #            圖         文字內容        座標            字型             大小    顏色BGR   粗細   樣式(實心)
        cv2.putText(frame, 'FPS=' + str(FPS), (20, 150), cv2.FONT_HERSHEY_PLAIN, 3, (0, 255, 255), 2, cv2.LINE_AA) #putText在影像寫字
        cv2.putText(frame, 'Car=' + str(carCount), (20, 50), cv2.FONT_HERSHEY_PLAIN, 3, (0, 0, 255), 2, cv2.LINE_AA)
        cv2.imshow('YOLOv8', frame)
        key=cv2.waitKey(1)
        if key==27: #27=esc
            break
    except Exception as e :
        print(e)
        break



