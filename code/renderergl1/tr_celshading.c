#include "tr_local.h"

/****************************
RGB GET/SET
****************************/

//RED
static byte getImageR(byte *targa_rgba, int x, int y, int columns, int rows)
{
	byte	*pixbuf;
	
	x*=((x<0)?-1:1);
	y*=((y<0)?-1:1);
  if(rows<=y)
		y=y%rows;
  if(columns<=x)
		x=x%columns;
	x*=((x<0)?-1:1);
	y*=((y<0)?-1:1);
  
	
	pixbuf = targa_rgba + y*columns*4;
	
	pixbuf+=(x*4);
	
	return *pixbuf;
}

static void setImageR(byte *targa_rgba, int x, int y, int columns, int rows, byte value)
{
	byte	*pixbuf;
	
	x*=((x<0)?-1:1);
	y*=((y<0)?-1:1);
	
	pixbuf = targa_rgba + y*columns*4;
	
	pixbuf+=(x*4);
	
	*pixbuf=value;
}
//GREEN
static byte getImageG(byte *targa_rgba, int x, int y, int columns, int rows)
{
	byte	*pixbuf;
	
	x*=((x<0)?-1:1);
	y*=((y<0)?-1:1);
  if(rows<=y)
		y=y%rows;
  if(columns<=x)
		x=x%columns;
	x*=((x<0)?-1:1);
	y*=((y<0)?-1:1);
	
	pixbuf = targa_rgba + y*columns*4;
	
	pixbuf+=(x*4);
	
	pixbuf++;
	return *pixbuf;
}

static void setImageG(byte *targa_rgba, int x, int y, int columns, int rows, byte value)
{
	byte	*pixbuf;

	x*=((x<0)?-1:1);
	y*=((y<0)?-1:1);

	pixbuf = targa_rgba + y*columns*4;
	
	pixbuf+=(x*4);
	pixbuf++;
	*pixbuf=value;
}
//BLUE
static byte getImageB(byte *targa_rgba, int x, int y, int columns, int rows)
{
	byte	*pixbuf;
	
	x*=((x<0)?-1:1);
	y*=((y<0)?-1:1);
  if(rows<=y)
		y=y%rows;
  if(columns<=x)
		x=x%columns;
	x*=((x<0)?-1:1);
	y*=((y<0)?-1:1);
	
	pixbuf = targa_rgba + y*columns*4;
	
	pixbuf+=(x*4);
	pixbuf+=2;
	return *pixbuf;
}

static void setImageB(byte *targa_rgba, int x, int y, int columns, int rows, byte value)
{
	byte	*pixbuf;
	
	x*=((x<0)?-1:1);
	y*=((y<0)?-1:1);
	
	pixbuf = targa_rgba + y*columns*4;
	
	pixbuf+=(x*4);
	pixbuf+=2;
	*pixbuf=value;
}
//ALPHA
static byte getImageA(byte *targa_rgba, int x, int y, int columns, int rows)
{
	byte	*pixbuf;
	
	x*=((x<0)?-1:1);
	y*=((y<0)?-1:1);
	
	pixbuf = targa_rgba + y*columns*4;
	
	pixbuf+=(x*4);
	pixbuf+=3;
	return *pixbuf;
}

static void setImageA(byte *targa_rgba, int x, int y, int columns, int rows, byte value)
{
	byte	*pixbuf;
	
	x*=((x<0)?-1:1);
	y*=((y<0)?-1:1);
	
	pixbuf = targa_rgba + y*columns*4;
	
	pixbuf+=(x*4);
	pixbuf+=3;
	*pixbuf=value;
}

//RGB
static void getImageRGB(byte *targa_rgba, int x, int y, int columns, int rows, vec3_t rgb)
{
	byte	*pixbuf;
	
	x*=((x<0)?-1:1);
	y*=((y<0)?-1:1);
  //if(rows<=y)
	y=y%rows;
  //if(columns<=x)
	x=x%columns;
	//x*=((x<0)?-1:1);
	//y*=((y<0)?-1:1);
	
	pixbuf = targa_rgba + y*columns*4 + x*4;
	
	rgb[0]=*pixbuf;
	rgb[1]=*(pixbuf+1);
	rgb[2]=*(pixbuf+2);
}

static void setImageRGB(byte *targa_rgba, int x, int y, int columns, int rows, vec3_t rgb)
{
	byte	*pixbuf;
	
	//x*=((x<0)?-1:1);
	//y*=((y<0)?-1:1);
	
	pixbuf = targa_rgba + y*columns*4 + (x*4);
	
	*pixbuf=(byte)(rgb[0]);
	*(pixbuf+1)=(byte)(rgb[1]);
	*(pixbuf+2)=(byte)(rgb[2]);
}

/****************************
NO BRAINER'S BLUR
****************************/
static void blur(int columns, int rows, byte *targa_rgba)
{
	int		row, column;
	float sum;
	
	
		for(row=0; row<rows; row++) 
		{
			//pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; column++) 
			{
				sum=0;
				sum+=getImageR(targa_rgba,column-1,row-1,columns,rows);
				sum+=getImageR(targa_rgba,column,row-1,columns,rows);
				sum+=getImageR(targa_rgba,column+1,row-1,columns,rows);
				sum+=getImageR(targa_rgba,column-1,row,columns,rows);
				sum+=getImageR(targa_rgba,column,row,columns,rows);
				sum+=getImageR(targa_rgba,column+1,row,columns,rows);
				sum+=getImageR(targa_rgba,column-1,row+1,columns,rows);
				sum+=getImageR(targa_rgba,column,row+1,columns,rows);
				sum+=getImageR(targa_rgba,column+1,row+1,columns,rows);
				
				sum/=9.0f;
				
				setImageR(targa_rgba, column, row, columns, rows, (byte)sum);
				////////////////////
				sum=0;
				sum+=getImageG(targa_rgba,column-1,row-1,columns,rows);
				sum+=getImageG(targa_rgba,column,row-1,columns,rows);
				sum+=getImageG(targa_rgba,column+1,row-1,columns,rows);
				sum+=getImageG(targa_rgba,column-1,row,columns,rows);
				sum+=getImageG(targa_rgba,column,row,columns,rows);
				sum+=getImageG(targa_rgba,column+1,row,columns,rows);
				sum+=getImageG(targa_rgba,column-1,row+1,columns,rows);
				sum+=getImageG(targa_rgba,column,row+1,columns,rows);
				sum+=getImageG(targa_rgba,column+1,row+1,columns,rows);
				
				sum/=9.0f;
				
				setImageG(targa_rgba, column, row, columns, rows, (byte)sum);
				////////////////////////
				sum=0;
				sum+=getImageB(targa_rgba,column-1,row-1,columns,rows);
				sum+=getImageB(targa_rgba,column,row-1,columns,rows);
				sum+=getImageB(targa_rgba,column+1,row-1,columns,rows);
				sum+=getImageB(targa_rgba,column-1,row,columns,rows);
				sum+=getImageB(targa_rgba,column,row,columns,rows);
				sum+=getImageB(targa_rgba,column+1,row,columns,rows);
				sum+=getImageB(targa_rgba,column-1,row+1,columns,rows);
				sum+=getImageB(targa_rgba,column,row+1,columns,rows);
				sum+=getImageB(targa_rgba,column+1,row+1,columns,rows);
				
				sum/=9.0f;
				
				setImageB(targa_rgba, column, row, columns, rows, (byte)sum);
				
				// "halftoning"
				/*if((row%5==0)&&(column%5==1))
				{
					gris=0;
					gris+=red;
					gris+=green;
					gris+=blue;
					gris/=3;
					
					gris=255-gris;
					if(gris<0)
						gris=0;
						
						setImageR(targa_rgba, column, row, columns, rows, (byte)gris);
						setImageG(targa_rgba, column, row, columns, rows, (byte)gris);
						setImageB(targa_rgba, column, row, columns, rows, (byte)gris);
					
				}*/
			
			}
		}

}


/****************************
COLORED LIGHTMAP
****************************/
void whiteTextureOne(int columns, int rows, byte *targa_rgba){
	//byte	*pixbyf;
	int		row, column;
	long	rMean=0, gMean=0, bMean=0;
	int		pixels=0;

	for(row=0;row<rows;row++){
		for(column=0;column<columns;column++){
			// Don't count fully transparent pixels
			if(getImageA(targa_rgba,column,row,columns,rows)==0)
				continue;
			// Sum pixels values
			rMean+=getImageR(targa_rgba,column,row,columns,rows);
			gMean+=getImageG(targa_rgba,column,row,columns,rows);
			bMean+=getImageB(targa_rgba,column,row,columns,rows);
			pixels++;
		}
	}

	// Calculate average
	if(pixels>0){
		rMean=((float)rMean/(float)pixels);
		gMean=((float)gMean/(float)pixels);
		bMean=((float)bMean/(float)pixels);
	}
	else{
		return;
	}

	for(row=0;row<rows;row++){
		for(column=0;column<columns;column++){
			if(getImageA(targa_rgba,column,row,columns,rows)<32)
				continue;
			setImageR(targa_rgba,column,row,columns,rows,rMean);
			setImageG(targa_rgba,column,row,columns,rows,gMean);
			setImageB(targa_rgba,column,row,columns,rows,bMean);
		}
	}
}

int diffSquare(int mean, int val){
	float variance = (val-mean)/255.0f;
	float radius = mean<128?mean:255-mean;
	return mean+(radius*variance);
}

/****************************
DECONTRAST
****************************/
void whiteTextureTwo(int columns, int rows, byte *targa_rgba){
	int		row, column;
	long	rMean=0, gMean=0, bMean=0;
	int r=0, g=0, b=0;
	int		pixels=0;

	
	for(row=0;row<rows;row++){
		for(column=0;column<columns;column++){
			// Don't count fully transparent pixels
			if(getImageA(targa_rgba,column,row,columns,rows)<32)
				continue;
			// Sum pixels values
			rMean+=getImageR(targa_rgba,column,row,columns,rows);
			gMean+=getImageG(targa_rgba,column,row,columns,rows);
			bMean+=getImageB(targa_rgba,column,row,columns,rows);
			pixels++;
		}
	}

	// Calculate average
	if(pixels>0){
		rMean=rMean/pixels;
		gMean=gMean/pixels;
		bMean=bMean/pixels;
	}
	else{
		return;
	}
	

	for(row=0;row<rows;row++){
		for(column=0;column<columns;column++){
			if(getImageA(targa_rgba,column,row,columns,rows)<32)
				continue;
			r=getImageR(targa_rgba,column,row,columns,rows);
			g=getImageG(targa_rgba,column,row,columns,rows);
			b=getImageB(targa_rgba,column,row,columns,rows);
			
			setImageR(targa_rgba,column,row,columns,rows,diffSquare(rMean,r));
			setImageG(targa_rgba,column,row,columns,rows,diffSquare(gMean,g));
			setImageB(targa_rgba,column,row,columns,rows,diffSquare(bMean,b));
			
		}
	}
}

/****************************
KUWAHARA ,FAILS SOMEWHERE
****************************/
#define KWH_RADIUS 2
static void mean_variance(int x0, int y0, int x1, int y1, int columns, int rows, byte *targa_rgba, vec4_t mv )
{
	short min=255*3, max=0;
	unsigned short count= 0;
	short row, column;
	unsigned short value;
	vec3_t rgb;
	
	mv[0]=mv[1]=mv[2]=mv[3]=0;

	for(row=y0;row<=y1;row++)
	{
		for(column=x0;column<=x1;column++)
		{
			getImageRGB(targa_rgba,column,row,columns,rows,rgb);
			
			VectorAdd(mv,rgb,mv);
			
			count++;
			value=rgb[0]+rgb[1]+rgb[2];
			if(value<min) min=value;
			if(value>max) max=value;
		}
	}

	mv[0]/=count;
	mv[1]/=count;
	mv[2]/=count;
	mv[3]= (max-min)/3.0f;
}


static void rgb_kuwahara(int x, int y, int columns, int rows, byte *targa_rgba, vec4_t bmv)
{
  vec4_t mv;
	bmv[0]=bmv[1]=bmv[2]=bmv[3]=255;
	
	mean_variance(x-KWH_RADIUS, y-KWH_RADIUS, x, y, columns, rows, targa_rgba, mv);
	if( mv[3] < bmv[3] )
	{
		Vector4Copy(mv,bmv);
	}
	
	mean_variance(x, y-KWH_RADIUS, x+KWH_RADIUS, y, columns, rows, targa_rgba, mv);
	if( mv[3] < bmv[3] )
	{
		Vector4Copy(mv,bmv);
	}
	
	mean_variance(x, y, x+KWH_RADIUS, y+KWH_RADIUS, columns, rows, targa_rgba, mv);
	if( mv[3] < bmv[3] )
	{
		Vector4Copy(mv,bmv);
	}
	
	mean_variance(x-KWH_RADIUS, y, x, y+KWH_RADIUS, columns, rows, targa_rgba, mv);
	if( mv[3] < bmv[3] )
	{
		Vector4Copy(mv,bmv);
	}
}

static void kuwahara(int columns, int rows, byte *targa_rgba){
	int		row, column;
	vec4_t rgbv;
	
	for(row=0;row<rows;row++){
		for(column=0;column<columns;column++){
			rgb_kuwahara(column, row, columns, rows, targa_rgba, rgbv);
			setImageRGB(targa_rgba,column,row,columns,rows,rgbv);
		}
	}
}


#define FLT_MAX		3.40282346638528860000e+38
static void kuwahara3(int columns, int rows, byte *targa_rgba)
{
	byte channel;
	int size = 10;
	int index1,index2;
	int width = columns-4;
	int height = rows-4;
	int size2 = (size+1)/2;
	int offset = (size-1)/2;
	const int width2 = columns + offset;
	const int height2 = rows + offset;
	int x1start = 4;
	int y1start = 4;
	int x2, y2;
	int sum, sum2, n, v=0, xbase, ybase;
	int y1,x1;
	int xbase2=0, ybase2=0;
	float var, min;
	float** mean, **variance;

	//blur(columns, rows, targa_rgba);

	// I hate malloc I hate malloc I hate malloc I hate malloc I hate malloc I hate malloc 
	mean = (float**)malloc(sizeof(float*)*width2);
	for(index1=0;index1<width2;index1++)
		mean[index1] = (float*)malloc(sizeof(float)*height2);

	variance = (float**)malloc(sizeof(float*)*width2);
	for(index2=0;index2<width2;index2++)
		variance[index2] = (float*)malloc(sizeof(float)*height2);

	// For each channel (R,G,B)
	// for(channel=0;channel<2;channel++)
	// FTL
	for(channel=0;channel<3;channel++){
		for (y1=y1start-offset; y1<y1start+height; y1++) {

			for (x1=x1start-offset; x1<x1start+width; x1++) {
				sum=0; sum2=0; n=0;
				for (x2=x1; x2<x1+size2; x2++) {
					for (y2=y1; y2<y1+size2; y2++) {
						//v = i(x2, y2);
						switch(channel){
							case 0:
								v = getImageR(targa_rgba,x2,y2,columns,rows);
								break;
							case 1:
								v = getImageG(targa_rgba,x2,y2,columns,rows);
								break;
							case 2:
								v = getImageB(targa_rgba,x2,y2,columns,rows);
								break;
						}
						//v = *targa_rgba + y2*columns*4+x2*4;
						v/=10;
						v*=10;
						sum += v;
						sum2 += v*v;
						n++;
					}
				}
				//cerr << "Accedo" << endl;
				mean[x1+offset][y1+offset] = (float)(sum/n);
				variance[x1+offset][y1+offset] = (float)((n*sum2-sum*sum)/n);
			}
		}

		for (y1=y1start; y1<y1start+height; y1++) {
			/*if ((y1%20)==0)
				cout << (0.7+0.3*(y1-y1start)/height);*/
			for (x1=x1start; x1<x1start+width; x1++) {
				min =  FLT_MAX;
				xbase = x1; ybase=y1;
				var = variance[xbase][ybase];
				if (var<min){
					min= var;
					xbase2=xbase;
					ybase2=ybase;
				}
				xbase = x1+offset;
				var = variance[xbase][ybase];
				if (var<min){
					min= var;
					xbase2=xbase;
					ybase2=ybase;
				}
				ybase = y1+offset;
				var = variance[xbase][ybase];
				if (var<min){
					min= var;
					xbase2=xbase;
					ybase2=ybase;
				}
				xbase = x1;
				var = variance[xbase][ybase];
				if (var<min){
					min= var;
					xbase2=xbase;
					ybase2=ybase;
				}
				//i(x1, y1)=(int)(mean[xbase2][ybase2]+0.5);
				switch(channel){
					case 0:
						setImageR(targa_rgba,x1,y1,columns,rows,(byte)(mean[xbase2][ybase2]+0.5));
						break;
					case 1:
						setImageG(targa_rgba,x1,y1,columns,rows,(byte)(mean[xbase2][ybase2]+0.5));
						break;
					case 2:
						setImageB(targa_rgba,x1,y1,columns,rows,(byte)(mean[xbase2][ybase2]+0.5));
						break;
				}
			}
		}
	}
	// Fuck mean & variance, this is hell (!+) Bad Religion
	for(index1=0;index1<width2;index1++)
		free(mean[index1]);
	free(mean);

	for(index2=0;index2<width2;index2++)
		free(variance[index2]);
	free(variance);
	
	//blur(columns, rows, targa_rgba);  
}

/****************************
Symmetric Nearest Neighbour
****************************/

#define SNN_RADIUS 3

static int deltaE(int l1,int a1,int b1,int l2,int a2,int b2)
{
	return (l1-l2)*(l1-l2) + (a1-a2)*(a1-a2) + (b1-b2)*(b1-b2);
}

static void snn(int columns, int rows, byte *targa_rgba)
{
	
	int row, column;
	unsigned short sumR, sumG, sumB;
	unsigned short count;
	short u, v;
	byte r, g, b;
	byte r1, g1, b1;
	byte r2, g2, b2;
	for(row=0;row<rows;row++){
		for(column=0;column<columns;column++){
			sumR=0;
			sumG=0;
			sumB=0;
			count=0;
			
			r=getImageR(targa_rgba,column,row,columns,rows);
			g=getImageG(targa_rgba,column,row,columns,rows);
			b=getImageB(targa_rgba,column,row,columns,rows);
			
			for(v=-SNN_RADIUS;v<=0;v++)
			{
				for(u=-SNN_RADIUS;u<=SNN_RADIUS;u++)
				{
					if(v==0&&u>=0) break;
					// Sum pixels values
					r1=getImageR(targa_rgba,column+u,row+v,columns,rows);
					g1=getImageG(targa_rgba,column+u,row+v,columns,rows);
					b1=getImageB(targa_rgba,column+u,row+v,columns,rows);
					
					r2=getImageR(targa_rgba,column-u,row-v,columns,rows);
					g2=getImageG(targa_rgba,column-u,row-v,columns,rows);
					b2=getImageB(targa_rgba,column-u,row-v,columns,rows);
					
					if ( deltaE(r,g,b,r1,g1,b1) < deltaE(r,g,b,r2,g2,b2))
					{
						sumR += r1;
						sumG += g1;
						sumB += b1;
					}
					else
					{
						sumR += r2;
						sumG += g2;
						sumB += b2;
					}
					count++;
				}
			}
			
			r=(byte)((int)(2*sumR+r)/(int)(2*count+1));
			g=(byte)((int)(2*sumG+g)/(int)(2*count+1));
			b=(byte)((int)(2*sumB+b)/(int)(2*count+1));
			
			setImageR(targa_rgba,column,row,columns,rows,r);
			setImageG(targa_rgba,column,row,columns,rows,g);
			setImageB(targa_rgba,column,row,columns,rows,b);
		}
	}
}

void R_ApplyCelShading(int *width, int *height, byte **pic) 
{
	switch (r_celshadalgo->integer)
	{
	case 1:
		whiteTextureOne(*width, *height, *pic);
		break;
	case 2:
		whiteTextureTwo(*width, *height, *pic);
		break;
	case 10:
		kuwahara(*width, *height, *pic);
		break;
	case 11:
		blur(*width, *height, *pic);
		kuwahara(*width, *height, *pic);
		break;
	case 12:
		kuwahara(*width, *height, *pic);
		blur(*width, *height, *pic);
		break;
	case 13:
		blur(*width, *height, *pic);
		kuwahara(*width, *height, *pic);
		blur(*width, *height, *pic);
		break;
	case 20:
		snn(*width, *height, *pic);
		break;
	case 21:
		blur(*width, *height, *pic);
		snn(*width, *height, *pic);
		break;
	case 22:
		snn(*width, *height, *pic);
		blur(*width, *height, *pic);
		break;
	case 23:
		blur(*width, *height, *pic);
		snn(*width, *height, *pic);
		blur(*width, *height, *pic);
		break;
	default:
		break;
	}
}