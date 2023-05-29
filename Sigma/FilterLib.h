struct ImageSet
{
  unsigned short  Width, Height;
  unsigned short *BufSrc;
};

struct WindowSet
{
  unsigned short NumWin;
  unsigned short *Xstart, *Ystart, *Xfin, *Yfin, (*FragYX)[2],*MultXY;
  unsigned short *BufWin;
  unsigned char IsRevers;
};

struct FilterSet
{
  unsigned short FltSize, FltPix;
  unsigned char  IsPix, IsFrame, IsRevers;
  unsigned char  NumTh;
  unsigned short (*TabTh)[2];
};

struct FilterRes
{
  double MeanSrc, MeanFlt, SigmaFlt;
  unsigned short *BufPix, *BufFrame;
  unsigned long   PixCount;
};

//Модель фильтрации в режиме НО/ТО для ОГ (мБОКЗ-2)
void LoadIniOG_F32(FilterSet *Filter);
void BinaryOG_F32 (ImageSet Image, unsigned short BinPow, unsigned short *BinFrame);
void FilterOG_F32(ImageSet Image, FilterSet Filter, FilterRes *Res);

//Модель фильтрации в режиме НО/ТО для ОГ-ВТ
void LoadIniOG_F125(FilterSet *Filter);
void BinaryOG_F125 (ImageSet Image, unsigned short BinPow, unsigned short *BinFrame);
void FilterOG_F125(ImageSet Image, FilterSet Filter, FilterRes *Res);

//
void GetLinesFromFrame(ImageSet Image, unsigned short(*TabBlocks)[2],
                        unsigned short NumBlocks, unsigned short *BufLines);
void GetWinFromLinesOG_F32(ImageSet Image, WindowSet *Window);
void GetWinFromLinesOG_F125(ImageSet Image, WindowSet *Window);                        

void FilterOG_N  (ImageSet Image, FilterSet Filter, FilterRes *Res);
void FilterLineN1(ImageSet Image, FilterSet Filter, FilterRes *Res);
void FilterLineN2(ImageSet Image, FilterSet Filter, FilterRes *Res);
void FilterMeanN (ImageSet Image, FilterSet Filter, FilterRes *Res);
void FilterCNT_N (ImageSet Image, FilterSet Filter, FilterRes *Res);
