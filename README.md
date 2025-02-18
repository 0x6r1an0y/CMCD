# chinese-magic-card-detector
Using arduino and PN532 create a UID(chinese magic card gen 1) and CUID(chinese magic card gen 2) card detector. 
## 軟體

### 安裝程式庫

1. 這邊使用[SigmaDolphin的Adafruit-PN532分支](https://github.com/SigmaDolphin/Adafruit-PN532)，雖然版本較舊，但在我的使用中不受影響
2. 在arduino ide的libary資料夾下，在我的例子中是"C:\Users\user\OneDrive\文件\Arduino\libraries\Adafruit_PN532"
3. 取代Adafruit_PN532.h和Adafruit_PN532.cpp

SigmaDolphin的分支下主要是做了UID卡偵測的改動，新增了以下function
Adafruit_PN532::UnlockBackdoor()

我引用了該分支再做了CUID卡偵測的改動，mifareclassic_WriteDataBlock()新增了寫入0號區時，raw responds如果pn532_packetbuffer[7] == 0x01或pn532_packetbuffer[8] == 0xE9就判斷為寫入失敗，因此不是CUID卡。這數字是觀察raw responds來的，可能要再去翻一下datasheet才會知道詳細發生了什麼事



## 硬體
