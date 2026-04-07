#include <stdint.h>

__attribute__((used)) int Init(void);
__attribute__((used)) int Write(uint32_t Address, uint32_t Size, uint8_t* buffer);
__attribute__((used)) int SectorErase(uint32_t StartAddress, uint32_t EndAddress);
__attribute__((used)) uint64_t Verify(uint32_t FlashAddr, uint32_t RAMBufferAddr, uint32_t Size, uint32_t misalignement);
__attribute__((used)) int MassErase(uint32_t Parallelism);
__attribute__((used)) uint32_t CheckSum(uint32_t StartAddress, uint32_t Size, uint32_t InitVal);