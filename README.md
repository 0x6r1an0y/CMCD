# chinese-magic-card-detector
Using arduino and PN532 create a UID(chinese magic card gen 1) and CUID(chinese magic card gen 2) card detector. 
## 軟體
### 前言
這邊使用[SigmaDolphin的Adafruit-PN532分支](https://github.com/SigmaDolphin/Adafruit-PN532)，原作者有提出PR但是一直未獲merge，其做了UID(gen 1)卡偵測的改動，新增了以下

public funtion : `UnlockBackdoor()`

private function : `WriteRegister(uint8_t *reg, uint8_t len)`、`InCommunicateThru(uint8_t *data, uint8_t len)`

我引用了該分支再多做了CUID(gen 2)卡偵測，mifareclassic_WriteDataBlock()新增了寫入0號區時，raw responds如果pn532_packetbuffer[7] == 0x01或pn532_packetbuffer[8] == 0xE9就判斷為寫入失敗，因此不是CUID卡。這數字是觀察raw responds來的，可能要再去翻一下datasheet才會知道詳細發生了什麼事。以及新增了reboot()這個public function

### 安裝程式庫

1. 這邊使用[SigmaDolphin的Adafruit-PN532分支](https://github.com/SigmaDolphin/Adafruit-PN532)，雖然版本舊了一點點，但在我的使用中不受影響
2. 找到程式庫資料夾，在arduino ide的libary資料夾下，在我的例子中是"C:\Users\user\OneDrive\文件\Arduino\libraries\Adafruit_PN532"
3. 取代Adafruit_PN532.h和Adafruit_PN532.cpp


## 硬體
