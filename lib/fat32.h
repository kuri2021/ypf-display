//////////////////////////////////////////////////////////////////////
#include "sys.h"
#include "sd.h"
#include "umath.h"

#define TRUE   		1
#define FALSE  		0
#define FAIL   		1
#define SUCCESS  		0

extern u8 fileSysFlag;

typedef struct _READ_FILE_STU
{
    u8 *filePath;//读取路径
    u32 fstartPos;//读取起始位置
    u32 flen;//读取长度
    u8 *dataBuf;//读取后的数据存放位置
    u32 fileSize;//文件长度,第一次读取才会赋值
    u32 realReadLen;//实际读取长度，读取长度超过范围，则按剩余长度读取
}READ_FILE_STU, *pREAD_FILE_STU;


u8 InitFileSystem(void);//初始化文件系统
u8 changeDirectory(const u8 *path);//改变当前路径
void getNowPathName(u8 *name);//获取当前路径的文本
u8 listDirectory(u8 *path, u8* name);//获取绝对路径或相对路径的一个文件夹或文件的名字。1表示文件夹，2表示文件
u8 mReadFile(pREAD_FILE_STU read);//为了减少传递参数,改用了结构体
void test(void);//测试用


//unsigned long FirstSectorofCluster32(unsigned long clusterNum);
// unsigned long ThisFatSecNum32(unsigned long clusterNum);
// unsigned long ThisFatEntOffset32(unsigned long clusterNum);
// unsigned long GetNextClusterNum32(unsigned long clusterNum);
//unsigned char GoToPointer32(unsigned long pointer);
//unsigned char DeleteClusterLink32(unsigned long clusterNum);
//unsigned long GetFreeCusterNum32(void);
//unsigned long CreateClusterLink32(unsigned long currentCluster);

//unsigned char UartHandler32(void);
//unsigned char List32(void);
//unsigned char OpenFile32(unsigned char *pBuffer);

// unsigned char ReadFile32(unsigned long readLength,unsigned char *pBuffer);
// unsigned char SetFilePointer32(unsigned long pointer);
// unsigned char CreateFile32(unsigned long len,unsigned char *pBuffer,unsigned char *pName);
// unsigned char WriteFile32(unsigned long writeLength,unsigned char *pBuffer);
// unsigned char RemoveFile32(unsigned char *pBuffer);
// unsigned char GetCapacity32(void);


// unsigned char CreateDir32(unsigned long len,unsigned char *pBuffer,unsigned char *pName);
// unsigned char DownDir32(unsigned char *pBuffer);
// unsigned char UpDir32(void);
// unsigned char UpRootDir32(void);
