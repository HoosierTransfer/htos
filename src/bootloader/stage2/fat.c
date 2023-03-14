#include "fat.h"
#include "stdio.h"
#include "memdefs.h"

#define SECTOR_SIZE 512
#define MAX_PATH_SIZE 256
#define MAX_FILE_HANDLES 10
#define ROOT_DIRECTORY_HANDLE -1

#pragma pack(push, 1)

typedef struct 
{
    uint8_t BootJumpInstruction[3];
    uint8_t OemIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FatCount;
    uint16_t DirEntryCount;
    uint16_t TotalSectors;
    uint8_t MediaDescriptorType;
    uint16_t SectorsPerFat;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;

    // extended boot record
    uint8_t DriveNumber;
    uint8_t _Reserved;
    uint8_t Signature;
    uint32_t VolumeId;          // serial number, value doesn't matter
    uint8_t VolumeLabel[11];    // 11 bytes, padded with spaces
    uint8_t SystemId[8];
} fat_BootSector;

#pragma pack(pop)

typedef struct {
    fat_File Public;
    bool Opened;
    uint32_t FirstCluster;
    uint32_t CurrentCluster;
    uint32_t CurrentSectorInCluster;
    uint8_t Buffer[SECTOR_SIZE];
} fat_FileData;

typedef struct {
    union {
        fat_BootSector BootSector;
        uint8_t BootSectorBytes[SECTOR_SIZE];
    } BS;

    fat_FileData RootDirectory;
    fat_FileData OpenedFiles[MAX_FILE_HANDLES];
} fat_Data;

static fat_Data far* g_Data;

static uint8_t far* g_Fat = NULL;
static uint32_t g_DataSectionLba;

bool fat_readBootSector(DISK* disk) {
    return disk_ReadSectors(disk, 0, 1, g_Data->BS.BootSectorBytes);
}

bool fat_ReadFat(DISK* disk) {
    return disk_ReadSectors(disk, g_Data->BS.BootSector.ReservedSectors, g_Data->BS.BootSector.SectorsPerFat, g_Fat);
}

bool fat_Initialize(DISK* disk) {
    g_Data = (fat_Data far*)MEMORY_FAT_ADDR;

    if (!(fat_readBootSector(disk))) {
        printf("FAT: failed to read boot sector\r\n");
        return false;
    }

    g_Fat = (uint8_t far*)g_Data + sizeof(fat_Data);

    uint32_t fatSize = g_Data->BS.BootSector.BytesPerSector * g_Data->BS.BootSector.SectorsPerFat;

    if (sizeof(fat_Data) + fatSize >= MEMORY_FAT_SIZE) {
        printf("FAT: not enough memory to read fat. You only have %lu but you need %lu\r\n", sizeof(fat_Data) + fatSize, MEMORY_FAT_SIZE);
        return false;
    }

    if (!readFat(disk)) {
        printf("FAT: read fat failed\r\n");
        return false;
    }
    uint32_t rootDirLba = g_Data->BS.BootSector.ReservedSectors + g_Data->BS.BootSector.SectorsPerFat * g_Data->BS.BootSector.FatCount;
    uint32_t rootDirSize = sizeof(fat_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
    
    g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
    g_Data->RootDirectory.Public.isDirectory = true;
    g_Data->RootDirectory.Public.Position = 0;
    g_Data->RootDirectory.Public.Size = sizeof(fat_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
    g_Data->RootDirectory.Opened = true;
    g_Data->RootDirectory.FirstCluster = 
    g_Data->RootDirectory.CurrentCluster = 0;
    g_Data->RootDirectory.CurrentSectorInCluster = 0;

    if (!disk_ReadSectors(disk, rootDirLba, 1, g_Data->RootDirectory.Buffer)) {
        printf("FAT: failed to read root directory");
        return false;
    }

    uint32_t rootDirSectors = (rootDirSize + g_Data->BS.BootSector.BytesPerSector - 1) / g_Data->BS.BootSector.BytesPerSector;
    g_DataSectionLba = rootDirLba + rootDirSectors;

    for (int i = 0; i < MAX_FILE_HANDLES; i++)
        g_Data->OpenedFiles[i].Opened = false;
}

fat_File* fat_Open(DISK* disk, fat_DirectoryEntry* entry) {
    int handle = -1;
    for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++) {
        if(!g_Data->OpenedFiles[i].Opened)
            handle = i;
    }

    if (handle < 0) {
        printf("FAT: out of handles\r\n");
        return false;
    }

    fat_FileData far* fd = &g_Data->OpenedFiles[handle];
}

fat_File* fat_Open(DISK* disk, const char* path) {
    char buffer[MAX_PATH_SIZE];

    if (path[0] == '/')
        path++;
}

DirectoryEntry* findFile(const char* name) {
    for(uint32_t i = 0; i < g_BootSector.DirEntryCount; i++) {
        if(memcmp(name, g_rootDirectory[i].Name, 11) == 0) {
            return &g_rootDirectory[i];
        }
    }

    return NULL;
}

bool readFile(DirectoryEntry* fileEntry, FILE* disk, uint8_t* outputBuffer)
{
    bool ok = true;
    uint16_t currentCluster = fileEntry->FirstClusterLow;
    do {
        uint32_t lba = g_rootDirectoryEnd + (currentCluster - 2) * g_BootSector.SectorsPerCluster;
        ok = ok && readSectors(disk, lba, g_BootSector.SectorsPerCluster, outputBuffer);
        outputBuffer += g_BootSector.SectorsPerCluster * g_BootSector.BytesPerSector;

        uint32_t fatIndex = currentCluster * 3 / 2;
        if(currentCluster % 2 == 0) {
            currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) & 0x0FFF;
        } else {
            currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) >> 4;
        }
    } while(ok && currentCluster < 0x0FF8);

    return ok;
}

int main(int argc, char** argv) {
    if(argc < 3) {
        printf("Syntax: %s <disk image> <file name>\n", argv[0]);
        return -1;
    }

    FILE* disk = fopen(argv[1], "rb");
    if(!disk) {
        fprintf(stderr, "Cannot open disk image %s\n", argv[1]);
        return -1;
    }

    if(!readBootSector(disk)) {
        fprintf(stderr, "Cannot read boot sector\n");
        return -2;
    }

    if(!readFat(disk)) {
        fprintf(stderr, "Cannot read FAT\n");
        free(g_Fat);
        return -3;
    }

    if(!readRootDirectory(disk)) {
        fprintf(stderr, "Cannot read FAT\n");
        free(g_Fat);
        free(g_rootDirectory);
        return -4;
    }

    DirectoryEntry* fileEntry = findFile(argv[2]);
    if(!fileEntry) {
        fprintf(stderr, "Cannot find file %s\n", argv[2]);
        free(g_Fat);
        free(g_rootDirectory);
        return -5;
    }

    uint8_t* buffer = (uint8_t*) malloc(fileEntry->Size + g_BootSector.BytesPerSector);
    if(!readFile(fileEntry, disk, buffer)) {
        fprintf(stderr, "Cannot read file %s\n", argv[2]);
        free(g_Fat);
        free(g_rootDirectory);
        free(buffer);
        return -5;
    }

    for(size_t i = 0; i < fileEntry->Size; i++) {
        if(isprint(buffer[i])) fputc(buffer[i], stdout);
        else printf("<%02x>", buffer[i]);
    }
    printf("\n");

    free(g_Fat);
    free(g_rootDirectory);
    free(buffer);
    return 0;
}