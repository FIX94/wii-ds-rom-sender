/*
 * Copyright (C) 2017 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#ifndef _NDSFILE_H_
#define _NDSFILE_H_

bool ndsfile_station_open();
void ndsfile_station_close();
bool ndsfile_station_getfile(uint8_t *buf, size_t *len, char *fname);
uint8_t *ndsfile_haxx_start(FILE *f, uint32_t *len);
void ndsfile_haxx_end();
uint16_t ndsfile_crc(const uint8_t *buf, const uint32_t len);

#endif
