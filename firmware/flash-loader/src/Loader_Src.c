#include "Loader_Src.h"

#include "flash.h"

int main()
{
    return 0;
}

__attribute__((used)) int Init(void) {
    if (flash_init() != STATUS_OK)
    {
        return 0;
    }

    return 1;
}

__attribute__((used)) int Write(uint32_t Address, uint32_t Size, uint8_t* buffer) {
    return 1;
}

__attribute__((used)) int SectorErase(uint32_t StartAddress, uint32_t EndAddress) {
    return 1;
}

__attribute__((used)) int Read(uint32_t Address, uint32_t Size, uint16_t* buffer) {
    return 1;
}

__attribute__((used)) uint64_t Verify(uint32_t FlashAddr, uint32_t RAMBufferAddr, uint32_t Size) {
    return 0;
}

__attribute__((used)) int MassErase(void) {
    return 1;
}

__attribute__((used)) uint32_t CheckSum(uint32_t StartAddress, uint32_t Size, uint32_t InitVal) {
    return 0;
}
