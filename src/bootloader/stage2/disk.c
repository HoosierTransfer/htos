#include "disk.h"
#include "x86.h"

bool disk_Initialize(DISK* disk, uint8_t diskNumber) {
    uint8_t driveNumber;
    uint16_t cylinders, sectors, heads;

    if(!x86_Disk_GetDriveParams(disk->id, &driveNumber, &cylinders, &sectors, &heads)) {
        return false;
    }
    disk->id = driveNumber;
    disk->cylinders = cylinders;
    disk->heads = heads;
    disk->sectors = sectors;

    return true;
}

void disk_lbaToChs(DISK* disk, uint32_t lba, uint16_t* cylinderOut, uint16_t* sectorOut, uint16_t* headOut) {
    *sectorOut = lba % disk->sectors + 1;

    *cylinderOut = (lba / disk->sectors) / disk->heads;

    *headOut = (lba / disk->sectors) % disk->heads;
}

bool disk_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, void far* dataOut) {
    uint16_t cylinder, sector, head;
    
    disk_lbaToChs(disk, lba, &cylinder, &sector, &head);

    for (int i = 0; i < 3; i++) {
        bool ok = x86_Disk_Read(disk->id, cylinder, sector, head, sectors, dataOut);
        if (ok)
            return true;

        ok = x86_Disk_Reset(disk->id);
    }

    return false;
}