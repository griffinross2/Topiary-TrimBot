#include <stdint.h>

__attribute__((used)) int Init(void);
__attribute__((used)) int Write(uint32_t Address, uint32_t Size, uint8_t* buffer);
__attribute__((used)) int SectorErase(uint32_t StartAddress, uint32_t EndAddress);
__attribute__((used)) int Read(uint32_t Address, uint32_t Size, uint16_t* buffer);
__attribute__((used)) uint64_t Verify(uint32_t FlashAddr, uint32_t RAMBufferAddr, uint32_t Size);
__attribute__((used)) int MassErase(void);
__attribute__((used)) uint32_t CheckSum(uint32_t StartAddress, uint32_t Size, uint32_t InitVal);