#include <vcl.h>
#define MAX_DIRECT 1000
#define MAX_FILE 5000
#define MaxObj 5000
// ---------------------------------------------------------------------------
struct FrameSigmaSet
{
	WORD **Kadr;		//C�� ����
	WORD **FiltKadr;    //������������� ���� ?
	float x,y;			//�������������� ����������
	int Is;				//������� ����� ���?
	int Is2;
	int N;				//���������� �������� ����� ���?
	int N2;
	int Ispic;			//����� ����� �������
	int Height;         //������ �����
	int Width;          //������ �����
	int HWframe;        //������ ����
	WORD **Frame;		//������ ��� ������������
	float ms;			//���������� (�� �������� �����)
	float mm;			//�� ����������� (�� �������� �����)
	int CMmMs;          //�������������� � ����������
};

struct BoxMmMsStr
{
	float x,y;			//�������������� ����������
	float ms;			//���������� (�� �������� �����)
	float mm;			//�� ����������� (�� �������� �����)
	int ColKad;         //���������� ������������ ������
};

//�������� ��������� ��� �������� ������ � ������
struct FullSigmaTable
{
	float x,y;			//�������������� ����������
	float xS,yS;        //���������� �� ����������� �����
	int Is;				//������������ ������� ����� ����������� �� ���������
	float Mu;           //�������� ����������� �����
	float ms;			//�� ���������� (�� �������� �����)
	float mm;			//�� ����������� (�� �������� �����)
	float SigX, SigY;   //����� �� � � �
	AnsiString Names;	//��� ����� ��� ��������� �� � ��
	int mark;			//�������������� � ������������ ������ (�������)
	int IsF;				//������������ ������� ����� ���������� ����������� �� ��������� (��������)
	int Nframe;
};

struct SBoxXY
{
	float x, y;
	float ms;
	float mm;
	int N;
	TColor ColGraf;
};

#ifdef GLOBVAL
	AnsiString DirNames[MAX_DIRECT];
	AnsiString FileNames[MAX_FILE];
	int DirCount;
	int FilesCount;
	FrameSigmaSet SigmaSet;
	int F4Lines;
	int Is_fot;
	float BufLOCdSKO;
	float BufLOC1;
	int MeanPor;
	WORD **BlackKadr;
	WORD **BlackKadrWindow;
	bool BKflag;

	unsigned int  NelLoc[MaxObj], BRloc[MaxObj];
	float Xloc[MaxObj], Yloc[MaxObj];
	unsigned short Nobj;

	void FindDirectories(AnsiString FindDir, AnsiString MaskF);     //����� �����
	void FindFiles(AnsiString MaskF);								//����� ������ � ������
	void IdentifFromName(char *name, char *Ind1, char *Ind2, float *Rez);
	void LocalNI (unsigned short *DATA_ALLOC, int Width, int Height, int BRmin, int BRmax, int NPmin, int NPmax);
	void SelectMaxObject(WORD **Kadr, int Wsize, int Hsize, float Xcor, float Ycor, bool SaveFrame);
	void Filter4Lines(int koff, WORD **Kadr, int Wsize, int Hsize);

#else
	extern AnsiString DirNames[MAX_DIRECT];
	extern AnsiString FileNames[MAX_FILE];
	extern int DirCount;
	extern int FilesCount;
	extern FrameSigmaSet SigmaSet;
	extern int F4Lines;
	extern int Is_fot;
	extern float BufLOCdSKO;
	extern float BufLOC1;
	extern int MeanPor;
	extern WORD **BlackKadr;
	extern WORD **BlackKadrWindow;
	extern bool BKflag;

	extern unsigned int NelLoc[MaxObj], BRloc[MaxObj];
	extern float Xloc[MaxObj], Yloc[MaxObj];
	extern unsigned short Nobj;

	extern void FindDirectories(AnsiString FindDir, AnsiString MaskF);	//����� �����
	extern void FindFiles(AnsiString MaskF);							//����� ������ � ������
	extern void IdentifFromName(char *name, char *Ind1, char *Ind2, float *Rez);
	extern void LocalNI (unsigned short *DATA_ALLOC, int Width, int Height, int BRmin, int BRmax, int NPmin, int NPmax);
	extern void SelectMaxObject(WORD **Kadr, int Wsize, int Hsize, float Xcor, float Ycor, bool SaveFrame);
	extern void Filter4Lines(int koff, WORD **Kadr, int Wsize, int Hsize);
#endif


//��������� ���������: (DELETE NOW)
//
//1. ����������� ��������� ��������� ������������ �������� �������� �����������  // �������� ������ ��������

