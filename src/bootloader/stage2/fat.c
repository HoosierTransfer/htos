#include "fat.h"
#include "stdio.h"
#include "memdefs.h"
#include "string.h"
#include "memory.h"
#include "ctype.h"
#include <stddef.h>
#include "minmax.h"

#define SECTOR_SIZE 512
#define MAX_PATH_SIZE 256
#define MAX_FILE_HANDLES 10
#define ROOT_DIRECTORY_HANDLE -1

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
    uint32_t VolumeId;       // serial number, value doesn't matter
    uint8_t VolumeLabel[11]; // 11 bytes, padded with spaces
    uint8_t SystemId[8];
} __attribute__((packed)) fat_BootSector;

typedef struct
{
    uint8_t Buffer[SECTOR_SIZE];
    fat_File Public;
    bool Opened;
    uint32_t FirstCluster;
    uint32_t CurrentCluster;
    uint32_t CurrentSectorInCluster;
} fat_FileData;

typedef struct
{
    union
    {
        fat_BootSector BootSector;
        uint8_t BootSectorBytes[SECTOR_SIZE];
    } BS;

    fat_FileData RootDirectory;
    fat_FileData OpenedFiles[MAX_FILE_HANDLES];
} fat_Data;

static fat_Data* g_Data;

static uint8_t* g_Fat = NULL;
static uint32_t g_DataSectionLba;

bool fat_readBootSector(DISK* disk)
{
    return disk_ReadSectors(disk, 0, 1, g_Data->BS.BootSectorBytes);
}

bool fat_ReadFat(DISK* disk)
{
    return disk_ReadSectors(disk, g_Data->BS.BootSector.ReservedSectors, g_Data->BS.BootSector.SectorsPerFat, g_Fat);
}

bool fat_Initialize(DISK* disk)
{
    g_Data = (fat_Data*)MEMORY_FAT_ADDR;

    if (!fat_readBootSector(disk))
    {
        printf("FAT: failed to read boot sector\r\n");
        return false;
    }

    g_Fat = (uint8_t*)g_Data + sizeof(fat_Data);
    uint32_t fatSize = g_Data->BS.BootSector.BytesPerSector * g_Data->BS.BootSector.SectorsPerFat;
    if (sizeof(fat_Data) + fatSize >= MEMORY_FAT_SIZE)
    {
        printf("FAT: not enough memory to read fat. You only have %lu but you need %lu\r\n", sizeof(fat_Data) + fatSize, MEMORY_FAT_SIZE);
        return false;
    }

    if (!fat_ReadFat(disk))
    {
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
    g_Data->RootDirectory.FirstCluster = rootDirLba;
    g_Data->RootDirectory.CurrentCluster = rootDirLba;
    g_Data->RootDirectory.CurrentSectorInCluster = 0;

    if (!disk_ReadSectors(disk, rootDirLba, 1, g_Data->RootDirectory.Buffer))
    {
        printf("FAT: failed to read root directory");
        return false;
    }

    uint32_t rootDirSectors = (rootDirSize + g_Data->BS.BootSector.BytesPerSector - 1) / g_Data->BS.BootSector.BytesPerSector;
    g_DataSectionLba = rootDirLba + rootDirSectors;

    for (int i = 0; i < MAX_FILE_HANDLES; i++)
        g_Data->OpenedFiles[i].Opened = false;

    return true;
}

uint32_t fat_ClusterToLba(uint32_t cluster)
{
    return g_DataSectionLba + (cluster - 2) * g_Data->BS.BootSector.SectorsPerCluster;
}

fat_File* fat_OpenEntry(DISK* disk, fat_DirectoryEntry* entry)
{
    int handle = -1;
    for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++)
    {
        if (!g_Data->OpenedFiles[i].Opened)
            handle = i;
    }

    if (handle < 0)
    {
        printf("FAT: out of handles\r\n");
        return false;
    }

    fat_FileData* fd = &g_Data->OpenedFiles[handle];
    fd->Public.Handle = handle;
    fd->Public.isDirectory = (entry->Attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
    fd->Public.Position = 0;
    fd->Public.Size = entry->Size;
    fd->FirstCluster = entry->FirstClusterLow + ((uint32_t)entry->FirstClusterHigh << 16);
    fd->CurrentCluster = fd->FirstCluster;
    fd->CurrentSectorInCluster = 0;

    if (!disk_ReadSectors(disk, fat_ClusterToLba(fd->CurrentCluster), 1, fd->Buffer))
    {
        printf("FAT: read error\r\n");
        return false;
    }

    fd->Opened = true;
    return &fd->Public;
}

uint32_t fat_NextCluster(uint32_t currentCluster)
{
    uint32_t fatIndex = currentCluster * 3 / 2;
    
    if (currentCluster % 2 == 0)
    {
        return (*(uint16_t*)(g_Fat + fatIndex)) & 0x0FFF;
    }
    else
    {
        return (*(uint16_t*)(g_Fat + fatIndex)) >> 4;
    }
}

uint32_t fat_Read(DISK* disk, fat_File* file, uint32_t byteCount, void* dataOut)
{
    fat_FileData* fd = (file->Handle == ROOT_DIRECTORY_HANDLE) ? &g_Data->RootDirectory : &g_Data->OpenedFiles[file->Handle];

    uint8_t* u8DataOut = (uint8_t*)dataOut;

    if (!fd->Public.isDirectory || (fd->Public.isDirectory && fd->Public.Size != 0))
        byteCount = min(byteCount, fd->Public.Size - fd->Public.Position);

    while (byteCount > 0)
    {
        uint32_t leftInBuffer = SECTOR_SIZE - (fd->Public.Position % SECTOR_SIZE);
        uint32_t take = min(byteCount, leftInBuffer);

        memcpy(u8DataOut, fd->Buffer + fd->Public.Position % SECTOR_SIZE, take);
        u8DataOut += take;
        fd->Public.Position += take;
        byteCount -= take;

        if (leftInBuffer == take)
        {
            if (fd->Public.Handle == ROOT_DIRECTORY_HANDLE)
            {
                ++fd->CurrentCluster;

                if (!disk_ReadSectors(disk, fd->CurrentCluster, 1, fd->Buffer))
                {
                    printf("FAT read oopsies!\r\n");
                    break;
                }
            }
            else
            {
                if (++fd->CurrentSectorInCluster >= g_Data->BS.BootSector.SectorsPerCluster)
                {
                    fd->CurrentSectorInCluster = 0;
                    fd->CurrentCluster = fat_NextCluster(fd->CurrentCluster);
                }

                if (fd->CurrentCluster >= 0xFF8)
                {
                    fd->Public.Size = fd->Public.Position;
                    break;
                }

                if (!disk_ReadSectors(disk, fat_ClusterToLba(fd->CurrentCluster) + fd->CurrentSectorInCluster, 1, fd->Buffer))
                {
                    printf("Fat read oopsies! =(\r\n");
                    break;
                }
            }
        }
    }

    return u8DataOut - (uint8_t*)dataOut;
}

bool fat_ReadEntry(DISK* disk, fat_File* file, fat_DirectoryEntry* dirEntry)
{
    return fat_Read(disk, file, sizeof(fat_DirectoryEntry), dirEntry) == sizeof(fat_DirectoryEntry);
}

void fat_Close(fat_File* file)
{
    if (file->Handle == ROOT_DIRECTORY_HANDLE)
    {
        file->Position = 0;
        g_Data->RootDirectory.CurrentCluster = g_Data->RootDirectory.FirstCluster;
    } 
    else 
    {
        g_Data->OpenedFiles[file->Handle].Opened = false;
    }
}

bool fat_findFile(DISK* disk, fat_File* file, const char* name, fat_DirectoryEntry* entryOut)
{
    char fatName[12];
    fat_DirectoryEntry entry;

    memset(fatName, ' ', sizeof(fatName));
    fatName[11] = '\0';
   
    const char* ext = strchr(name, '.');
    if (ext == NULL)
        ext = name + 11;

    for (int i = 0; i < 8 && name[i] && name + i < ext; i++)
        fatName[i] = toupper(name[i]);

    if (ext != name + 11)
    {
        for (int i = 0; i < 3 && ext[i + 1]; i++)
            fatName[i + 8] = toupper(ext[i + 1]);
    }

    while (fat_ReadEntry(disk, file, &entry))
    {
        if (memcmp(fatName, entry.Name, 11) == 0)
        {
            *entryOut = entry;
            return true;
        }
    }

    return false;
}

fat_File* fat_Open(DISK* disk, const char* path)
{
    char name[MAX_PATH_SIZE];

    if (path[0] == '/')
        path++;

    fat_File* current = &g_Data->RootDirectory.Public;

    while (*path) {
        bool isLast = false;
        const char* delim = strchr(path, '/');
        if (delim != NULL)
        {
            memcpy(name, path, delim - path);
            name[delim - path + 1] = '\0';
            path = delim + 1;
        }
        else
        {
            unsigned len = strlen(path);
            memcpy(name, path, len);
            name[len + 1] = '\0';
            path += len;
            isLast = true;
        }

        fat_DirectoryEntry entry;
        if (fat_findFile(disk, current, name, &entry))
        {
            fat_Close(current);

            if (!isLast && entry.Attributes & FAT_ATTRIBUTE_DIRECTORY == 0)
            {
                printf("FAT: %s not a directory\r\n", name);
                return NULL;
            }
            current = fat_OpenEntry(disk, &entry);
        }
        else
        {
            fat_Close(current);
            printf("FAT: %s not found D=\r\n", name);
            return NULL;
        }
    }

    return current;
}
