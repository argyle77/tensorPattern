/*
 * pattern.h
 *
 *  Created on: Jul 17, 2010
 *      Author: argyle
 */

#ifndef PATTERN_H_
#define PATTERN_H_

void ColorAll(color_t color, unsigned char *fb);
void SetPixel(int x, int y, color_t color, unsigned char *buffer);
color_t GetPixel(int x, int y, unsigned char *buffer);
void RandomDots(color_t color, unsigned int rFreq, unsigned char *buffer);
void PatternEngine(void);

#endif /* PATTERN_H_ */
