/*
 * Copyright (C) 2017 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <inttypes.h>
#include "ndsfile.h"

static bool station_open = false;
static FILE *station_f = NULL;
static uint8_t *station_fntbuf = NULL;
static uint8_t *station_fatbuf = NULL;
static uint32_t station_len = 0;
static uint32_t station_fnt = 0;
static uint32_t station_fat = 0;
static uint32_t station_rom = 0;
static uint32_t station_fntlen = 0;
static uint32_t station_fatlen = 0;

bool ndsfile_station_open()
{
	station_f = fopen("/haxxstation.nds","rb");
	if(!station_f) return false;
	fseek(station_f,0,SEEK_END);
	station_len = ftell(station_f);
	fseek(station_f,0,SEEK_SET);
	uint32_t tmp;
	fseek(station_f,0x40,SEEK_SET);
	fread(&tmp,1,4,station_f);
	station_fnt = __builtin_bswap32(tmp);
	//printf("%08x\n",station_fnt);
	fseek(station_f,0x48,SEEK_SET);
	fread(&tmp,1,4,station_f);
	station_fat = __builtin_bswap32(tmp);
	//printf("%08x\n",station_fat);
	if(station_fnt > 0 && station_fat > 0 && station_fnt < station_fat)
	{
		fseek(station_f,station_fat,SEEK_SET);
		fread(&tmp,1,4,station_f);
		station_rom = __builtin_bswap32(tmp);
		//printf("%08x\n",station_rom);
		if(station_rom > 0 && station_fat < station_rom)
		{
			station_fntlen = station_fat-station_fnt;
			station_fntbuf = malloc(station_fntlen);
			fseek(station_f,station_fnt,SEEK_SET);
			fread(station_fntbuf,1,station_fntlen,station_f);
			station_fatlen = station_rom-station_fat;
			station_fatbuf = malloc(station_fatlen);
			fseek(station_f,station_fat,SEEK_SET);
			fread(station_fatbuf,1,station_fatlen,station_f);
			station_open = true;
			return true;
		}
	}
	fclose(station_f);
	station_f = NULL;
	return false;
}

bool ndsfile_station_getfile(uint8_t *buf, size_t *len, char *fname)
{
	if(!station_open || !buf || !len || !fname)
		return false;

	uint32_t i, tmp, offStart, offEnd;
	memcpy(&tmp, station_fntbuf, 4);
	i = __builtin_bswap32(tmp);
	//printf("%08x\n",i);
	uint16_t tmp16;
	memcpy(&tmp16, station_fntbuf+4, 2);
	uint16_t fCnt = __builtin_bswap16(tmp16);
	//printf("%04x\n",fCnt);
	while(i < station_fntlen)
	{
		char lenByte = station_fntbuf[i];
		i++; //len byte
		if(lenByte == 0) //directory end
			continue;
		else if(lenByte&0x80) //directory start
		{
			i+=(lenByte&0x7F);
			i+=2; //directory id
			continue;
		}
		else //file entry
		{
			//printf("%i %s\n", lenByte, station_fntbuf+i);
			if(lenByte == strlen(fname))
			{
				if(memcmp(fname,station_fntbuf+i,lenByte) == 0)
				{
					uint32_t clientfatoffset = station_fat+(fCnt*8);
					//printf("FAT from entry %04x offset %08x\n", fCnt, clientfatoffset);
					fseek(station_f,clientfatoffset,SEEK_SET);
					fread(&tmp,1,4,station_f);
					offStart = __builtin_bswap32(tmp);
					fread(&tmp,1,4,station_f);
					offEnd = __builtin_bswap32(tmp);
					if(offStart && offEnd && offStart < offEnd)
					{
						fseek(station_f,offStart,SEEK_SET);
						*len = offEnd-offStart;
						//printf("Reading %i bytes from %08x\n",*len,offStart);
						fread(buf,1,*len,station_f);
						return true;
					}
				}
			}
			i+=lenByte;
			fCnt++; //for next entry
			continue;
		}
	}
	return false;
}

void ndsfile_station_close()
{
	if(station_open)
	{
		station_open = false;
		if(station_f)
			fclose(station_f);
		station_f = NULL;
		if(station_fntbuf)
			free(station_fntbuf);
		station_fntbuf = NULL;
		if(station_fatbuf)
			free(station_fatbuf);
		station_fatbuf = NULL;
	}
}

/* https://github.com/Gericom/dspatch/blob/master/dspatch/DownloadStationPatcher.cs#L100-L119 */
static const uint8_t exploitData[] =
{
	0x44, 0x30, 0x9F, 0xE5, 0x2C, 0x00, 0x93, 0xE5, 0x2C, 0x10, 0x9F, 0xE5,
	0x01, 0x00, 0x40, 0xE0, 0x28, 0x10, 0x9F, 0xE5, 0x01, 0x00, 0x80, 0xE0,
	0x24, 0x10, 0x9F, 0xE5, 0x24, 0x20, 0x9F, 0xE5, 0x28, 0xE0, 0x9F, 0xE5,
	0x3E, 0xFF, 0x2F, 0xE1, 0x24, 0xE0, 0x9F, 0xE5, 0x3E, 0xFF, 0x2F, 0xE1,
	0x01, 0x00, 0xA0, 0xE3, 0x1C, 0xE0, 0x9F, 0xE5, 0x1E, 0xFF, 0x2F, 0xE1,
	0x00, 0xAE, 0x11, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
	0x20, 0x6D, 0x11, 0x00, 0x40, 0xE8, 0x3F, 0x02, 0x08, 0xBB, 0x32, 0x02,
	0x24, 0xAF, 0x32, 0x02, 0x78, 0x3A, 0x32, 0x02
};

static const uint8_t arm7Fix[] =
{
	0x2C, 0x00, 0x9F, 0xE5, 0x8E, 0x07, 0x80, 0xE2, 0x1C, 0x10, 0x9F, 0xE5,
	0x1C, 0x20, 0x9F, 0xE5, 0x01, 0x30, 0xD0, 0xE4, 0x01, 0x30, 0xC1, 0xE4,
	0x01, 0x20, 0x52, 0xE2, 0xFB, 0xFF, 0xFF, 0xCA, 0x00, 0x00, 0x9F, 0xE5,
	0x10, 0xFF, 0x2F, 0xE1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00
};

static uint8_t *haxxbuf = NULL;

uint8_t *ndsfile_haxx_start(FILE *f, uint32_t *len)
{
	fseek(f,0,SEEK_SET);
	uint8_t oriHdr[0x160];
	fread(oriHdr,1,0x160,f);

	//Set up ARM9 Data
	uint32_t tmp;
	memcpy(&tmp, oriHdr+0x20, 4);
	uint32_t arm9off = __builtin_bswap32(tmp);
	memcpy(&tmp, oriHdr+0x24, 4);
	uint32_t arm9entry = __builtin_bswap32(tmp);
	memcpy(&tmp, oriHdr+0x28, 4);
	uint32_t arm9ram = __builtin_bswap32(tmp);
	memcpy(&tmp, oriHdr+0x2C, 4);
	uint32_t arm9len = __builtin_bswap32(tmp);
	uint8_t *arm9buf = malloc(arm9len);
	fseek(f,arm9off,SEEK_SET);
	fread(arm9buf,1,arm9len,f);
	//Set up ARM7 Data
	memcpy(&tmp, oriHdr+0x30, 4);
	uint32_t arm7off = __builtin_bswap32(tmp);
	memcpy(&tmp, oriHdr+0x34, 4);
	uint32_t arm7entry = __builtin_bswap32(tmp);
	memcpy(&tmp, oriHdr+0x38, 4);
	uint32_t arm7ram = __builtin_bswap32(tmp);
	memcpy(&tmp, oriHdr+0x3C, 4);
	uint32_t arm7len = __builtin_bswap32(tmp);
	uint8_t *arm7buf;
	//Do ARM7 Fix if needed
	if(arm7ram >= 0x03000000)
	{
		arm7buf = malloc(arm7len+sizeof(arm7Fix));
		memcpy(arm7buf, arm7Fix, sizeof(arm7Fix));
		fseek(f,arm7off,SEEK_SET);
		fread(arm7buf+sizeof(arm7Fix),1,arm7len,f);
		//Secure original ARM7 vals in ARM7 Fix
		memcpy(arm7buf+0x28, oriHdr+0x34, 4);
		memcpy(arm7buf+0x2C, oriHdr+0x38, 4);
		memcpy(arm7buf+0x30, oriHdr+0x3C, 4);
		arm7entry = 0x02380000;
		arm7ram = 0x02380000;
		arm7len = arm7len+sizeof(arm7Fix);
	}
	else
	{
		arm7buf = malloc(arm7len);
		fseek(f,arm7off,SEEK_SET);
		fread(arm7buf,1,arm7len,f);
	}
	uint32_t arm9len_align = (arm9len+0x1FF)&(~0x1FF);
	uint32_t arm7len_align = (arm7len+0x1FF)&(~0x1FF);
	uint32_t total_len = 0x200+arm9len_align+arm7len_align;
	haxxbuf = malloc(total_len);
	memset(haxxbuf,0,total_len);
	//base hdr setup
	memcpy(haxxbuf,"HOMEBREW",8);
	memset(haxxbuf+0xC,0x23,4);
	haxxbuf[0x14] = 0xB;
	//new arm9 and arm7 offsets
	arm9off = 0x200;
	arm7off = arm9off+arm9len_align;
	//arm9 offset becomes 0x180
	tmp = __builtin_bswap32(0x180);
	memcpy(haxxbuf+0x20, &tmp, 4);
	//copy from original?
	tmp = __builtin_bswap32(arm9entry);
	memcpy(haxxbuf+0x24, &tmp, 4);
	//arm9 load becomes 0x02332C40 (rsa_GetDecodedHash)
	tmp = __builtin_bswap32(0x02332C40);
	memcpy(haxxbuf+0x28, &tmp, 4);
	//arm9 size becomes 0x100
	tmp = __builtin_bswap32(0x100);
	memcpy(haxxbuf+0x2C, &tmp, 4);
	//set new arm7 offset
	tmp = __builtin_bswap32(arm7off);
	memcpy(haxxbuf+0x30, &tmp, 4);
	//set new arm7 entry
	tmp = __builtin_bswap32(arm7entry);
	memcpy(haxxbuf+0x34, &tmp, 4);
	//set new arm7 load
	tmp = __builtin_bswap32(arm7ram);
	memcpy(haxxbuf+0x38, &tmp, 4);
	//set new arm7 size
	tmp = __builtin_bswap32(arm7len);
	memcpy(haxxbuf+0x3C, &tmp, 4);
	//file end
	tmp = __builtin_bswap32(total_len);
	memcpy(haxxbuf+0x80,&tmp,4);
	//copy from original
	memcpy(haxxbuf+0xC0,oriHdr+0xC0,0x9E);
	//fix up new header crc16
	uint16_t newcrc = ndsfile_crc(haxxbuf, 0x15E);
	haxxbuf[0x15E] = (newcrc & 0xFF);
	haxxbuf[0x15F] = (newcrc >> 8);

	//actual arm9 exploit
	memcpy(haxxbuf+0x180, exploitData, sizeof(exploitData));

	haxxbuf[0x180 + 0x3C] = (arm7off & 0xFF);
	haxxbuf[0x180 + 0x3D] = ((arm7off >> 8) & 0xFF);
	haxxbuf[0x180 + 0x3E] = ((arm7off >> 16) & 0xFF);
	haxxbuf[0x180 + 0x3F] = ((arm7off >> 24) & 0xFF);

	haxxbuf[0x180 + 0x40] = (arm9off & 0xFF);
	haxxbuf[0x180 + 0x41] = ((arm9off >> 8) & 0xFF);
	haxxbuf[0x180 + 0x42] = ((arm9off >> 16) & 0xFF);
	haxxbuf[0x180 + 0x43] = ((arm9off >> 24) & 0xFF);

	haxxbuf[0x180 + 0x44] = (arm9ram & 0xFF);
	haxxbuf[0x180 + 0x45] = ((arm9ram >> 8) & 0xFF);
	haxxbuf[0x180 + 0x46] = ((arm9ram >> 16) & 0xFF);
	haxxbuf[0x180 + 0x47] = ((arm9ram >> 24) & 0xFF);

	haxxbuf[0x180 + 0x48] = (arm9len & 0xFF);
	haxxbuf[0x180 + 0x49] = ((arm9len >> 8) & 0xFF);
	haxxbuf[0x180 + 0x4A] = ((arm9len >> 16) & 0xFF);
	haxxbuf[0x180 + 0x4B] = ((arm9len >> 24) & 0xFF);

	//new arm9 and arm7 bins
	memcpy(haxxbuf+arm9off,arm9buf,arm9len);
	memcpy(haxxbuf+arm7off,arm7buf,arm7len);

	//done!
	*len = total_len;
	return haxxbuf;
}

void ndsfile_haxx_end()
{
	if(haxxbuf)
		free(haxxbuf);
	haxxbuf = NULL;
}

//DS CRC16 Function
static const uint16_t crcTbl[16] = {
	0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
	0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,
};
uint16_t ndsfile_crc(const uint8_t *buf, const uint32_t len)
{
	uint16_t crc = 0xFFFF;
	uint32_t i, j;
	for(i = 0; i < len; i+=2)
	{
		uint16_t curval = buf[i]|(buf[i+1]<<8);
		for(j = 0; j < 0x10; j+= 4)
		{
			uint16_t tmp = crcTbl[crc&0xF]^crcTbl[(curval>>j)&0xF];
			crc >>= 4;
			crc ^= tmp;
		}
	}
	return crc;
}
