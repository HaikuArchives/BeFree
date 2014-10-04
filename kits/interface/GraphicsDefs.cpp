/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * ETK++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: GraphicsDefs.cpp
 *
 * --------------------------------------------------------------------------*/

#include "GraphicsDefs.h"
#include "Point.h"


static uint32 color_maps[256] = {
	/* R = D * 8, G = D * 8, B = D * 8 */
	0xFF000000, /* color 0 */
	0xFF080808, /* color 1 */
	0xFF101010, /* color 2 */
	0xFF181818, /* color 3 */
	0xFF202020, /* color 4 */
	0xFF282828, /* color 5 */
	0xFF303030, /* color 6 */
	0xFF383838, /* color 7 */
	0xFF404040, /* color 8 */
	0xFF484848, /* color 9 */
	0xFF505050, /* color 10 */
	0xFF585858, /* color 11 */
	0xFF606060, /* color 12 */
	0xFF686868, /* color 13 */
	0xFF707070, /* color 14 */
	0xFF787878, /* color 15 */
	0xFF808080, /* color 16 */
	0xFF888888, /* color 17 */
	0xFF909090, /* color 18 */
	0xFF989898, /* color 19 */
	0xFFA0A0A0, /* color 20 */
	0xFFA8A8A8, /* color 21 */
	0xFFB0B0B0, /* color 22 */
	0xFFB8B8B8, /* color 23 */
	0xFFC0C0C0, /* color 24 */
	0xFFC8C8C8, /* color 25 */
	0xFFD0D0D0, /* color 26 */
	0xFFD8D8D8, /* color 27 */
	0xFFE0E0E0, /* color 28 */
	0xFFE8E8E8, /* color 29 */
	0xFFF0F0F0, /* color 30 */
	0xFFF8F8F8, /* color 31 */

	/* R = 0, G = 0, B = (255 - (D - 32) * 25) */
	0xFFFF0000, /* color 32 */
	0xFFE50000, /* color 33 */
	0xFFCC0000, /* color 34 */
	0xFFB30000, /* color 35 */
	0xFF9A0000, /* color 36 */
	0xFF810000, /* color 37 */
	0xFF690000, /* color 38 */
	0xFF500000, /* color 39 */
	0xFF370000, /* color 40 */
	0xFF1E0000, /* color 41 */

	/* R = (255 - (D - 42) * 25), G = 0, B = 0 */
	0xFF0000FF, /* color 42 */
	0xFF0000E4, /* color 43 */
	0xFF0000CB, /* color 44 */
	0xFF0000B2, /* color 45 */
	0xFF000099, /* color 46 */
	0xFF000080, /* color 47 */
	0xFF000069, /* color 48 */
	0xFF000050, /* color 49 */
	0xFF000037, /* color 50 */
	0xFF00001E, /* color 51 */

	/* R = 0, G = (255 - (D - 52) * 25), B = 0 */
	0xFF00FF00, /* color 52 */
	0xFF00E400, /* color 53 */
	0xFF00CB00, /* color 54 */
	0xFF00B200, /* color 55 */
	0xFF009900, /* color 56 */
	0xFF008000, /* color 57 */
	0xFF006900, /* color 58 */
	0xFF005000, /* color 59 */
	0xFF003700, /* color 60 */
	0xFF001E00, /* color 61 */

	0xFF339800, /* color 62 */
	0xFFFFFFFF, /* color 63 */

	/* R = (255 - (((D - 64) % 32) / 6 + 1) * 51), G = 255 - ((D - 64) / 32 * 51) , B = (255 - (((D - 64) % 32) % 6) * 51) */
	0xFFFFFFCB, /* color 64 */
	0xFFCBFFCB, /* color 65 */
	0xFF98FFCB, /* color 66 */
	0xFF66FFCB, /* color 67 */
	0xFF33FFCB, /* color 68 */
	0xFF00FFCB, /* color 69 */
	0xFFFFFF98, /* color 70 */
	0xFFCBFF98, /* color 71 */
	0xFF98FF98, /* color 72 */
	0xFF66FF98, /* color 73 */
	0xFF33FF98, /* color 74 */
	0xFF00FF98, /* color 75 */
	0xFFFFFF66, /* color 76 */
	0xFFCBFF66, /* color 77 */
	0xFF98FF66, /* color 78 */
	0xFF66FF66, /* color 79 */
	0xFF33FF66, /* color 80 */
	0xFF00FF66, /* color 81 */
	0xFFFFFF33, /* color 82 */
	0xFFCBFF33, /* color 83 */
	0xFF98FF33, /* color 84 */
	0xFF66FF33, /* color 85 */
	0xFF33FF33, /* color 86 */
	0xFF00FF33, /* color 87 */

	/* R = 255, G = 255 - (((D - 88) / 32 + 2) * 51) , B = (255 - (((D - 88) % 32) % 6) * 51) */
	0xFFFF98FF, /* color 88 */
	0xFFCB98FF, /* color 89 */
	0xFF9898FF, /* color 90 */
	0xFF6698FF, /* color 91 */
	0xFF3398FF, /* color 92 */
	0xFF0098FF, /* color 93 */

	0xFFFF6600, /* color 94 */
	0xFFCB6600, /* color 95 */

	/* R = (255 - (((D - 64) % 32) / 6 + 1) * 51), G = 255 - ((D - 64) / 32 * 51) , B = (255 - (((D - 64) % 32) % 6) * 51) */
	0xFFFFCBCB, /* color 96 */
	0xFFCBCBCB, /* color 97 */
	0xFF98CBCB, /* color 98 */
	0xFF66CBCB, /* color 99 */
	0xFF33CBCB, /* color 100 */
	0xFF00CBCB, /* color 101 */
	0xFFFFCB98, /* color 102 */
	0xFFCBCB98, /* color 103 */
	0xFF98CB98, /* color 104 */
	0xFF66CB98, /* color 105 */
	0xFF33CB98, /* color 106 */
	0xFF00CB98, /* color 107 */
	0xFFFFCB66, /* color 108 */
	0xFFCBCB66, /* color 109 */
	0xFF98CB66, /* color 110 */
	0xFF66CB66, /* color 111 */
	0xFF33CB66, /* color 112 */
	0xFF00CB66, /* color 113 */
	0xFFFFCB33, /* color 114 */
	0xFFCBCB33, /* color 115 */
	0xFF98CB33, /* color 116 */
	0xFF66CB33, /* color 117 */
	0xFF33CB33, /* color 118 */
	0xFF00CB33, /* color 119 */

	/* R = 255, G = 255 - (((D - 88) / 32 + 2) * 51), B = (255 - (((D - 88) % 32) % 6) * 51) */
	0xFFFF66FF, /* color 120 */
	0xFFCB66FF, /* color 121 */
	0xFF9866FF, /* color 122 */
	0xFF6666FF, /* color 123 */
	0xFF3366FF, /* color 124 */
	0xFF0066FF, /* color 125 */

	0xFF986600, /* color 126 */
	0xFF666600, /* color 127 */

	/* R = (255 - (((D - 64) % 32) / 6 + 1) * 51), G = 255 - ((D - 64) / 32 * 51) , B = (255 - (((D - 64) % 32) % 6) * 51) */
	0xFFFF98CB, /* color 128 */
	0xFFCB98CB, /* color 129 */
	0xFF9898CB, /* color 130 */
	0xFF6698CB, /* color 131 */
	0xFF3398CB, /* color 132 */
	0xFF0098CB, /* color 133 */
	0xFFFF9898, /* color 134 */
	0xFFCB9898, /* color 135 */
	0xFF989898, /* color 136 */
	0xFF669898, /* color 137 */
	0xFF339898, /* color 138 */
	0xFF009898, /* color 139 */
	0xFFFF9866, /* color 140 */
	0xFFCB9866, /* color 141 */
	0xFF989866, /* color 142 */
	0xFF669866, /* color 143 */
	0xFF339866, /* color 144 */
	0xFF009866, /* color 145 */
	0xFFFF9833, /* color 146 */
	0xFFCB9833, /* color 147 */
	0xFF989833, /* color 148 */
	0xFF669833, /* color 149 */
	0xFF339833, /* color 150 */
	0xFF009833, /* color 151 */

	/* R = 255, G = 255 - (((D - 88) / 32 + 2) * 51), B = (255 - (((D - 88) % 32) % 6) * 51) */
	0xFF0086E6, /* color 152 */
	0xFFCB33FF, /* color 153 */
	0xFF9833FF, /* color 154 */
	0xFF6633FF, /* color 155 */
	0xFF3333FF, /* color 156 */
	0xFF0033FF, /* color 157 */

	0xFF336600, /* color 158 */
	0xFF006600, /* color 159 */

	/* R = (255 - (((D - 64) % 32) / 6 + 1) * 51), G = 255 - ((D - 64) / 32 * 51) , B = (255 - (((D - 64) % 32) % 6) * 51) */
	0xFFFF66CB, /* color 160 */
	0xFFCB66CB, /* color 161 */
	0xFF9866CB, /* color 162 */
	0xFF6666CB, /* color 163 */
	0xFF3366CB, /* color 164 */
	0xFF0066CB, /* color 165 */
	0xFFFF6698, /* color 166 */
	0xFFCB6698, /* color 167 */
	0xFF986698, /* color 168 */
	0xFF666698, /* color 169 */
	0xFF336698, /* color 170 */
	0xFF006698, /* color 171 */
	0xFFFF6666, /* color 172 */
	0xFFCB6666, /* color 173 */
	0xFF986666, /* color 174 */
	0xFF666666, /* color 175 */
	0xFF336666, /* color 176 */
	0xFF006666, /* color 177 */
	0xFFFF6633, /* color 178 */
	0xFFCB6633, /* color 179 */
	0xFF986633, /* color 180 */
	0xFF666633, /* color 181 */
	0xFF336633, /* color 182 */
	0xFF006633, /* color 183 */

	/* R = 255, G = 255 - (((D - 88) / 32 + 2) * 51), B = (255 - (((D - 88) % 32) % 6) * 51) */
	0xFFFF00FF, /* color 184 */
	0xFFCB00FF, /* color 185 */
	0xFF9800FF, /* color 186 */
	0xFF6600FF, /* color 187 */
	0xFF3300FF, /* color 188 */
	0xFF13AFFF, /* color 189 */

	0xFFFF3300, /* color 190 */
	0xFFCB3300, /* color 191 */

	/* R = (255 - (((D - 64) % 32) / 6 + 1) * 51), G = 255 - ((D - 64) / 32 * 51) , B = (255 - (((D - 64) % 32) % 6) * 51) */
	0xFFFF33CB, /* color 192 */
	0xFFCB33CB, /* color 193 */
	0xFF9833CB, /* color 194 */
	0xFF6633CB, /* color 195 */
	0xFF3333CB, /* color 196 */
	0xFF0033CB, /* color 197 */
	0xFFFF3398, /* color 198 */
	0xFFCB3398, /* color 199 */
	0xFF983398, /* color 200 */
	0xFF663398, /* color 201 */
	0xFF333398, /* color 202 */
	0xFF003398, /* color 203 */
	0xFFFF3366, /* color 204 */
	0xFFCB3366, /* color 205 */
	0xFF983366, /* color 206 */
	0xFF663366, /* color 207 */
	0xFF333366, /* color 208 */
	0xFF003366, /* color 209 */
	0xFFFF3333, /* color 210 */
	0xFFCB3333, /* color 211 */
	0xFF983333, /* color 212 */
	0xFF663333, /* color 213 */
	0xFF333333, /* color 214 */
	0xFF003333, /* color 215 */

	0xFF66CBFF, /* color 216 */
	0xFF98CBFF, /* color 217 */
	0xFFCBCBFF, /* color 218 */
	0xFFFFCBFF, /* color 219 */

	0xFF983300, /* color 220 */
	0xFF663300, /* color 221 */
	0xFF333300, /* color 222 */
	0xFF003300, /* color 223 */

	/* R = (255 - (((D - 64) % 32) / 6 + 1) * 51), G = 255 - ((D - 64) / 32 * 51) , B = (255 - (((D - 64) % 32) % 6) * 51) */
	0xFFFF00CB, /* color 224 */
	0xFFCB00CB, /* color 225 */
	0xFF9800CB, /* color 226 */
	0xFF6600CB, /* color 227 */
	0xFF3300CB, /* color 228 */
	0xFF46E3FF, /* color 229 */
	0xFFFF0098, /* color 230 */
	0xFFCB0098, /* color 231 */
	0xFF980098, /* color 232 */
	0xFF660098, /* color 233 */
	0xFF330098, /* color 234 */
	0xFF000098, /* color 235 */
	0xFFFF0066, /* color 236 */
	0xFFCB0066, /* color 237 */
	0xFF980066, /* color 238 */
	0xFF660066, /* color 239 */
	0xFF330066, /* color 240 */
	0xFF000066, /* color 241 */
	0xFFFF0033, /* color 242 */
	0xFFCB0033, /* color 243 */
	0xFF980033, /* color 244 */
	0xFF660033, /* color 245 */
	0xFF330033, /* color 246 */
	0xFF000033, /* color 247 */

	0xFF33CBFF, /* color 248 */
	0xFF00CBFF, /* color 249 */

	0xFF00FFFF, /* color 250 */
	0xFF33FFFF, /* color 251 */
	0xFF66FFFF, /* color 252 */
	0xFF98FFFF, /* color 253 */
	0xFFCBFFFF, /* color 254 */
	0xFFFFFFFF, /* color 255 */
};

extern const BPoint B_ORIGIN(0, 0);

extern "C"
{

	extern const pattern B_SOLID_HIGH = e_makpattern(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
	extern const pattern B_MIXED_COLORS = e_makpattern(0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55);
	extern const pattern B_SOLID_LOW = e_makpattern(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

	extern const rgb_color B_TRANSPARENT_COLOR = e_makrgb_color(233, 233, 233, 255);

#ifdef ETK_BIG_ENDIAN
	extern const uint32 B_TRANSPARENT_MAGIC_RGBA32 = 0xE9E9E9FF;
#else
	extern const uint32 B_TRANSPARENT_MAGIC_RGBA32 = 0xFFE9E9E9;
#endif


	uint8 find_index_for_color(uint8 r, uint8 g, uint8 b) {
		return 0;
#if 0
		if (r == g && g == b) {
			if (r == 255) return 255;
			return(r / 8);
		}

		if (r == 0 && g == 0) return(32 + (255 - b) / 26);
		if (g == 0 && b == 0) return(42 + (255 - r) / 26);
		if (r == 0 && b == 0) return(52 + (255 - g) / 26);

		if (r == 255) {
			if (g == 255) return(250 + b / 51);
			if (g == 203) {
				uint8 i = b / 51;
				return(i < 2 ? (249 - i) : (214 + i));
			}
			return(88 + 32 * ((255 - g) / 51 - 2) + (255 - b) / 51);
		}

		if (r == 0) {
		}

		return(((255 - g) / 51) * 32 + (((255 - r) / 51 - 1) * 6) + (255 - b) / 51);
#endif
	}


	rgb_color find_color_for_index(uint8 index) {
		uint32 c = color_maps[index];
		return e_makrgb_color(c & 0xff, (c >> 8) & 0xff, (c >> 16) & 0xff, 0xff);
	}


} // extern "C"


rgb_color&
rgb_color::mix(uint8 r, uint8 g, uint8 b, uint8 a)
{
	if (a == 0xff) {
		red = r;
		green = g;
		blue = b;
	} else if (a != 0) {
		red = (uint8)(((uint16)red * ((uint16)0xff - (uint16)a) + (uint16)r * (uint16)a) / (uint16)0xff);
		green = (uint8)(((uint16)green * ((uint16)0xff - (uint16)a) + (uint16)g * (uint16)a) / (uint16)0xff);
		blue = (uint8)(((uint16)blue * ((uint16)0xff - (uint16)a) + (uint16)b * (uint16)a) / (uint16)0xff);
	}

	return *this;
}


rgb_color&
rgb_color::mix(const rgb_color &o)
{
	return mix(o.red, o.green, o.blue, o.alpha);
}


rgb_color&
rgb_color::mix_copy(uint8 r, uint8 g, uint8 b, uint8 a) const
{
	rgb_color color = *this;
	return color.mix(r, g, b, a);
}


rgb_color&
rgb_color::mix_copy(const rgb_color &o) const
{
	rgb_color color = *this;
	return color.mix(o.red, o.green, o.blue, o.alpha);
}


rgb_color&
rgb_color::disable(uint8 r, uint8 g, uint8 b, uint8 a)
{
	rgb_color color;
	color.set_to(r, g, b, a);
	return disable(color);
}


rgb_color&
rgb_color::disable(const rgb_color &background)
{
	*this = background.mix_copy(red, green, blue, 150);
	return mix(0, 0, 0, 20);
}


rgb_color&
rgb_color::disable_copy(uint8 r, uint8 g, uint8 b, uint8 a) const
{
	rgb_color color = *this;
	return color.disable(r, g, b, a);
}


rgb_color&
rgb_color::disable_copy(const rgb_color &background) const
{
	rgb_color color = *this;
	return color.disable(background.red, background.green, background.blue, background.alpha);
}

