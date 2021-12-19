#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32

class SkCamera
{
private:
    void InitSdCard();

    /* data */
    bool m_bIsReady = false;
    bool m_bIsSdCardMounted = false;
    fs::FS &m_fs = SD_MMC; 

    const uint16_t MAX_PICT_NUM = 256; 
    uint16_t m_pictNum = 0; 
public:
    void init(uint16_t  server_port);
    void addToRingBuffer();
};
