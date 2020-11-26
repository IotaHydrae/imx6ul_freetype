#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/fb.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <wchar.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H


static int fd_fb;
static struct fb_var_screeninfo var;
static int screen_size;
static unsigned char *fb_base;

static unsigned int line_width, pixel_width;

void lcd_put_pixel(int x, int y, unsigned int color)
{
	unsigned char *pen_8 = fb_base + y*line_width + x*pixel_width;
	unsigned short *pen_16;
	unsigned int *pen_32;

	unsigned int red, green, blue;

	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;

	switch(var.bits_per_pixel)
	{
		case 8:
		{
			*pen_8 = color;
			break;
		}
		case 16:
		{
			red = (color >> 16) & 0xff;
			green = (color >> 8) & 0xff;
			blue = (color >> 0) & 0xff;
			color = ((red >> 3) << 11) | ((green >> 2) << 5) | ((blue >> 3) << 0);
			*pen_16 = color;
			break;
		}
		case 32:
		{
			*pen_32 = color;
			break;
		}
		default:
		{
			printf("can't surport %dbpp\n", var.bits_per_pixel);
		    break;
		}
	}
}

void
draw_bitmap( FT_Bitmap*  bitmap,
             FT_Int      x,
             FT_Int      y)
{
	FT_Int  i, j, p, q;
	FT_Int  x_max = x + bitmap->width;
	FT_Int  y_max = y + bitmap->rows;

	//printf("x = %d, y = %d\n", x, y);

	for ( j = y, q = 0; j < y_max; j++, q++ )
	{
		for ( i = x, p = 0; i < x_max; i++, p++ )
		{
			if ( i < 0      || j < 0       ||
				i >= var.xres || j >= var.yres )
			continue;

			//image[j][i] |= bitmap->buffer[q * bitmap->width + p];
			lcd_put_pixel(i, j, bitmap->buffer[q * bitmap->width + p]);
		}
	}
}

int compute_string_bbox(FT_Face face, wchar_t *wstr, FT_BBox *abbox)
{
	int i;
	int error;
	FT_BBox bbox;
	FT_BBox glyph_bbox;
	FT_Vector pen;
	FT_Glyph glyph;
	FT_GlyphSlot slot = face->glyph;

	bbox.xMin = bbox.yMin = 32000;
	bbox.xMax = bbox.yMax = -32000;

	pen.x = 0;
	pen.y = 0;

	/* 
		计算每个字符的bounding box 
		这些数据保存在face->glyph中
	*/
	for(i=0; i< wcslen(wstr); i++)
	{
		FT_Set_Transform(face, 0, &pen);
		error = FT_Load_Char(face, wstr[i], FT_LOAD_RENDER);
		if(error)
		{
            printf("FT_Load_Char error\n");
            return -1;
		}
		error = FT_Get_Glyph(face->glyph, &glyph);
        if (error)
        {
            printf("FT_Get_Glyph error!\n");
            return -1;
        }

        FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &glyph_bbox);
        if(glyph_bbox.xMin < bbox.xMin)
        {
			bbox.xMin = glyph_bbox.xMin;
        }
        if(glyph_bbox.yMin < bbox.yMin)
        {
			bbox.yMin = glyph_bbox.yMin;
        }
        if(glyph_bbox.xMax > bbox.xMax)
        {
			bbox.xMax = glyph_bbox.xMax;
        }
        if(glyph_bbox.yMax > bbox.yMax)
        {
			bbox.yMax = glyph_bbox.yMax;
        }

        pen.x += slot->advance.x;
        pen.y += slot->advance.y;
	}
	*abbox = bbox;
}

int display_string(FT_Face face, wchar_t *wstr, int lcd_x, int lcd_y, int row_spacing)
{
	int i;
	int error;
	FT_BBox bbox;
	FT_Vector pen;	
	FT_Glyph glyph;
	FT_GlyphSlot slot = face->glyph;
	int x,y;
	
	compute_string_bbox(face, wstr, &bbox);


	x = lcd_x;
	y = var.yres - lcd_y;
	pen.x = (x - bbox.xMin) * 64;
	pen.y = (y - bbox.yMax) * 64;	

	printf("******current  value: x:%d y:%d\n", x, y);

	for(i=0; i< wcslen(wstr); i++)
	{
		FT_Set_Transform(face, 0, &pen);
		error = FT_Load_Char(face, wstr[i], FT_LOAD_RENDER);
		if(error)
		{
            printf("FT_Load_Char error\n");
		}

		draw_bitmap(&slot->bitmap, 
						slot->bitmap_left,
						var.yres - slot->bitmap_top);

		pen.x += slot->advance.x;
        pen.y += slot->advance.y;
	}
	printf("xMin: %d、xMax: %d、yMin: %d、yMax: %d\n",bbox.xMin, bbox.xMax,bbox.yMin, bbox.yMax);
	/* 计算当前y值——笛卡尔坐标系 */
	y = pen.y/64 + bbox.yMax;
	printf("y: %d\n", y);
	/* y值减去文字外框顶部 */
	y = y - bbox.yMax - row_spacing;
	//printf("lcd_y: %d\n", lcd_y);
	/* 转换为LCD坐标系 */
	lcd_y = (var.yres - y);
	
	printf("lcd_y: %d\n", lcd_y);
	return lcd_y;
}

int main(int argc, char **argv)
{
	wchar_t *wstr = L"huazheng";
	wchar_t *wstr_grp[] = {L"华政ngrok.huazheng.club", L"百问网www.100ask.net", L"韦东山嵌入式专注于嵌入式课程及硬件研发"} ;
	
	FT_Library library;
	FT_Face face;
	FT_BBox bbox;
	int error;
	int font_size = 24;
	int lcd_x, lcd_y;
	int i;
	int len;
	char *str_in;
	int row_spacing;
	
	if(argc < 4)
	{
		printf("Usage: %s <font-file> <lcd_x> <lcd_y> [font-size] [row-spacing]\n", argv[0]);
		return -1;
	}

	lcd_x = strtoul(argv[2], NULL, 0);
	lcd_y = strtoul(argv[3], NULL, 0);
	if(argc > 4)
	{
		font_size = strtoul(argv[4], NULL, 0);
	}
	
	if(argc == 6)
	{
		row_spacing = strtoul(argv[5], NULL, 0);
	}else{
		row_spacing = 0;
	}

	
	fd_fb = open("/dev/fb0", O_RDWR);
	if(fd_fb < 0)
	{
		printf("can`t open /dev/fb0\n");
		return -1;
	}

	if(ioctl(fd_fb, FBIOGET_VSCREENINFO, &var))
	{
		printf("can`t get screen info\n");
		return -1;
	}
	line_width = var.xres * var.bits_per_pixel / 8;
	pixel_width = var.bits_per_pixel / 8;

	screen_size = var.xres * var.yres * var.bits_per_pixel / 8;

	fb_base = (unsigned char *)mmap(NULL, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb,0);

	if(fb_base == (unsigned char *)-1)
	{
		printf("can`t mmap\n");
		return -1;
	}
	memset(fb_base, 0, screen_size);



	/* 初始化库 */
	error = FT_Init_FreeType(&library);
	/*  */
	error = FT_New_Face(library, argv[1], 0, &face);
	FT_Set_Pixel_Sizes(face, font_size, 0);

	len = sizeof(wstr_grp)/sizeof(wchar_t);
	for(i=0; i<len; i++)
	{
		lcd_y = display_string(face, wstr_grp[i], lcd_x, lcd_y, row_spacing);
	}
	
	
	munmap(fb_base, screen_size);
	close(fd_fb);
	
	return 0;
}


