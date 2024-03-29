#include <math.h>
#include <System.hpp>
#include "FilterLib.h"

void GetLinesFromFrame(ImageSet Image, unsigned short(*TabBlocks)[2],
                        unsigned short NumBlocks, unsigned short *BufLines)
{
unsigned short NumLin=0;
  for (int i=0; i<NumBlocks; i++)
  {
    for (int j=TabBlocks[i][0]; j<TabBlocks[i][0]+TabBlocks[i][1]; j++)
    {
      Move(&Image.BufSrc[j*Image.Width],&BufLines[NumLin*Image.Width],Image.Width*sizeof(short));
      NumLin++;
    }
  }
}

#define DEF_WIN_OG32 24
void GetWinFromLinesOG_F32(ImageSet Image, WindowSet *Window)
{
unsigned short *row_buf, WinX, WinY;
unsigned long Shift;

  Shift=0;
  for (int k=0; k<Window->NumWin; k++)
  {
    WinX=((~Window->MultXY[k])&0xF)>>2;
    WinX=(WinX+1)*DEF_WIN_OG32;
    WinY=((~Window->MultXY[k])&0xF);
    WinY=(WinY+1)*DEF_WIN_OG32;

    for (int i=Window->FragYX[k][0]; i>Window->FragYX[k][0]-WinY; i--)
    {
      row_buf=(unsigned short *) (&Image.BufSrc[i*Image.Width]);
      for (int j=Window->FragYX[k][1]; j>Window->FragYX[k][1]-WinX; j--)
      {
        Window->BufWin[Shift]=row_buf[j];
        Shift++;
      }
    }
  }
}

void GetWinFromLinesOG_F125(ImageSet Image, WindowSet *Window)
{
unsigned short *row_buf;
unsigned long Shift;

  Shift=0;
  for (int k=0; k<Window->NumWin; k++)
  {
    for (int i=Window->Ystart[k]; i<=Window->Yfin[k]; i++)
    {
      row_buf=(unsigned short *) (Image.BufSrc[i*Image.Width]);
      for (int j=Window->Xstart[k]; j<=Window->Xfin[k]; j++)
      {
        Window->BufWin[Shift]=row_buf[j];
        Shift++;
      }
    }
  }
}

double sqrtm(double xf)
{
  if (xf<1e-20)
    return 0.;
  else
    return sqrt(xf);
}

void BinaryOG_F32(ImageSet Image, unsigned short BinPow, unsigned short *BinFrame)
{
unsigned short *frame_raw, *frame_bin, *sum_bin, sum_pix;
unsigned short WidthBin, BinPix;

  WidthBin=Image.Width>>BinPow;
  BinPix=BinPow*2;

  sum_bin= new unsigned short [WidthBin];
  for (int j=0; j<WidthBin; j++)
  {
    sum_bin[j]=0;
  }

  for (int i=0,i_bin=0; i<Image.Height; i+=BinPix, i_bin++)
  {
    frame_bin=(unsigned short *) (BinFrame+WidthBin*i_bin);
    for (int l=0; l<BinPix; l++) //���� �� ������ �����������
    {
      frame_raw=(unsigned short *) (Image.BufSrc+Image.Width*(i+l));

      for (int j=0,j_bin=0; j<Image.Width; j+=BinPix, j_bin++)
      {
        sum_pix=0;
        for (int k=0; k<BinPix; k++)  //��������� ��������������� BinPix � ������
        {
          sum_pix+=frame_raw[j+k];
        }
        sum_pix>>=BinPow; //�������� ��� � ����
        sum_bin[j_bin]+=sum_pix;
      }
    }
    for (int j_bin=0; j_bin<WidthBin; j_bin++)
    {
        frame_bin[j_bin]=sum_bin[j_bin]>>BinPow; //�������� ��� � ����
        sum_bin[j_bin]=0;
    }
  }
  delete [] sum_bin;
}

void BinaryOG_F125(ImageSet Image, unsigned short BinPow, unsigned short *BinFrame)
{
unsigned short *frame_raw, *frame_bin, *sum_bin, sum_pix;
unsigned short WidthBin, BinPix;

  WidthBin=Image.Width>>BinPow;
  BinPix=BinPow*2;

  sum_bin= new unsigned short [WidthBin];
  for (int j=0; j<WidthBin; j++)
  {
    sum_bin[j]=0;
  }

  for (int i=0,i_bin=0; i<Image.Height; i+=BinPix, i_bin++)
  {
    frame_bin=(unsigned short *) (BinFrame+WidthBin*i_bin);
    for (int l=0; l<BinPix; l++) //���� �� ������ �����������
    {
      frame_raw=(unsigned short *) (Image.BufSrc+Image.Width*(i+l));

      for (int j=0,j_bin=0; j<Image.Width; j+=BinPix, j_bin++)
      {
        sum_pix=0;
        for (int k=0; k<BinPix; k++)  //��������� ��������������� BinPix � ������
        {
          sum_pix+=frame_raw[j+k];
        }
        sum_bin[j_bin]+=sum_pix;
      }
    }
    for (int j_bin=0; j_bin<WidthBin; j_bin++)
    {
        frame_bin[j_bin]=sum_bin[j_bin]; 
        sum_bin[j_bin]=0;
    }
  }

  delete [] sum_bin;
}

//������ ������� �� (�����-2)
#define FLT_SIZE_OG32 16
#define FLT_PIX_OG32   4
#define NUM_TH_OG32    4

unsigned short CMV4000[NUM_TH_OG32][2]={{255,16},{511,32},{1535,64},{4095,128}};

void  LoadIniOG_F32(FilterSet *Filter)
{
  Filter->FltSize=FLT_SIZE_OG32;
  Filter->FltPix =FLT_PIX_OG32;
  Filter->NumTh  =NUM_TH_OG32;
  for (int i=0; i<NUM_TH_OG32; i++)
  {
    Filter->TabTh[i][0]=CMV4000[i][0];
    Filter->TabTh[i][1]=CMV4000[i][1];
  }
}

void FilterOG_F32(ImageSet Image, FilterSet Filter, FilterRes *Res)
{
unsigned long sum_col, ThCur, Shift;
int Nflt_h, fl_obj, fl_buf, fl_find, k;
unsigned short i_save, j_save, br_save;
float var, mean_cur;
unsigned short *frame_raw, *frame_flt;
unsigned long PixFrame;

  Res->MeanSrc=0.; Res->MeanFlt=0.; Res->SigmaFlt=0.;
  Nflt_h=Filter.FltSize>>1;
  PixFrame=Image.Width*Image.Height;
  for (int i=0;i<PixFrame;i++)
  {
      Res->MeanSrc+=(double)Image.BufSrc[i];
  }
  if (Filter.IsFrame)
  {
    for (int i=0;i<PixFrame;i++)
    {
      Res->BufFrame[i]=0;
    }
  }
  Res->MeanSrc/=(double)(PixFrame);

  Res->PixCount=0;
  PixFrame=0;
  for (int i=Image.Height-1; i>=0; i--) //Ok
  {
    //� ������ ������ ���������� ������� �������� �������
    fl_obj=0;
    fl_buf=0;

    frame_raw=(unsigned short *) (Image.BufSrc+Image.Width*i);
    if (Filter.IsFrame)
    {
      frame_flt=(unsigned short *) (Res->BufFrame+Image.Width*i);
    }
    for (int j=Image.Width-Nflt_h; j>=Nflt_h;j=j-2) //Ok
    {
      sum_col=0;
      for (k=j-Nflt_h;k<j-Nflt_h+Filter.FltPix;k++)
        sum_col+=frame_raw[k]+frame_raw[k+Filter.FltSize-Filter.FltPix];

      mean_cur=sum_col/(double)(2*Filter.FltPix);

      k=0; fl_find=0;
      while ((!fl_find)&&(k<Filter.NumTh))
      {
        if (mean_cur<Filter.TabTh[k][0])
        {
          ThCur=Filter.TabTh[k][1];
          fl_find=1;
        }
        k++;
      }
      for (k=j;k>=j-1;k--)   //Ok
      {
        var=(double) (frame_raw[k]-mean_cur);
        Res->MeanFlt+=(double)var;
        Res->SigmaFlt+=(double)var*(double)var;
        PixFrame++;

        if (var>ThCur)
        {
          if (fl_obj)
          {
            if (fl_buf)
            {
              if (Filter.IsFrame)
              {
                frame_flt[j_save]=br_save;
              }
              if (Filter.IsPix)
              {
              //��������� ������� �� ������
                Shift=3*(Res->PixCount);
                Res->BufPix[Shift]  =i;       //Y
                Res->BufPix[Shift+1]=j_save;  //X
                Res->BufPix[Shift+2]=br_save;
              }
              fl_buf=0;
              Res->PixCount++;
            }
            if (Filter.IsFrame)
            {
              frame_flt[k]=(unsigned short)(var+0.5);
            }
            if (Filter.IsPix)
            {
              //��������� X, Y, BR �������� �������
              Shift=3*(Res->PixCount);
              Res->BufPix[Shift]  =i;   //Y
              Res->BufPix[Shift+1]=k;   //X  ;
              Res->BufPix[Shift+2]=(unsigned short)(var+0.5);
            }
            Res->PixCount++;
          }
          else
          {
            //��������� � �����
            fl_buf=1;
            j_save=k;
            br_save=(unsigned short)(var+0.5);
          }
          fl_obj=1;
        }
        else
        {
          if (Filter.IsFrame)
          {
            frame_flt[k]=0;
          }
          fl_obj=0;
          fl_buf=0;
        }
      }
    }
  }

  if (PixFrame)
  {
    Res->MeanFlt/=(double)PixFrame;
    Res->SigmaFlt=sqrtm(Res->SigmaFlt/(double)PixFrame-Res->MeanFlt*Res->MeanFlt);
  }
}

//������ ������� ��-��
#define FLT_SIZE_OG125 32
#define FLT_PIX_OG125   8
#define NUM_TH_OG125    6

//unsigned short CMV20000[NUM_TH_OG125][2]={{127,5},{255,8},{511,12},{1023,16},{2047,32},{4095,40}};
unsigned short CMV20000[NUM_TH_OG125][2]={{127,30},{255,48},{511,72},{1023,96},{2047,192},{4095,240}};

void  LoadIniOG_F125(FilterSet *Filter)
{
  Filter->FltSize=FLT_SIZE_OG125;
  Filter->FltPix =FLT_PIX_OG125;
  Filter->NumTh  =NUM_TH_OG125;
  for (int i=0; i<NUM_TH_OG125; i++)
  {
    Filter->TabTh[i][0]=CMV20000[i][0];
    Filter->TabTh[i][1]=CMV20000[i][1];
  }
}

void FilterOG_F125(ImageSet Image, FilterSet Filter, FilterRes *Res)
{
unsigned long sum_col, ThCur, Shift;
int Nflt_h, fl_obj, fl_buf, fl_find, k;
unsigned short i_save, j_save, br_save, br_cur;
unsigned short *frame_raw, *frame_flt;
double var, mean_cur;
unsigned long PixFrame;

  Res->MeanSrc=0.; Res->MeanFlt=0.; Res->SigmaFlt=0.;
  Nflt_h=Filter.FltSize>>1;
  PixFrame=Image.Width*Image.Height;
  for (int i=0;i<PixFrame;i++)
  {
      Res->MeanSrc+=(double)Image.BufSrc[i];
  }
  if (Filter.IsFrame)
  {
    for (int i=0;i<PixFrame;i++)
    {
      Res->BufFrame[i]=0;
    }
  }
  Res->MeanSrc/=(double)(PixFrame);

  Res->PixCount=0;
  PixFrame=0;
  for (int i=0; i<Image.Height; i++)
  {
    //� ������ ������ ���������� ������� �������� �������
    fl_obj=0;
    fl_buf=0;

    frame_raw=(unsigned short *) (Image.BufSrc+Image.Width*i);
    if (Filter.IsFrame)
    {
      frame_flt=(unsigned short *) (Res->BufFrame+Image.Width*i);
    }
    for (int j=Nflt_h;j<Image.Width-Nflt_h+1;j=j+2)     //+1 (!!!)
    {
      if (j==Nflt_h)
      {
        sum_col=0;
        for (k=j-Nflt_h;k<j-Nflt_h+Filter.FltPix;k++) //+ ???
          sum_col+=frame_raw[k]+frame_raw[k+Filter.FltSize-Filter.FltPix];
      }
      else
        sum_col+=frame_raw[j+Nflt_h-1]+frame_raw[j+Nflt_h-2]
                +frame_raw[j-Nflt_h-1+Filter.FltPix]
                +frame_raw[j-Nflt_h-2+Filter.FltPix]
                -frame_raw[j-Nflt_h-1]-frame_raw[j-Nflt_h-2]
                -frame_raw[j+Nflt_h-1-Filter.FltPix]
                -frame_raw[j+Nflt_h-2-Filter.FltPix];

      mean_cur=sum_col/(double)(2*Filter.FltPix);

      k=0; fl_find=0;
      while ((!fl_find)&&(k<Filter.NumTh))
      {
        if (mean_cur<=Filter.TabTh[k][0])
        {
          ThCur=Filter.TabTh[k][1];
          fl_find=1;
        }
        k++;
      }
      for (k=j-1;k<=j;k++)
      {
        var=(double) (frame_raw[k]-mean_cur);
        Res->MeanFlt+=(double)var;
        Res->SigmaFlt+=(double)var*(double)var;
        PixFrame++;

        if (frame_raw[k]>ThCur+(unsigned short)mean_cur)
        {
          br_cur=frame_raw[k]-(unsigned short)mean_cur;
          if (fl_obj)
          {
            if (fl_buf)
            {
              if (Filter.IsFrame)
              {
                frame_flt[j_save]=br_save;
              }
              if (Filter.IsPix)
              {
              //��������� ������� �� ������
                Shift=3*(Res->PixCount);
                Res->BufPix[Shift]  =j_save;  //X
                Res->BufPix[Shift+1]=i;       //Y
                Res->BufPix[Shift+2]=br_save;
              }
              fl_buf=0;
              Res->PixCount++;
            }

            if (Filter.IsFrame)
            {
              frame_flt[k]=br_cur;
            }
            if (Filter.IsPix)
            {
              //��������� X, Y, BR �������� �������
              Shift=3*(Res->PixCount);
              Res->BufPix[Shift]  =k;   //X
              Res->BufPix[Shift+1]=i;   //Y
              Res->BufPix[Shift+2]=br_cur;
            }
            Res->PixCount++;
          }
          else
          {
            //��������� � �����
            fl_buf=1;
            j_save=k;
            br_save=br_cur;
          }
          fl_obj=1;
        }
        else
        {
          if (Filter.IsFrame)
          {
            frame_flt[k]=0;
          }
          fl_obj=0;
          fl_buf=0;
        }
      }
    }
  }

  if (PixFrame)
  {
    Res->MeanFlt/=(double)PixFrame;
    Res->SigmaFlt=sqrtm(Res->SigmaFlt/(double)PixFrame-Res->MeanFlt*Res->MeanFlt);
  }
}

//������������� ������ �� (Y, X, BR)
#define CellSize 3
void ReversArr(unsigned short *BufSrc, unsigned long CellCount)
{
unsigned short *Source, *Dest, *SaveBuf, *BufEnd;
unsigned long Shift, BufSize;

  SaveBuf = new unsigned short [CellSize];
  BufEnd  =(unsigned short*)(BufSrc+(CellCount-1)*CellSize);
  BufSize = CellSize*sizeof(short);

  for (int i=0; i<(CellCount>>1); i++)
  {
    Shift=i*CellSize;
    Source=(unsigned short *)(BufSrc+Shift);
    Dest  =(unsigned short *)(BufEnd-Shift);
    Move(Source, SaveBuf, BufSize);
    Move(Dest, Source, BufSize);
    Move(SaveBuf, Dest, BufSize);
  }
  delete [] SaveBuf;
}

void FilterOG_N(ImageSet Image, FilterSet Filter, FilterRes *Res)
{
unsigned long sum_col, ThCur, Shift;
int Nflt_h, fl_obj, fl_buf, fl_find, k;
unsigned short i_save, j_save, br_save;
float var, mean_cur;
unsigned short *frame_raw, *frame_flt;
unsigned long PixFrame;

  Res->MeanSrc=0.; Res->MeanFlt=0.; Res->SigmaFlt=0.;
  Nflt_h=Filter.FltSize>>1;
  PixFrame=Image.Width*Image.Height;
  for (int i=0;i<PixFrame;i++)
  {
      Res->MeanSrc+=(double)Image.BufSrc[i];
  }
  if (Filter.IsFrame)
  {
    for (int i=0;i<PixFrame;i++)
    {
      Res->BufFrame[i]=0;
    }
  }
  Res->MeanSrc/=(double)(PixFrame);

  Res->PixCount=0;
  PixFrame=0;
  for (int i=0; i<Image.Height; i++)
  {
    //� ������ ������ ���������� ������� �������� �������
    fl_obj=0;
    fl_buf=0;

    frame_raw=(unsigned short *) (Image.BufSrc+Image.Width*i);
    if (Filter.IsFrame)
    {
      frame_flt=(unsigned short *) (Res->BufFrame+Image.Width*i);
    }
    for (int j=Nflt_h;j<=Image.Width-Nflt_h;j=j+2)
    {
      sum_col=0;
      for (k=j-Nflt_h;k<j-Nflt_h+Filter.FltPix;k++)
        sum_col+=frame_raw[k]+frame_raw[k+Filter.FltSize-Filter.FltPix];

      mean_cur=sum_col/(double)(2*Filter.FltPix);

      k=0; fl_find=0;
      while ((!fl_find)&&(k<Filter.NumTh))
      {
        if (mean_cur<Filter.TabTh[k][0])
        {
          ThCur=Filter.TabTh[k][1];
          fl_find=1;
        }
        k++;
      }
      for (k=j-1;k<=j;k++)
      {
        var=(double) (frame_raw[k]-mean_cur);
        Res->MeanFlt+=(double)var;
        Res->SigmaFlt+=(double)var*(double)var;
        PixFrame++;

        if (var>ThCur)
        {
          if (fl_obj)
          {
            if (fl_buf)
            {
              if (Filter.IsFrame)
              {
                frame_flt[j_save]=br_save;
              }
              if (Filter.IsPix)
              {
              //��������� ������� �� ������
                Shift=3*(Res->PixCount);
                Res->BufPix[Shift]  =i;       //Y
                Res->BufPix[Shift+1]=j_save;  //X
                Res->BufPix[Shift+2]=br_save;
              }
              fl_buf=0;
              Res->PixCount++;
            }
            if (Filter.IsFrame)
            {
              frame_flt[k]=(unsigned short)(var+0.5);
            }
            if (Filter.IsPix)
            {
              //��������� X, Y, BR �������� �������
              Shift=3*(Res->PixCount);
              Res->BufPix[Shift]  =i;   //Y
              Res->BufPix[Shift+1]=k;   //X  ;
              Res->BufPix[Shift+2]=(unsigned short)(var+0.5);
            }
            Res->PixCount++;
          }
          else
          {
            //��������� � �����
            fl_buf=1;
            j_save=k;
            br_save=(unsigned short)(var+0.5);
          }
          fl_obj=1;
        }
        else
        {
          if (Filter.IsFrame)
          {
            frame_flt[k]=0;
          }
          fl_obj=0;
          fl_buf=0;
        }
      }
    }
  }

  if (PixFrame)
  {
    Res->MeanFlt/=(double)PixFrame;
    Res->SigmaFlt=sqrtm(Res->SigmaFlt/(double)PixFrame-Res->MeanFlt*Res->MeanFlt);
  }

//  if (Filter.IsRevers) ReversArr(Res->BufPix, Res->PixCount);
}

void FilterLineN1(ImageSet Image, FilterSet Filter, FilterRes *Res)
{
unsigned int sum_col;
int Nflt_h;
float var;
unsigned short *frame_raw, *frame_flt;
unsigned long PixFrame;

  Res->MeanSrc=0.; Res->MeanFlt=0.; Res->SigmaFlt=0.;
  Nflt_h=Filter.FltSize>>1;
  PixFrame=Image.Width*Image.Height;
  for (int i=0;i<PixFrame;i++)
  {
      Res->MeanSrc+=(double)Image.BufSrc[i];
      Res->BufFrame[i]=0; //!!!
  }
  Res->MeanSrc/=(double)(PixFrame);

  Res->PixCount=0;
  PixFrame=0;
  for (int i=0; i<Image.Height; i++)
  {
    frame_raw=(unsigned short *) (Image.BufSrc+Image.Width*i);
    frame_flt=(unsigned short *) (Res->BufFrame+Image.Width*i);


    for (int j=Nflt_h;j<Image.Width-Nflt_h;j++)
    {
      if (j==Nflt_h)
      {
        sum_col=0;
        for (int k=0;k<Filter.FltSize;k++)
        {
          sum_col+=frame_raw[k];
        }
      }
      else
        sum_col+=frame_raw[j+Nflt_h]-frame_raw[j-Nflt_h-1];

      var=(double) (frame_raw[j]-sum_col/(double)(Filter.FltSize));
      Res->MeanFlt+=(double)var;
      Res->SigmaFlt+=(double)var*(double)var;
      PixFrame++;
      if (var>0) frame_flt[j]=(unsigned short)(var+0.5);
      else frame_flt[j]=0;
    }
  }

  if (PixFrame)
  {
    Res->MeanFlt/=(double)PixFrame;
    Res->SigmaFlt=sqrtm(Res->SigmaFlt/(double)PixFrame-Res->MeanFlt*Res->MeanFlt);
  }
}

void FilterLineN2(ImageSet Image, FilterSet Filter, FilterRes *Res)
{
unsigned int sum_col;
int Nflt_h;
float var;
unsigned short *frame_raw, *frame_flt;
unsigned long PixFrame;

  Res->MeanSrc=0.; Res->MeanFlt=0.; Res->SigmaFlt=0.;
  Nflt_h=Filter.FltSize>>1;

  PixFrame=Image.Width*Image.Height;
  for (int i=0;i<PixFrame;i++)
  {
      Res->MeanSrc+=(double)Image.BufSrc[i];
      Res->BufFrame[i]=0; //!!!
  }
  Res->MeanSrc/=(double)(PixFrame);

  Res->PixCount=0;
  PixFrame=0;
  for (int i=0; i<Image.Height; i++)
  {
    frame_raw=(unsigned short *) (Image.BufSrc+Image.Width*i);
    frame_flt=(unsigned short *) (Res->BufFrame+Image.Width*i);
    for (int j=Nflt_h;j<Image.Width-Nflt_h;j=j+2)
    {
      if (j==Nflt_h)
      {
        sum_col=0;
        for (int k=0;k<Filter.FltSize;k++)
          sum_col+=frame_raw[k];
      }
      else  sum_col=sum_col+frame_raw[j+Nflt_h-1]+frame_raw[j+Nflt_h-2]
                    -frame_raw[j-Nflt_h-1]-frame_raw[j-Nflt_h-2];

      for (int k=0;k<2;k++)
      {
        var=(double) (frame_raw[j-k]-sum_col/(double)(Filter.FltSize));
        Res->MeanFlt+=(double)var;
        Res->SigmaFlt+=(double)var*(double)var;
        PixFrame++;
        if (var>0) frame_flt[j-k]=(unsigned short)(var+0.5);
        else frame_flt[j-k]=0;
      }
    }
  }

  if (PixFrame)
  {
    Res->MeanFlt/=(double)PixFrame;
    Res->SigmaFlt=sqrtm(Res->SigmaFlt/(double)PixFrame-Res->MeanFlt*Res->MeanFlt);
  }
}

void FilterMeanN(ImageSet Image, FilterSet Filter, FilterRes *Res)
{
unsigned int sum_row_col, *sum_row_n;
int Nflt_h, Nflt_2;
float var;
unsigned long PixFrame;
unsigned short *frame_raw_c, *frame_raw_t, *frame_raw_b, *frame_flt;

  sum_row_n= new unsigned int [Image.Width];

  Res->MeanSrc=0.; Res->MeanFlt=0.; Res->SigmaFlt=0.;
  Nflt_h=Filter.FltSize>>1;
  Nflt_2=Filter.FltSize*Filter.FltSize;
  PixFrame=Image.Width*Image.Height;
  for (int i=0;i<PixFrame;i++)
  {
      Res->MeanSrc+=(double)Image.BufSrc[i];
      Res->BufFrame[i]=0; //!!! ��������
  }
  Res->MeanSrc/=(double)(PixFrame);

  for (int j=0;j<Image.Width;j++)
    sum_row_n[j]=0;

  for (int i=0;i<Filter.FltSize;i++)
  {
    frame_raw_c=(unsigned short *) (Image.BufSrc+Image.Width*i);
    for (int j=0;j<Image.Width;j++)
    {
      sum_row_n[j]+=frame_raw_c[j];
    }
  }

  Res->PixCount=0;
  PixFrame=0;
  for (int i=Nflt_h; i<Image.Height-Nflt_h; i++)
  {
    frame_raw_c=(unsigned short *) (Image.BufSrc+Image.Width*i);
    frame_flt=(unsigned short *) (Res->BufFrame+Image.Width*i);

    if (i>Nflt_h)
    {
      frame_raw_t=(unsigned short *) (Image.BufSrc+Image.Width*(i-Nflt_h-1));
      frame_raw_b=(unsigned short *) (Image.BufSrc+Image.Width*(i+Nflt_h));
      for (int k=0;k<Image.Width;k++)
        sum_row_n[k]=sum_row_n[k]+frame_raw_b[k]-frame_raw_t[k];
    }

    for (int j=Nflt_h;j<Image.Width-Nflt_h;j++)
    {
      if (j==Nflt_h)
      {
        sum_row_col=0;
        for (int k=0;k<Filter.FltSize;k++) sum_row_col+=sum_row_n[k];
      }
      else  sum_row_col=sum_row_col+sum_row_n[j+Nflt_h]-sum_row_n[j-Nflt_h-1];

      var=(double) (frame_raw_c[j]-sum_row_col/(double)(Nflt_2));
      Res->MeanFlt+=(double)var;
      Res->SigmaFlt+=(double)var*(double)var;
      PixFrame++;
      if (var>0) frame_flt[j]=(unsigned short)(var+0.5);
      else frame_flt[j]=0;
    }
  }

  if (PixFrame)
  {
    Res->MeanFlt/=(double)PixFrame;
	Res->SigmaFlt=sqrt(Res->SigmaFlt/(double)PixFrame-Res->MeanFlt*Res->MeanFlt);
  }
  delete [] sum_row_n;
}

void FilterCNT_N (ImageSet Image, FilterSet Filter, FilterRes *Res)
{
unsigned int sum_row_col, *sum_row_n;
int Ncl0, Nflt_h, Nflt_2;
float var;
unsigned long PixFrame;
unsigned short *frame_raw_c, *frame_raw_t, *frame_raw_b, *frame_flt;

  sum_row_n= new unsigned int [Image.Width];

  Res->MeanSrc=0.; Res->MeanFlt=0.; Res->SigmaFlt=0.;
  Ncl0=Image.Width>>1;
  Nflt_h=Filter.FltSize>>1;
  Nflt_2=Filter.FltSize*Filter.FltSize;
  PixFrame=Image.Width*Image.Height;

  for (int i=0;i<PixFrame;i++)
  {
      Res->MeanSrc+=(double)Image.BufSrc[i];
      Res->BufFrame[i]=0; //!!! ��������
  }
  Res->MeanSrc/=(double)(PixFrame);

  for (int j=0;j<Image.Width;j++)
    sum_row_n[j]=0;

  for (int i=0;i<Filter.FltSize;i++)
  {
    frame_raw_c=(unsigned short *) (Image.BufSrc+Image.Width*i);
    for (int j=0;j<Image.Width;j++)
    {
      sum_row_n[j]+=frame_raw_c[j];
    }
  }

  Res->PixCount=0;
  PixFrame=0;
  for (int i=Nflt_h; i<Image.Height-Nflt_h; i++)
  {
    frame_raw_c=(unsigned short *) (Image.BufSrc+Image.Width*i);
    frame_flt=(unsigned short *) (Res->BufFrame+Image.Width*i);

    if (i>Nflt_h)
    {
      frame_raw_t=(unsigned short *) (Image.BufSrc+Image.Width*(i-Nflt_h-1));
      frame_raw_b=(unsigned short *) (Image.BufSrc+Image.Width*(i+Nflt_h));
      for (int k=0;k<Image.Width;k++)
        sum_row_n[k]=sum_row_n[k]+frame_raw_b[k]-frame_raw_t[k];
    }

    for (int j=Nflt_h;j<Ncl0;j++)
    {
      if (j==Nflt_h)
      {
        sum_row_col=0;
        for (int k=0;k<Filter.FltSize;k++) sum_row_col+=sum_row_n[k];
      }
      else  if (j<Ncl0-Nflt_h) sum_row_col=sum_row_col+sum_row_n[j+Nflt_h]-sum_row_n[j-Nflt_h-1];

      var=(float) (frame_raw_c[j]-sum_row_col/(float)(Nflt_2));
      Res->MeanFlt+=(double)var;
      Res->SigmaFlt+=(double)var*(double)var;
      PixFrame++;
      if (var>0) frame_flt[j]=(unsigned short)(var+0.5);
      else frame_flt[j]=0;
    }

    for (int j=Ncl0+Nflt_h;j<Image.Width-Nflt_h;j++)
    {
      if (j==Ncl0+Nflt_h)
      {
        sum_row_col=0;
        for (int k=Ncl0;k<Ncl0+Filter.FltSize;k++) sum_row_col+=sum_row_n[k];

        for (int j1=Ncl0;j1<Ncl0+Nflt_h;j1++)
        {
          var=(float) (frame_raw_c[j1]-sum_row_col/(float)(Nflt_2));
          Res->MeanFlt+= (double)var;
          Res->SigmaFlt+=(double)var*(double)var;
          PixFrame++;
          if (var>0) frame_flt[j1]=(unsigned short)(var+0.5);
          else frame_flt[j1]=0;
        }
      }
      else  sum_row_col=sum_row_col+sum_row_n[j+Nflt_h]-sum_row_n[j-Nflt_h-1];

      var=(float) (frame_raw_c[j]-sum_row_col/(float)(Nflt_2));
      Res->MeanFlt+=(double)var;
      Res->SigmaFlt+=(double)var*(double)var;
      PixFrame++;
      if (var>0) frame_flt[j]=(unsigned short)(var+0.5);
      else frame_flt[j]=0;
    }
  }

  if (PixFrame)
  {
    Res->MeanFlt/=(double)PixFrame;
    Res->SigmaFlt=sqrtm(Res->SigmaFlt/(double)PixFrame-Res->MeanFlt*Res->MeanFlt);
  }

  delete [] sum_row_n;
}

