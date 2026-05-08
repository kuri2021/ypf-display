#include "fat32.h"
#include "debug.h"
#include "uart.h"

/*
只适用于USB2.0 并且只有读文件功能(写功能有版权，禁止自行添加) 扇区大小固定为512Bytes

读文件的整个过程
方式1（主要用于未知的文件夹访问）：
1、进入到对应目录，调用changeDirectory
2、调用搜索命令，搜索当前文件夹的文件listDirectory，传入的是绝对路径或者相对路径。
3、调用读取命令，mReadFile，读取对应数据，传入的是相对路径，比如："./32.icl（注：32.icl是listDirectory扫描出来的名称）"

方式2（主要用于已知的文件访问）：
1、直接mReadFile 传入绝对路径比如："/DWIN_SET/32.icl"，

注意：分隔符为/，只有调用changeDirectory后才会改变系统路径
*/

#define SECTOR_SIZE 512
#define BUFFER_LENGTH	512//8192//520
#define MAX_PATH_LEN 512
#define MAX_FILE_NAME_LEN 128

typedef struct _SYS_INFO_BLOCK{
  unsigned long StartSector;//定位DBR扇区的位置
  unsigned long TotalSector;
  
  INT16 BPB_BytesPerSec;//每个扇区的字节数，512
  INT8 BPB_SecPerClus;//每簇扇区数，8
  
  INT8 BPB_NumFATs;//FAT表的数目，2
  INT16 BPB_RootEntCnt;
  unsigned long BPB_TotSec16;
 // INT8 BPB_Media;
  INT16 BPB_FATSz16;
  INT16 BPB_SecPerTrk;
  INT16 BPB_NumHeads;
  unsigned long BPB_HiddSec;
  unsigned long BPB_TotSec32;//文件系统大小扇区数
  //INT8 BS_DrvNum;
 // INT8 BS_BootSig;
  //INT8 BS_VolID[4];
  //INT8 BS_VolLab[11];
  //INT8 BS_FilSysType[8];
   ///////////////////////////////
  unsigned long FatStartSector;//第一个FAT表所在扇区
  unsigned long RootStartSector;
  unsigned long FirstDataSector;//第一个目录所在扇区
/////////////////////FAT32///////////////////////////////
  INT32 BPB_FATSz32;//每个FAT表的大小扇区数
  unsigned long RootStartCluster;//根目录簇号，2
  unsigned long TotCluster;
  unsigned long LastFreeCluster;
  unsigned char FAT;	
} SYS_INFO_BLOCK;


typedef struct _MPATH
{
	u8 name[MAX_PATH_LEN];//当前文件夹的绝对路径。/DWIN_SET
	u32 startFatNum;//文件夹的起始簇
	u32 nowFatNum;//正在读取当前文件夹的哪个簇
	u8 nowSectorNum;//簇的哪个块
	u16 nowSectorPtr;//块的哪个地址
	u8 sectorData[SECTOR_SIZE];//当前块的内容
}MPATH,*pMPATH;

typedef struct _MFILE
{
	u8 name[MAX_FILE_NAME_LEN];
	u32 startFatNum;//当前文件夹的绝对路径。/DWIN_SET
	u32 nowFatNum;//正在读取当前文件夹的哪个簇
	u8 nowSectorNum;//簇的哪个块
	u32 size;//文件的大小
	u8 sectorData[SECTOR_SIZE];//当前块的内容
}MFILE,*pMFILE;

u8 fileSysFlag;
SYS_INFO_BLOCK xdata DeviceInfo;
MPATH sysPath,listPath,readPath;
MFILE sysFile;

unsigned long ThisFatSecNum32(unsigned long clusterNum)
{
   unsigned long temp;
   temp=clusterNum/(DeviceInfo.BPB_BytesPerSec/4);
   temp=temp+DeviceInfo.FatStartSector;
   return temp;
}

unsigned long ThisFatEntOffset32(unsigned long clusterNum)
{
	return (clusterNum%(DeviceInfo.BPB_BytesPerSec/4))*4;
}

static u32 fatSecNumBackup;
unsigned char xdata FATBUF[SECTOR_SIZE];
unsigned long GetNextClusterNum32(unsigned long clusterNum)
{
	unsigned long FatSecNum,FatEntOffset;
	
	FatSecNum=ThisFatSecNum32(clusterNum);
	FatEntOffset=ThisFatEntOffset32(clusterNum);	

	if(fatSecNumBackup != FatSecNum)
	{
		if(SdReadSector(FatSecNum,FATBUF))
		{
			return 0x0FFFFFFF;
		}
		else
		{
			fatSecNumBackup = FatSecNum;//FATBUF中为fatSecNumBackup扇区的缓存。如果访问其他扇区则重新读取。这样减少读取次数
		}	
	}
	clusterNum=LSwapINT32(FATBUF[FatEntOffset],FATBUF[FatEntOffset+1],FATBUF[FatEntOffset+2],FATBUF[FatEntOffset+3]);
	return clusterNum;
}

u32 getActualSectorNum(u32 clusterNum, u32 fatNum)
{
	u32 fileSector;

	fileSector = DeviceInfo.FirstDataSector + (clusterNum - 2)*(DeviceInfo.BPB_SecPerClus) + fatNum;
	return fileSector;
}

//初始化文件系统
u8 InitFileSystem(void)
{
	u16 tmp16 = 0;
	u32 tmp32 = 0;
	unsigned int ReservedSectorsNum;//保留扇区数，6350
	unsigned char xdata DBUF[BUFFER_LENGTH];

	fileSysFlag = 0;
	if(RESET_OK == SD_Reset())
	{
		// DEBUGINFO("SD卡复位成功\n");
		if(INIT_OK == SD_Init())
		{
			// DEBUGINFO("SD卡初始化成功\n");
		}
		else
		{
			// DEBUGINFO("SD卡初始化失败\n");
			return FALSE;
		}
	}
	else
	{
		// DEBUGINFO("SD卡复位失败\n");
		return FALSE;
	}

	DeviceInfo.BPB_BytesPerSec=SECTOR_SIZE; //暂假设为512
	
	if(SdReadSector(0,DBUF))
	{	
		return FALSE;
	}
  	if(DBUF[510] != 0x55 || DBUF[511] != 0xaa) return FALSE;
	if(DBUF[0]==0xeb||DBUF[0]==0xe9)
	{	
		DeviceInfo.StartSector=0;
	}
	else
	{
		if(DBUF[446] != 0x80 && DBUF[446] != 0)  return FALSE;
			DeviceInfo.StartSector=LSwapINT32(DBUF[454],DBUF[455],DBUF[456],DBUF[457]);
	}

	if(SdReadSector(DeviceInfo.StartSector,DBUF))
		return FALSE;
	
	if(DBUF[510] != 0x55 || DBUF[511] != 0xaa) return FALSE;

	DeviceInfo.BPB_BytesPerSec=LSwapINT16(DBUF[11],DBUF[12]);

	DeviceInfo.BPB_SecPerClus=DBUF[13];
	ReservedSectorsNum=LSwapINT16(DBUF[14],DBUF[15]);
	DeviceInfo.BPB_NumFATs=DBUF[16];

	DeviceInfo.BPB_RootEntCnt=LSwapINT16(DBUF[17],DBUF[18]);//此项通常为0
	DeviceInfo.BPB_RootEntCnt=(DeviceInfo.BPB_RootEntCnt)*32/DeviceInfo.BPB_BytesPerSec;
	DeviceInfo.BPB_TotSec32=LSwapINT32(DBUF[32],DBUF[33],DBUF[34],DBUF[35]);//文件系统大小扇区数
	DeviceInfo.BPB_FATSz32=LSwapINT32(DBUF[36],DBUF[37],DBUF[38],DBUF[39]);//每个FAT表的大小扇区数
	DeviceInfo.RootStartCluster=LSwapINT32(DBUF[44],DBUF[45],DBUF[46],DBUF[47]);//根目录簇号，2
	DeviceInfo.FatStartSector=DeviceInfo.StartSector+ReservedSectorsNum;//第一个FAT表所在扇区
	DeviceInfo.FirstDataSector=DeviceInfo.FatStartSector+DeviceInfo.BPB_NumFATs*DeviceInfo.BPB_FATSz32;//第一个目录所在扇区
	//DeviceInfo.TotCluster=(DeviceInfo.BPB_TotSec32-DeviceInfo.FirstDataSector+1)/DeviceInfo.BPB_SecPerClus+1;
	DeviceInfo.TotCluster=(DeviceInfo.BPB_TotSec32-ReservedSectorsNum-DeviceInfo.BPB_NumFATs*DeviceInfo.BPB_FATSz32-DeviceInfo.BPB_RootEntCnt)/DeviceInfo.BPB_SecPerClus;
//	DirStartCluster32=DeviceInfo.RootStartCluster;
	// if(SdReadSector((DeviceInfo.FirstDataSector),DBUF))
	// 	return FALSE;
	DeviceInfo.FAT=1;	//FAT16=0,FAT32=1;			

	sysPath.name[0] = '/';
	sysPath.name[1] = 0;//初始化后，当前文件夹路径为根目录
	sysPath.startFatNum = DeviceInfo.RootStartCluster;//根目录的起始簇
	sysPath.nowFatNum = sysPath.startFatNum;//根目录的起始簇
	sysPath.nowSectorPtr = 0;//根目录的起始簇的第0个扇区
	sysPath.nowSectorNum = 0;//根目录的起始簇的第0个扇区的第0个字节
	if(SdReadSector((DeviceInfo.FirstDataSector),sysPath.sectorData))
		return FALSE;
	fileSysFlag = 1;
	StrCopy((u8*)&listPath,(u8*)&sysPath,sizeof(MPATH));//备份listDirectory函数所需的参数
	StrCopy((u8*)&readPath,(u8*)&sysPath,sizeof(MPATH));//备份mReadFile函数所需的参数
	sysFile.name[0] = '\0';
	return TRUE;
}

static u32 listStartFatNum,listSize;//文件内部调用，当是用mReadFile函数是，匹配成功后则使用该数据
//0表示失败，1表示搜索到文件夹，2表示搜索到文件
u8 getOneName(pMPATH dir, u8 *name)
{
	u8 nameTmp[1024];//长文件名最多31*13个字
	u16 i,j;
	u8 flag,tmp;
	u8 longNameFlag,longNameNum,longNameNumBackup;
	u16 tmpUnicode;

	flag = 0;
	longNameFlag = 0;
	if(dir->nowSectorPtr%32)//因为32个字节为一组，如果不是32对齐，则数据有问题,从头开始搜索
	{
		dir->nowSectorPtr = 0;
	}
	if(dir->nowFatNum == 0x0fffffff)//表示应搜索到了底部
		return 0;
	while (1)
	{
		if(dir->sectorData[dir->nowSectorPtr] == 0)//0表示搜索到了文件夹的尾部，也就没找到下一个文件或文件夹
			return 0;
		if(dir->sectorData[dir->nowSectorPtr]!=0xe5 )//0xe5表示文件被删除了
		{
			if(longNameFlag)//表示正在处理长文件
			{
				if(dir->sectorData[dir->nowSectorPtr+0x0b] == 0x0f)
				{
					if(dir->sectorData[dir->nowSectorPtr] == (longNameNum-1))
					{
						longNameNum--;
						if(longNameNum)
						{
							StrCopy(&nameTmp[(longNameNum-1)*26],&dir->sectorData[dir->nowSectorPtr+1],10);
							StrCopy(&nameTmp[(longNameNum-1)*26+10],&dir->sectorData[dir->nowSectorPtr+0x0e],12);
							StrCopy(&nameTmp[(longNameNum-1)*26+22],&dir->sectorData[dir->nowSectorPtr+0x1c],4);
						}
						else
							longNameFlag = 0;
					}
					else//访问出错
					{
						longNameFlag = 0;
					}
				}
				else
				{
                    longNameFlag = 0;
					if(1==longNameNum)//搜索到了本命
					{
						if((dir->sectorData[dir->nowSectorPtr+0x0b]&0x26) == 0x20)//表示为正常文件，系统文件不处理，隐藏文件不处理
						{
							flag = 3;
						}
						else if((dir->sectorData[dir->nowSectorPtr+0x0b]&0x16) == 0x10)//表示为正常文件夹，系统文件夹不处理，隐藏文件夹不处理
						{
							flag = 4;
						}
						if(flag)
						{
							listStartFatNum = LSwapINT32(dir->sectorData[26+dir->nowSectorPtr],dir->sectorData[27+dir->nowSectorPtr], \
											dir->sectorData[20+dir->nowSectorPtr],dir->sectorData[21+dir->nowSectorPtr]);
							listSize = LSwapINT32(dir->sectorData[28+dir->nowSectorPtr],dir->sectorData[29+dir->nowSectorPtr], \
											dir->sectorData[30+dir->nowSectorPtr],dir->sectorData[31+dir->nowSectorPtr]);
							j = 0;
							for(i=0;i<longNameNumBackup*13;i++)
							{
								// tmpUnicode = *(u16*)&nameTmp[i*2];
								tmpUnicode = nameTmp[i*2+1];
								tmpUnicode <<= 8;
								tmpUnicode |= nameTmp[i*2];
								if(tmpUnicode == 0 || tmpUnicode == 0xffff)
								{
									break;
								}
								else if(tmpUnicode < 0x0080)
								{
									name[j] = tmpUnicode;
									j++;
								}
								else
								{
									*(u16*)&name[j] = ffConvert(tmpUnicode,0);
									// *(u16*)&name[j] = tmpUnicode;
									j += 2;
								}
							}
							name[j] = '\0';
						}
					}
//					else
//					{
//						longNameFlag = 0;
//					}
				}
			}
			else
			{
				if((dir->sectorData[dir->nowSectorPtr+0x0b]&0x26) == 0x20)//表示为正常文件，系统文件不处理，隐藏文件不处理
				{
					for(i=0;i<11;i++)
					{
						nameTmp[i] = dir->sectorData[dir->nowSectorPtr+i];
					}
					nameTmp[11] = '\0';
					listStartFatNum = LSwapINT32(dir->sectorData[26+dir->nowSectorPtr],dir->sectorData[27+dir->nowSectorPtr], \
											dir->sectorData[20+dir->nowSectorPtr],dir->sectorData[21+dir->nowSectorPtr]);
					listSize = LSwapINT32(dir->sectorData[28+dir->nowSectorPtr],dir->sectorData[29+dir->nowSectorPtr], \
											dir->sectorData[30+dir->nowSectorPtr],dir->sectorData[31+dir->nowSectorPtr]);
					tmp = dir->sectorData[dir->nowSectorPtr+0x0c];//转换字符大小写
					if(tmp&0x10)//扩展名的大小写,1代表小写
					{
						for ( i = 8; i < 11; i++)
						{
							nameTmp[i] = charUpperOrLower(nameTmp[i],1);
						}
					}
					else
					{
						for ( i = 8; i < 11; i++)
						{
							nameTmp[i] = charUpperOrLower(nameTmp[i],0);
						}
					}
					if(tmp&0x08)//文件名的大小写
					{
						for ( i = 0; i < 8; i++)
						{
							nameTmp[i] = charUpperOrLower(nameTmp[i],1);
						}
					}
					else
					{
						for ( i = 0; i < 8; i++)
						{
							nameTmp[i] = charUpperOrLower(nameTmp[i],0);
						}
					}
					j=0;
					for ( i = 0; i < 8; i++)
					{
						if(nameTmp[i] != 0x20)
						{
							name[j] = nameTmp[i];
							j++;
						}
					}
					name[j] = '.';
					j++;
					for ( ; i < 11; i++)
					{
						if(nameTmp[i] != 0x20)
						{
							name[j] = nameTmp[i];
							j++;
						}
					}
					name[j] = '\0';
					flag = 1;//表示搜索到了短名文件
				}
				else if((dir->sectorData[dir->nowSectorPtr+0x0b]&0x16) == 0x10)//表示为正常文件夹，系统文件夹不处理，隐藏文件夹不处理
				{
					if(dir->sectorData[dir->nowSectorPtr] != 0x2e)//0x2e表示为系统文件夹，不需要访问。
					{
						for(i=0;i<8;i++)
						{
							nameTmp[i] = dir->sectorData[dir->nowSectorPtr+i];
						}
						nameTmp[8] = '\0';
						listStartFatNum = LSwapINT32(dir->sectorData[26+dir->nowSectorPtr],dir->sectorData[27+dir->nowSectorPtr], \
											dir->sectorData[20+dir->nowSectorPtr],dir->sectorData[21+dir->nowSectorPtr]);
						listSize = LSwapINT32(dir->sectorData[28+dir->nowSectorPtr],dir->sectorData[29+dir->nowSectorPtr], \
											dir->sectorData[30+dir->nowSectorPtr],dir->sectorData[31+dir->nowSectorPtr]);
						tmp = dir->sectorData[dir->nowSectorPtr+0x0c];//转换字符大小写
						if(tmp&0x08)//文件名的大小写
						{
							for ( i = 0; i < 8; i++)
							{
								nameTmp[i] = charUpperOrLower(nameTmp[i],1);
							}
						}
						else
						{
							for ( i = 0; i < 8; i++)
							{
								nameTmp[i] = charUpperOrLower(nameTmp[i],0);
							}
						}
						j=0;
						for ( i = 0; i < 8; i++)
						{
							if(nameTmp[i] != 0x20)
							{
								name[j] = nameTmp[i];
								j++;
							}
						}
						name[j] = '\0';
						flag = 2;//表示搜索到了短名文件夹
					}
				}
				else if((dir->sectorData[dir->nowSectorPtr+0x0b] == 0x0f))//表示为长文件名或长文件夹名
				{
					if((dir->sectorData[dir->nowSectorPtr]&0x40) == 0x40)//表示长文件的开始
					{
						longNameNum = dir->sectorData[dir->nowSectorPtr] & 0x1F;
						if(longNameNum)//表示有效的长文件名
						{
							longNameNumBackup = longNameNum;
							longNameFlag = 1;
							StrCopy(&nameTmp[(longNameNum-1)*26],&dir->sectorData[dir->nowSectorPtr+1],10);
							StrCopy(&nameTmp[(longNameNum-1)*26+10],&dir->sectorData[dir->nowSectorPtr+0x0e],12);
							StrCopy(&nameTmp[(longNameNum-1)*26+22],&dir->sectorData[dir->nowSectorPtr+0x1c],4);
						}
					}
				}
			}
		}

		dir->nowSectorPtr += 32;
		if(dir->nowSectorPtr >= 512)//DeviceInfo.BPB_BytesPerSec
		{
			dir->nowSectorPtr = 0;
			dir->nowSectorNum++;
			if(dir->nowSectorNum >= DeviceInfo.BPB_SecPerClus)//当前簇搜索完了，需要找下一个簇
			{
				dir->nowSectorNum = 0;
				dir->nowFatNum = GetNextClusterNum32(dir->nowFatNum);
				if(dir->nowFatNum == 0x0fffffff)//表示没有后续簇了
				{
					if(0==flag)
						return 0;
				}	
			}
			if(dir->nowFatNum != 0x0fffffff)
			{
				if(SdReadSector((getActualSectorNum(dir->nowFatNum,dir->nowSectorNum)),dir->sectorData))//加载当前路径的起始扇区数据
					return 0;
			}
		}
		if (flag)//搜索到一个有效文件或文件夹，处理好名称后即可返回。搜到文件夹后需要将nowSectorPtr指向下一个。否则下次进来还是搜到当前文件
		{
			break;
		}
	}
	if(1==flag || 3==flag)
	{
		return 2;
	}
	else
		return 1;
}

//找到路径中的/并返回位置.FFFF表示搜索失败.FFFE表示搜索到了文件的尾部
u16 findSeparator(const u8 *path)
{
	u16 i;

	i = 0;
	while(1)
	{
		if(path[i] == '/')
		{
			if(path[i+1] != '/')//搜索到类似于 /E 这种组合才算有效
			{
				return i;
			}
		}
		else if(path[i] == '\0' || path[i] == 0xff)
		{
			i |= 0x8000;
			return i;
		}
		i++;
		if(i >= MAX_PATH_LEN)
		{
			return 0xffff;
		}
	}
}

void getNowPathName(u8 *name)
{
	u16 tmp;
	
	if(fileSysFlag)
		tmp = sizeArray(sysPath.name);
	else	
		tmp = 0;
	if(tmp)
	{
		StrCopy(name,sysPath.name,tmp);
	}
	name[tmp] = '\0';
}

//判断路径是否一致 1代表一致，0代表发生变化
u8 pathJudge(const u8 *newPath, const u8 *oldPath)
{
	u16 len;
	u16 i,j;
	u8 newTemp[MAX_PATH_LEN];
	u8 speparatorFlag,alreadySep;

	if(newPath[0] == '.')//相对路径
	{
		for(i=1;i<MAX_PATH_LEN;i++)
		{
			if(newPath[i] != '\0')//
			{
				if(newPath[i] != '/')//类似于 ./DWIN_SET这种
					return 0;
			}
			else
				return 1;//表示一致，也就是类似于 ./ 这种字符
		}
	}
	else
	{
		len = sizeArray(newPath);
		j = 1;
		newTemp[0] = '/';
		speparatorFlag = 1;
		alreadySep = 0;
		for(i=0;i<len;i++)
		{
			if(speparatorFlag)//需要寻找不是'/'的数据
			{
				if(newPath[i] != '/')
				{
					speparatorFlag = 0;
					newTemp[j] = newPath[i];
					j++;
				}
			}
			else
			{
				if(newPath[i] == '/')//如果是/DWIN_SET/这种路径，需要把最后一个/滤除掉
				{
					alreadySep = 1;
				}
				else
				{
					if(alreadySep)
					{
						alreadySep = 0;
						speparatorFlag = 1;
						newTemp[j] = '/';
						j++;
					}
					newTemp[j] = newPath[i];
					j++;
				}
			}
		}
        if(j>1)
        {
            newTemp[j] = '/';
            j++;
        }
		newTemp[j] = '\0';
		if(0==mStrCmp(newTemp,oldPath))
			return 1;
		else
			return 0;
	}
	return 0;
}
//进入目录
//0表示进入成功 1表示进入失败
//相对路径 ./123
//绝对路径 /dwin_set/123
//注：123是一个文件夹，不是一个文件
u8 changeDirectory(const u8 *path)
{
	u8 pathName[MAX_PATH_LEN];
	u8 searchName[1024];
	u16 pathPos1,pathPos2;
	MPATH pathTmp;
	u16 tmp,len;
	u8 endFlag;

	if(0==fileSysFlag)//文件系统初始化后才能进行操作
		return FAIL;
	if(pathJudge(path,sysPath.name))
	{
		if(sysPath.nowFatNum == sysPath.startFatNum && sysPath.nowSectorNum ==0 && sysPath.nowSectorPtr ==0)
			return SUCCESS;
		sysPath.nowFatNum = sysPath.startFatNum;
		sysPath.nowSectorNum = 0;
		sysPath.nowSectorPtr = 0;
		if(SdReadSector((getActualSectorNum(pathTmp.startFatNum,pathTmp.nowSectorNum)),pathTmp.sectorData))//加载根目录的起始扇区数据
			return FAIL;
		return SUCCESS;
	}
	if('.' == path[0])//相对路径，从sysPath开始搜素
	{
		StrCopy((u8*)&pathTmp,(u8*)&sysPath,sizeof(MPATH));//拷贝当前的路径信息
		pathTmp.nowFatNum = pathTmp.startFatNum;
		pathTmp.nowSectorNum = 0;
		pathTmp.nowSectorPtr = 0;
		if(SdReadSector((getActualSectorNum(pathTmp.startFatNum,pathTmp.nowSectorNum)),pathTmp.sectorData))//加载当前路径的起始扇区数据
			return FAIL;
	}
	else//绝对路径,从头开始搜索
	{
		pathTmp.name[0] = '/';
		pathTmp.name[1] = 0;//初始化后，当前文件夹路径为根目录
		pathTmp.startFatNum = DeviceInfo.RootStartCluster;//根目录的起始簇
		pathTmp.nowFatNum = sysPath.startFatNum;//根目录的起始簇
		pathTmp.nowSectorNum = 0;//根目录的起始簇的第0个扇区
		pathTmp.nowSectorPtr = 0;//根目录的起始簇的第0个扇区的第0个字节
		if(SdReadSector((DeviceInfo.FirstDataSector),pathTmp.sectorData))//加载根目录的起始扇区数据
			return FAIL;
	}
	pathPos1 = findSeparator(path);//查找第一个 /E这种字符
	if(pathPos1 == 0xffff)
		return FAIL;
	if(pathPos1 & 0x8000)//表示搜索到了尾部
		return FAIL;
	while(1)
	{
		pathPos2 = findSeparator(&path[pathPos1+1]);//查找第二个'/'或者\0
		if(pathPos2 == 0xffff)
			return FAIL;
        tmp = pathPos2 & 0x7fff;
		if(pathPos2 & 0x8000)//表示搜索到了尾部
		{
			if(tmp)//表示尾部和上一个/之间有数据,处理完后即可退出
			{
				// StrCopy(pathName,&path[pathPos1+1],tmp);
				// pathName[tmp] = '\0';//加上结束符
				endFlag = 1;
			}
			else
				break;
		}
		StrCopy(pathName,&path[pathPos1+1],tmp);
		pathName[tmp] = '\0';//加上结束符
		pathPos1 += (tmp + 1);
		while(1)
		{
			tmp = getOneName(&pathTmp,searchName);
			if(1 == tmp)//表示搜索到了一个文件夹
			{
				len = sizeArray(pathName);
				if(0==mStrCmp(pathName,searchName))//搜索到了这个文件夹
				{
					tmp = sizeArray(pathTmp.name);
					if(len+tmp+2 > MAX_PATH_LEN)
						return FAIL;
					
					StrCopy(&pathTmp.name[tmp],pathName,len);//多拷贝一个\0
					pathTmp.name[tmp+len] = '/';
					pathTmp.name[tmp+len+1] = '\0';
					pathTmp.startFatNum = listStartFatNum;
					pathTmp.nowFatNum = listStartFatNum;
					pathTmp.nowSectorNum = 0;
					pathTmp.nowSectorPtr = 0;
					if(SdReadSector((getActualSectorNum(pathTmp.startFatNum,pathTmp.nowSectorNum)),pathTmp.sectorData))//加载根目录的起始扇区数据
						return FAIL;
					break;
				}
			}
			else if(0==tmp)
			{
				return FAIL;
			}
		}
		if(endFlag)//处理完了需要退出
			break;
	}
	StrCopy((u8*)&sysPath,(u8*)&pathTmp,sizeof(MPATH));//拷贝当前的路径信息
	return SUCCESS;
}

//mode 0表示从头开始搜索，1表示从当前位置继续搜索
//由于空间有限，每次只搜去一个文件或文件夹的名字
//当路径发生改变的时候从头搜索。
//0代表搜索失败，2代表搜索到文件，1代表搜索到文件夹
u8 listDirectory(u8 *path, u8* name)
{
	u8 flag;
	MPATH pathTmp;

	flag = FAIL;

	if(pathJudge(path,listPath.name))//表示路径相同
	{
		flag = getOneName(&listPath,name);
	}
	else
	{
		StrCopy((u8*)&pathTmp,(u8*)&sysPath,sizeof(MPATH));//保护系统路径

		if(SUCCESS == changeDirectory(path))//表示进入了对应的文件夹
		{
			StrCopy((u8*)&listPath,(u8*)&sysPath,sizeof(MPATH));
			flag = getOneName(&listPath,name);
		}
		StrCopy((u8*)&sysPath,(u8*)&pathTmp,sizeof(MPATH));//恢复系统路径
	}
	return flag;
}

//读取文件
u8 mReadFile(pREAD_FILE_STU read)
// u8 mReadFile(u8* pathAddName, u32 *fstartPos, u32 *flen, u8 *dataBuf)
{


	u8 pathName[MAX_PATH_LEN];
	u8 fileName[MAX_FILE_NAME_LEN];
	u8 fileNameTmp[MAX_FILE_NAME_LEN];
	u16 lenTmp,i,k,tmpSectorPtr;
	MPATH pathTmp;
	u8 flag,tmpSectorNum;
	u32 tmpFatNum,j;
	u32 startPos,len;
	u8 *pathAddName;
	u8 *dataBuf;

	startPos = read->fstartPos;
	len = read->flen;
	pathAddName = read->filePath;
	dataBuf = read->dataBuf;
	if(0==len)
		return FAIL;
	lenTmp = sizeArray(pathAddName);
	k = 0;
	for(i=0;i<lenTmp;i++)
	{
		if(pathAddName[lenTmp-i-1] == '/')
			break;
		else
			k++;
	}
	if(i==lenTmp)//表示没有分隔符
		return FAIL;
	if(k==0)//没搜到带有除了分隔符以外的字符
		return FAIL;
	if(k >= MAX_FILE_NAME_LEN)
		return FAIL;
	if(lenTmp-k >= MAX_PATH_LEN)
		return FAIL;
	
	StrCopy(fileName,&pathAddName[lenTmp-k],k);
	fileName[k] = '\0';
	StrCopy(pathName,&pathAddName[0],lenTmp-k);
	pathName[lenTmp-k] = '\0';//分离出路径和文件名字

	i = 0;
	if(pathJudge(pathName,readPath.name))//表示相同路径
	{
		if(mStrCmp(fileName,sysFile.name))//表示文件不相同，需要重新搜索
		{
			i = 1;
		}
	}
	else
	{
		i = 1;
		StrCopy((u8*)&pathTmp,(u8*)&sysPath,sizeof(MPATH));//保护系统路径
		flag = changeDirectory(pathName);
		StrCopy((u8*)&sysPath,(u8*)&pathTmp,sizeof(MPATH));//恢复系统路径
		if(SUCCESS == flag)//表示进入了对应的文件夹
		{
			StrCopy((u8*)&readPath,(u8*)&sysPath,sizeof(MPATH));
		}
		else
			return FAIL;
	}
	if(i)//表示需要重新搜索文件
	{
		while (1)
		{
			flag = getOneName(&readPath,fileNameTmp);
			if(2==flag)//搜索到了一个文件
			{
				if(0==mStrCmp(fileNameTmp,fileName))
				{
					sysFile.startFatNum = listStartFatNum;
					sysFile.size = listSize;
					read->fileSize = listSize;
					sysFile.nowFatNum = sysFile.startFatNum;
					sysFile.nowSectorNum = 0;
					lenTmp = sizeArray(fileName);
					StrCopy(sysFile.name,fileName,lenTmp);
					sysFile.name[lenTmp] = '\0';
					break;
				}
			}
			else if(0==flag)//搜索失败
			{
				return FAIL;
			}
		}
	}
	if(startPos > sysFile.size)
		return FAIL;
	if(startPos+len >= sysFile.size)
	{
		flag = 1;//表示读到尾了
		len = sysFile.size - startPos;
	}
	else
	{
		flag = 0;
	}
		
	read->realReadLen = len;
	tmpFatNum = startPos / (512UL * DeviceInfo.BPB_SecPerClus);//表示起始位置在第几簇
	tmpSectorNum = (startPos % (512UL * DeviceInfo.BPB_SecPerClus)) / 512;//从当前簇的第几个FAT开始
	tmpSectorPtr = (startPos % (512UL * DeviceInfo.BPB_SecPerClus)) % 512;//从当前FAT块的哪个位置开始
	

	sysFile.nowFatNum = sysFile.startFatNum;
	sysFile.nowSectorNum = tmpSectorNum;
	for(j=0;j<tmpFatNum;j++)
		sysFile.nowFatNum = GetNextClusterNum32(sysFile.nowFatNum);
	lenTmp = sysFile.nowSectorNum;

	tmpFatNum = 0;
	while(1)
	{
		if(SdReadSector((getActualSectorNum(sysFile.nowFatNum,sysFile.nowSectorNum)),sysFile.sectorData))//加载根目录的起始扇区数据
			return FAIL;

		if(len <= (512-tmpSectorPtr))//剩余长度小于该块剩余长度，则读取完成
		{
			StrCopy(&dataBuf[tmpFatNum],&sysFile.sectorData[tmpSectorPtr],len);
			break;
		}
		else
		{
			StrCopy(&dataBuf[tmpFatNum],&sysFile.sectorData[tmpSectorPtr],512-tmpSectorPtr);
			tmpFatNum += (512-tmpSectorPtr);
			len -= (512-tmpSectorPtr);
			sysFile.nowSectorNum++;
			if(sysFile.nowSectorNum >= DeviceInfo.BPB_SecPerClus)
			{
				sysFile.nowSectorNum = 0;
				sysFile.nowFatNum = GetNextClusterNum32(sysFile.nowFatNum);
				if(sysFile.nowFatNum == 0x0fffffff)//表示没有后续簇了
				{
					return FAIL;
				}	
			}
		}
		tmpSectorPtr = 0;
	}
	if (flag)
		return 0x80|SUCCESS;
	else
		return SUCCESS;
}

#if 1
code u8 pathTest[]={"/DWIN_SET/Abc/0_DWIN_ASC.HZK"};
void test(void)
{
	u8 tmp;
	u8 name[1024];
	READ_FILE_STU readTmp;



	tmp = InitFileSystem();
	if(TRUE == tmp)
		DEBUGINFO("初始化文件系统成功\n");
	else
		DEBUGINFO("初始化文件系统失败\n");

	tmp = changeDirectory("/DWIN_SET/Abc");
	if(SUCCESS == tmp)
		DEBUGINFO("成功进入Abc\n");
	else
		DEBUGINFO("未找到Abc\n");

	getNowPathName(name);
	DEBUGINFO("path:%s\n",name);

	while (listDirectory("/DWIN_SET",name))
	{
		DEBUGINFO("file:%s\n",name);
	}

	getNowPathName(name);
	DEBUGINFO("path:%s\n",name);

	while (listDirectory("/DWIN_SET/Abc",name))
	{
		DEBUGINFO("file:%s\n",name);
	}

	readTmp.dataBuf = name;
	readTmp.filePath = pathTest;
	readTmp.fstartPos = 0;
	readTmp.flen = 1024;
	readTmp.fileSize = 0;
	readTmp.realReadLen = 0;
	DEBUGINFO("读取文件路径%s\n",readTmp.filePath);
	while(1)
	{
		tmp = mReadFile(&readTmp);
		if(FAIL == tmp)
		{
			DEBUGINFO("读取文件失败\n");
			break;
		}
		else
		{
			Uart_Send_Data(UART2,readTmp.realReadLen,name);
			delay_ms(100);
			if(tmp&0x80)
			{
				DEBUGINFO("读取到文件尾部,大小%ld\n",readTmp.fileSize);

				break;
			}
			
		}
		DEBUGINFO("读取实际长度%ld\n",readTmp.realReadLen);
		readTmp.fstartPos += readTmp.realReadLen;
	}
	while (1);
}
#endif


















// unsigned int displayOnePicture(unsigned long cluster)
// {
// 	unsigned long i,j;
// 	unsigned long fileCluster = 0;//文件的簇号
// 	unsigned long fileSectorsNum;//文件的扇区号
// 	u16 flag = 0;
// //	u8 tmp8 = 0;
// 	u16 tmp16 = 0;
// 	u32 tmp32 = 0;
	
	
// 	fileCluster = cluster;
// 	j = 0;

// 	do
// 	{
// 		for(i =0;i<DeviceInfo.BPB_SecPerClus;i++)
// 		{
// 			fileSectorsNum = DeviceInfo.FirstDataSector + (fileCluster - 2)*(DeviceInfo.BPB_SecPerClus) + i;		
// //			if(SdReadSector(fileSectorsNum,&FATBUFPIC[(512*i)+(j*512*8)]))
// //				return FALSE;
// 		}
// 		j++;
// 		fileCluster = GetNextClusterNum32(fileCluster); 
// 	}while((fileCluster != 0x0fffffff)&&(j <6));////图片最大20k	
	
// 	return 0;
// }

// //循环显示一轮图片
// unsigned int displayPicture(void)
// {
// 	u8 tmp8 = 0;
// 	u16 tmp16 = 0;	
// 	u32 i,j;
// 	u32 fileCluster = 0;//文件的簇号
// 	u32 RootDirSectorsNum;//根目录的扇区号
// 	//第一步，寻找根目录的位置
// 	RootDirSectorsNum = DeviceInfo.FirstDataSector;
	
// 	//根目录有8/16/...个扇区
// 	for(i = 0;i<DeviceInfo.BPB_SecPerClus;i++)
// 	{
// 		if(SdReadSector((RootDirSectorsNum + i),DBUF))
// 		{
// 			return FALSE;
// 		}
// 		//每个目录项32个字节，遍历所有的目录项
// 		for(j = 0;j<16;j++)
// 		{ 		
// 			if(0xe5 == DBUF[0 + 32*j])
// 			{
// 				continue;
// 			}
// 			else
// 			{
// 				//寻找文件的起始簇号
// 				fileCluster = LSwapINT32(DBUF[26+(32*j)],DBUF[27+(32*j)],DBUF[20+(32*j)],DBUF[21+(32*j)]);		
// 				if(0 == fileCluster)
// 				{	
// 					return 0;
// 				}
// 				else
// 				{
// 					tmp8 = 0;
// 					do
// 					{
// 						tmp8++;
// 						read_dgus_vp(0x15,(u8*)&tmp16,1);
// 						delay_ms(1);
// 						if(250 == tmp8)
// 						{
// 							return 0;
// 						}
// 					}while(1 == tmp16);
					
// 					if(0 == tmp16)
// 					{
// 						//展示图片
// 						displayOnePicture(fileCluster);
// 					}				
// 				}
// 			}
// 		}
// 	}
// 	return 0;
// }