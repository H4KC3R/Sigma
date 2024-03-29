//---------------------------------------------------------------------------
#ifndef SigmaU1H
#define SigmaU1H
//---------------------------------------------------------------------------
#include <OleServer.hpp>
#include "FrontPage_XP_srvr.h"

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <pngimage.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>

#include <VCLTee.Chart.hpp>
#include <VCLTee.TeEngine.hpp>
#include <VCLTee.TeeProcs.hpp>
#include <VCLTee.Series.hpp>
//#include <VCLTee.Keyboard.hpp>
#include <VCLTee.BubbleCh.hpp>
//#include "Chart.hpp"
//#include "TeEngine.hpp"
//#include "TeeProcs.hpp"
//#include "Series.hpp"
#include <Vcl.Touch.Keyboard.hpp>
//#include "BubbleCh.hpp"
#include <FileCtrl.hpp>
#include <DockTabSet.hpp>
#include <Tabs.hpp>
#include <ImgList.hpp>
#include <System.ImageList.hpp>
#include <VclTee.TeeGDIPlus.hpp>
//#include <VclTee.TeeGDIPlus.hpp>
//int __declspec(dllimport) LocalGauss(int *xP, int *yP, int *BrP, int NumPts, int NumIt, int NumTr, double *xc, double *yc, double *sx, double *sy, double *muG);
int __declspec(dllimport) LocalGauss(int *xP, int *yP, int *BrP, int NumPts, int NumIt, int NumTr, int flagCInit,
									 double sigmaInit,  double *xc, double *yc,
									 double *sx, double *sy, double *muG, double *xi, double *yi);

//������� ������:
//xP			- ������ �� �
//yP          - ������ �� �
//BrP         - ������ ��������
//NumPts      - ����� �������
//NumIt       - ����� �������� (10)
//NumTr       - ��� ������ ������� (10)
//flagCInit   - ����� �������
//			  0 - ����������������
//			  1 - ������� ��������������
//			  2 - �������� �������
//sigmaInit   - ��������� ����� (0.5)
//
//����������:
//xc          - ����� ��������� ������������� �� �
//yc          - ����� ��������� ������������� �� �
//sx          - �������� ����� �� �
//sy          - �������� ����� �� �
//muG         - �������� ������� ������ ������������� (��� ������ - ��� ������)
//xi          - ���������� ������ ��������� � flagCInit �� �
//yi          - ���������� ������ ��������� � flagCInit �� �

//extern __declspec(dllimport) LocalGauss(int *, int *, int *, int, double *, double *, double *, double *, double *);
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TMainMenu *MainMenu1;
	TMenuItem *Exit1;
	TButton *Button1;
	TOpenDialog *OpenDialog1;
	TMemo *Memo1;
	TButton *Button4;
	TEdit *Edit23;
	TButton *Button5;
	TOpenDialog *OpenDialog2;
	TStatusBar *StatusBar1;
	TProgressBar *ProgressBar1;
	TMenuItem *N1;
	TButton *Button6;
	TOpenDialog *OpenDialog3;
	TChart *Chart1;
	TPointSeries *Series1;
	TLabel *Label18;
	TPointSeries *Series2;
	TChart *Chart2;
	TPageControl *PageControl1;
	TTabSheet *TabSheet1;
	TTabSheet *TabSheet2;
	TLabel *Label1;
	TLabel *Label5;
	TEdit *Edit2;
	TEdit *Edit1;
	TGroupBox *GroupBox3;
	TLabel *Label2;
	TLabel *Label3;
	TEdit *Edit3;
	TEdit *Edit4;
	TCheckBox *CheckBox1;
	TGroupBox *GroupBox2;
	TLabel *Label6;
	TLabel *Label7;
	TLabel *Label8;
	TLabel *Label9;
	TCheckBox *CheckBox3;
	TEdit *Edit10;
	TEdit *Edit11;
	TEdit *Edit12;
	TEdit *Edit13;
	TEdit *Edit14;
	TEdit *Edit17;
	TGroupBox *GroupBox1;
	TLabel *Label4;
	TEdit *Edit5;
	TCheckBox *CheckBox2;
	TEdit *Edit9;
	TLabel *Label10;
	TEdit *Edit15;
	TEdit *Edit16;
	TCheckBox *CheckBox4;
	TLabel *Label19;
	TEdit *Edit28;
	TRadioGroup *RadioGroup1;
	TCheckBox *CheckBox5;
	TLabel *Label20;
	TLabel *Label21;
	TGroupBox *GroupBox5;
	TEdit *Edit24;
	TEdit *Edit25;
	TEdit *Edit26;
	TButton *Button2;
	TOpenDialog *OpenDialog4;
	TDirectoryListBox *DirectoryListBox1;
	TDirectoryListBox *DirectoryListBox2;
	TButton *Button3;
	TButton *Button7;
	TButton *Button8;
	TLabel *Label17;
	TLabel *Label22;
	TLabel *Label23;
	TMenuItem *ini1;
	TMenuItem *N2;
	TMenuItem *ini2;
	TOpenDialog *OpenDialog5;
	TSaveDialog *SaveDialog1;
	TButton *Button9;
	TMemo *Memo2;
	TLabel *Label27;
	TLabel *Label28;
	TLabel *Label29;
	TLabel *Label30;
	TLabel *Label31;
	TLabel *Label32;
	TEdit *Edit32;
	TEdit *Edit33;
	TEdit *Edit34;
	TEdit *Edit35;
	TEdit *Edit36;
	TEdit *Edit37;
	TLabel *Label33;
	TEdit *Edit43;
	TLabel *Label39;
	TBubbleSeries *Series3;
	TBubbleSeries *Series4;
	TLabel *Label40;
	TCheckBox *CheckBox8;
	TLabel *Label41;
	TLineSeries *Series5;
	TLineSeries *Series6;
	TLineSeries *Series7;
	TLineSeries *Series8;
	TLineSeries *Series9;
	TMenuItem *N3;
	TChart *Chart3;
	TLineSeries *Series10;
	TLineSeries *Series11;
	TLineSeries *Series12;
	TLineSeries *Series13;
	TEdit *Edit52;
	TLabel *Label49;
	TCheckBox *CheckBox13;
	TChart *Chart4;
	TPointSeries *Series14;
	TPointSeries *Series15;
	TCheckBox *CheckBox14;
	TPageControl *PageControl2;
	TTabSheet *TabSheet3;
	TTabSheet *TabSheet4;
	TTabSheet *TabSheet5;
	TLabel *Label24;
	TLabel *Label26;
	TEdit *Edit27;
	TEdit *Edit29;
	TLabel *Label25;
	TEdit *Edit30;
	TEdit *Edit31;
	TCheckBox *CheckBox7;
	TCheckBox *CheckBox6;
	TEdit *Edit53;
	TEdit *Edit41;
	TEdit *Edit40;
	TLabel *Label37;
	TLabel *Label36;
	TLabel *Label34;
	TEdit *Edit38;
	TLabel *Label35;
	TEdit *Edit39;
	TCheckBox *CheckBox12;
	TCheckBox *CheckBox11;
	TEdit *Edit44;
	TLabel *Label42;
	TLabel *Label11;
	TLabel *Label16;
	TEdit *Edit8;
	TEdit *Edit6;
	TEdit *Edit7;
	TEdit *Edit22;
	TLabel *Label12;
	TLabel *Label13;
	TEdit *Edit18;
	TEdit *Edit42;
	TEdit *Edit46;
	TLabel *Label44;
	TLabel *Label38;
	TLabel *Label15;
	TLabel *Label45;
	TLabel *Label46;
	TEdit *Edit20;
	TEdit *Edit21;
	TEdit *Edit47;
	TEdit *Edit48;
	TEdit *Edit49;
	TCheckBox *CheckBox9;
	TCheckBox *CheckBox10;
	TEdit *Edit55;
	TEdit *Edit51;
	TLabel *Label48;
	TLabel *Label50;
	TCheckBox *CheckBox15;
	TLabel *Label51;
	TEdit *Edit54;
	TEdit *Edit45;
	TEdit *Edit50;
	TLabel *Label47;
	TLabel *Label43;
	TEdit *Edit19;
	TLabel *Label14;
	TLabel *Label52;
	TEdit *Edit56;
	TChart *Chart6;
	TCheckBox *CheckBox16;
	TFileOpenDialog *FileOpenDialog1;
	TFileOpenDialog *FileOpenDialog2;
	TPointSeries *Series16;
	TTabSheet *TabSheetFot;
	TGroupBox *GroupBoxIOZ;
	TLabel *Label_IOZ6;
	TLabel *Label_IOZ5;
	TLabel *Label_IOZ4;
	TEdit *Edit_IOZ6;
	TEdit *Edit_IOZ5;
	TEdit *Edit_IOZ4;
	TLabel *Label53;
	TLabel *Label54;
	TLabel *Label55;
	TEdit *EditBaric;
	TLabel *Label56;
	TLabel *Label57;
	TLabel *Label58;
	TLabel *Label59;
	TLineSeries *Series17;
	TCheckBox *CheckBoxBaric;
	TCheckBox *CheckBoxPhotometry;
	TChart *ChartPhotogrammetry;
	TPointSeries *PointSeries1;
	TLineSeries *Series18;
	TLineSeries *Series19;
	TLineSeries *Series20;
	TButton *ButtonBlackFrame;
	TLabel *Label60;
	TLabel *LabelBFstate;
	TOpenDialog *OpenDialog6;
	TImageList *ImageList1;
	TImage *Image1;
	TCheckBox *CheckBoxCloseBF;
	TButton *ButtonBFsave;
	TSaveDialog *SaveDialog2;
	TComboBox *ComboBoxPixIs;
	TButton *ButtonNOP;
	TOpenDialog *OpenDialogNOP;
	TLineSeries *Series21;
	TMenuItem *ini3;
	TLabel *Label61;
	TLabel *Label62;
	TCheckBox *CheckBoxNF;
	TEdit *EditNFmin;
	TEdit *EditNFmax;
	TLabel *Label63;
	TLabel *LabelMNK;
	TEdit *EditMNK;
	TCheckBox *CheckBoxNframe;
	TLabel *Label64;
	TComboBox *ComboBoxMainPoint;
	TLabel *Label65;
	TComboBox *ComboBox1;
	TButton *SelectDarkFramesBtn;
	TButton *allTogetherButton;
	void __fastcall IniSave(String dir);
	void __fastcall CheckBox1Click(TObject *Sender);
	void __fastcall CheckBox2Click(TObject *Sender);
	void __fastcall Exit1Click(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall RadioGroup1Click(TObject *Sender);
	void __fastcall CheckBox3Click(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall Button1MouseLeave(TObject *Sender);
	void __fastcall Button1MouseEnter(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall Button4Click(TObject *Sender);
	void __fastcall Button5Click(TObject *Sender);
	void __fastcall StatusBar1DrawPanel(TStatusBar *StatusBar, TStatusPanel *Panel,
          const TRect &Rect);
	void __fastcall N1Click(TObject *Sender);
	void __fastcall Button4MouseEnter(TObject *Sender);
	void __fastcall Button4MouseLeave(TObject *Sender);
	void __fastcall Button6Click(TObject *Sender);
	void __fastcall Button5MouseEnter(TObject *Sender);
	void __fastcall Button5MouseLeave(TObject *Sender);
	void __fastcall Button6MouseEnter(TObject *Sender);
	void __fastcall Button6MouseLeave(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall Button3Click(TObject *Sender);
	void __fastcall Button7Click(TObject *Sender);
	void __fastcall Button8Click(TObject *Sender);
	void __fastcall N2Click(TObject *Sender);
	void __fastcall ini2Click(TObject *Sender);
	void __fastcall FormClick(TObject *Sender);
	void __fastcall TabSheet2Show(TObject *Sender);
	void __fastcall Memo1Click(TObject *Sender);
	void __fastcall Button9Click(TObject *Sender);
	void __fastcall CheckBox6Click(TObject *Sender);
	void __fastcall CheckBox7Click(TObject *Sender);
	void __fastcall Edit1Change(TObject *Sender);
	void __fastcall Edit2Change(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall N3Click(TObject *Sender);
	void __fastcall CheckBox9Click(TObject *Sender);
	void __fastcall CheckBox10Click(TObject *Sender);
	void __fastcall CheckBox5Click(TObject *Sender);
	void __fastcall Edit11Change(TObject *Sender);
	void __fastcall Edit9Change(TObject *Sender);
	void __fastcall CheckBox15Click(TObject *Sender);
	void __fastcall CheckBox16Click(TObject *Sender);
	void __fastcall Edit_IOZ6Change(TObject *Sender);
	void __fastcall Edit_IOZ5Change(TObject *Sender);
	void __fastcall Edit_IOZ4Change(TObject *Sender);
	void __fastcall CheckBoxBaricClick(TObject *Sender);
	void __fastcall CheckBoxPhotometryClick(TObject *Sender);
	void __fastcall ButtonBlackFrameClick(TObject *Sender);
	void __fastcall CheckBoxCloseBFClick(TObject *Sender);
	void __fastcall ButtonBFsaveClick(TObject *Sender);
	void __fastcall CheckBox4Click(TObject *Sender);
	void __fastcall ButtonNOPClick(TObject *Sender);
	void __fastcall ini3Click(TObject *Sender);
	void __fastcall EditBaricChange(TObject *Sender);
	void __fastcall CheckBoxNFClick(TObject *Sender);
	void __fastcall CheckBox12Click(TObject *Sender);
	void __fastcall SelectDarkFramesBtnClick(TObject *Sender);
	void __fastcall allTogetherButtonClick(TObject *Sender);

    bool __fastcall localizationStep(AnsiString resultFileNames[], int* size);
	bool __fastcall sigmaStep(AnsiString resultFileNames[], int size, AnsiString* sigmaResultFileName);
	bool __fastcall reportResult(AnsiString sigmaResultFileName);

private:	// User declarations
public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
