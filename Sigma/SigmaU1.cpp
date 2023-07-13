//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//////////////////////-- Версия 5.05 --//////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#include <vcl.h>
#pragma hdrstop
#include <cmath>
#include <System.hpp>
#include <iostream>
#include <fstream>
#include <string.h>
#include "SigmaU1.h"
#include "FilterLib.h"
#include "HeadSigma.h"
#include "iki_img.cpp"
#include <IniFiles.hpp>
#include <time.h>
#include "LocalGaussForm.h"
#include "WordLibOffice.h"
#include "StatLib.h"
#include <string.h>
#include "pngimage.hpp"
#include <gsl/gsl_poly.h>  // Заголовочный файл с функциями gsl для полиномов
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "Word_XP_srvr"
//#pragma link "Word_2K_SRVR"
//#pragma link "BubbleCh"
#pragma resource "*.dfm"
//#pragma link "Chart"
//#pragma link "TeEngine"
//#pragma link "TeeProcs"
//#pragma link "Series"
//#pragma link "FrontPage_XP_srvr"
#pragma resource "*.dfm"
#include <algorithm>
#include <memory>
TForm1 *Form1;
using namespace std;
IKI_img* ikimg;
bool Ostanov = false;
TIniFile *ini;

bool FlagClose = true, FlagClose2 = false;
AnsiString ReportName;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
float min2 ( float a, float  b){
	if (a<b) return a;
	else return b;
}
float max2 ( float a, float  b){
	if (a>b) return a;
	else return b;
}

float FixRoundTo(float x, int y)
{
	asm
	{
		fnclex
	}
	return RoundTo(x, y);
}

void __fastcall TForm1::CheckBox1Click(TObject *Sender)
{
	if (CheckBox1 -> Checked) {
		Edit3 -> Enabled = false;
		Edit4 -> Enabled = false;
		Edit3 -> Color = cl3DLight;
		Edit4 -> Color = cl3DLight;
	}
	else {
		Edit3 -> Enabled = true;
		Edit4 -> Enabled = true;
		Edit3 -> Color = clWindow;
		Edit4 -> Color = clWindow;
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::CheckBox2Click(TObject *Sender)
{
	if (CheckBox2 -> Checked)
	{
		Edit5 -> Enabled = true;
		Edit5 -> Color = clWindow;
	}
	else
	{
		Edit5 -> Enabled = false;
		Edit5 -> Color = cl3DLight;
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Exit1Click(TObject *Sender)
{
    FlagClose = false;
	Form1 -> Close();
}
//---------------------------------------------------------------------------
void SelectingFilesFrames() //Выбор файлов по папкам или вручную
{
	int i;
	//Очищаем переменные для файлов
	FilesCount = 0;
	for (i = 0; i < MAX_FILE; i++)
		FileNames[i] = "";

	if (Form1->CheckBox1->Checked) {
		Form1->OpenDialog1->Options.Clear();
		Form1->OpenDialog1->Options << ofAllowMultiSelect << ofFileMustExist;

		if (Form1->OpenDialog1->Execute()) {
			for (i = 0; i < Form1->OpenDialog1->Files->Count; i++)
				FileNames[i] = Form1->OpenDialog1->Files->Strings[i];
			FilesCount = Form1->OpenDialog1->Files->Count;
		}
	}
	else {
		//Очищаем переменные для папок
		for (i = 0; i < MAX_DIRECT; i++)
			DirNames[i] = "";
		DirCount = 0;
		if (Form1->Edit3->Text != "" && Form1->Edit4->Text != "")
		{
			FindDirectories(Form1->Edit3->Text, Form1->Edit4->Text);
			FindFiles(Form1->Edit4->Text);
		}
		else ShowMessage("Не указан путь или маска файла");
	}
}
//---------------------------------------------------------------------------
void ReadFilesFrames(int mainFOR, bool *result)   //Считываем файл Стандартный или Новый (IKI)
{
	int i,j;
	FILE *stream;
	unsigned char t[1];
	if (FileExists(FileNames[mainFOR]))
	{
		switch (Form1->RadioGroup1->ItemIndex)
		{
			case 0:
				stream = fopen(AnsiString(FileNames[mainFOR]).c_str(), "rb");
				for (i = 0; i < StrToInt(Form1->Edit2->Text); i++)
				fread(SigmaSet.Kadr[i], sizeof(WORD), StrToInt(Form1->Edit1->Text), stream);
				fclose(stream);
			break;

			case 1:
				stream = fopen(AnsiString(FileNames[mainFOR]).c_str(), "rb");
				for (i = 0; i < StrToInt(Form1->Edit2->Text); i++)
				for (j = 0; j < StrToInt(Form1->Edit1->Text); j++)  {
	//			fgets(t, 1, stream);
				fread(t, sizeof(unsigned char), 1, stream);
				SigmaSet.Kadr[i][j] = t[0];
				}
				fclose(stream);
			break;

			case 2:
				if (ikimg->ReadFormat(FileNames[mainFOR], true))
				if (ikimg->ImageData.FrameData.Data != NULL)
				{
					int che = 0;
					for (i = 0; i < ikimg->ImageData.FrameData.FrameHeight; i++)
					for (j = 0; j < ikimg->ImageData.FrameData.FrameWidth; j++)
					SigmaSet.Kadr[i][j] = ((unsigned short*)ikimg->ImageData.FrameData.Data)[che++];    //		  Указатели		Word (*Gh)[10][1024] = (Word(*)[10][1024])ikimg->ImageData.FrameData.Data;   *Gh[0][0] += 1;
				}
			break;

			default: ShowMessage("Не выбран формат кадра");  break;
		}
		*result = true;
	}
	else
	{
		AnsiString Message1 = "Не найден файл по адресу: " + FileNames[mainFOR];
		ShowMessage(Message1);
		*result = false;
	}
}
//---------------------------------------------------------------------------
void MemoryForFrame()    //Выделение памяти для кадра SigmaSet.Kadr[i][j]
{
	int i;
	switch (Form1->RadioGroup1->ItemIndex)
	{
		case 0:
			SigmaSet.Kadr = new WORD*[StrToInt(Form1->Edit2->Text)];
			SigmaSet.FiltKadr = new WORD*[StrToInt(Form1->Edit2->Text)];

			for (i = 0; i < StrToInt(Form1->Edit2->Text); i++) {
				SigmaSet.Kadr[i] = new WORD[StrToInt(Form1->Edit1->Text)];
				SigmaSet.FiltKadr[i] = new WORD[StrToInt(Form1->Edit1->Text)];
			}
			SigmaSet.Height = StrToInt(Form1->Edit2->Text);
			SigmaSet.Width  = StrToInt(Form1->Edit1->Text);
		break;

		case 1:
			SigmaSet.Kadr = new WORD*[StrToInt(Form1->Edit2->Text)];
			SigmaSet.FiltKadr = new WORD*[StrToInt(Form1->Edit2->Text)];

			for (i = 0; i < StrToInt(Form1->Edit2->Text); i++) {
				SigmaSet.Kadr[i] = new WORD[StrToInt(Form1->Edit1->Text)];
				SigmaSet.FiltKadr[i] = new WORD[StrToInt(Form1->Edit1->Text)];
			}
			SigmaSet.Height = StrToInt(Form1->Edit2->Text);
			SigmaSet.Width  = StrToInt(Form1->Edit1->Text);
		break;

		case 2:
			ikimg = new IKI_img();
			ikimg->ReadFormat(FileNames[0], true);

			SigmaSet.Height = ikimg->ImageData.FrameData.FrameHeight;
			SigmaSet.Width  = ikimg->ImageData.FrameData.FrameWidth;

			SigmaSet.Kadr	  = new WORD*[SigmaSet.Height];
			SigmaSet.FiltKadr = new WORD*[SigmaSet.Height];

			for (i = 0; i < SigmaSet.Height; i++)
			{
				SigmaSet.Kadr[i]     = new WORD[ikimg->ImageData.FrameData.FrameWidth];
				SigmaSet.FiltKadr[i] = new WORD[ikimg->ImageData.FrameData.FrameWidth];
			}


		break;
	}
	SigmaSet.Frame = new WORD*[StrToInt(Form1->Edit9->Text)];
	for (i = 0; i < StrToInt(Form1->Edit9->Text); i++)
	SigmaSet.Frame[i] = new WORD[StrToInt(Form1->Edit9->Text)];
}
//---------------------------------------------------------------------------
void FilteringMovingAverage()
{
unsigned int start_time, end_time;
//start_time = clock();

	ImageSet fock1;
	fock1.Height = SigmaSet.Height;
	fock1.Width = SigmaSet.Width;

	WORD *BufKadV = new WORD [SigmaSet.Width*SigmaSet.Height];
	int i,j,n = 0;
	for (i = 0; i < SigmaSet.Height; i++)
	for (j = 0; j < SigmaSet.Width; j++)	{
		BufKadV[n++] = SigmaSet.Kadr[i][j];
	}
//end_time = clock();
//Form1->Memo1->Lines->Add("1 -  " + TimeToStr(Time()) + " - " + FloatToStr((end_time - start_time)/1000.0));
//start_time = clock();
	fock1.BufSrc = &BufKadV[0];

	FilterSet Filter;
	Filter.FltSize = StrToInt(Form1->Edit10->Text);

	FilterRes Res1;
	Res1.BufFrame = new unsigned short [SigmaSet.Width*SigmaSet.Height];
//end_time = clock();
//Form1->Memo1->Lines->Add("2 -  " + TimeToStr(Time()) + " - " + FloatToStr((end_time - start_time)/1000.0));
//start_time = clock();
	FilterMeanN(fock1, Filter, &Res1);
//end_time = clock();
//Form1->Memo1->Lines->Add("3 -  " + TimeToStr(Time()) + " - " + FloatToStr((end_time - start_time)/1000.0));
//start_time = clock();
	MeanPor = 0;
	MeanPor = (int) (Res1.MeanFlt + StrToInt(Form1->Edit17->Text)*Res1.SigmaFlt);

	//1
	for (n = 0; n < SigmaSet.Width*SigmaSet.Height; n++)
	if ((Res1.BufFrame[n] - MeanPor) > 0)
			Res1.BufFrame[n] = Res1.BufFrame[n] - MeanPor;
	else  	Res1.BufFrame[n] = 0;

	n = 0;
	for (i = 0; i < SigmaSet.Height; i++)
	for (j = 0; j < SigmaSet.Width; j++)	{
		SigmaSet.FiltKadr[i][j] = Res1.BufFrame[n++];
	}
//end_time = clock();
//Form1->Memo1->Lines->Add("4 -  " + TimeToStr(Time()) + " - " + FloatToStr((end_time - start_time)/1000.0));
//start_time = clock();
	LocalNI (Res1.BufFrame, SigmaSet.Width, SigmaSet.Height, StrToInt(Form1->Edit13->Text), StrToInt(Form1->Edit14->Text),
			 StrToInt(Form1->Edit11->Text), StrToInt(Form1->Edit12->Text));
//end_time = clock();
//Form1->Memo1->Lines->Add("5 -  " + TimeToStr(Time()) + " - " + FloatToStr((end_time - start_time)/1000.0));

	//2
//	n = 0;
//	for (i = 0; i < SigmaSet.Height; i++)
//	for (j = 0; j < SigmaSet.Width; j++)	{
//		if ((Res1.BufFrame[n] - MeanPor) > 0)
//			SigmaSet.FiltKadr[i][j] = Res1.BufFrame[n] - MeanPor;
//		else  SigmaSet.FiltKadr[i][j] = 0;
//		n++;
//	}

	//3
//	n = 0;
//	for (i = 0; i < SigmaSet.Height; i++)
//	for (j = 0; j < SigmaSet.Width; j++)
//	SigmaSet.FiltKadr[i][j] = Res1.BufFrame[n++];


//	//Проверка - вывод кадра
//	FILE *streG = fopen(AnsiString("C:\\Cybertron\\Programms\\FiltK.dat").c_str(), "wb");
//	for (i = 0; i < SigmaSet.Height; i++) {
//	   fwrite(SigmaSet.FiltKadr[i], sizeof(WORD), SigmaSet.Width, streG);}
//	fclose(streG);

	delete [] BufKadV;
	delete [] Res1.BufFrame;
}
//---------------------------------------------------------------------------
void BinFrameSigma()    //заменить 2 на коэф бинирования
{
	int i,j,n = 0;
	//Вырезание окошка
	for (i = 0; i < SigmaSet.HWframe; i++)
	for (j = 0; j < SigmaSet.HWframe; j++)
	if ( (int)(i+SigmaSet.y-SigmaSet.HWframe/2) >= 0 && (int)(i+SigmaSet.y-SigmaSet.HWframe/2) < SigmaSet.Height &&
		 (int)(j+SigmaSet.x-SigmaSet.HWframe/2) >= 0 && (int)(j+SigmaSet.x-SigmaSet.HWframe/2) < SigmaSet.Width )
	{
		SigmaSet.Frame[i][j] = SigmaSet.Kadr[(int)(i+SigmaSet.y-SigmaSet.HWframe/2)][(int)(j+SigmaSet.x-SigmaSet.HWframe/2)];
	}
	else SigmaSet.Frame[i][j] = 0;

	WORD *BufKad = new WORD [SigmaSet.HWframe*SigmaSet.HWframe];
	for (i = 0; i < SigmaSet.HWframe; i++)
	for (j = 0; j < SigmaSet.HWframe; j++)
		BufKad[n++] = SigmaSet.Frame[i][j];

	ImageSet bin1;
	bin1.Height = SigmaSet.HWframe;
	bin1.Width = SigmaSet.HWframe;
	bin1.BufSrc = &BufKad[0];

	SigmaSet.HWframe = StrToInt(Form1->Edit9->Text)/2;

	WORD *BufKad2 = new WORD [SigmaSet.HWframe*SigmaSet.HWframe];

	BinaryOG_F32(bin1, StrToInt(Form1->Edit5->Text) - 1, &BufKad2[0]);

	n = 0;
	for (i = 0; i < SigmaSet.HWframe; i++)
		for (j = 0; j < SigmaSet.HWframe; j++)
			SigmaSet.Frame[i][j] = BufKad2[n++];

	//Проверка - вывод кадра
//	FILE *streG = fopen(AnsiString("C:\\Cybertron\\Programms\\BinK_"+IntToStr(SigmaSet.HWframe)+".dat").c_str(), "wb");
//	for (i = 0; i < SigmaSet.HWframe; i++) {
//	   fwrite(SigmaSet.Frame[i], sizeof(WORD), SigmaSet.HWframe, streG);}
//	fclose(streG);

	delete [] BufKad;
	delete [] BufKad2;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button1Click(TObject *Sender)     
{
	int i,j, mainFOR;
	FILE *streG;

	SelectingFilesFrames();  //Выбор файлов по папкам или вручную, получаем все пути к файлам в FileNames[FilesCount]

	if (FilesCount > 0)
	{
		MemoryForFrame(); //Выделение памяти для кадра SigmaSet.Kadr[i][j]
		int CMmMs = 0;
		BoxMmMsStr *BoxMmMs = new BoxMmMsStr[FilesCount];
		for (mainFOR = 0; mainFOR < FilesCount; mainFOR++) //Цикл по файлам
		{
			// ММ и МС из названия файла в SigmaSet.mm и SigmaSet.ms
			IdentifFromName(AnsiString(FileNames[mainFOR]).c_str(), AnsiString(Edit8->Text).c_str(), AnsiString(Edit6->Text).c_str(), &SigmaSet.ms);
			IdentifFromName(AnsiString(FileNames[mainFOR]).c_str(), AnsiString(Edit22->Text).c_str(), AnsiString(Edit7->Text).c_str(), &SigmaSet.mm);

			bool resultReadFiles = false;
			ReadFilesFrames(mainFOR, &resultReadFiles);  //Считываем файл Стандартный или Новый (IKI), получаем SigmaSet.Kadr[i][j]
			if (resultReadFiles == false)
			{
				Ostanov = true;
			}
			if (Ostanov == false)
			{
				if (CheckBox3->Checked)
				FilteringMovingAverage(); //Фильтруем заданным окном и вычитаем среднее+Edit17*СКО

				//Множество проверок после полученных координат при локализации
				if (CheckBox3->Checked)
				if (Nobj != 1) {    //Кол-во локализованых звезд больше 1 или = 0
					Form1->Memo1->Lines->Add("Звезда не найдена");
					Form1->Memo1->Lines->Add(AnsiString(FileNames[mainFOR]).c_str());
				}
				else {              //Кол-во локализованых звезд = 1
					SigmaSet.x = Xloc[0];
					SigmaSet.y = Yloc[0];
					//Заполнение массива BoxMmMs (учет координат)
					if (CMmMs == 0) { //Первое вхождение в структуру BoxMmMs
						BoxMmMs[0].ms = SigmaSet.ms;
						BoxMmMs[0].mm = SigmaSet.mm;
						BoxMmMs[0].x = SigmaSet.x;
						BoxMmMs[0].y = SigmaSet.y;
						BoxMmMs[0].ColKad = 1;
						CMmMs = 1;
						SigmaSet.CMmMs = 1;
					}
					else {  //Очередное вхождение в структуру BoxMmMs
						bool Fl1 = true;
						for (i = 0; i < CMmMs; i++) {
							if ( (BoxMmMs[i].ms == SigmaSet.ms) && (BoxMmMs[i].mm == SigmaSet.mm) )
							if ( ((BoxMmMs[i].x+StrToInt(Form1->Edit11->Text)) >= SigmaSet.x) && ((BoxMmMs[i].x-StrToInt(Form1->Edit11->Text)) <= SigmaSet.x)
								  && ((BoxMmMs[i].y+StrToInt(Form1->Edit11->Text)) >= SigmaSet.y) && ((BoxMmMs[i].y-StrToInt(Form1->Edit11->Text)) <= SigmaSet.y) )
							{
								// Условия выполнены, такие координаты и идентификаторы уже существуют
								BoxMmMs[i].ColKad++;
								SigmaSet.CMmMs = i;
								Fl1 = false;
							}
						}
							if (Fl1) { // В цикле ничего не найдено, значит это новый набор
								BoxMmMs[CMmMs].ms = SigmaSet.ms;
								BoxMmMs[CMmMs].mm = SigmaSet.mm;
								BoxMmMs[CMmMs].x = SigmaSet.x;
								BoxMmMs[CMmMs].y = SigmaSet.y;
								BoxMmMs[CMmMs].ColKad++;
								SigmaSet.CMmMs = CMmMs;
								CMmMs++;
							}
					}


					SigmaSet.HWframe = StrToInt(Edit9->Text);
					//Вырезание окошка
					for (i = 0; i < SigmaSet.HWframe; i++)
					for (j = 0; j < SigmaSet.HWframe; j++)
					if ( (int)(i+SigmaSet.y-SigmaSet.HWframe/2) >= 0 && (int)(i+SigmaSet.y-SigmaSet.HWframe/2) < SigmaSet.Height &&
						 (int)(j+SigmaSet.x-SigmaSet.HWframe/2) >= 0 && (int)(j+SigmaSet.x-SigmaSet.HWframe/2) < SigmaSet.Width )
					{
						SigmaSet.Frame[i][j] = SigmaSet.Kadr[(int)(i+SigmaSet.y-SigmaSet.HWframe/2)][(int)(j+SigmaSet.x-SigmaSet.HWframe/2)];
					}
					else SigmaSet.Frame[i][j] = 0;


					//Проверка - вывод кадра
	//				streG = fopen(AnsiString("C:\\Cybertron\\Programms\\WindK.dat").c_str(), "wb");
	//				for (i = 0; i < StrToInt(Form1->Edit9->Text); i++) {
	//				   fwrite(SigmaSet.Frame[i], sizeof(WORD), StrToInt(Form1->Edit9->Text), streG);}
	//				fclose(streG);


					Form1->Memo1->Lines->Add("ОК");
					Form1->Memo1->Lines->Add(IntToStr(Nobj));
					Form1->Memo1->Lines->Add(FloatToStr(Xloc[0]));
					Form1->Memo1->Lines->Add(FloatToStr(Yloc[0]));
				}

				//Не понятно как рабоать без фильтрации
				if (CheckBox3->Checked) {
					F4Lines = 0;
					Filter4Lines(StrToInt(Form1->Edit17->Text), SigmaSet.Frame, SigmaSet.HWframe, SigmaSet.HWframe); //Фильтрация по 4м строкам
	//				SelectMaxObject ();

					SigmaSet.Ispic = 0;
					for (i = 0; i < SigmaSet.HWframe; i++)
					for (j = 0; j < SigmaSet.HWframe; j++)
					if (SigmaSet.Frame[i][j] > SigmaSet.Ispic) {
						SigmaSet.Ispic = SigmaSet.Frame[i][j];
					}

					if (SigmaSet.Ispic < StrToInt(Edit15->Text) || SigmaSet.Ispic > StrToInt(Edit16->Text))
					{
						Form1->Memo1->Lines->Add("Превышение по ярчайшему пикселю, #" + IntToStr(mainFOR+1) + " _ " + IntToStr(SigmaSet.Ispic));
					}
					else
					{
						if (CheckBox2->Checked)
						BinFrameSigma();  //Бинирование

						//Вычитаем F4Lines обратно
						for (i = 0; i < SigmaSet.HWframe; i++)
						for (j = 0; j < SigmaSet.HWframe; j++)
						if ((SigmaSet.Frame[i][j] - F4Lines) > 0)
							SigmaSet.Frame[i][j] = SigmaSet.Frame[i][j] - F4Lines;
						else  SigmaSet.Frame[i][j] = 0;

						WORD **Mask;
						Mask = new WORD*[SigmaSet.HWframe];
						for (i = 0; i < SigmaSet.HWframe; i++) Mask[i] = new WORD[SigmaSet.HWframe];

						int Fl3 = 0;
						for (i = 1; i < SigmaSet.HWframe-1; i++)
						for (j = 1; j < SigmaSet.HWframe-1; j++) {
							if (SigmaSet.Frame[i][j] == 0){
								if (SigmaSet.Frame[i-1][j] > 0) Fl3++;
								if (SigmaSet.Frame[i+1][j] > 0) Fl3++;
								if (SigmaSet.Frame[i][j-1] > 0) Fl3++;
								if (SigmaSet.Frame[i][j+1] > 0) Fl3++;

								if (Fl3 >= 3 )
								Mask[i][j] = 1;
								Fl3 = 0;
							}
							else Mask[i][j] = SigmaSet.Frame[i][j];

							}

							//Сигма
							double Mug=0, SigX=0, SigY=0, Sx0=0, Sy0=0;
							if (true) {


				//				int *pix_val0 = new int [StrToInt(Edit7->Text)*StrToInt(Edit7->Text)];
				//				int schic = 0;
				//
				//				int *pix_X0 = new int [StrToInt(Edit7->Text)];
				//				int *pix_Y0 = new int [StrToInt(Edit7->Text)];
				//				bool plusN = false;
				//
				//				int WindSizeXY =  4+StrToInt(Edit12->Text)/2;
				//
				//				for(int i=0; i<StrToInt(Edit7->Text); i++) { pix_X0[i] = i; pix_Y0[i] = i; }
				//
				//				for (i25 = 0; i25 < StrToInt(Edit7->Text); i25++)
				//				for (j25 = 0; j25 < StrToInt(Edit7->Text); j25++) {
				//					pix_val0[schic] = BuffLoc[i25][j25] - (int)(BufLOC + koff*SKOloc + StrToInt(Edit9->Text));
				//					if (pix_val0[schic] < 0) {  pix_val0[schic] = 0;	}
				//					schic++;
				//				}
				//
								int *pix_X = new int [10];
								int *pix_Y = new int [10];
								int *pix_val = new int [10];

								int numMeas = 0;
				//				for (ii = 0; ii < StrToInt(Edit7->Text)*StrToInt(Edit7->Text); ii++)
				//				{
				//					if (pix_val0[ii] > 0)  pix_val[numMeas] = pix_val0[ii];
				//					else   				pix_val[numMeas] = 0;
				//
				//					pix_X[numMeas] = pix_X0[ii%StrToInt(Edit7->Text)];
				//					pix_Y[numMeas++] = pix_Y0[ii/StrToInt(Edit7->Text)];
				//				}
				//
				//				numMeas = 0;
				//				FILE *stre3 = fopen(AnsiString(AdrFolder + "pix_val0.txt").c_str(), "a");
				//				for (j = 0; j < StrToInt(Edit7->Text); j++)
				//				{
				//					for (ii = 0; ii < StrToInt(Edit7->Text); ii++)
				//					fprintf(stre3, "%d	", pix_val[numMeas++]);
				//					fprintf(stre3, "\n");
				//				}
				//				fclose(stre3);
				//
				//				numMeas = 0;
				//				stre3 = fopen(AnsiString(AdrFolder + "pix_X.txt").c_str(), "a");
				//				for (j = 0; j < StrToInt(Edit7->Text); j++)
				//				{
				//					for (ii = 0; ii < StrToInt(Edit7->Text); ii++)
				//					fprintf(stre3, "%d	", pix_X[numMeas++]);
				//					fprintf(stre3, "\n");
				//				}
				//				fclose(stre3);
				//
				//				numMeas = 0;
				//				stre3 = fopen(AnsiString(AdrFolder + "pix_Y.txt").c_str(), "a");
				//				for (j = 0; j < StrToInt(Edit7->Text); j++)
				//				{
				//					for (ii = 0; ii < StrToInt(Edit7->Text); ii++)
				//					fprintf(stre3, "%d	", pix_Y[numMeas++]);
				//					fprintf(stre3, "\n");
				//				}
				//				fclose(stre3);
				//				Mug=0, SigX=0, SigY=0, Sx0=0, Sy0=0;
				//
	//							LocalGauss(pix_X, pix_Y, pix_val, numMeas-1, 5, 10, &Sx0, &Sy0, &SigX, &SigY, &Mug);  //numMeas or numMeas-1
				//				Sx0 = Sx0+0.5;
				//				Sy0 = Sy0+0.5;
				//
				//
				//				delete [] pix_val0;
								delete [] pix_X;
								delete [] pix_Y;
								delete [] pix_val;
				//				delete [] pix_X0;
				//				delete [] pix_Y0;
						}

						for (i = 0; i < SigmaSet.HWframe; i++) {
							delete [] Mask[i];
						}
						delete [] Mask;
					}

	//				//Проверка - вывод кадра
	//				streG = fopen(AnsiString("C:\\Cybertron\\Programms\\WindKBM5.dat").c_str(), "wb");
	//				for (i = 0; i < StrToInt(Form1->Edit9->Text); i++) {
	//				   fwrite(Mask[i], sizeof(WORD), StrToInt(Form1->Edit9->Text), streG);}
	//				fclose(streG);

				}
			} //Ostanov
			else mainFOR = FilesCount;

		}

		for (i = 0; i < StrToInt(Form1->Edit2->Text); i++) delete [] SigmaSet.FiltKadr[i];
		delete [] SigmaSet.FiltKadr;

		for (i = 0; i < StrToInt(Form1->Edit2->Text); i++) delete [] SigmaSet.Kadr[i];
		delete [] SigmaSet.Kadr;

		for (i = 0; i < StrToInt(Form1->Edit9->Text); i++) delete [] SigmaSet.Frame[i];
		delete [] SigmaSet.Frame;

		delete [] BoxMmMs;
		if (RadioGroup1->ItemIndex == 1)
		delete ikimg;
	}
	else  ShowMessage("Не выбраны файлы");

}
//---------------------------------------------------------------------------

//Переменные для OLE серверов Word
//Variant		WordDoc;
//bool		fStart;

//Кнопка 1
//	Chart1->Series[0]->FillSampleValues(100);  // Рандомные 100 точек
//	Chart1->Width = 900;
//	Chart1->Height = 500;
//
//	fStart = true;  //Флаг для выхода WordDoc.OleProcedure("Quit");
//
//	WordDoc = CreateOleObject("Word.Application");  //Вызов Word
//	WordDoc.OlePropertySet("Visible",true);         //Видимость создания файла
//
//	WordDoc.OlePropertyGet("Documents").OleProcedure("Add");   //Создаем один документ Word
//	WordDoc.OlePropertyGet("Documents").OleFunction("Item",1).OlePropertyGet("Paragraphs").OleProcedure("Add"); //Обращаемся к нему ("Item",1) и добавляем параграф
//
//	// Добавляем текст
//	AnsiString str = "\t\tОзнакомлен\t\t\n\n\n";
//	WordDoc.OlePropertyGet("Documents").OleFunction("Item",1).OlePropertyGet("Paragraphs").OleFunction("Item",1).OlePropertyGet("Range").OlePropertySet("Text",str.c_str());
//
//	//Добавляем график
//	Chart1->SaveToBitmapFile("aaa.bmp");
//	WordDoc.OlePropertyGet("Selection").OlePropertyGet("InlineShapes").OleProcedure("AddPicture","C:\\Cybertron\\Programms\\Sigma\\Debug\\Win32\\aaa.bmp",false,true);

//Кнопка 2
//	if(fStart) WordDoc.OleProcedure("Quit");


void __fastcall TForm1::RadioGroup1Click(TObject *Sender)
{
	if (RadioGroup1->ItemIndex != 2)
	{
		Edit1 -> Enabled = true;
		Edit2 -> Enabled = true;
		Edit1 -> Color = clWindow;
		Edit2 -> Color = clWindow;
	}
	else
		if (RadioGroup1->ItemIndex == 2)
		{
			Edit1 -> Enabled = false;
			Edit2 -> Enabled = false;
			Edit1 -> Color = cl3DLight;
			Edit2 -> Color = cl3DLight;
		}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBox3Click(TObject *Sender)
{
	if (CheckBox3 -> Checked)
	{
		Edit10 -> Enabled = true;
		Edit10 -> Color = clWindow;
	}
	else
	{
		Edit10 -> Enabled = false;
		Edit10 -> Color = cl3DLight;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormCreate(TObject *Sender)
{
	ini = new TIniFile(ExtractFilePath(Application->ExeName)+"INI_Sigma.ini");
	Memo1->Clear();
	ProgressBar1->Parent = StatusBar1;

	FormatSettings.DecimalSeparator = '.';
	BKflag = false;

	if (EditBaric->Text == "") {
		EditBaric->Text = 0;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormShow(TObject *Sender)
{
//	Edit1->Text=ini->ReadString("Edits","Edit1-X","");
//	Edit2->Text=ini->ReadString("Edits","Edit2-Y","");
//	Edit3->Text=ini->ReadString("Edits","Edit3-Dir","");
//	Edit4->Text=ini->ReadString("Edits","Edit4-Mask","");
//	Edit5->Text=ini->ReadString("Edits","Edit5-Bin","");
//	Edit8->Text=ini->ReadString("Edits","Edit8-Id1ms","");
//	Edit6->Text=ini->ReadString("Edits","Edit6-Id2ms","");
//	Edit22->Text=ini->ReadString("Edits","Edit22-Id1mm","");
//	Edit7->Text=ini->ReadString("Edits","Edit7-Id2mm","");
//
//	Edit9->Text=ini->ReadString("Edits","Edit9-frame","");
//	Edit10->Text=ini->ReadString("Edits","Edit10-filt","");
//	Edit11->Text=ini->ReadString("Edits","Edit11-pixMin","");
//	Edit12->Text=ini->ReadString("Edits","Edit12-pixMax","");
//	Edit13->Text=ini->ReadString("Edits","Edit13-IsMin","");
//	Edit14->Text=ini->ReadString("Edits","Edit14-IsMax","");
//	Edit15->Text=ini->ReadString("Edits","Edit15-pixMaxIs","");
//	Edit16->Text=ini->ReadString("Edits","Edit16-pixMinIs","");
//	Edit17->Text=ini->ReadString("Edits","Edit17-kofSKO1","");
//	Edit18->Text=ini->ReadString("Edits","Edit18-infKol","");
//	Edit19->Text=ini->ReadString("Edits","Edit19-tochnost","");
//	Edit20->Text=ini->ReadString("Edits","Edit20-optSigma1","");
//	Edit21->Text=ini->ReadString("Edits","Edit21-optSigma2","");
//
//	Edit23->Text=ini->ReadString("Edits","Edit23-SaveDir","");
//	Edit24->Text=ini->ReadString("Edits","Edit24-forLG_step","");
//	Edit25->Text=ini->ReadString("Edits","Edit25-forLG_iter","");
//	Edit26->Text=ini->ReadString("Edits","Edit26-emptyPix","");
//	Edit27->Text=ini->ReadString("Edits","Edit27-maxYGraf","");
//	Edit28->Text=ini->ReadString("Edits","Edit28-kofSKO2","");
//
//	Edit29->Text=ini->ReadString("Edits","Edit29","");
//	Edit30->Text=ini->ReadString("Edits","Edit30","");
//	Edit31->Text=ini->ReadString("Edits","Edit31","");
//	Edit32->Text=ini->ReadString("Edits","Edit32","");
//	Edit33->Text=ini->ReadString("Edits","Edit33","");
//	Edit34->Text=ini->ReadString("Edits","Edit34","");
//	Edit35->Text=ini->ReadString("Edits","Edit35","");
//	Edit36->Text=ini->ReadString("Edits","Edit36","");
//	Edit37->Text=ini->ReadString("Edits","Edit37","");
//	Edit38->Text=ini->ReadString("Edits","Edit38","");
//	Edit39->Text=ini->ReadString("Edits","Edit39","");
//	Edit40->Text=ini->ReadString("Edits","Edit40","");
//	Edit41->Text=ini->ReadString("Edits","Edit41","");
//	Edit42->Text=ini->ReadString("Edits","Edit42","");
//	Edit43->Text=ini->ReadString("Edits","Edit43","");
//	Edit44->Text=ini->ReadString("Edits","Edit44","");
//	Edit45->Text=ini->ReadString("Edits","Edit45","");
//	Edit46->Text=ini->ReadString("Edits","Edit46","");
//
//	Edit47->Text=ini->ReadString("Edits","Edit47","");
//	Edit48->Text=ini->ReadString("Edits","Edit48","");
//	Edit49->Text=ini->ReadString("Edits","Edit49","");
//	Edit50->Text=ini->ReadString("Edits","Edit50","");
//	Edit51->Text=ini->ReadString("Edits","Edit51","");
//
//	CheckBox1->Checked=ini->ReadBool("CheckBoxes","CheckBox1","");
//	CheckBox2->Checked=ini->ReadBool("CheckBoxes","CheckBox2","");
//	CheckBox3->Checked=ini->ReadBool("CheckBoxes","CheckBox3","");
//	CheckBox4->Checked=ini->ReadBool("CheckBoxes","CheckBox4","");
//	CheckBox5->Checked=ini->ReadBool("CheckBoxes","CheckBox5","");
//	CheckBox6->Checked=ini->ReadBool("CheckBoxes","CheckBox6","");
//	CheckBox7->Checked=ini->ReadBool("CheckBoxes","CheckBox7","");
//
//	CheckBox9->Checked=ini->ReadBool("CheckBoxes","CheckBox9","");
//	CheckBox10->Checked=ini->ReadBool("CheckBoxes","CheckBox10","");
//	Memo2->Text=ini->ReadString("Memo","Memo2","");

		Edit1->Text=ini->ReadString("Overall","X","");
		Edit2->Text=ini->ReadString("Overall","Y","");
		Edit3->Text=ini->ReadString("Overall","Dir","");
		Edit4->Text=ini->ReadString("Overall","Mask","");
		CheckBox1->Checked=ini->ReadBool("Overall","CheckChoice","");
		Edit23->Text=ini->ReadString("Overall","SaveDir","");
		if (ini->ValueExists("Overall","FormatFrame"))
			RadioGroup1->ItemIndex = ini->ReadInteger("Overall","FormatFrame", 0);

		Edit10->Text=ini->ReadString("LOC-options","KofFilt","");
		CheckBox3->Checked=ini->ReadBool("LOC-options","CheckFilt","");
		Edit17->Text=ini->ReadString("LOC-options","kofSKO1","");
		Edit11->Text=ini->ReadString("LOC-options","pixMin","");
		Edit12->Text=ini->ReadString("LOC-options","pixMax","");
		Edit13->Text=ini->ReadString("LOC-options","IsMin","");
		Edit14->Text=ini->ReadString("LOC-options","IsMax","");

		Edit9->Text=ini->ReadString("Sigma-options","frame","");
		Edit5->Text=ini->ReadString("Sigma-options","KofBin","");
		CheckBox2->Checked=ini->ReadBool  ("Sigma-options","CheckBin","");
		Edit28->Text=ini->ReadString("Sigma-options","kofSKO2","");
		Edit15->Text=ini->ReadString("Sigma-options","pixMaxIs","");
		Edit16->Text=ini->ReadString("Sigma-options","pixMinIs","");
		CheckBox4->Checked=ini->ReadBool("Sigma-options","CheckMostLight","");
		if (ini->ValueExists("Sigma-options","Check_Is-pix"))
			ComboBoxPixIs->ItemIndex = ini->ReadInteger("Sigma-options","Check_Is-pix",0);
		Edit24->Text=ini->ReadString("Sigma-options","forLG_step","");
		Edit25->Text=ini->ReadString("Sigma-options","forLG_iter","");
		Edit52->Text=ini->ReadString("Sigma-options","forLG_MinPix","");
        Edit25->Text=ini->ReadString("Sigma-options","forLG_iter","");
		CheckBox14->Checked=ini->ReadBool("Sigma-options","CheckSector","");

		Edit32->Text=ini->ReadString("Report-options","NameDevice","");
		Edit33->Text=ini->ReadString("Report-options","NomberDevice","");
		Edit34->Text=ini->ReadString("Report-options","Xmatr","");
		Edit43->Text=ini->ReadString("Report-options","Ymatr","");
		Edit35->Text=ini->ReadString("Report-options","NameShooter","");
		Edit36->Text=ini->ReadString("Report-options","DataShoot","");
		Edit37->Text=ini->ReadString("Report-options","NameReporter","");
		Memo2->Text=ini->ReadString("Report-options","Comment","");

		Edit_IOZ6->Text=ini->ReadString("Photogrammetry-options","IsStar6","");
		Edit_IOZ5->Text=ini->ReadString("Photogrammetry-options","IsStar5","");
		Edit_IOZ4->Text=ini->ReadString("Photogrammetry-options","IsStar4","");
		CheckBoxPhotometry->Checked=ini->ReadBool("Photogrammetry-options","PhotometryCheck","");

		Edit8->Text=ini->ReadString("Research-options","Id1ms","");
		Edit6->Text=ini->ReadString("Research-options","Id2ms","");
		Edit22->Text=ini->ReadString("Research-options","Id1mm","");
		Edit7->Text=ini->ReadString("Research-options","Id2mm","");
		Edit18->Text=ini->ReadString("Research-options","infKol","");
		Edit19->Text=ini->ReadString("Research-options","tochnost","");
		Edit45->Text=ini->ReadString("Research-options","FocDevice","");
		Edit46->Text=ini->ReadString("Research-options","FocCol","");
		Edit50->Text=ini->ReadString("Research-options","PixSize","");
		EditBaric->Text=ini->ReadString("Research-options","Baric","");
		CheckBoxBaric->Checked = ini->ReadBool("Research-options","BaricCheck","");
		ComboBoxMainPoint->ItemIndex = ini->ReadInteger("Research-options","SlopeParam",0);

		Edit27->Text=ini->ReadString("Graphic-options","minXGraf","");
		Edit29->Text=ini->ReadString("Graphic-options","maxXGraf","");
		CheckBox6->Checked=ini->ReadBool  ("Graphic-options","CheckGrafX","");
		Edit30->Text=ini->ReadString("Graphic-options","minYGraf","");
		Edit31->Text=ini->ReadString("Graphic-options","maxYGraf","");
		CheckBox7->Checked=ini->ReadBool  ("Graphic-options","CheckGrafY","");
		Edit40->Text=ini->ReadString("Graphic-options","HeightGraf","");
		Edit41->Text=ini->ReadString("Graphic-options","WidthGraf","");
		Edit53->Text=ini->ReadString("Graphic-options","Step","");
		Edit38->Text=ini->ReadString("Graphic-options","StepGrafX","");
		Edit39->Text=ini->ReadString("Graphic-options","StepGrafY","");
		CheckBox12->Checked=ini->ReadBool  ("Graphic-options","CheckStep","");
		Edit44->Text=ini->ReadString("Graphic-options","SizeBubble","");
		CheckBox11->Checked=ini->ReadBool  ("Graphic-options","FixSizeBubble","");
		Edit56->Text=ini->ReadString("Graphic-options","BrGist","");
		CheckBox16->Checked=ini->ReadBool  ("Graphic-options","CheckBrGist","");
		Edit20->Text=ini->ReadString("Graphic-options","optSigma1","");
		Edit21->Text=ini->ReadString("Graphic-options","optSigma2","");
		Edit47->Text=ini->ReadString("Graphic-options","dopSigma1","");
		Edit48->Text=ini->ReadString("Graphic-options","dopSigma2","");
		CheckBox9->Checked=ini->ReadBool  ("Graphic-options","CheckDopSigma","");
		Edit49->Text=ini->ReadString("Graphic-options","Mu","");
		CheckBox10->Checked=ini->ReadBool  ("Graphic-options","CheckMu","");

		Edit55->Text=ini->ReadString("Graphic-options","DeviationCenter","");
		CheckBox15->Checked=ini->ReadBool  ("Graphic-options","CheckDC","");
		Edit54->Text=ini->ReadString("Graphic-options","SizeFrag","");
		EditNFmax->Text=ini->ReadString("Graphic-options","NFrameMAX","");
		EditNFmin->Text=ini->ReadString("Graphic-options","NFrameMIN","");
		CheckBoxNF->Checked=ini->ReadBool  ("Graphic-options","CheckNF","");
		CheckBoxNframe->Checked=ini->ReadBool  ("Graphic-options","CheckNFgraf","");
		EditMNK->Text = ini->ReadString("Graphic-options","PolinomDegree","");

		CheckBox5->Checked=ini->ReadBool  ("Sky-options","CheckSkyGraf","");
		Edit51->Text=ini->ReadString("Sky-options","MinStarsInSector","");

	delete ini; ini = NULL;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button1MouseLeave(TObject *Sender)
{
	Edit1 -> Color = clWindow;
	Edit2 -> Color = clWindow;
	Edit6 -> Color = clWindow;
	Edit7 -> Color = clWindow;
	Edit9 -> Color = clWindow;
	Edit11 -> Color = clWindow;
	Edit12 -> Color = clWindow;
	Edit13 -> Color = clWindow;
	Edit14 -> Color = clWindow;
	Edit15 -> Color = clWindow;
	Edit16 -> Color = clWindow;
	Edit17 -> Color = clWindow;
	Edit19 -> Color = clWindow;
	Edit20 -> Color = clWindow;
	Edit21 -> Color = clWindow;

	if (CheckBox1->Checked == false)
	{
		Edit3 -> Color = clWindow;
		Edit4 -> Color = clWindow;
	}
	if (CheckBox2->Checked) Edit5 -> Color = clWindow;
	if (CheckBox3->Checked) Edit10 -> Color = clWindow;

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button1MouseEnter(TObject *Sender)
{
	Edit1 -> Color = clInfoBk;
	Edit2 -> Color = clInfoBk;
	Edit6 -> Color = clInfoBk;
	Edit7 -> Color = clInfoBk;
	Edit9 -> Color = clInfoBk;
	Edit11 -> Color = clInfoBk;
	Edit12 -> Color = clInfoBk;
	Edit13 -> Color = clInfoBk;
	Edit14 -> Color = clInfoBk;
	Edit15 -> Color = clInfoBk;
	Edit16 -> Color = clInfoBk;
	Edit17 -> Color = clInfoBk;
	Edit19 -> Color = clInfoBk;
	Edit20 -> Color = clInfoBk;
	Edit21 -> Color = clInfoBk;

	if (CheckBox1->Checked == false)
	{
		Edit3 -> Color = clInfoBk;
		Edit4 -> Color = clInfoBk;
	}
	if (CheckBox2->Checked) Edit5 -> Color = clInfoBk;
	if (CheckBox3->Checked) Edit10 -> Color = clInfoBk;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormResize(TObject *Sender)
{
	if (Form1->Height <= 670) {
		Form1->Height  = 670;
	}
	if (Form1->Width <= 870) {
		Form1->Width  = 870;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button4Click(TObject *Sender)
{
	int i,j, mainFOR, Gsch = 0, Bsch = 0;
	FILE *streG;
	FILE *Buono, *Brutto, *Cattivo;
	Label40->Visible = true;
	StatusBar1->Panels->Items[0]->Text = "Поиск файлов...";
	Application->ProcessMessages();
	FilesCount = 0;
	SelectingFilesFrames();  //Выбор файлов по папкам или вручную, получаем все пути к файлам в FileNames[FilesCount]

	if (DirectoryListBox2->Visible == true) {
		DirectoryListBox2->Visible = false;
		Button8->Visible = false;
		Button3->Caption = "Обзор...";
	}
	if (DirectoryListBox1->Visible == true) {
		DirectoryListBox1->Visible = false;
		Button7->Visible = false;
		Button2->Caption = "Обзор...";
	}

	struct tm *local;
	time_t t = time(NULL);
	local = localtime(&t);
	char bufftime [80];        // строка, в которой будет храниться текущее время
	strftime(bufftime,80,"%Y.%m.%d %H-%M-%S",local);

	if (DirectoryExists(AnsiString(Edit23->Text).c_str()) == true) {
	if (FilesCount > 0)
	{
		MemoryForFrame(); //Выделение памяти для кадра SigmaSet.Kadr[i][j]

		Buono = fopen(AnsiString(Edit23->Text + "\\Sigma - " + bufftime + " - OneStar.txt").c_str(), "a");   //Good
		Brutto = fopen(AnsiString(Edit23->Text + "\\Sigma - " + bufftime + " - AllStars.txt").c_str(), "a"); //Bad
		Cattivo = fopen(AnsiString(Edit23->Text + "\\Sigma - " + bufftime + " - AllStarsList.txt").c_str(), "a");

		//IniSave(Edit23->Text + "\\INI_Sigma_" + bufftime);

		String TabSpace = "	";
		switch(ComboBox1->ItemIndex)
		{
			case 0:
			TabSpace = "	";
			break;
					
			case 1:
			TabSpace = "		";
			break;
					
			case 2:
			TabSpace = "			";
			break;
					
			case 3:
			TabSpace = "				";
			break;	
					
			case 4:
			TabSpace = "					";
			break;										
		}
		
		ProgressBar1->Position = 0;
		ProgressBar1->Max = FilesCount;
		StatusBar1->Panels->Items[0]->Text = IntToStr(ProgressBar1->Position) + " / " + IntToStr(ProgressBar1->Max);
		Form1->Memo1->Lines->Add("Локализация - Начало");
		Form1->Memo1->Lines->Add("№" + TabSpace + "Время" + TabSpace + "Порог" + TabSpace + "Лок-но" + TabSpace + "Вывод" + TabSpace + "Имя файла");
		for (mainFOR = 0; mainFOR < FilesCount; mainFOR++) //Цикл по файлам
		{
			for (i = 0; i < SigmaSet.Height; i++)
			for (j = 0; j < SigmaSet.Width; j++)
			{
				SigmaSet.Kadr[i][j] 	= 0;
				SigmaSet.FiltKadr[i][j] = 0;
			}

			bool resultReadFiles = false;
			ReadFilesFrames(mainFOR, &resultReadFiles);  //Считываем файл Стандартный или Новый (IKI), получаем SigmaSet.Kadr[i][j]
			if (resultReadFiles == false)
			{
				Ostanov = true;
			}
			if (Ostanov == false)
			{
				if (CheckBox3->Checked)
				FilteringMovingAverage(); //Фильтруем заданным окном и вычитаем среднее+Edit17*СКО
				else
				{    //сюда не идем, не готово, вопрос: что делать без фильтраци?
	//				F4Lines = 0;
	//				Filter4Lines(StrToInt(Form1->Edit17->Text), SigmaSet.Kadr, SigmaSet.Width, SigmaSet.Height); //Фильтрация по 4м строкам
	//				SelectMaxObject(SigmaSet.Kadr, SigmaSet.Width, SigmaSet.Height, 12.5, 12.5, true);
	//
	//				WORD *BufKad = new WORD [SigmaSet.Width*SigmaSet.Height];
	//				int n = 0;
	//				for (i = 0; i < SigmaSet.Height; i++)
	//				for (j = 0; j < SigmaSet.Width; j++)
	//					BufKad[n++] = SigmaSet.Kadr[i][j];
	//
	//				LocalNI (BufKad, SigmaSet.Width, SigmaSet.Height, StrToInt(Form1->Edit13->Text), StrToInt(Form1->Edit14->Text),
	//							StrToInt(Form1->Edit11->Text), StrToInt(Form1->Edit12->Text));
	//				delete [] BufKad;
					FilteringMovingAverage();
				}

				// Результаты локализации при 3х случаях
				if (Nobj > -1) {
					switch (Nobj)
					{
						case 0:
							Form1->Memo1->Lines->Add(IntToStr(mainFOR+1) + TabSpace + TimeToStr(Time()) + TabSpace +  IntToStr(MeanPor) + TabSpace + IntToStr(Nobj) + TabSpace + "ПЛОХ" + TabSpace+ FileNames[mainFOR]);
							fprintf(Cattivo, "%s	%d\n",  FileNames[mainFOR], Nobj);
							Bsch++;
						break;

						case 1:
							if (CheckBox8->Checked == false) //Можно выводить не только ошибки
							Form1->Memo1->Lines->Add(IntToStr(mainFOR+1) + TabSpace + TimeToStr(Time()) + TabSpace +  IntToStr(MeanPor) + TabSpace + IntToStr(Nobj) + TabSpace + "ХОР" + TabSpace + FileNames[mainFOR]);
							fprintf(Cattivo, "%s	%d\n",  FileNames[mainFOR], Nobj);
							Gsch++;

							SelectMaxObject(SigmaSet.FiltKadr, SigmaSet.Width, SigmaSet.Height, Xloc[0], Yloc[0], false);
							fprintf(Buono, "%d	%f	%f	%d	%d	%d	%s\n", Nobj, Xloc[0], Yloc[0], BRloc[0], NelLoc[0], SigmaSet.Ispic, FileNames[mainFOR]);
						break;

					   default:
							if (CheckBox8->Checked == false) //Можно выводить не только ошибки
							Form1->Memo1->Lines->Add(IntToStr(mainFOR+1) + TabSpace + TimeToStr(Time()) + TabSpace +  IntToStr(MeanPor) + TabSpace + IntToStr(Nobj) + TabSpace + "ХОР" + TabSpace + FileNames[mainFOR]);
							fprintf(Cattivo, "%s	%d\n",  FileNames[mainFOR], Nobj);
							Gsch++;

							for (i = 0; i < Nobj; i++)
							{
								SelectMaxObject(SigmaSet.FiltKadr, SigmaSet.Width, SigmaSet.Height, Xloc[i], Yloc[i], false);
								fprintf(Brutto, "%d	%f	%f	%d	%d	%d	%s\n", Nobj, Xloc[i], Yloc[i], BRloc[i], NelLoc[i], SigmaSet.Ispic, FileNames[mainFOR]);
							}
					   break;
					}
					if (ini3->Checked == true) {
						IniSave(AnsiString(Edit23->Text + "\\INI_Sigma - " + bufftime + " - AllStarsList").c_str());
					}
				}

				//Удалить, преобразовано выше
	//			if (Nobj != 1) {    //Кол-во локализованых звезд больше 1 или = 0
	//				Form1->Memo1->Lines->Add(IntToStr(mainFOR+1) + "	" + TimeToStr(Time()) + "	" +  IntToStr(MeanPor) + "	" + IntToStr(Nobj) + "	ПЛОХ	" + FileNames[mainFOR]);
	//				Bsch++;
	//
	//				fprintf(Cattivo, "%s	%d\n",  FileNames[mainFOR], Nobj);
	//
	//				for (i = 0; i < Nobj; i++) {
	//					SelectMaxObject(SigmaSet.FiltKadr, SigmaSet.Width, SigmaSet.Height, Xloc[i], Yloc[i], false);
	//					fprintf(Brutto, "%d	%f	%f	%d	%d	%d	%s\n", Nobj, Xloc[i], Yloc[i], BRloc[i], NelLoc[i], SigmaSet.Ispic, FileNames[mainFOR]);
	//				}
	//
	//			}
	//			else {              //Кол-во локализованых звезд = 1
	//				if (CheckBox8->Checked == false)
	//				Form1->Memo1->Lines->Add(IntToStr(mainFOR+1) + "	" + TimeToStr(Time()) + "	" + IntToStr(MeanPor) + "	" + IntToStr(Nobj) + "	ХОР	" + FileNames[mainFOR]);
	//				Gsch++;
	//				SelectMaxObject(SigmaSet.FiltKadr, SigmaSet.Width, SigmaSet.Height, Xloc[0], Yloc[0], false);
	//				fprintf(Buono, "%d	%f	%f	%d	%d	%d	%s\n", Nobj, Xloc[0], Yloc[0], BRloc[0], NelLoc[0], SigmaSet.Ispic, FileNames[mainFOR]);
	//			}

				ProgressBar1->Position++;
				StatusBar1->Panels->Items[0]->Text = IntToStr(mainFOR+1) + " / " + IntToStr(FilesCount);
				StatusBar1->Panels->Items[2]->Text = "G: " + IntToStr(Gsch) + " / " + IntToStr(FilesCount);
				StatusBar1->Panels->Items[3]->Text = "B: " + IntToStr(Bsch) + " / " + IntToStr(FilesCount);
				Application->ProcessMessages();

				if (Ostanov == true)	mainFOR = FilesCount;
			} //Ostanov
			else mainFOR = FilesCount;
		}
		ProgressBar1->Position = ProgressBar1->Max;
		if (Ostanov == false)
		Form1->Memo1->Lines->Add("№" + TabSpace + "Время" + TabSpace + "Порог" + TabSpace + "Лок-но" + TabSpace + "Вывод" + TabSpace + "Имя файла");
		Form1->Memo1->Lines->Add("Локализация - Завершено");
		fclose(Cattivo);
		fclose(Brutto);
		fclose(Buono);
		for (i = 0; i < StrToInt(Form1->Edit2->Text); i++) delete [] SigmaSet.FiltKadr[i];
		delete [] SigmaSet.FiltKadr;

		for (i = 0; i < StrToInt(Form1->Edit2->Text); i++) delete [] SigmaSet.Kadr[i];
		delete [] SigmaSet.Kadr;

		for (i = 0; i < StrToInt(Form1->Edit9->Text); i++) delete [] SigmaSet.Frame[i];
		delete [] SigmaSet.Frame;
		if (RadioGroup1->ItemIndex == 1)
		delete ikimg;
	}
	else  ShowMessage("Не выбраны файлы");
	}
	else
	{
		ShowMessage("Папка для сохранения результатов не существует!");
	}

	if (Ostanov == true)
	Form1->Memo1->Lines->Add("---Преждевременноое завершение---");

	Ostanov = false;
	Label40->Visible = false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button5Click(TObject *Sender)
{
	int ifile, i, j, goool = 1, Gsch = 0, Bsch = 0;
	FILE *Strim, *Rezu;
	int SigKol, prov;
	const int SigmaNameFi = 200;
	char SigmaNameF[SigmaNameFi];
	bool flagOnceTitle = true;

	Form1->OpenDialog2->Options.Clear();
	Form1->OpenDialog2->Options << ofAllowMultiSelect << ofFileMustExist;

	int CMmMs;
	BoxMmMsStr *BoxMmMs = new BoxMmMsStr[MAX_FILE];

	if (DirectoryListBox2->Visible == true) {
		DirectoryListBox2->Visible = false;
		Button8->Visible = false;
		Button3->Caption = "Обзор...";
	}
	if (DirectoryListBox1->Visible == true) {
		DirectoryListBox1->Visible = false;
		Button7->Visible = false;
		Button2->Caption = "Обзор...";
	}
	struct tm *local;
	time_t t = time(NULL);
	local = localtime(&t);
	char bufftime [80];        // строка, в которой будет храниться текущее время
	strftime(bufftime,80,"%Y.%m.%d %H-%M-%S",local);

	unsigned int start_time, end_time;
	if (Form1->OpenDialog2->Execute())
	{
		start_time =  clock();
//		Form1->Memo1->Lines->Add(TimeToStr(Time()));
		String MemoText = "Start Sigma - ";
		MemoText += TimeToStr(Time());
		Form1->Memo1->Lines->Add(MemoText);
//		Form1->Memo1->Lines->Add("Сигма - Начало");

		//Оценка для термометра и выделение памяти
		ProgressBar1->Position = 0;
		ProgressBar1->Max = 0;
		for (ifile = 0; ifile < Form1->OpenDialog2->Files->Count; ifile++)
		{
			char BufForProgress[300];
			Strim = fopen(AnsiString(OpenDialog2 -> Files -> Strings[ifile]).c_str(), "r");
			while (feof(Strim) == 0)
			{
				 prov = fscanf(Strim, "%s	%s	%s	%s	%s	%s	%[^\n]s", &BufForProgress,&BufForProgress,&BufForProgress,&BufForProgress,&BufForProgress,&BufForProgress,&BufForProgress);
				 ProgressBar1->Max++;
				 StatusBar1->Panels->Items[0]->Text = "Оценка... " + IntToStr(ProgressBar1->Max);
				 Application->ProcessMessages();

				if (flagOnceTitle && feof(Strim) == 0)
				{
					for (i = 0; i < SigmaNameFi; i++) SigmaNameF[i]	 = '\0';
					prov = fscanf(Strim, "%s	%s	%s	%s	%s	%s	%[^\n]s", &BufForProgress,&BufForProgress,&BufForProgress,&BufForProgress,&BufForProgress,&BufForProgress,&SigmaNameF);
					ProgressBar1->Max++;
					StatusBar1->Panels->Items[0]->Text = "Оценка... " + IntToStr(ProgressBar1->Max);
					Application->ProcessMessages();

					FileNames[0] = SigmaNameF;
					MemoryForFrame(); //Выделение памяти для кадра SigmaSet.Kadr[i][j]
					flagOnceTitle = false;
				}

			}
			fclose(Strim);
			ProgressBar1->Max--;
		}

		flagOnceTitle = true;
		StatusBar1->Panels->Items[0]->Text = IntToStr(ProgressBar1->Position) + " / " + IntToStr(ProgressBar1->Max);
		Application->ProcessMessages();

		Label41->Visible = true;

		String TabSpace = "	";
		switch(ComboBox1->ItemIndex)
		{
			case 0:
			TabSpace = "	";
			break;

			case 1:
			TabSpace = "		";
			break;

			case 2:
			TabSpace = "			";
			break;

			case 3:
			TabSpace = "				";
			break;

			case 4:
			TabSpace = "					";
			break;
		}

		Form1->Memo1->Lines->Add("№" + TabSpace + "Время" + TabSpace + "Вывод" + TabSpace + "Х-Y" + TabSpace + "SigmaX" + TabSpace + "SigmaY" + TabSpace + "Mu" + TabSpace + "Имя файла");
		//Form1->Memo1->Lines->Add("№	Время	Результат   Х-Y   SigmaX   SigmaY   Mu   Имя файла");

        //Создание отчета
		if (CheckBox13->Checked) {
//			OpenWord(false);
//			AddDoc();
//
//			SetTextFormat(10, 0, 0, 0, 1, 1);
//			AddTable(ProgressBar1->Max+3, 13);
//
//			UnionCell(1, 1, 2, 1);
//			UnionCell(1, 2, 2, 2);
//			UnionCell(1, 3, 2, 3);
//			UnionCell(1, 4, 2, 4);
//			UnionCell(1, 5, 2, 5);
//			UnionCell(1, 8, 2, 8);
//			UnionCell(1, 9, 2, 9);
//			UnionCell(1, 10, 2, 10);
//			UnionCell(1, 11, 2, 11);
//			UnionCell(1, 12, 2, 12);
//			UnionCell(1, 13, 2, 13);
//			UnionCell(1, 6, 1, 7);
//
//			SetCell(1, 1, "ЗВ\nm");
//			SetCell(1, 2, "t, мс");
//			SetCell(1, 3, "I");
//			SetCell(1, 4, "I mean");
//			SetCell(1, 5, "N");
//			SetCell(1, 6, "Sigma");
//			SetCell(2, 6, "X");
//			SetCell(2, 7, "Y");
//			SetCell(1, 7, "Mean");
//			SetCell(1, 8, "SKO");
//			SetCell(1, 9, "Порог");
//			SetCell(1, 10, "Imax");
//			SetCell(1, 11, "I фотом");
//			SetCell(1, 12, "I приб");
		}

		//IniSave(Edit23->Text + "\\INI_Sigma_" + bufftime);

		//Цикл по выбранным текстовым файлам
		for (ifile = 0; ifile < Form1->OpenDialog2->Files->Count; ifile++)
		{
			Strim = fopen(AnsiString(OpenDialog2 -> Files -> Strings[ifile]).c_str(), "r");
			if (DirectoryExists(AnsiString(Edit23->Text).c_str()) == true)
			{
				Rezu =  fopen(AnsiString(Edit23->Text + "\\Sigma - " + bufftime + " - REZ.txt").c_str(), "a");
			}
			else    ShowMessage("Папка для сохранения результатов не существует!");
			CMmMs = 0;
			int rarr = 1;
			//Обработка каждого выбранного текстового файла до конца построчно
			while (feof(Strim) == 0)
			{
				if (flagOnceTitle)
				{
//					fprintf(Rezu, "N_Stars	X	Y	Is_loc	N_loc	CenterPix	Is_frame	N_frame	Mu	X_Gauss	Y_Gauss	X_CenterPixGauss	Y_CenterPixGauss	SigmaX	SigmaY	NameFile\n");
					fprintf(Rezu, "N_Stars	X	Y	Is_loc	N_loc	CenterPix	Is_frame	N_frame	Mean_Th	SKO_Th	Th	Is_fot	Mu	X_Gauss	Y_Gauss	X_CenterPixGauss	Y_CenterPixGauss	SigmaX	SigmaY	NameFile\n");
					flagOnceTitle = false;
				}
				FileNames[0] = "";
				for (i = 0; i < SigmaNameFi; i++) SigmaNameF[i]	 = '\0';
				SigKol       = 0;
				SigmaSet.x   = 0;
				SigmaSet.y   = 0;
				SigmaSet.Is  = 0;
				SigmaSet.N   = 0;
				SigmaSet.Is2   = 0;
				SigmaSet.N2   = 0;
				SigmaSet.Ispic = 0;
				prov = fscanf(Strim, "%d	%f	%f	%d	%d	%d	%[^\n]s", &SigKol, &SigmaSet.x, &SigmaSet.y, &SigmaSet.Is, &SigmaSet.N, &SigmaSet.Ispic, &SigmaNameF);
				FileNames[0] = SigmaNameF;

				//Если стоит обработка самого яркого элемента на кадре
				//Первая колонка в текстовом файле - количество локализованных объектов
				if (CheckBox4->Checked && SigKol > 1 && prov == 7)
				{
					switch (ComboBoxPixIs->ItemIndex)
					{
						case 0:  // по яркости
							for (i = 0; i < SigKol-1; i++)
							{
								int BufIs, BufN, BufKol, BufIsM;
								float BufX, BufY;
								prov = fscanf(Strim, "%d	%f	%f	%d	%d	%d	%[^\n]s", &BufKol, &BufX, &BufY, &BufIs, &BufN, &BufIsM, &SigmaNameF);
								if (BufIs > SigmaSet.Is && SigKol == BufKol && prov == 7)
								{
									FileNames[0] = SigmaNameF;
									SigKol       = BufKol;
									SigmaSet.x   = BufX;
									SigmaSet.y   = BufY;
									SigmaSet.Is  = BufIs;
									SigmaSet.N   = BufN;
									SigmaSet.Ispic = BufIsM;
								}
							}
							break;

						case 1:  //по ярчайшему пикселю в объекте
							for (i = 0; i < SigKol-1; i++)
							{
								int BufIs, BufN, BufKol, BufIsM;
								float BufX, BufY;
								prov = fscanf(Strim, "%d	%f	%f	%d	%d	%d	%[^\n]s", &BufKol, &BufX, &BufY, &BufIs, &BufN, &BufIsM, &SigmaNameF);
								if (BufIsM > SigmaSet.Ispic && SigKol == BufKol && prov == 7)
								{
									FileNames[0] = SigmaNameF;
									SigKol       = BufKol;
									SigmaSet.x   = BufX;
									SigmaSet.y   = BufY;
									SigmaSet.Is  = BufIs;
									SigmaSet.N   = BufN;
									SigmaSet.Ispic = BufIsM;
								}
							}
							break;
					}

				}

				if (Ostanov == false) {
				if (FileNames[0] != "" && prov == 7) {
				if (SigmaSet.Ispic < StrToInt(Edit15->Text) || SigmaSet.Ispic > StrToInt(Edit16->Text))
				{
					Form1->Memo1->Lines->Add(IntToStr(ProgressBar1->Position+1) + TabSpace + TimeToStr(Time()) + TabSpace + "ЯРЧ!" + TabSpace + FloatToStr((int)SigmaSet.x) + "x" + FloatToStr((int)SigmaSet.y) + TabSpace + "-" + TabSpace + "-" + TabSpace + "-" + TabSpace + FileNames[0]);
					Bsch++;
				}
				else
				{
					// ММ и МС из названия файла в SigmaSet.mm и SigmaSet.ms
//					IdentifFromName(AnsiString(FileNames[0]).c_str(), AnsiString(Edit8->Text).c_str(), AnsiString(Edit6->Text).c_str(), &SigmaSet.ms);
//					IdentifFromName(AnsiString(FileNames[0]).c_str(), AnsiString(Edit22->Text).c_str(), AnsiString(Edit7->Text).c_str(), &SigmaSet.mm);
//					   SigmaSet.ms = 0; SigmaSet.mm = 0;
					   //%10.3f	%10.3f
					//Заполнение массива BoxMmMs (учет координат)
//					if (CMmMs == 0) { //Первое вхождение в структуру BoxMmMs
//						BoxMmMs[0].ms = SigmaSet.ms;
//						BoxMmMs[0].mm = SigmaSet.mm;
//						BoxMmMs[0].x = SigmaSet.x;
//						BoxMmMs[0].y = SigmaSet.y;
//						BoxMmMs[0].ColKad = 0;
//						SigmaSet.CMmMs = CMmMs++;
//					}
//					else {  //Очередное вхождение в структуру BoxMmMs
//						bool Fl1 = true;
//						for (i = 0; i < CMmMs; i++) {
//							if ( (BoxMmMs[i].ms == SigmaSet.ms) && (BoxMmMs[i].mm == SigmaSet.mm) )
//							if ( ((BoxMmMs[i].x+StrToInt(Form1->Edit11->Text)) >= SigmaSet.x) && ((BoxMmMs[i].x-StrToInt(Form1->Edit11->Text)) <= SigmaSet.x)
//								  && ((BoxMmMs[i].y+StrToInt(Form1->Edit11->Text)) >= SigmaSet.y) && ((BoxMmMs[i].y-StrToInt(Form1->Edit11->Text)) <= SigmaSet.y) )
//							{
//								// Условия выполнены, такие координаты и идентификаторы уже существуют
//								BoxMmMs[i].ColKad++;
//								SigmaSet.CMmMs = i;
//								Fl1 = false;
//							}
//						}
//							if (Fl1) { // В цикле ничего не найдено, значит это новый набор
//								BoxMmMs[CMmMs].ms = SigmaSet.ms;
//								BoxMmMs[CMmMs].mm = SigmaSet.mm;
//								BoxMmMs[CMmMs].x = SigmaSet.x;
//								BoxMmMs[CMmMs].y = SigmaSet.y;
//								BoxMmMs[CMmMs].ColKad++;
//								SigmaSet.CMmMs = CMmMs;
//								CMmMs++;
//							}
//					}
					//КОНЕЦ - Заполнение массива BoxMmMs (учет координат)
					bool resultReadFiles = false;
					ReadFilesFrames(0, &resultReadFiles);  //Считываем файл Стандартный или Новый (IKI), получаем SigmaSet.Kadr[i][j]
					if (resultReadFiles == false)
					{
						if (MessageDlg("Продолжить выполнение?", mtConfirmation, TMsgDlgButtons() << mbYes << mbNo,0) == mrNo)
							Ostanov = true;
					}
//					if (Ostanov == false)


					SigmaSet.HWframe = StrToInt(Edit9->Text);
					if (CheckBox2->Checked)
					BinFrameSigma();  //Бинирование
					else
					{
						//Вырезание окошка
						for (i = 0; i < SigmaSet.HWframe; i++)
						for (j = 0; j < SigmaSet.HWframe; j++)
						if ( (int)(i+SigmaSet.y-SigmaSet.HWframe/2) >= 0 && (int)(i+SigmaSet.y-SigmaSet.HWframe/2) < SigmaSet.Height &&
							 (int)(j+SigmaSet.x-SigmaSet.HWframe/2) >= 0 && (int)(j+SigmaSet.x-SigmaSet.HWframe/2) < SigmaSet.Width )
						{
							SigmaSet.Frame[i][j] = SigmaSet.Kadr[(int)(i+SigmaSet.y-SigmaSet.HWframe/2)][(int)(j+SigmaSet.x-SigmaSet.HWframe/2)];
						}
						else SigmaSet.Frame[i][j] = 0;
					}

					if (BKflag)
					{
						for (i = 0; i < SigmaSet.HWframe; i++)
						for (j = 0; j < SigmaSet.HWframe; j++)
							BlackKadrWindow[i][j] = BlackKadr[(int)(i+SigmaSet.y-SigmaSet.HWframe/2)][(int)(j+SigmaSet.x-SigmaSet.HWframe/2)];
					}

					float IYasa = 0;
					BufLOC1 = 0;
					if (CheckBox13->Checked)
					{
						int jLOC = 0;
						for (j = 0; j < 4; j++){
						for (i = 0; i < SigmaSet.HWframe; i++)
							BufLOC1 += SigmaSet.Frame[jLOC][i];
							if (j == 0) jLOC++;
							else jLOC = SigmaSet.HWframe-j;
						}

						BufLOC1 = BufLOC1/(SigmaSet.HWframe*4);

						for (i = 0; i < SigmaSet.HWframe; i++)
						for (j = 0; j < SigmaSet.HWframe; j++)
						IYasa += SigmaSet.Frame[i][j] - BufLOC1;
					}
//				//Проверка - вывод кадра
//				FILE *streG = fopen(AnsiString("C:\\Cybertron\\Programms\\WindKBM41.dat").c_str(), "wb");
//				for (i = 0; i < SigmaSet.HWframe; i++) {
//				   fwrite(SigmaSet.Frame[i], sizeof(WORD), SigmaSet.HWframe, streG);}
//				fclose(streG);

					F4Lines = 0;

					if (CheckBox14->Checked) { // расчет сигмы по сектору, не выделяя звезду
						BufLOCdSKO = 0;
						int jLOC = 0;
						for (j = 0; j < 4; j++){
						for (i = 0; i < SigmaSet.HWframe; i++)
							BufLOCdSKO += SigmaSet.Frame[jLOC][i];
							if (j == 0) jLOC++;
							else jLOC = SigmaSet.HWframe-j;
						}

						BufLOCdSKO = BufLOCdSKO/(SigmaSet.HWframe*4);
						Form1->Edit26->Text = "10";
					}
					else
					Filter4Lines(StrToInt(Form1->Edit28->Text), SigmaSet.Frame, SigmaSet.HWframe, SigmaSet.HWframe); //Фильтрация по 4м строкам







//						for (i = 0; i < SigmaSet.HWframe; i++)
//						for (j = 0; j < SigmaSet.HWframe; j++)
//						if ((SigmaSet.Frame[i][j] - 360) > 0)
//						{
//							SigmaSet.Frame[i][j] = SigmaSet.Frame[i][j] - 360;
//						}
//						else SigmaSet.Frame[i][j] = 0;
//				//Проверка - вывод кадра
//				streG = fopen(AnsiString("C:\\Cybertron\\Programms\\WindKBM42.dat").c_str(), "wb");
//				for (i = 0; i < SigmaSet.HWframe; i++) {
//				   fwrite(SigmaSet.Frame[i], sizeof(WORD), SigmaSet.HWframe, streG);}
//				fclose(streG);

					SelectMaxObject(SigmaSet.Frame, SigmaSet.HWframe, SigmaSet.HWframe, SigmaSet.HWframe/2, SigmaSet.HWframe/2, true);

					if (SigmaSet.N2 >= StrToInt(Edit52->Text))
					{
					fprintf(Rezu, "%d	%f	%f	%d	%d	%d", SigKol, SigmaSet.x, SigmaSet.y, SigmaSet.Is, SigmaSet.N, SigmaSet.Ispic);
//					fprintf(Rezu, "	%d	%d", SigmaSet.Is2, SigmaSet.N2);
					fprintf(Rezu, "	%d	%d	%5.2f	%5.2f	%d	%d", SigmaSet.Is2, SigmaSet.N2, BufLOC1, BufLOCdSKO, F4Lines, Is_fot);
					//Вычитаем F4Lines обратно
//					for (i = 0; i < SigmaSet.HWframe; i++)
//					for (j = 0; j < SigmaSet.HWframe; j++)
//					if ((SigmaSet.Frame[i][j] - F4Lines) > 0)
//						SigmaSet.Frame[i][j] = SigmaSet.Frame[i][j] - F4Lines;
//					else  SigmaSet.Frame[i][j] = 0;


//				//Проверка - вывод кадра
//				streG = fopen(AnsiString("C:\\Cybertron\\Programms\\WindKBM43.dat").c_str(), "wb");
//				for (i = 0; i < SigmaSet.HWframe; i++) {
//				   fwrite(SigmaSet.Frame[i], sizeof(WORD), SigmaSet.HWframe, streG);}
//				fclose(streG);


						double Mug=0, SigX=0, SigY=0, Sx0=0, Sy0=0, xi_G = 0, yi_G = 0;
						//Сигма
						if (true) {                                  //SigmaSet.HWframe*SigmaSet.HWframe

//							int pix_X[MAX_FILE];
//							int pix_Y[MAX_FILE];
//							int pix_val[MAX_FILE];
							int *pix_X	= new int [SigmaSet.HWframe*SigmaSet.HWframe];
							int *pix_Y	= new int [SigmaSet.HWframe*SigmaSet.HWframe];
							int *pix_val= new int [SigmaSet.HWframe*SigmaSet.HWframe];

							int numMeas = 0;

							for (i = 0; i < SigmaSet.HWframe; i++)
							{
								SigmaSet.Frame[i][0] = 0;
								SigmaSet.Frame[i][SigmaSet.HWframe-1] = 0;
								SigmaSet.Frame[0][i] = 0;
								SigmaSet.Frame[SigmaSet.HWframe-1][i] = 0;
							}

							int Fl3 = 0;
							for (i = 1; i < SigmaSet.HWframe-1; i++)
							for (j = 1; j < SigmaSet.HWframe-1; j++)
							{
								if (SigmaSet.Frame[i][j] == 0)
								{
									if (SigmaSet.Frame[i-1][j] > 0) Fl3++;
									if (SigmaSet.Frame[i+1][j] > 0) Fl3++;
									if (SigmaSet.Frame[i][j-1] > 0) Fl3++;
									if (SigmaSet.Frame[i][j+1] > 0) Fl3++;

									if (SigmaSet.Frame[i-1][j-1] > 0) Fl3++;
									if (SigmaSet.Frame[i+1][j+1] > 0) Fl3++;
									if (SigmaSet.Frame[i+1][j-1] > 0) Fl3++;
									if (SigmaSet.Frame[i-1][j+1] > 0) Fl3++;

									if (Fl3 >= StrToInt(Form1->Edit26->Text) ) {
										pix_val[numMeas] = 1;
										pix_X[numMeas] = j;
										pix_Y[numMeas++] = i;
									}
									Fl3 = 0;
								}
								else
									if (SigmaSet.Frame[i][j] > 0)
									{
										if (CheckBox14->Checked)
										pix_val[numMeas] = SigmaSet.Frame[i][j] - BufLOCdSKO;//+ 1 + BufLOCdSKO*StrToInt(Form1->Edit28->Text); // DELETE NOW
										else
										pix_val[numMeas] = SigmaSet.Frame[i][j] + 1 + BufLOCdSKO*StrToInt(Form1->Edit28->Text); // DELETE NOW

										pix_X[numMeas] = j;
										pix_Y[numMeas++] = i;
									}
								}


//					FILE *stre3 = fopen(AnsiString("C:\\Cybertron\\Programms\\pix_val-"+IntToStr(rarr++)+".txt").c_str(), "a");
//					Fl3 = 0;
//					if (stre3 != NULL)
//					for (i = 0; i < SigmaSet.HWframe; i++)
//					{
//						for (j = 0; j < SigmaSet.HWframe; j++)
//						{
//							if (i == pix_Y[Fl3] && j == pix_X[Fl3]) {
//								fprintf(stre3, "%d	", pix_val[Fl3++]);
//							}
//							else fprintf(stre3, "0	");
//
//						}
//						fprintf(stre3, "\n");
//					}
//					fclose(stre3);

//				//Проверка - вывод кадра
//					for (i = 0; i < SigmaSet.HWframe; i++)
//					for (j = 0; j < SigmaSet.HWframe; j++)
//						SigmaSet.Frame[i][j] = 0;
//
//					for (Fl3 = 0; Fl3 < numMeas; Fl3++)
//						SigmaSet.Frame[pix_Y[Fl3]][pix_X[Fl3]] = pix_val[Fl3];
//
//				FILE *streG = fopen(AnsiString(Edit23->Text + "\\WindKBM-Итог-"+IntToStr(goool++)+".dat").c_str(), "wb");
//				for (i = 0; i < SigmaSet.HWframe; i++) {
//				   fwrite(SigmaSet.Frame[i], sizeof(WORD), SigmaSet.HWframe, streG);}
//				fclose(streG);


//							LocalGauss(pix_X, pix_Y, pix_val, numMeas, , , &Sx0, &Sy0, &SigX, &SigY, &Mug);  //numMeas or numMeas-1

							LocalGauss(pix_X, pix_Y, pix_val, numMeas, StrToInt(Form1->Edit25->Text), StrToInt(Form1->Edit24->Text), 2, 0.5, &Sx0, &Sy0, &SigX, &SigY, &Mug, &xi_G, &yi_G);
							fprintf(Rezu, "	%f	%f	%f	%f	%f	%f	%f	%s\n", Mug, Sx0, Sy0, xi_G, yi_G, SigX, SigY, FileNames[0]);

							delete [] pix_X;
							delete [] pix_Y;
							delete [] pix_val;
							Gsch++;

							if (CheckBox13->Checked)
							{
								float BufReport10 = 0;
								IdentifFromName(AnsiString(FileNames[0]).c_str(), AnsiString(Edit8->Text).c_str() , AnsiString(Edit6->Text).c_str(), &BufReport10);
								SetCell(ProgressBar1->Position+3, 2, FloatToStr(FixRoundTo(BufReport10,-3)));
								BufReport10 = 0;
								IdentifFromName(AnsiString(FileNames[0]).c_str(), AnsiString(Edit22->Text).c_str(), AnsiString(Edit7->Text).c_str(), &BufReport10);
								SetCell(ProgressBar1->Position+3, 1, FloatToStr(BufReport10));

								SetCell(ProgressBar1->Position+3, 3, IntToStr(SigmaSet.Is2));
								SetCell(ProgressBar1->Position+3, 4, FloatToStr(FixRoundTo(IYasa,-1)));
								SetCell(ProgressBar1->Position+3, 5, IntToStr(SigmaSet.N2));
								SetCell(ProgressBar1->Position+3, 6, FloatToStr(FixRoundTo(SigX,-3)));
								SetCell(ProgressBar1->Position+3, 7, FloatToStr(FixRoundTo(SigY,-3)));
								SetCell(ProgressBar1->Position+3, 8, FloatToStr(FixRoundTo(BufLOC1,-1)));
								SetCell(ProgressBar1->Position+3, 9, FloatToStr(FixRoundTo(BufLOCdSKO,-1)));
								SetCell(ProgressBar1->Position+3, 10, IntToStr(F4Lines));
								SetCell(ProgressBar1->Position+3, 11, IntToStr(SigmaSet.Ispic));
								SetCell(ProgressBar1->Position+3, 12, FloatToStr(FixRoundTo(pow(2.512f,BufReport10)*IYasa,-1)));
								SetCell(ProgressBar1->Position+3, 13, FloatToStr(FixRoundTo(pow(2.512f,BufReport10)*SigmaSet.Is2,-1)));
							}


//							int *pix_val0 = new int [SigmaSet.HWframe*SigmaSet.HWframe];
//							int schic = 0;
//
//							int *pix_X0 = new int [SigmaSet.HWframe];
//							int *pix_Y0 = new int [SigmaSet.HWframe];
//							for(int i=0; i<SigmaSet.HWframe; i++) { pix_X0[i] = i; pix_Y0[i] = i; }
//
//							for (i = 0; i < SigmaSet.HWframe; i++)
//							for (j = 0; j < SigmaSet.HWframe; j++) {
//								pix_val0[schic] = Mask[i][j];
//								if (pix_val0[schic] < 0) {  pix_val0[schic] = 0;	}
//								schic++;
//							}

//							int *pix_X = new int [SigmaSet.HWframe];
//							int *pix_Y = new int [SigmaSet.HWframe];
//							int *pix_val = new int [SigmaSet.HWframe*SigmaSet.HWframe];
//
//							int numMeas = 0;
//							 ii;
//							for (ii = 0; ii < SigmaSet.HWframe*SigmaSet.HWframe; ii++)
//							{
//								if (pix_val0[ii] > 0) {
//									pix_val[numMeas] = pix_val0[ii];
//									pix_X[numMeas] = pix_X0[ii%SigmaSet.HWframe];
//									pix_Y[numMeas++] = pix_Y0[ii/SigmaSet.HWframe];
//								}
//							}
//
//
//
//					int Fl3 = 0;
//					for (i = 1; i < SigmaSet.HWframe-1; i++)
//					for (j = 1; j < SigmaSet.HWframe-1; j++) {
//						if (SigmaSet.Frame[i][j] == 0){
//							if (SigmaSet.Frame[i-1][j] > 0) Fl3++;
//							if (SigmaSet.Frame[i+1][j] > 0) Fl3++;
//							if (SigmaSet.Frame[i][j-1] > 0) Fl3++;
//							if (SigmaSet.Frame[i][j+1] > 0) Fl3++;
//
//							if (SigmaSet.Frame[i-1][j-1] > 0) Fl3++;
//							if (SigmaSet.Frame[i+1][j+1] > 0) Fl3++;
//							if (SigmaSet.Frame[i+1][j-1] > 0) Fl3++;
//							if (SigmaSet.Frame[i-1][j+1] > 0) Fl3++;
//
//							if (Fl3 >= 3 ) {
//								pix_val[numMeas] = 1;
//								pix_X[numMeas] = j;
//								pix_Y[numMeas++] = i;
//							}
//							Fl3 = 0;
//						}
//						else
//						if (SigmaSet.Frame[i][j] > 0) {
//							pix_val[numMeas] = SigmaSet.Frame[i][j]+1;
//							pix_X[numMeas] = j;
//							pix_Y[numMeas++] = i;
//						}
//
//						}
//
//
////					FILE *stre3 = fopen(AnsiString("C:\\Cybertron\\Programms\\pix_val-"+IntToStr(rarr++)+".txt").c_str(), "a");
////					Fl3 = 0;
////					if (stre3 != NULL)
////					for (i = 0; i < SigmaSet.HWframe; i++)
////					{
////						for (j = 0; j < SigmaSet.HWframe; j++)
////						{
////							if (i == pix_Y[Fl3] && j == pix_X[Fl3]) {
////								fprintf(stre3, "%d	", pix_val[Fl3++]);
////							}
////							else fprintf(stre3, "0	");
////
////						}
////						fprintf(stre3, "\n");
////					}
////					fclose(stre3);
//
//
//
//
////							Fl3 = 0;
////
////							for (j = 0; j < numMeas; j++)
////							{
////								fprintf(stre3, "%d	", pix_val[Fl3++]);
////							}
////							fclose(stre3);
////
////							Fl3 = 0;
////							stre3 = fopen(AnsiString("C:\\Cybertron\\Programms\\pix_X.txt").c_str(), "a");
////							for (j = 0; j < numMeas; j++)
////							{
////								fprintf(stre3, "%d	", pix_X[Fl3++]);
////							}
////							fclose(stre3);
////
////							Fl3 = 0;
////							stre3 = fopen(AnsiString("C:\\Cybertron\\Programms\\pix_Y.txt").c_str(), "a");
////							for (j = 0; j < numMeas; j++)
////							{
////								fprintf(stre3, "%d	", pix_Y[Fl3++]);
////							}
////							fclose(stre3);
//
//
////							end_time = clock();
////							Form1->Memo1->Lines->Add(TimeToStr(Time()) + " - " + FloatToStr((end_time - start_time)/1000.0));
////							fprintf(Rezu, "\n");
////							int it, tr;
////							for (it = 1; it < 11; it++)
////							for (tr = 1; tr < 101; tr++) {
////								start_time = clock();
//								LocalGauss(pix_X, pix_Y, pix_val, numMeas, 4, 10, &Sx0, &Sy0, &SigX, &SigY, &Mug);  //numMeas or numMeas-1
////								end_time = clock();
////								fprintf(Rezu, "%d	%d	%f	%f	%f	%f	%f	%f	%s\n", it, tr, Mug, Sx0, Sy0, SigX, SigY, (end_time - start_time)/1000.0, FileNames[0]);
////							}
//
////							end_time = clock();
////							Form1->Memo1->Lines->Add(TimeToStr(Time()) + " - " + FloatToStr((end_time - start_time)/1000.0));
////							start_time = clock();
////							LocalGauss(pix_X, pix_Y, pix_val, numMeas-1, 5, 10, &Sx0, &Sy0, &SigX, &SigY, &Mug);  //numMeas or numMeas-1
////							end_time = clock();
////							Form1->Memo1->Lines->Add(TimeToStr(Time()) + " - " + FloatToStr((end_time - start_time)/1000.0));
////							start_time = clock();
////							Sx0 = Sx0+0.5;
////							Sy0 = Sy0+0.5;
//							fprintf(Rezu, "	%f	%f	%f	%f	%f\n", Mug, Sx0, Sy0, SigX, SigY);
//
////							delete [] pix_val0;
//							delete [] pix_X;
//							delete [] pix_Y;
//							delete [] pix_val;
////							delete [] pix_X0;
////							delete [] pix_Y0;
					}

				   if (Mug>10)
					   Form1->Memo1->Lines->Add(IntToStr(ProgressBar1->Position+1) + TabSpace + TimeToStr(Time()) + TabSpace + "Плохое уравнивание по Мю" + TabSpace+ FloatToStr((int)SigmaSet.x) + "x" + FloatToStr((int)SigmaSet.y) + TabSpace + FloatToStr(FixRoundTo(SigX,-3)) + TabSpace + FloatToStr(FixRoundTo(SigY,-3)) + TabSpace + FloatToStr(FixRoundTo(Mug,-3)) + TabSpace + FileNames[0]);
				   else
					 if (CheckBox8->Checked == false)
					 Form1->Memo1->Lines->Add(IntToStr(ProgressBar1->Position+1) + TabSpace + TimeToStr(Time()) + TabSpace +"ОК" + TabSpace + FloatToStr((int)SigmaSet.x) + "x" + FloatToStr((int)SigmaSet.y) + TabSpace + FloatToStr(FixRoundTo(SigX,-3)) + TabSpace + FloatToStr(FixRoundTo(SigY,-3)) + TabSpace + FloatToStr(FixRoundTo(Mug,-3)) + TabSpace + FileNames[0]);

				}
				else {
					Form1->Memo1->Lines->Add(IntToStr(ProgressBar1->Position+1) + TabSpace + TimeToStr(Time()) + TabSpace + "ПИКС!" + TabSpace + FloatToStr((int)SigmaSet.x) + "x" + FloatToStr((int)SigmaSet.y) + TabSpace + "-" + TabSpace + "-" + TabSpace + "-" + TabSpace + FileNames[0]);
					Bsch++;
				}

				} }}
			ProgressBar1->Position++;
			StatusBar1->Panels->Items[0]->Text = IntToStr(ProgressBar1->Position) + " / " + IntToStr(ProgressBar1->Max);
			StatusBar1->Panels->Items[2]->Text = "G: " + IntToStr(Gsch) + " / " + IntToStr(ProgressBar1->Max);
			StatusBar1->Panels->Items[3]->Text = "B: " + IntToStr(Bsch) + " / " + IntToStr(ProgressBar1->Max);
			Application->ProcessMessages();
			}
			fclose(Rezu);
			fclose(Strim);

			if (ini3->Checked == true) {
				IniSave(AnsiString(Edit23->Text + "\\INI_Sigma - " + bufftime + " - REZ").c_str());
			}
		}
//	Form1->Memo1->Lines->Add("№	Время	Результат   Х-Y   SigmaX   SigmaY   Mu   Имя файла");
//	Form1->Memo1->Lines->Add("Сигма - Завершено");
	end_time = clock();
//	Form1->Memo1->Lines->Add(TimeToStr(Time()) + " - " + FloatToStr((end_time - start_time)/1000.0));

	MemoText = "Done Sigma - ";
	MemoText += TimeToStr(Time());
	Form1->Memo1->Lines->Add(MemoText);
	MemoText = "Total Time: ";
	MemoText += FloatToStr((end_time - start_time)/1000.0);
	Form1->Memo1->Lines->Add(MemoText);

	if (CheckBox13->Checked)
	{
//		SaveDoc(Edit23->Text+"\\Report_IOZ.doc");
//		CloseDoc();
//		CloseWord();
	}

	}
	else ShowMessage("Не выбраны файлы");

	for (i = 0; i < SigmaSet.Height; i++) delete [] SigmaSet.FiltKadr[i];
	delete [] SigmaSet.FiltKadr;

	for (i = 0; i < SigmaSet.Height; i++) delete [] SigmaSet.Kadr[i];
	delete [] SigmaSet.Kadr;

	for (i = 0; i < StrToInt(Form1->Edit9->Text); i++) delete [] SigmaSet.Frame[i];
	delete [] SigmaSet.Frame;
	delete [] BoxMmMs;
	if (RadioGroup1->ItemIndex == 1)
	delete ikimg;


	Label41->Visible = false;

	if (Ostanov == true)
	Form1->Memo1->Lines->Add("Stoppage");

	Ostanov = false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::StatusBar1DrawPanel(TStatusBar *StatusBar, TStatusPanel *Panel,
		  const TRect &Rect)
{
if (Panel->Index == 1)
	{
	  ProgressBar1->SetBounds(Rect.Left - 1, Rect.Top - 1,  Rect.Right - Rect.Left + 2, Rect.Bottom - Rect.Top + 2 );
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N1Click(TObject *Sender)
{
	Form2 -> Show();
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button4MouseEnter(TObject *Sender)
{
	Edit1 -> Color = clInfoBk;
	Edit2 -> Color = clInfoBk;
	Edit11 -> Color = clInfoBk;
	Edit12 -> Color = clInfoBk;
	Edit13 -> Color = clInfoBk;
	Edit14 -> Color = clInfoBk;
	Edit17 -> Color = clInfoBk;
	Edit23 -> Color = clInfoBk;

	if (CheckBox1->Checked == false)
	{
		Edit3 -> Color = clInfoBk;
		Edit4 -> Color = clInfoBk;
	}
	if (CheckBox3->Checked) Edit10 -> Color = clInfoBk;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button4MouseLeave(TObject *Sender)
{
	Edit1 -> Color = clWindow;
	Edit2 -> Color = clWindow;
	Edit11 -> Color = clWindow;
	Edit12 -> Color = clWindow;
	Edit13 -> Color = clWindow;
	Edit14 -> Color = clWindow;
	Edit17 -> Color = clWindow;
	Edit23 -> Color = clWindow;

	if (CheckBox1->Checked == false)
	{
		Edit3 -> Color = clWindow;
		Edit4 -> Color = clWindow;
	}
	if (CheckBox3->Checked) Edit10 -> Color = clWindow;
}
//---------------------------------------------------------------------------

//Расчет BoxXY  BoxMS  BoxMM  для отчета  ПЕРЕДЕЛАТЬ
void ReportSort(FullSigmaTable *FullSigmaFS, int MeterFS, int PorogPxFS,
				SBoxXY *BoxXYFS, int *NomBoxXYFS, SBoxXY *BoxMSFS, int *NomBoxMSFS, SBoxXY *BoxMMFS, int *NomBoxMMFS)
{

	float EPSILON = 0.0001;
	int i, f, jj;
	bool flagFS;

	//Сортировка по координатам (массив наборов, например 5 наборов по 35 (BoxXYFS[i].N) точек)
	jj = *NomBoxXYFS;
	for (f = 1; f < MeterFS; f++)
	{
		flagFS = false;
		for (i = 0; i < *NomBoxXYFS; i++)
		{
			if ( ((FullSigmaFS[f].x + PorogPxFS) >= BoxXYFS[i].x) && ((FullSigmaFS[f].x - PorogPxFS) <= BoxXYFS[i].x) &&
				 ((FullSigmaFS[f].y + PorogPxFS) >= BoxXYFS[i].y) && ((FullSigmaFS[f].y - PorogPxFS) <= BoxXYFS[i].y) )
				 {
					BoxXYFS[i].N++;
					flagFS = true;
				 }
			 else
			 {
				if ((i+1) == *NomBoxXYFS && flagFS == false)
				{
					BoxXYFS[*NomBoxXYFS].x = FullSigmaFS[f].x;
					BoxXYFS[*NomBoxXYFS].y = FullSigmaFS[f].y;
					jj++;
					*NomBoxXYFS = jj;
				}
			 }
		}
	}

	//Сортировка по мс (массив наборов, например 10 наборов по 20 (BoxMSFS[i].N) точек)
	jj = *NomBoxMSFS;
	for (f = 1; f < MeterFS; f++)
	{
		flagFS = false;
		for (i = 0; i < *NomBoxMSFS; i++)
		{
			if(std::fabs(FullSigmaFS[f].ms - BoxMSFS[i].ms) < EPSILON)
			{
				BoxMSFS[i].N++;
				flagFS = true;
			}
			/// А что если у нас есть уже 125 мс, а мы хотим добавить 124, плохая проверка
			//if ( ((FullSigmaFS[f].ms + 1) >= BoxMSFS[i].ms) && ((FullSigmaFS[f].ms - 1) <= BoxMSFS[i].ms) )
			//	 {
			//		BoxMSFS[i].N++;
			//		flagFS = true;
			//	 }
			 else
			 {
				if (((i+1) == *NomBoxMSFS) && (flagFS == false))
				{
					BoxMSFS[*NomBoxMSFS].ms = FullSigmaFS[f].ms;
					jj++;
					*NomBoxMSFS = jj;
				}
			 }
		}
	}

	//Сортировка по мм (массив наборов, например 10 наборов по 20 (BoxMMFS[i].N) точек)
	jj = *NomBoxMMFS;
	for (f = 1; f < MeterFS; f++)
	{
		flagFS = false;
		for (i = 0; i < *NomBoxMMFS; i++)
		{
			if ( (FullSigmaFS[f].mm >= BoxMMFS[i].mm) && (FullSigmaFS[f].mm <= BoxMMFS[i].mm) )
				 {
					BoxMMFS[i].N++;
					flagFS = true;
				 }
			else
			{
				if (((i+1) == *NomBoxMMFS) && (flagFS == false))
				{
					BoxMMFS[*NomBoxMMFS].mm = FullSigmaFS[f].mm;
					jj++;
					*NomBoxMMFS = jj;
				}
			}
		}
	}
}
//---------------------------------------------------------------------------
float ConvMM (float a)
{
	float b;
	b = (a - StrToFloat(Form1->Edit18->Text)) * (StrToFloat(Form1->Edit45->Text)
		* StrToFloat(Form1->Edit45->Text)) / (StrToFloat(Form1->Edit46->Text)
		* StrToFloat(Form1->Edit46->Text));
	return b;
}

int qfloatX(const void *pa, const void *pb) {
	float a = *(float*)pa, b = *(float*)pb;
    if (a > b) return 1;
    if (a < b) return -1;
    return 0;
}
//---------------------------------------------------------------------------

float FindOneRootOfEquation(float X, int MNK_deg, double* K)
{
	double Xstepen = 1;
	float Y = 0;

	for (int deg = 0; deg < MNK_deg+1; deg++)
	{
		if (deg > 0)
		{
			Xstepen *= X;
		}
		Y += K[deg] * Xstepen;
	}

	return Y;
}


void MakeMassive(float FirstStep, int Size, int MNK_deg, float SpetApproc, double* K, double* mass)
{
	float FirstStepThere = FirstStep;
	for (int ifile = 0; ifile < Size; ifile++)
	{
//		double polynom = 0;
//		double Xstepen = 1;
//
//		for (int deg = 0; deg < MNK_deg+1; deg++)
//		{
//			if (deg > 0)
//			{
//				Xstepen *= FirstStepThere;
//			}
//			polynom += K[deg] * Xstepen;
//		}

		mass[ifile] = FindOneRootOfEquation(FirstStepThere, MNK_deg, K); //     polynom;
		FirstStepThere += SpetApproc;
	}
}


void MinimumMas (double *massive1, int N, float FirstStep, float SpetApproc, double* min, double* minN)
{
	*min = massive1[0];
	for (int i = 0; i < N; i++)
	{
		if (massive1[i] < *min) {
			*min = massive1[i];
			*minN = FirstStep;
		}
		FirstStep += SpetApproc;
	}
}


void FindRootsOfEquation(double* K, int MNK_deg, float CrossLine, float BorderMin, float BorderMax, double* RootMax, double* RootMin)
{
   /* coefficients of P(x) =  -1 + x^5  */
//  double a[6] = { -1, 0, 0, 0, 0, 1 };  // Полином 5-й степени, значит 6 коэффициентов
//  double z[10]; // У полинома 5-й степени 5 корней, но в массиве идут две компоненты (действительная и мнимая части) поэтому 10 элементов.

	double *Apoli = new double[MNK_deg+1];
	double *Zpoli = new double[(MNK_deg+1)*2];

	for (int deg = 0; deg < MNK_deg+1; deg++)
	{
		Apoli[deg] = K[deg];
		if (deg == 0)
		{
			Apoli[deg] -= CrossLine;
		}
	}

	gsl_poly_complex_workspace * w = gsl_poly_complex_workspace_alloc (MNK_deg+1);
	gsl_poly_complex_solve (Apoli, MNK_deg+1, w, Zpoli);
	gsl_poly_complex_workspace_free (w);

	*RootMin = 1000;
	*RootMax = -1000;
	for (int deg = 0; deg < (MNK_deg+1)*2; deg += 2)
	{
		if (Zpoli[deg+1] == 0 && Zpoli[deg] <= BorderMax && Zpoli[deg] >= BorderMin)
		{
			if (*RootMin > Zpoli[deg])
			{
				*RootMin = Zpoli[deg];
			}
			if (*RootMax < Zpoli[deg])
			{
				*RootMax = Zpoli[deg];
			}
		}
	}

	delete [] Apoli;
	delete [] Zpoli;
}

double MeanABorAC(double a, double b, double c, int defaultValue)
{
	if (a != defaultValue) 
	{
		if (b != defaultValue) 
		{
			return (a+b)/2;
		}
		else 
		{
			return (a+c)/2;
		}
	}
	return 1000;
}

void Parameters(float x, float y, float *Xrez, float *Yrez, float *XYrez, float inf, float focP, float focC)
{
	*Xrez = (x - inf)*(focP*focP)/(focC*focC);
	*Yrez = (y - inf)*(focP*focP)/(focC*focC);
	*XYrez= ((x + y)/2 - inf)*(focP*focP)/(focC*focC);
}
	
void __fastcall TForm1::Button6Click(TObject *Sender)    //Отчет
{

	int ifile, i, j;
	FILE *Strim, *Rezu;
	int prov;
//	const int SigmaNameFi = 200;
//	char SigmaNameF[SigmaNameFi];
	FormatSettings.DecimalSeparator = '.';
	Form1->OpenDialog3->Options.Clear();
	Form1->OpenDialog3->Options << ofAllowMultiSelect << ofFileMustExist;
	int MaxIsForAxis = 0;

//	MemoryForFrame(); //Выделение памяти для кадра SigmaSet.Kadr[i][j]
//	int CMmMs;
//	BoxMmMsStr *BoxMmMs = new BoxMmMsStr[MAX_FILE];
	if (DirectoryListBox2->Visible == true) {
		DirectoryListBox2->Visible = false;
		Button8->Visible = false;
		Button3->Caption = "Обзор...";
	}
	if (DirectoryListBox1->Visible == true) {
		DirectoryListBox1->Visible = false;
		Button7->Visible = false;
		Button2->Caption = "Обзор...";
	}
	//Время
	struct tm *local;
	time_t t = time(NULL);
	local = localtime(&t);
	char bufftime [80];        // строка, в которой будет храниться текущее время
	strftime(bufftime,80,"%d.%m.%Y",local);
	String buffDate = bufftime;

	strftime(bufftime,80,"%Y.%m.%d %H-%M-%S",local);

//  float fLimits[7];
//		fLimits[0] = StrToFloat(Edit47->Text);
//		fLimits[1] = (StrToFloat(Edit20->Text) - StrToFloat(Edit47->Text))/3 + StrToFloat(Edit47->Text);
//		fLimits[2] = fLimits[1] + (StrToFloat(Edit20->Text) - StrToFloat(Edit47->Text))/3;
//		fLimits[3] = StrToFloat(Edit20->Text);
//		fLimits[4] = StrToFloat(Edit21->Text);
//		fLimits[5] = (StrToFloat(Edit48->Text) - StrToFloat(Edit21->Text))/3 + StrToFloat(Edit21->Text);
//		fLimits[6] = fLimits[5] + (StrToFloat(Edit48->Text) - StrToFloat(Edit21->Text))/3;
//		fLimits[7] = StrToFloat(Edit48->Text);
//
//	for (i = 0; i < 7; i++)  fLimits[i] = RoundTo( fLimits[i], -2 );

	if (DirectoryExists(AnsiString(Edit23->Text).c_str()) == true) {
	if ( StrToFloat(Edit20->Text)<=StrToFloat(Edit21->Text) && StrToFloat(Edit47->Text)<=StrToFloat(Edit48->Text) ) {
	if (Edit19->Text != "" && Edit20->Text != "" && Edit21->Text != "" && Edit18->Text != "" &&
		Edit38->Text != "" && Edit39->Text != "" && Edit40->Text != "" && Edit41->Text != "" && Edit42->Text != "") {
	if (Form1->OpenDialog3->Execute())
	{
	String MemoText = "Start - ";
	MemoText += TimeToStr(Time());
	Form1->Memo1->Lines->Add(MemoText);
//	Form1->Memo1->Lines->Add(TimeToStr(Time()));

	//Оценка для термометра
	ProgressBar1->Position = 0;
	ProgressBar1->Max = 0;
	for (ifile = 0; ifile < Form1->OpenDialog3->Files->Count; ifile++)
	{
		char BuFP[300];
		Strim = fopen(AnsiString(OpenDialog3 -> Files -> Strings[ifile]).c_str(), "r");
		while (feof(Strim) == 0)  //Пустые проходы для оценки количества ячеек в FullSigma
		{
			 prov = fscanf(Strim, "%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%[^\n]s", &BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP);
			 if (prov == 20)
			 ProgressBar1->Max++;
			 StatusBar1->Panels->Items[0]->Text = "Оценка... " + IntToStr(ProgressBar1->Max);
			 Application->ProcessMessages();
		}
		fclose(Strim);
		ProgressBar1->Max--;
	}
	StatusBar1->Panels->Items[0]->Text = IntToStr(ProgressBar1->Position) + " / " + IntToStr(ProgressBar1->Max);
	Application->ProcessMessages();

	//Место возможной ошибки
	FullSigmaTable *FullSigma = new FullSigmaTable[ProgressBar1->Max+Form1->OpenDialog3->Files->Count];

	int Meter = 0, BadMeter = 1;
	const SigmaNameFi = 200;
	char SigmaNameF[SigmaNameFi];
	bool Condition1;

	//Цикл по выбранным текстовым файлам, считывание данных из всех файлов (перемешивание)
	for (ifile = 0; ifile < Form1->OpenDialog3->Files->Count; ifile++)
	{
        bool flagOnceTitle = true;
		char BuFP[300];
		Strim = fopen(AnsiString(OpenDialog3 -> Files -> Strings[ifile]).c_str(), "r");
		while (feof(Strim) == 0)
		{
			for (i = 0; i < SigmaNameFi; i++) SigmaNameF[i]	 = '\0';
			FullSigma[Meter].Names = "";

			//Считывание строки названий
			char BuFP[300];
			if (flagOnceTitle)
			{
				prov = fscanf(Strim, "%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%[^\n]s", &BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP,&BuFP);
				flagOnceTitle = false;
			}
			prov = fscanf(Strim, "%s	%f	%f	%s	%s	%s	%d	%d	%s	%s	%s	%d	%f	%f	%f	%f	%f	%f	%f	%[^\n]s", &BuFP, &FullSigma[Meter].x, &FullSigma[Meter].y, &BuFP,&BuFP,&BuFP, &FullSigma[Meter].Is, &FullSigma[Meter].Nframe, &BuFP, &BuFP, &BuFP, &FullSigma[Meter].IsF, &FullSigma[Meter].Mu, &FullSigma[Meter].xS, &FullSigma[Meter].yS, &BuFP,&BuFP, &FullSigma[Meter].SigX, &FullSigma[Meter].SigY, &SigmaNameF);
			FullSigma[Meter].Names = SigmaNameF;

			if (FullSigma[Meter].Names != "" && prov == 20)
			{
				Condition1 = true;
				if (CheckBox9->Checked == false)
				if ( StrToFloat(Edit47->Text) > FullSigma[Meter].SigX || StrToFloat(Edit48->Text) < FullSigma[Meter].SigX ||
					 StrToFloat(Edit47->Text) > FullSigma[Meter].SigY || StrToFloat(Edit48->Text) < FullSigma[Meter].SigY)
					 Condition1 = false;

				if (CheckBox10->Checked == false)
				if ( StrToFloat(Edit49->Text) < FullSigma[Meter].Mu )
					 Condition1 = false;

				if (CheckBox15->Checked == false)
				if ( FullSigma[Meter].xS < ((StrToFloat(Edit54->Text)/2) - StrToFloat(Edit55->Text)) ||
					 FullSigma[Meter].xS > ((StrToFloat(Edit54->Text)/2) + StrToFloat(Edit55->Text)) ||
					 FullSigma[Meter].yS < ((StrToFloat(Edit54->Text)/2) - StrToFloat(Edit55->Text)) ||
					 FullSigma[Meter].yS > ((StrToFloat(Edit54->Text)/2) + StrToFloat(Edit55->Text)) )
					 Condition1 = false;

				if (Condition1)
				{
					Meter++;
					StatusBar1->Panels->Items[2]->Text = "G: " + IntToStr(Meter) + " / " + IntToStr(ProgressBar1->Max);
				}
			}

			else StatusBar1->Panels->Items[3]->Text = "B: " + IntToStr(BadMeter++) + " / " + IntToStr(ProgressBar1->Max);

			Application->ProcessMessages();
		}
		fclose(Strim);
	}
	// Meter - расчетное значение значимых строк
	//На этом этапе все данные получены
	//Вывод полученных данных в FullSigma
	Strim = fopen(AnsiString(Edit23->Text + "\\Sigma - " + bufftime + " - report.txt").c_str(), "a");
	fprintf(Strim, "X	Y	Is_w	Mu	X_w	Y_w	SigmaX	SigmaY\n");
	for (i = 0; i < Meter; i++)
	fprintf(Strim, "%f	%f	%d	%f	%f	%f	%f	%f\n", FullSigma[i].x, FullSigma[i].y, FullSigma[i].Is, FullSigma[i].Mu, FullSigma[i].xS, FullSigma[i].yS, FullSigma[i].SigX, FullSigma[i].SigY);
	fclose(Strim);

	int PorogPx = StrToInt(Form1->Edit19->Text);
	if (PorogPx%2 != 0) PorogPx++;   //нужно, чтобы оно было четным

	const int BoxRazm = ((StrToInt(Form1->Edit34->Text)+PorogPx-1)/PorogPx)*((StrToInt(Form1->Edit43->Text)+PorogPx-1)/PorogPx)+1;
	int NomBoxXY = 1, NomBoxMS = 1, NomBoxMM = 1;
	bool fl5 = false, fl6;

	Graphics::TBitmap *bm  = new Graphics::TBitmap;
	Graphics::TBitmap *bm2 = new Graphics::TBitmap;
	if (ini3->Checked == true) {
		IniSave(AnsiString(Edit23->Text + "\\INI_Sigma - " + bufftime + " - report").c_str());
	}
	//Обработка по небу
	if (CheckBox5->Checked) {

		SBoxXY *BoxXY = new SBoxXY[BoxRazm];
		SBoxXY *BoxMS = new SBoxXY[BoxRazm];
		SBoxXY *BoxMM = new SBoxXY[BoxRazm];

		ProgressBar1->Max = 16;
		for (ifile = 0; ifile < BoxRazm; ifile++)
		{
			BoxXY[ifile].x = 0;
			BoxXY[ifile].y = 0;
			BoxXY[ifile].N = 0;
			BoxMS[ifile].ms = 0;
		}

		BoxXY[0].x = FullSigma[0].x;
		BoxXY[0].y = FullSigma[0].y;
		BoxMS[0].ms = FullSigma[0].ms;
		BoxMM[0].mm = FullSigma[0].mm;
		BoxXY[0].N++;
		FullSigma[0].mark = 0;

		NomBoxXY = 0;
		int StepForBoxX, StepForBoxY = PorogPx/2;  //Создание сетки секторов
		for (i = 0; i < ((StrToInt(Form1->Edit43->Text)+PorogPx-1)/PorogPx); i++) {
		StepForBoxX = PorogPx/2;
			for (j = 0; j < ((StrToInt(Form1->Edit34->Text)+PorogPx-1)/PorogPx); j++) {
				BoxXY[NomBoxXY].x = StepForBoxX;
				StepForBoxX += PorogPx;
				BoxXY[NomBoxXY++].y = StepForBoxY;
			}
			StepForBoxY += PorogPx;
		}
//		NomBoxXY -= 1;
		ProgressBar1->Position++;
		//Сортировка по координатам (массив наборов, например 5 наборов по 35 (BoxXY[i].N) точек)
		for (ifile = 0; ifile < Meter; ifile++) {
		fl5 = false;
		for (i = 0; i < NomBoxXY; i++) {
		if ( ((FullSigma[ifile].x) < (BoxXY[i].x + PorogPx/2)) && ((FullSigma[ifile].x) >= (BoxXY[i].x - PorogPx/2)) &&
			 ((FullSigma[ifile].y) < (BoxXY[i].y + PorogPx/2)) && ((FullSigma[ifile].y) >= (BoxXY[i].y - PorogPx/2)) )
			 {
				BoxXY[i].N++;
				fl5 = true;
				FullSigma[ifile].mark = i;
			 }
			 else
			 {
				if ((i+1) == NomBoxXY && fl5 == false) {
					BoxXY[NomBoxXY].N++;
					FullSigma[ifile].mark = NomBoxXY;
				}
			 }
		}
		}
		ProgressBar1->Position++;
//		for (ifile = 1; ifile < Meter; ifile++) {
//		fl5 = false;
//		fl6 = false;
//		for (i = 0; i < NomBoxXY; i++) {
//		if ( ((FullSigma[ifile].x) <= (BoxXY[i].x + PorogPx)) && ((FullSigma[ifile].x) >= (BoxXY[i].x - PorogPx)) &&
//			 ((FullSigma[ifile].y) <= (BoxXY[i].y + PorogPx)) && ((FullSigma[ifile].y) >= (BoxXY[i].y - PorogPx)) )
//			 {
//				BoxXY[i].N++;
//				fl5 = true;
//				FullSigma[ifile].mark = i;
//			 }
//			 else {
//				if ((i+1) == NomBoxXY && fl5 == false) {
//					BoxXY[NomBoxXY].x = FullSigma[ifile].x;
//					BoxXY[NomBoxXY].y = FullSigma[ifile].y;
//					BoxXY[NomBoxXY].N++;
//					fl6 = true;
//					FullSigma[ifile].mark = NomBoxXY;
//				}
//			 }}
//			 if (fl6) NomBoxXY++;
//			 }

		//Картинка
		Chart2->Visible = false;
		Chart2->Walls->Visible = false;
		Chart2->Width = StrToFloat(Edit41->Text);
		Chart2->Height = Chart2->Width*StrToFloat(Edit43->Text)/StrToFloat(Edit34->Text);

		Chart2->Axes->Left->Minimum = -10000;
		Chart2->Axes->Left->Maximum = StrToFloat(Edit2->Text);
		Chart2->Axes->Left->Minimum = 0;

		Chart2->Axes->Bottom->Minimum = -10000;
		Chart2->Axes->Bottom->Maximum = StrToFloat(Edit1->Text);
		Chart2->Axes->Bottom->Minimum = 0;

		bm2->PixelFormat = pf32bit;
		bm2->Width = Chart2->Width;
		bm2->Height = Chart2->Height;
		Chart2->Series[0]->Clear();
		Chart2->Series[1]->Clear();

		float SigMaxOne, SigMinOne;
		float SigMaxTwo, SigMinTwo;
		for (ifile = 0; ifile < NomBoxXY; ifile++) {
			SigMaxOne = 0;
			SigMinOne = 100;
			for (i = 0; i < Meter; i++) {
				if (FullSigma[i].mark == ifile) {
					if (SigMaxOne < FullSigma[i].SigX)
						SigMaxOne = FullSigma[i].SigX;
					if (SigMinOne > FullSigma[i].SigX)
						SigMinOne = FullSigma[i].SigX;
				}
			}

			if (SigMinOne != 100 && SigMaxOne != 0) {
				if ((StrToFloat(Edit20->Text)<=SigMaxOne) && (StrToFloat(Edit21->Text)>=SigMaxOne))
						Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMaxOne*StrToFloat(Edit44->Text),"", RGB (0,255,0));
				else    Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMaxOne*StrToFloat(Edit44->Text),"", RGB (255,0,0));

				if ((StrToFloat(Edit20->Text)<=SigMinOne) && (StrToFloat(Edit21->Text)>=SigMinOne))
						Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMinOne*StrToFloat(Edit44->Text),"", RGB (0,255,0));
				else    Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMinOne*StrToFloat(Edit44->Text),"", RGB (255,0,0));
			}
		}

		Chart2->Title->Text->Text = AnsiString("MIN/MAX\r\n" + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")").c_str();
		Chart2->Refresh();
		Application->ProcessMessages();
//		Chart2->PaintTo(bm2->Canvas->Handle,0,0);
		String NameGraf3 = (Edit23->Text+"\\Sigma - "+ bufftime + " - Graf BUBBLE Sky1.bmp");
		Chart2->SaveToBitmapFile(NameGraf3);
		ProgressBar1->Position++;

		OpenWord(false);
		AddDoc();

		SetTextFormat(14, 1, 0, 0, 1, 1.5);
		AddParagraph("Отчет по параметрам фокусировки по небу");

		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label27->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1);AddParagraph("  " + Edit32->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label28->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1);AddParagraph("  " + Edit33->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label29->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit34->Text + "x" + Edit43->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label43->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit45->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph("Оптимальная сигма:"); 			SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  от " + Edit20->Text + "  до " + Edit21->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label32->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Memo2->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph("Название первого кадра:"); 		SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + FullSigma[0].Names);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph("Название файлов для отчета:");

		SetTextFormat(12, 0, 0, 0, 0, 0.1);
		for (i = 0; i < OpenDialog3 -> Files -> Count; i++)
		AddParagraph("  " + IntToStr(i+1) + ". " + OpenDialog3 -> Files -> Strings[i]);

		SetTextFormat(12, 1, 1, 0, 0, 0.02); AddParagraphNo("\n"+Label30->Caption); 			SetTextFormat(12, 0, 0, 0, 0, 0.02); AddParagraph("  " + Edit35->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.02); AddParagraphNo(Label33->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 0.02); AddParagraph("  " + Edit36->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.02); AddParagraphNo(Label31->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 0.02); AddParagraph("  " + Edit37->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.02); AddParagraphNo("Дата формирования отчета:"); 	SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + buffDate);

//		AddParagraph("____________________________________________");
		SetTextFormat(14, 1, 0, 0, 1, 1);
		AddParagraph("Результат");
		SetTextFormat(12, 0, 0, 0, 0, 0);

		// Расчет оптимальных
		int AllGoodPointX = 0, AllGoodPointY = 0;
		for (i = 0; i < Meter; i++) {
			if ((StrToFloat(Edit20->Text)<=FullSigma[i].SigX ) && (StrToFloat(Edit21->Text)>=FullSigma[i].SigX ))
				AllGoodPointX++;
			if ((StrToFloat(Edit20->Text)<=FullSigma[i].SigY ) && (StrToFloat(Edit21->Text)>=FullSigma[i].SigY ))
				AllGoodPointY++;
		}
		SetTextFormat(12, 0, 0, 0, 0, 0.02);
		AddParagraph("Всего объектов:	" + IntToStr(Meter) + "\nИз них оптимальных:\nпо оси Х -	" + IntToStr(AllGoodPointX) + "	( " + FloatToStr(FixRoundTo((float)(AllGoodPointX*100)/Meter,-3))+" % )");
		SetTextFormat(12, 0, 0, 0, 0, 1);
		AddParagraph("по оси Y -	" + IntToStr(AllGoodPointY) + "	( " + FloatToStr(FixRoundTo((float)(AllGoodPointY*100)/Meter,-3))+" % )");
		SetTextFormat(12, 0, 0, 0, 0, 0.02);
		// Расчет медианы
		float *MassSortX = new float [Meter];
		float *MassSortY = new float [Meter];
		for (i = 0; i < Meter; i++)
		{
			MassSortX[i] = FullSigma[i].SigX;
			MassSortY[i] = FullSigma[i].SigY;
		}

//        Правильная сортировка по структуре!
//		struct {operator()(FullSigmaTable& a, FullSigmaTable& b){ return a.SigX < b.SigX;}} sortSigX;
//		std::sort(FullSigma, FullSigma + Meter, sortSigX);

		qsort(MassSortX, Meter, sizeof(float), qfloatX);
		qsort(MassSortY, Meter, sizeof(float), qfloatX);
		AddParagraph("Медиана (значение параметра Sigma):");
//        SetTextFormat(12, 0, 0, 0, 0, 0.02);
		if(Meter%2 == 0)  //четное
		AddParagraph("по оси Х -	" + FloatToStr(FixRoundTo((float)(MassSortX[Meter/2]+MassSortX[Meter/2-1])/2,-3)) + "\nпо оси Y -	" + FloatToStr(FixRoundTo((float)(MassSortY[Meter/2]+MassSortY[Meter/2-1])/2,-3)) );
		else   //не четное
		AddParagraph("по оси Х -	" + FloatToStr(FixRoundTo((float)MassSortX[Meter/2],-3)) + "\nпо оси Y -	" + FloatToStr(FixRoundTo((float)MassSortY[Meter/2],-3)));

		delete [] MassSortX;
		delete [] MassSortY;

		AddParagraph("\f");
		FlagClose2 = true;
//		AddParagraph("Расчет по небу максимальных и минимальных значений в каждом секторе.\nЗеленый цвет указывает на то, что крайнее значение (минимум или максимум) попадает в заданный оптимум.");
//		AddPicture(NameGraf3);
		ProgressBar1->Position++;
		//----------------------------------------------------
//		Chart2->Series[0]->Clear();
//		Chart2->Series[1]->Clear();
//		for (ifile = 0; ifile < NomBoxXY; ifile++) {
//			SigMaxOne = 0;
//			SigMinOne = 100;
//			for (i = 0; i < Meter; i++) {
//				if (FullSigma[i].mark == ifile) {
//					if (SigMaxOne < FullSigma[i].SigX)
//						SigMaxOne = FullSigma[i].SigX;
//					if (SigMinOne > FullSigma[i].SigX)
//						SigMinOne = FullSigma[i].SigX;
//				}
//			}
//
//			Series3->Marks->Visible = true;
//			if (SigMinOne != 100 && SigMaxOne != 0) {
//				if ((StrToFloat(Edit20->Text)<=SigMaxOne) && (StrToFloat(Edit21->Text)>=SigMaxOne))
//						Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMaxOne*StrToFloat(Edit44->Text),BoxXY[ifile].N, RGB (0,255,0));
//				else    Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMaxOne*StrToFloat(Edit44->Text),BoxXY[ifile].N, RGB (255,0,0));
//
//				if ((StrToFloat(Edit20->Text)<=SigMinOne) && (StrToFloat(Edit21->Text)>=SigMinOne))
//						Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMinOne*StrToFloat(Edit44->Text),"\n", RGB (0,255,0));
//				else    Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMinOne*StrToFloat(Edit44->Text),"\n", RGB (255,0,0));
//			}
//		}
//
//        Chart2->Refresh();
//		Application->ProcessMessages();
		Chart2->PaintTo(bm2->Canvas->Handle,0,0);
//		NameGraf3 = (Edit23->Text+"\\Graf_BUBBLE_Sky2_"+IntToStr(local->tm_mday)+"."+IntToStr(local->tm_mon+1)+"."+IntToStr(local->tm_year+1900)+"_"+IntToStr(local->tm_hour)+"-"+IntToStr(local->tm_min)+"-"+IntToStr(local->tm_sec)+".bmp");
//		Chart2->SaveToBitmapFile(NameGraf3);
//
//		FlagClose2 = true;
//		AddParagraph("Расчет по небу с подписями количества попавших звезд в условный сектор");
//		AddPicture(NameGraf3);
//		AddParagraph("\f");
//		Series3->Marks->Visible = false;
		//----------------------------------------------------
		Chart2->Series[0]->Clear();
		Chart2->Series[1]->Clear();
		for (ifile = 0; ifile < NomBoxXY; ifile++) {
			SigMaxOne = 0;
			SigMinOne = 100;
			for (i = 0; i < Meter; i++) {
				if (FullSigma[i].mark == ifile) {
					if (SigMaxOne < FullSigma[i].SigX)
						SigMaxOne = FullSigma[i].SigX;
					if (SigMinOne > FullSigma[i].SigX)
						SigMinOne = FullSigma[i].SigX;
				}
			}

			SigMaxTwo = 0;
			SigMinTwo = 100;
			for (i = 0; i < Meter; i++) {
				if (FullSigma[i].mark == ifile) {
					if (SigMaxTwo < FullSigma[i].SigX && SigMaxOne != FullSigma[i].SigX && BoxXY[ifile].N>2)
						SigMaxTwo = FullSigma[i].SigX;
					if (SigMinTwo > FullSigma[i].SigX && SigMinOne != FullSigma[i].SigX && BoxXY[ifile].N>2)
						SigMinTwo = FullSigma[i].SigX;
				}
			}

			if (SigMinTwo != 100 && SigMaxTwo != 0) {
				if ((StrToFloat(Edit20->Text)<=SigMaxTwo) && (StrToFloat(Edit21->Text)>=SigMaxTwo))
						Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMaxTwo*StrToFloat(Edit44->Text),"", RGB (0,255,0));
				else    Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMaxTwo*StrToFloat(Edit44->Text),"", RGB (255,0,0));

				if ((StrToFloat(Edit20->Text)<=SigMinTwo) && (StrToFloat(Edit21->Text)>=SigMinTwo))
						Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMinTwo*StrToFloat(Edit44->Text),"", RGB (0,255,0));
				else    Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMinTwo*StrToFloat(Edit44->Text),"", RGB (255,0,0));
			}
		}

		Chart2->Title->Text->Text = AnsiString("MIN/MAX без крайних значений по каждому сектору\r\n" + Edit32->Text + " №" + Edit33->Text + "  ("+ buffDate +")").c_str();
		Chart2->Refresh();
		Application->ProcessMessages();
//		Chart2->PaintTo(bm2->Canvas->Handle,0,0);
		NameGraf3 = (Edit23->Text+"\\Sigma - "+ bufftime + " - Graf BUBBLE Sky3.bmp");
		Chart2->SaveToBitmapFile(NameGraf3);

		FlagClose2 = true;
//		AddParagraph("Расчет по небу без крайних значений.\nВсе сигмы в секторе сортируются от меньшего к большему и крайние значения отбрасываются. Таким образом, если цвет/размер круга в секторе изменился по отношению к предыдущему графику, то крайние (максимальное и минимальное) значения считаются некорректными выбросами.");
//		AddPicture(NameGraf3);
//		AddParagraph("\f");
		ProgressBar1->Position++;
		//----------------------------------------------------


		//----------------------------------------------------
		AddParagraph("Графики отражающие среднее значение параметра сигма по центральным осям Х и У, частичное представление предыдущего графика.\n");
		Chart3->Visible = false;
		Chart3->Walls->Visible = false;
		Chart3->Width = StrToInt(Edit41->Text);
		Chart3->Height = StrToInt(Edit40->Text);
		Chart3->Series[0]->Clear();
		Chart3->Series[1]->Clear();
		Chart3->Series[2]->Clear();
		Chart3->Series[3]->Clear();

		Chart3->Series[2]->AddXY(-20,  StrToFloat(Edit20->Text), 0, RGB (123,0,28));
		Chart3->Series[2]->AddXY(StrToInt(Form1->Edit34->Text)+20,StrToFloat(Edit20->Text), "", RGB (123,0,28));

		Chart3->Series[3]->AddXY(-20,  StrToFloat(Edit21->Text), 0, RGB (123,0,28));
		Chart3->Series[3]->AddXY(StrToInt(Form1->Edit34->Text)+20,StrToFloat(Edit21->Text), "", RGB (123,0,28));

		Chart3->Legend->Visible = false;
		Chart3->Refresh();

		for (ifile = 0; ifile < NomBoxXY; ifile++)
		{
			SigMaxOne = 0;
			for (i = 0; i < Meter; i++)
				if (FullSigma[i].mark == ifile)
					SigMaxOne += FullSigma[i].SigX;

			if ( ((StrToInt(Form1->Edit43->Text)/2) < (BoxXY[ifile].y + PorogPx/2)) && ((StrToInt(Form1->Edit43->Text)/2) >= (BoxXY[ifile].y - PorogPx/2)) && BoxXY[ifile].N >= StrToInt(Form1->Edit51->Text) )
				if (BoxXY[ifile].x > StrToInt(Form1->Edit34->Text))
				Chart3->Series[0]->AddXY(StrToInt(Form1->Edit34->Text)-1,SigMaxOne/BoxXY[ifile].N, StrToInt(Form1->Edit34->Text), RGB (30,144,255));
				else
				Chart3->Series[0]->AddXY(BoxXY[ifile].x,SigMaxOne/BoxXY[ifile].N, BoxXY[ifile].x, RGB (30,144,255));
		}

		Chart3->Title->Text->Text = AnsiString("Значение сигмы по центру матрицы (ось X)\r\n" + Edit32->Text + " №" + Edit33->Text + "  ("+ buffDate +")\r\n ").c_str();
		Chart3->Axes->Bottom->AutomaticMaximum = false;
		Chart3->Axes->Bottom->Minimum = -10000;
		Chart3->Axes->Bottom->Maximum = StrToInt(Form1->Edit34->Text);

		Chart3->Axes->Bottom->AutomaticMinimum = false;
		Chart3->Axes->Bottom->Minimum = 0;

		Chart3->Axes->Left->AutomaticMaximum = false;
		Chart3->Axes->Left->AutomaticMinimum = false;
		Chart3->Axes->Left->Minimum = -10000;
		Chart3->Axes->Left->Maximum = StrToFloat(Edit31->Text);
		Chart3->Axes->Left->Minimum = StrToFloat(Edit30->Text);
		Chart3->Axes->Left->Increment = 0.1;
		Chart3->Axes->Bottom->Title->Caption = "\r\nКоордината по оси X";
		Chart3->Axes->Left->Title->Caption = "Sigma";

		Chart3->Refresh();
		Application->ProcessMessages();
//		Chart3->PaintTo(bm2->Canvas->Handle,0,0);
		NameGraf3 = (Edit23->Text+"\\Sigma - "+ bufftime + " - Graf Sky5.bmp");
		Chart3->SaveToBitmapFile(NameGraf3);

		AddPicture(NameGraf3);
		ProgressBar1->Position++;
		//----------------------------------------------------

		Chart3->Series[0]->Clear();
		Chart3->Series[2]->Clear();
		Chart3->Series[3]->Clear();
		Chart3->Refresh();

		Chart3->Series[2]->AddXY(-20,  StrToFloat(Edit20->Text), 0, RGB (123,0,28));
		Chart3->Series[2]->AddXY(StrToInt(Form1->Edit43->Text)+20,StrToFloat(Edit20->Text), "", RGB (123,0,28));

		Chart3->Series[3]->AddXY(-20,  StrToFloat(Edit21->Text), 0, RGB (123,0,28));
		Chart3->Series[3]->AddXY(StrToInt(Form1->Edit43->Text)+20,StrToFloat(Edit21->Text), "", RGB (123,0,28));

		Application->ProcessMessages();

		for (ifile = 0; ifile < NomBoxXY; ifile++)
		{
			SigMaxOne = 0;
			for (i = 0; i < Meter; i++)
				if (FullSigma[i].mark == ifile)
					SigMaxOne += FullSigma[i].SigX;

			if ( ((StrToInt(Form1->Edit34->Text)/2) < (BoxXY[ifile].x + PorogPx/2)) && ((StrToInt(Form1->Edit34->Text)/2) >= (BoxXY[ifile].x - PorogPx/2)) && BoxXY[ifile].N >= StrToInt(Form1->Edit51->Text) )
				if (BoxXY[ifile].y > StrToInt(Form1->Edit43->Text))
				Chart3->Series[0]->AddXY(StrToInt(Form1->Edit43->Text)-1,SigMaxOne/BoxXY[ifile].N, StrToInt(Form1->Edit43->Text), RGB (30,144,255));
				else
				Chart3->Series[0]->AddXY(BoxXY[ifile].y,SigMaxOne/BoxXY[ifile].N, BoxXY[ifile].y, RGB (30,144,255));
		}

		Chart3->Title->Text->Text = AnsiString("Значение сигмы по центру матрицы (ось Y)\r\n" + Edit32->Text + " №" + Edit33->Text + "  ("+ buffDate +")\r\n ").c_str();
		Chart3->Axes->Bottom->AutomaticMaximum = false;
		Chart3->Axes->Bottom->Minimum = -10000;
		Chart3->Axes->Bottom->Maximum = StrToInt(Form1->Edit43->Text);

		Chart3->Axes->Bottom->AutomaticMinimum = false;
		Chart3->Axes->Bottom->Minimum = 0;

		Chart3->Axes->Left->AutomaticMaximum = false;
		Chart3->Axes->Left->AutomaticMinimum = false;

		Chart3->Axes->Left->Minimum = -10000;
		Chart3->Axes->Left->Maximum = StrToFloat(Edit31->Text);
		Chart3->Axes->Left->Minimum = StrToFloat(Edit30->Text);

		Chart3->Axes->Left->Increment = 0.1;
		Chart3->Axes->Bottom->Title->Caption = "\r\nКоордината по оси Y";
		Chart3->Axes->Left->Title->Caption = "Sigma";

//		Chart3->Refresh();
//		Application->ProcessMessages();
//		Chart3->PaintTo(bm2->Canvas->Handle,0,0);
		NameGraf3 = (Edit23->Text+"\\Sigma - "+ bufftime + " - Graf Sky6.bmp");
		Chart3->SaveToBitmapFile(NameGraf3);

		AddPicture(NameGraf3);
		ProgressBar1->Position++;
		//---------------------------------------------------- 1
		//Гистограммы!

		int IsMax = 0;
		int IsMin = 10000000;

		if (CheckBox16->Checked == true)
		{
			for (i = 0; i < Meter; i++) {
				if (FullSigma[i].Is > IsMax) IsMax = FullSigma[i].Is;
				if (FullSigma[i].Is < IsMin) IsMin = FullSigma[i].Is;  }
			IsMax = (IsMax - IsMin)/4;
		}
		else {
			IsMax = StrToInt(Edit56->Text);
		}

		Chart6->Walls->Visible = false;
		Chart6->Width = StrToInt(Edit41->Text);
		Chart6->Height = StrToInt(Edit40->Text);
		Chart6->Refresh();

		TColor CColor[4] = {RGB (32,159,223), RGB (153,202,83), RGB (246,166,37), RGB (191,89,62)};
		TBarSeries **BarSeries = new TBarSeries*[4];
		for (int i = 0; i < 4; i++)
		{
			BarSeries[i]= new TBarSeries(Chart6);
			Chart6->AddSeries(BarSeries[i]);
			BarSeries[i]->Marks->Visible = false;
			BarSeries[i]->Color = CColor[i];
//			BarSeries[i]->BarWidthPercent = 22;
		}

		int Ustep = IsMax;
		int Dstep = 1;

		int SIGstepInt = 0;
		float SIGstep = (float)SIGstepInt/10;

		bool Flag4For = true;
		int NewMeter = 0;
		int Shet1 = 0;
		int Savesh = 0; //Против зацикливания на всякий случай
		while (Flag4For && SIGstep < StrToFloat(Edit48->Text))
		{
			if (SIGstep >= StrToFloat(Edit47->Text)) {
			for (j = 0; j < 4; j++)
			{
				for (i = 0; i < Meter; i++) {
					if ( (FullSigma[i].SigX >= SIGstep) && (FullSigma[i].SigX < (SIGstep+0.1)) ) {
					if (j != 3) {
					if ( (FullSigma[i].Is >= Dstep) && (FullSigma[i].Is < Ustep) ) //Ограничения с двух сторон
					{
						NewMeter++;
						Shet1++;
					} }
					else
					if ( (FullSigma[i].Is >= Dstep))      //Ограничение для последнего значения яркости с одной стороны
					{
						NewMeter++;
						Shet1++;
					}} }

				BarSeries[j]->AddXY(SIGstep,Shet1);
				Dstep = Ustep;

				if (CheckBox16->Checked == true)
				Ustep = Ustep+IsMax;
				else
				Ustep = Ustep*2;

				Shet1 = 0;
				Savesh++;
				if (Savesh > 100) break;
			}}
			Flag4For = false;

			if (NewMeter < Meter)
			{
				Flag4For = true;
				SIGstepInt++;     //Приращение значения сигмы (в чистом виде значение int делится на 10)
				SIGstep = (float)SIGstepInt/10;
				Ustep = IsMax;
				Dstep = 1;
			}

		}

		Ustep = IsMax;
		if (CheckBox16->Checked == true) {
			BarSeries[0]->Title = "< " + IntToStr(Ustep);
			BarSeries[1]->Title = IntToStr(Ustep) +   " - " + IntToStr(Ustep*2);
			BarSeries[2]->Title = IntToStr(Ustep*2) + " - " + IntToStr(Ustep*3);
			BarSeries[3]->Title = "> " + IntToStr(Ustep*3);
		}
		else {
			BarSeries[0]->Title = "< " + IntToStr(Ustep);
			BarSeries[1]->Title = IntToStr(Ustep) +   " - " + IntToStr(Ustep*2);
			BarSeries[2]->Title = IntToStr(Ustep*2) + " - " + IntToStr(Ustep*4);
			BarSeries[3]->Title = "> " + IntToStr(Ustep*4);
		}
		Chart6->Axes->Bottom->Minimum = -10000;
		Chart6->Axes->Bottom->Maximum = StrToFloat(Edit48->Text)+0.05;
		Chart6->Axes->Bottom->Minimum = StrToFloat(Edit47->Text)-0.05;
		Chart6->Refresh();

		Application->ProcessMessages();
		Chart6->LeftAxis->Title->Caption = AnsiString("Интегральная яркость, град. АЦП").c_str();
		Chart6->Title->Text->Text = AnsiString("Гистограмма параметра SigmaX по интегральной яркости\r\n" + Edit32->Text + " №" + Edit33->Text + "  ("+ buffDate +")\r\n ").c_str();
		NameGraf3 = (Edit23->Text+"\\Sigma - "+ bufftime + " - Graf SkyGX.bmp");
		Chart6->SaveToBitmapFile(NameGraf3);
		AddPicture(NameGraf3);

		for (int i = 0; i < 4; i++)
		BarSeries[i]->Clear();
		ProgressBar1->Position++;
		//---------------------------------------- Гистограмма № 2 по У
		Chart6->Walls->Visible = false;
		Chart6->Width = StrToInt(Edit41->Text);
		Chart6->Height = StrToInt(Edit40->Text);
		Chart6->Refresh();

		Ustep = IsMax;
		Dstep = 1;

		SIGstepInt = 0;
		SIGstep = (float)SIGstepInt/10;

		Flag4For = true;
		NewMeter = Shet1 = Savesh = 0;
		while (Flag4For && SIGstep < StrToFloat(Edit48->Text))
		{
			if (SIGstep >= StrToFloat(Edit47->Text)) {
			for (j = 0; j < 4; j++)
			{
				for (i = 0; i < Meter; i++) {
					if ( (FullSigma[i].SigY >= SIGstep) && (FullSigma[i].SigY < (SIGstep+0.1)) ) {
					if (j != 3) {
					if ( (FullSigma[i].Is >= Dstep) && (FullSigma[i].Is < Ustep) ) //Ограничения с двух сторон
					{
						NewMeter++;
						Shet1++;
					} }
					else
					if ( (FullSigma[i].Is >= Dstep))      //Ограничение для последнего значения яркости с одной стороны
					{
						NewMeter++;
						Shet1++;
					}} }

				BarSeries[j]->AddXY(SIGstep,Shet1);
				Dstep = Ustep;

				if (CheckBox16->Checked == true)
				Ustep = Ustep+IsMax;
				else
				Ustep = Ustep*2;

				Shet1 = 0;
				Savesh++;
				if (Savesh > 100) break;
			}}

			Flag4For = false;
			if (NewMeter < Meter)
			{
				Flag4For = true;
				SIGstepInt++;     //Приращение значения сигмы (в чистом виде значение int делится на 10)
				SIGstep = (float)SIGstepInt/10;
				Ustep = IsMax;
				Dstep = 1;
			}
		}

		Ustep = IsMax;
		if (CheckBox16->Checked == true) {
			BarSeries[0]->Title = "< " + IntToStr(Ustep);
			BarSeries[1]->Title = IntToStr(Ustep) +   " - " + IntToStr(Ustep*2);
			BarSeries[2]->Title = IntToStr(Ustep*2) + " - " + IntToStr(Ustep*3);
			BarSeries[3]->Title = "> " + IntToStr(Ustep*3);
		}
		else {
			BarSeries[0]->Title = "< " + IntToStr(Ustep);
			BarSeries[1]->Title = IntToStr(Ustep) +   " - " + IntToStr(Ustep*2);
			BarSeries[2]->Title = IntToStr(Ustep*2) + " - " + IntToStr(Ustep*4);
			BarSeries[3]->Title = "> " + IntToStr(Ustep*4);
		}
        Chart6->Axes->Bottom->Minimum = -10000;
		Chart6->Axes->Bottom->Maximum = StrToFloat(Edit48->Text)+0.05;
		Chart6->Axes->Bottom->Minimum = StrToFloat(Edit47->Text)-0.05;
		Chart6->Refresh();

		Application->ProcessMessages();
		Chart6->LeftAxis->Title->Caption = AnsiString("Интегральная яркость, град. АЦП").c_str();
		Chart6->Title->Text->Text = AnsiString("Гистограмма параметра SigmaY по интегральной яркости\r\n" + Edit32->Text + " №" + Edit33->Text + "  ("+ buffDate +")\r\n ").c_str();
		NameGraf3 = (Edit23->Text+"\\Sigma - "+ bufftime + " - Graf SkyGY.bmp");
		Chart6->SaveToBitmapFile(NameGraf3);
		AddPicture(NameGraf3);

		for (int i = 0; i < 4; i++)
		BarSeries[i]->Clear();
		ProgressBar1->Position++;
		//---------------------------------------- 3
		Chart6->Walls->Visible = false;
		Chart6->Width = StrToInt(Edit41->Text);
		Chart6->Height = StrToInt(Edit40->Text);
		Chart6->Refresh();

		Chart6->RemoveAllSeries();
		for (int i = 0; i < 2; i++)
		{
			Chart6->AddSeries(BarSeries[i]);
			BarSeries[i]->Marks->Visible = false;
		}

		SIGstepInt = 0;
		SIGstep = (float)SIGstepInt/10;

		int Shet2;
		Shet1 = Shet2 = 0;

		BarSeries[0]->Color = CColor[0];
		BarSeries[1]->Color = CColor[3];

		while (SIGstep < StrToFloat(Edit48->Text)) {
			if (SIGstep >= StrToFloat(Edit47->Text)) {
			for (i = 0; i < Meter; i++) {
				if ( (FullSigma[i].SigX >= SIGstep) && (FullSigma[i].SigX < (SIGstep+0.1)) )  Shet1++;
				if ( (FullSigma[i].SigY >= SIGstep) && (FullSigma[i].SigY < (SIGstep+0.1)) )  Shet2++;
			}

			BarSeries[0]->AddXY(SIGstep,Shet1);
			BarSeries[1]->AddXY(SIGstep,Shet2);
			Shet1 = Shet2 = 0;
			}
			SIGstepInt++;
			SIGstep = (float)SIGstepInt/10;
		}

		BarSeries[0]->Title = "Значения по Х";
		BarSeries[1]->Title = "Значения по Y";
		Chart6->Refresh();

		Application->ProcessMessages();
		Chart6->Axes->Bottom->Minimum = -10000;
		Chart6->Axes->Bottom->Maximum = StrToFloat(Edit48->Text)+0.05;
		Chart6->Axes->Bottom->Minimum = StrToFloat(Edit47->Text)-0.05;
		Chart6->LeftAxis->Title->Caption = AnsiString("").c_str();
		Chart6->Title->Text->Text = AnsiString("Количество значений SigmaХ/SigmaY\r\n" + Edit32->Text + " №" + Edit33->Text + "  ("+ buffDate +")\r\n ").c_str();
		NameGraf3 = (Edit23->Text+"\\Sigma - "+ bufftime + " - Graf SkyXY.bmp");
		Chart6->SaveToBitmapFile(NameGraf3);
		AddPicture(NameGraf3);
		Chart6->RemoveAllSeries();

		delete [] BarSeries;
		ProgressBar1->Position++;
		//----------------------------------------------------
//		Chart4->Visible = false;
		Chart4->Walls->Visible = false;
		Chart4->Width = StrToInt(Edit41->Text);
		Chart4->Height = StrToInt(Edit40->Text);
		Chart4->Series[0]->Clear();
		Chart4->Series[1]->Clear();
		Chart4->Refresh();
		Application->ProcessMessages();

		MaxIsForAxis = 0;
		for (i = 0; i < Meter; i++)
		{
			if (MaxIsForAxis < FullSigma[i].Is)  MaxIsForAxis = FullSigma[i].Is;

			if (StrToFloat(Edit20->Text) <= FullSigma[i].SigX && StrToFloat(Edit21->Text) >= FullSigma[i].SigX)
			Chart4->Series[0]->AddXY(FullSigma[i].Is, FullSigma[i].SigX);
			else
			Chart4->Series[1]->AddXY(FullSigma[i].Is, FullSigma[i].SigX);
		}
		Chart4->Title->Text->Text = AnsiString("Значение параметра sigma X от интегральной яркости\r\n" + Edit32->Text + " №" + Edit33->Text + "  ("+ buffDate +")\r\n ").c_str();

		Chart4->Axes->Left->Increment = 0.1;
		Chart4->Axes->Bottom->Title->Caption = "\r\nИнтегральная яркость, град. АЦП";
		Chart4->Axes->Left->Title->Caption = "Sigma";
		Chart4->Axes->Left->AutomaticMaximum = false;
		Chart4->Axes->Left->Maximum = StrToFloat(Edit48->Text);

		Chart4->Axes->Left->AutomaticMinimum = false;
		Chart4->Axes->Left->Minimum = 0;

		Chart4->Axes->Bottom->AutomaticMaximum = false;
		Chart4->Axes->Bottom->Maximum = MaxIsForAxis;

		Chart4->Axes->Bottom->AutomaticMinimum = false;
		Chart4->Axes->Bottom->Minimum = 0;

		Chart4->Axes->Bottom->Increment = 100;

		Application->ProcessMessages();

		NameGraf3 = (Edit23->Text+"\\Sigma - "+ bufftime + " - Graf Sky7.bmp");
		Chart4->SaveToBitmapFile(NameGraf3);

		AddPicture(NameGraf3);
		ProgressBar1->Position++;

		//----------------------------------------------------
//		Chart4->Visible = false;
		Chart4->Walls->Visible = false;
		Chart4->Width = StrToInt(Edit41->Text);
		Chart4->Height = StrToInt(Edit40->Text);
		Chart4->Series[0]->Clear();
		Chart4->Series[1]->Clear();
		Chart4->Refresh();
		Application->ProcessMessages();

		MaxIsForAxis = 0;
		for (i = 0; i < Meter; i++)
		{
			if (MaxIsForAxis < FullSigma[i].Is)  MaxIsForAxis = FullSigma[i].Is;

			if (StrToFloat(Edit20->Text) <= FullSigma[i].SigY && StrToFloat(Edit21->Text) >= FullSigma[i].SigY)
			Chart4->Series[0]->AddXY(FullSigma[i].Is, FullSigma[i].SigY);
			else
			Chart4->Series[1]->AddXY(FullSigma[i].Is, FullSigma[i].SigY);
		}
		Chart4->Title->Text->Text = AnsiString("Значение параметра sigma Y от интегральной яркости\r\n" + Edit32->Text + " №" + Edit33->Text + "  ("+ buffDate +")\r\n ").c_str();

		Chart4->Axes->Left->Increment = 0.1;
		Chart4->Axes->Bottom->Title->Caption = "\r\nИнтегральная яркость, град. АЦП";
		Chart4->Axes->Left->Title->Caption = "Sigma";
		Chart4->Axes->Left->AutomaticMaximum = false;
		Chart4->Axes->Left->Maximum = StrToFloat(Edit48->Text);

		Chart4->Axes->Left->AutomaticMinimum = false;
		Chart4->Axes->Left->Minimum = 0;

		Chart4->Axes->Bottom->AutomaticMaximum = false;
		Chart4->Axes->Bottom->Maximum = MaxIsForAxis;

		Chart4->Axes->Bottom->AutomaticMinimum = false;
		Chart4->Axes->Bottom->Minimum = 0;

		Chart4->Axes->Bottom->Increment = 100;

		Application->ProcessMessages();

		NameGraf3 = (Edit23->Text+"\\Sigma - "+ bufftime + " - Graf Sky8.bmp");
		Chart4->SaveToBitmapFile(NameGraf3);

		AddPicture(NameGraf3);
		AddParagraph("\f");
		ProgressBar1->Position++;
		//----------------------------------------------------
		if (CheckBoxPhotometry->Checked == true)
		{
			ChartPhotogrammetry->Walls->Visible = false;
			ChartPhotogrammetry->Width = StrToInt(Edit41->Text);
			ChartPhotogrammetry->Height = StrToInt(Edit40->Text);
			ChartPhotogrammetry->Series[0]->Clear();
			ChartPhotogrammetry->Series[1]->Clear();
			ChartPhotogrammetry->Series[2]->Clear();
			ChartPhotogrammetry->Series[3]->Clear();
			ChartPhotogrammetry->Refresh();
			Application->ProcessMessages();

			//Отрисовка 3х линий с яркостями
			ChartPhotogrammetry->Series[1]->AddXY(StrToInt(Label_IOZ4->Caption), 0);
			ChartPhotogrammetry->Series[1]->AddXY(StrToInt(Label_IOZ4->Caption), 20);

			ChartPhotogrammetry->Series[2]->AddXY(StrToInt(Label_IOZ5->Caption), 0);
			ChartPhotogrammetry->Series[2]->AddXY(StrToInt(Label_IOZ5->Caption), 20);

			ChartPhotogrammetry->Series[3]->AddXY(StrToInt(Label_IOZ6->Caption), 0);
			ChartPhotogrammetry->Series[3]->AddXY(StrToInt(Label_IOZ6->Caption), 20);

			for (i = 0; i < Meter; i++)
			{
				ChartPhotogrammetry->Series[0]->AddXY(FullSigma[i].IsF, FullSigma[i].SigX);
			}
			ChartPhotogrammetry->Title->Text->Text = AnsiString("Значение параметра sigma X от интегральной яркости IsF\r\n" + Edit32->Text + " №" + Edit33->Text + "  ("+ buffDate +")\r\n ").c_str();

			ChartPhotogrammetry->Axes->Left->Increment = 0.1;
			ChartPhotogrammetry->Axes->Bottom->Title->Caption = "\r\nИнтегральная яркость IsF, град. АЦП";
			ChartPhotogrammetry->Axes->Left->Title->Caption = "Sigma";
			ChartPhotogrammetry->Axes->Left->AutomaticMaximum = false;
			ChartPhotogrammetry->Axes->Left->Maximum = StrToFloat(Edit48->Text);

			ChartPhotogrammetry->Axes->Left->AutomaticMinimum = false;
			ChartPhotogrammetry->Axes->Left->Minimum = 0;

			ChartPhotogrammetry->Axes->Bottom->AutomaticMaximum = false;
			ChartPhotogrammetry->Axes->Bottom->Maximum = StrToInt(Label_IOZ4->Caption) + StrToInt(Label_IOZ6->Caption);

			ChartPhotogrammetry->Axes->Bottom->AutomaticMinimum = false;
			ChartPhotogrammetry->Axes->Bottom->Minimum = 0;

			ChartPhotogrammetry->Axes->Bottom->Increment = 100;

			Application->ProcessMessages();

			NameGraf3 = (Edit23->Text+"\\Sigma - "+ bufftime + " - Graf Sky IsF.bmp");
			ChartPhotogrammetry->SaveToBitmapFile(NameGraf3);

			AddPicture(NameGraf3);
			ProgressBar1->Position++;
		}

		//----------------------------------------------------

		Chart2->Series[0]->Clear();
		Chart2->Series[1]->Clear();

		for (i = 0; i < Meter; i++)
			if ( FullSigma[i].SigX >= (StrToFloat(Edit21->Text) + 0.4) )
			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (255,0,0));

		for (i = 0; i < Meter; i++)
			if ( FullSigma[i].SigX < (StrToFloat(Edit21->Text) + 0.4) && FullSigma[i].SigX >= (StrToFloat(Edit21->Text) + 0.2))
			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (255,165,0));

		for (i = 0; i < Meter; i++)
			if ( FullSigma[i].SigX < (StrToFloat(Edit21->Text) + 0.2)  && FullSigma[i].SigX > (StrToFloat(Edit21->Text)))
			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (255,255,0));

		for (i = 0; i < Meter; i++)
			if ( FullSigma[i].SigX <= (StrToFloat(Edit21->Text)) && FullSigma[i].SigX >= (StrToFloat(Edit20->Text)))
			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (0,255,0));

		for (i = 0; i < Meter; i++)
			if ( FullSigma[i].SigX < (StrToFloat(Edit20->Text)) && FullSigma[i].SigX >= (StrToFloat(Edit20->Text) - 0.1))
			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (66,170,255));

		for (i = 0; i < Meter; i++)
			if ( FullSigma[i].SigX < (StrToFloat(Edit20->Text) - 0.1) && FullSigma[i].SigX >= (StrToFloat(Edit20->Text) - 0.2))
			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (0,87,250));

		for (i = 0; i < Meter; i++)
			if ( FullSigma[i].SigX < (StrToFloat(Edit20->Text) - 0.2) )
			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (153,50,204));


//		for (i = 0; i < Meter; i++)
//		{
//			if ( FullSigma[i].SigX < (StrToFloat(Edit20->Text) - 0.2) )
//			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (153,50,204));
//			else
//			if ( FullSigma[i].SigX < (StrToFloat(Edit20->Text) - 0.1) )
//			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (0,87,250));
//			else
//			if ( FullSigma[i].SigX < (StrToFloat(Edit20->Text)) )
//			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (66,170,255));
//			else
//			if ( FullSigma[i].SigX <= (StrToFloat(Edit21->Text)) )
//			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (0,255,0));
//			else
//			if ( FullSigma[i].SigX < (StrToFloat(Edit21->Text) + 0.2)  )
//			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (255,255,0));
//			else
//			if ( FullSigma[i].SigX < (StrToFloat(Edit21->Text) + 0.4) )
//			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (255,165,0));
//			else
//			if ( FullSigma[i].SigX >= (StrToFloat(Edit21->Text) + 0.4) )
//			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (255,0,0));
//		}

//		for (i = 0; i < Meter; i++)
//		{
//			if ( FullSigma[i].SigX > (StrToFloat(Edit21->Text) + 0.4) )
//			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (255,0,0));
//			else
//			if ( FullSigma[i].SigX > (StrToFloat(Edit21->Text) + 0.2) )
//			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (255,165,0));
//			else
//			if ( FullSigma[i].SigX > (StrToFloat(Edit21->Text))  )
//			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (255,255,0));
//			else
//			if ( FullSigma[i].SigX >= (StrToFloat(Edit20->Text)) )
//			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (0,255,0));
//			else
//			if ( FullSigma[i].SigX > (StrToFloat(Edit20->Text) - 0.1) )
//			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (66,170,255));
//			else
//			if ( FullSigma[i].SigX > (StrToFloat(Edit20->Text) - 0.2) )
//			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (0,87,250));
//            else
//			if ( FullSigma[i].SigX < (StrToFloat(Edit20->Text) - 0.2) )
//			Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (153,50,204));
//		}
//		float StepRainbow[7]={-0.3, -0.2, -0.1, 0, 0.2, 0.4, 0.6};
//		bool Rainbow;
//
//		for (i = 0; i < Meter; i++)
//		{
//			Rainbow = false;
//			for (j = 7; j <= 0; j--)
//			if (j >= 4) {
//				if ( FullSigma[i].SigX > (StrToFloat(Edit20->Text) + StepRainbow[j]) && Rainbow == false)
//				{
//					Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (153,50,204));
//					Rainbow = true;
//				}
//			}
//			else {
//				if ( FullSigma[i].SigX > (StrToFloat(Edit20->Text) + StepRainbow[j]) && Rainbow == false)
//				{
//					Series3->AddBubble(BoxXY[FullSigma[i].mark].x,BoxXY[FullSigma[i].mark].y,FullSigma[i].SigX*StrToFloat(Edit44->Text),"", RGB (153,50,204));
//					Rainbow = true;
//				}
//            }
//		}
		Chart2->Title->Text->Text = AnsiString("Все значения по каждому сектору\r\n" + Edit32->Text + " №" + Edit33->Text + "  ("+ buffDate +")").c_str();
		Chart2->Refresh();
		Application->ProcessMessages();
//		Chart2->PaintTo(bm2->Canvas->Handle,0,0);
		NameGraf3 = (Edit23->Text+"\\Sigma - "+ bufftime + " - Graf BUBBLE Sky4.bmp");
		Chart2->SaveToBitmapFile(NameGraf3);

		FlagClose2 = true;
		AddParagraph("Все значения попавшие в сектор");
		AddPicture(NameGraf3);
		AddParagraph("\f");
		ProgressBar1->Position++;
		//----------------------------------------------------
//		float sum12 = 0;
//		for (ifile = 0; ifile < NomBoxXY; ifile++)
//		sum12 += BoxXY[ifile].N;

		Chart2->Series[0]->Clear();
		Chart2->Series[1]->Clear();

		//Тут SigMaxOne используется как сумма
		int GoodPoint;
		Series3->Marks->Visible = true;
		for (ifile = 0; ifile < NomBoxXY; ifile++) {
			SigMaxOne = 0;
			GoodPoint = 0;
			for (i = 0; i < Meter; i++) {
				if (FullSigma[i].mark == ifile) 
				{
					SigMaxOne += FullSigma[i].SigX;
					if ((StrToFloat(Edit20->Text)<=FullSigma[i].SigX ) && (StrToFloat(Edit21->Text)>=FullSigma[i].SigX ))
						GoodPoint++;
				}
			}
//			if (SigMaxOne != 0) {
//				if ((StrToFloat(Edit20->Text)<=SigMaxOne/BoxXY[ifile].N) && (StrToFloat(Edit21->Text)>=SigMaxOne/BoxXY[ifile].N))
//						Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMaxOne*StrToFloat(Edit44->Text)/BoxXY[ifile].N,"", RGB (0,255,0));
//				else    Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMaxOne*StrToFloat(Edit44->Text)/BoxXY[ifile].N,"", RGB (255,0,0));
//			}
			if (SigMaxOne != 0 && BoxXY[ifile].N >= StrToInt(Edit51->Text))
			if (CheckBox11->Checked){
			if ( SigMaxOne/BoxXY[ifile].N < (StrToFloat(Edit20->Text) - 0.2) )
			Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,StrToFloat(Edit44->Text), FloatToStr(FixRoundTo(SigMaxOne/BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint)+"/"+IntToStr(BoxXY[ifile].N), RGB (153,50,204));
			else
			if ( SigMaxOne/BoxXY[ifile].N < (StrToFloat(Edit20->Text) - 0.1) )
			Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,StrToFloat(Edit44->Text), FloatToStr(FixRoundTo(SigMaxOne/BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint)+"/"+IntToStr(BoxXY[ifile].N), RGB (0,87,250));
			else
			if ( SigMaxOne/BoxXY[ifile].N < (StrToFloat(Edit20->Text)) )
			Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,StrToFloat(Edit44->Text), FloatToStr(FixRoundTo(SigMaxOne/BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint)+"/"+IntToStr(BoxXY[ifile].N), RGB (66,170,255));
			else
			if ( SigMaxOne/BoxXY[ifile].N <= (StrToFloat(Edit21->Text)) )
			Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,StrToFloat(Edit44->Text), FloatToStr(FixRoundTo(SigMaxOne/BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint)+"/"+IntToStr(BoxXY[ifile].N), RGB (0,255,0));
			else
			if ( SigMaxOne/BoxXY[ifile].N < (StrToFloat(Edit21->Text) + 0.2)  )
			Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,StrToFloat(Edit44->Text), FloatToStr(FixRoundTo(SigMaxOne/BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint)+"/"+IntToStr(BoxXY[ifile].N), RGB (255,255,0));
			else
			if ( SigMaxOne/BoxXY[ifile].N < (StrToFloat(Edit21->Text) + 0.4) )
			Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,StrToFloat(Edit44->Text), FloatToStr(FixRoundTo(SigMaxOne/BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint)+"/"+IntToStr(BoxXY[ifile].N), RGB (255,165,0));
			else
			if ( SigMaxOne/BoxXY[ifile].N >= (StrToFloat(Edit21->Text) + 0.4) )
			Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,StrToFloat(Edit44->Text), FloatToStr(FixRoundTo(SigMaxOne/BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint)+"/"+IntToStr(BoxXY[ifile].N), RGB (255,0,0));
			}
			else {
			if ( SigMaxOne/BoxXY[ifile].N < (StrToFloat(Edit20->Text) - 0.2) )
			Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMaxOne*StrToFloat(Edit44->Text)/BoxXY[ifile].N, FloatToStr(FixRoundTo(SigMaxOne/BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint)+"/"+IntToStr(BoxXY[ifile].N), RGB (153,50,204));
			else
			if ( SigMaxOne/BoxXY[ifile].N < (StrToFloat(Edit20->Text) - 0.1) )
			Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMaxOne*StrToFloat(Edit44->Text)/BoxXY[ifile].N, FloatToStr(FixRoundTo(SigMaxOne/BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint)+"/"+IntToStr(BoxXY[ifile].N), RGB (0,87,250));
			else
			if ( SigMaxOne/BoxXY[ifile].N < (StrToFloat(Edit20->Text)) )
			Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMaxOne*StrToFloat(Edit44->Text)/BoxXY[ifile].N, FloatToStr(FixRoundTo(SigMaxOne/BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint)+"/"+IntToStr(BoxXY[ifile].N), RGB (66,170,255));
			else
			if ( SigMaxOne/BoxXY[ifile].N <= (StrToFloat(Edit21->Text)) )
			Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMaxOne*StrToFloat(Edit44->Text)/BoxXY[ifile].N, FloatToStr(FixRoundTo(SigMaxOne/BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint)+"/"+IntToStr(BoxXY[ifile].N), RGB (0,255,0));
			else
			if ( SigMaxOne/BoxXY[ifile].N < (StrToFloat(Edit21->Text) + 0.2)  )
			Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMaxOne*StrToFloat(Edit44->Text)/BoxXY[ifile].N, FloatToStr(FixRoundTo(SigMaxOne/BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint)+"/"+IntToStr(BoxXY[ifile].N), RGB (255,255,0));
			else
			if ( SigMaxOne/BoxXY[ifile].N < (StrToFloat(Edit21->Text) + 0.4) )
			Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMaxOne*StrToFloat(Edit44->Text)/BoxXY[ifile].N, FloatToStr(FixRoundTo(SigMaxOne/BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint)+"/"+IntToStr(BoxXY[ifile].N), RGB (255,165,0));
			else
			if ( SigMaxOne/BoxXY[ifile].N >= (StrToFloat(Edit21->Text) + 0.4) )
			Series3->AddBubble(BoxXY[ifile].x,BoxXY[ifile].y,SigMaxOne*StrToFloat(Edit44->Text)/BoxXY[ifile].N, FloatToStr(FixRoundTo(SigMaxOne/BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint)+"/"+IntToStr(BoxXY[ifile].N), RGB (255,0,0));
			}

		}

		Chart2->Title->Text->Text = AnsiString("Среднее значение по сектору (оптимум/всего)\r\n" + Edit32->Text + " №" + Edit33->Text + "  ("+ buffDate +")\r\n\r\n\r\n").c_str();
		Chart2->Refresh();
		Application->ProcessMessages();
//		Chart2->PaintTo(bm2->Canvas->Handle,0,0);
		NameGraf3 = (Edit23->Text+"\\Sigma - "+ bufftime + " - Graf BUBBLE Sky5.bmp");
		Chart2->SaveToBitmapFile(NameGraf3);

		FlagClose2 = true;
		AddParagraph("Расчет среднего значения по каждому сектору");

		AddPicture(NameGraf3);
		ProgressBar1->Position++;
		//----------------------------------------------------
		//Легенда
		Chart2->Axes->Visible = false;
		Chart2->Width  = 700;
		Chart2->Height = 150;
		Chart2->Series[0]->Clear();
		Chart2->Series[1]->Clear();
		Chart2->Refresh();
		Chart2->Title->Text->Text = AnsiString("Легенда\r\n").c_str();

		int FirstStepBubble = 5;
		int StepBubble = 15;
		int SBiter = FirstStepBubble;

		Series3->AddBubble(SBiter, 10,2,(Edit47->Text) +	 						    " - " + FloatToStr(StrToFloat(Edit20->Text) - 0.2)+"\r"		  	, RGB (153,50,204));
		Series3->AddBubble(SBiter += StepBubble,10,2,FloatToStr(StrToFloat(Edit20->Text) - 0.2) + " - " + FloatToStr(StrToFloat(Edit20->Text) - 0.1)+"\r"	, RGB (0,87,250));
		Series3->AddBubble(SBiter += StepBubble,10,2,FloatToStr(StrToFloat(Edit20->Text) - 0.1) + " - " + (Edit20->Text)+"\r"							  	, RGB (66,170,255));
		Series3->AddBubble(SBiter += StepBubble,10,2,(Edit20->Text)+							    " - " + (Edit21->Text)+"\r"							, RGB (0,255,0));
		Series3->AddBubble(SBiter += StepBubble,10,2,(Edit21->Text)+							    " - " + FloatToStr(StrToFloat(Edit21->Text) + 0.2)+"\r", RGB (255,255,0));
		Series3->AddBubble(SBiter += StepBubble,10,2,FloatToStr(StrToFloat(Edit21->Text) + 0.2) + " - " + FloatToStr(StrToFloat(Edit21->Text) + 0.4)+"\r"	, RGB (255,165,0));
		Series3->AddBubble(SBiter += StepBubble,10,2,FloatToStr(StrToFloat(Edit21->Text) + 0.4) + " - " + (Edit48->Text)+"\r"								, RGB (255,0,0));

		Series3->AddBubble(1500,10,8,"0", RGB (255,0,0));

		Chart2->Axes->Bottom->AutomaticMaximum = false;
		Chart2->Axes->Bottom->AutomaticMinimum = false;
		Chart2->Axes->Bottom->Minimum = 0;
		Chart2->Axes->Bottom->Maximum = 100;
		Chart2->Axes->Left->AutomaticMaximum = true;
		Chart2->Axes->Left->AutomaticMinimum = true;

		Chart2->Refresh();
		Application->ProcessMessages();
//		Chart2->PaintTo(bm2->Canvas->Handle,0,0);
		NameGraf3 = (Edit23->Text+"\\Sigma - "+ bufftime + " - Graf Leg.bmp");
		Chart2->SaveToBitmapFile(NameGraf3);
		AddPicture(NameGraf3);

		Chart2->Width = StrToFloat(Edit41->Text);
		Chart2->Height = Chart2->Width*StrToFloat(Edit43->Text)/StrToFloat(Edit34->Text);
		Chart2->Axes->Visible = true;
		Chart2->Refresh();
		AddParagraph("\f");
		Series3->Marks->Visible = false;
		ProgressBar1->Position++;
		//----------------------------------------------------
		//INI
//		AddParagraph("\f");
		SetTextFormat(8, 0, 0, 0, 0, 0.1);
		AddTable(2, 2);
		UnionCell(1, 1, 1, 2);
		SetCell(1, 1, "INI файл");
		SetCell(2, 1,"[Overall]\nX=" + Edit1->Text +
					 "\nY=" + Edit2->Text +
					 "\nDir=" + Edit3->Text +
					 "\nMask=" + Edit4->Text +
					 "\nCheckChoice=" + BoolToStr(CheckBox1->Checked) +
					 "\nSaveDir=" + Edit23->Text +
					 "\nFormatFrame=" + IntToStr(RadioGroup1->ItemIndex) +

					 "\n[LOC-options]\nKofFilt=" + Edit10->Text +
					 "\nCheckFilt=" + BoolToStr(CheckBox3->Checked) +
					 "\nkofSKO1=" + Edit17->Text +
					 "\npixMin=" + Edit11->Text +
					 "\npixMax=" + Edit12->Text +
					 "\nIsMin=" + Edit13->Text +
					 "\nIsMax=" + Edit14->Text +

					 "\n[Sigma-options]\nframe=" + Edit9->Text +
					 "\nKofBin=" + Edit5->Text +
					 "\nCheckBin=" + BoolToStr(CheckBox2->Checked) +
					 "\nkofSKO2=" + Edit28->Text +
					 "\npixMaxIs=" + Edit15->Text +
					 "\npixMinIs=" + Edit16->Text +
					 "\nCheckMostLight=" + BoolToStr(CheckBox4->Checked) +
					 "\nCheck_Is-pix=" + IntToStr(ComboBoxPixIs->ItemIndex) +
					 "\nforLG_step=" + Edit24->Text +
					 "\nforLG_iter=" + Edit25->Text +
					 "\nforLG_emptyPix=" + Edit26->Text +
					 "\nforLG_MinPix=" + Edit52->Text +
					 "\nCheckSector=" + BoolToStr(CheckBox14->Checked) +

					 "\n[Report-options]\nNameDevice=" + Edit32->Text +
					 "\nNomberDevice=" + Edit33->Text +
					 "\nXmatr=" + Edit34->Text +
					 "\nYmatr=" + Edit43->Text +
					 "\nNameShooter=" + Edit35->Text +
					 "\nDataShoot=" + Edit36->Text +
					 "\nNameReporter=" + Edit37->Text +
					 "\nComment=" + Memo2->Text +

					 "\n[Photogrammetry-options]\nIsStar6=" + Edit_IOZ6->Text +
					 "\nIsStar5=" + Edit_IOZ5->Text +
					 "\nIsStar4=" + Edit_IOZ4->Text +
					 "\nPhotometryCheck=" + BoolToStr(CheckBoxPhotometry->Checked));

		SetCell(2, 2,"[Research-options]\nId1ms=" + Edit8->Text +
					 "\nId2ms=" + Edit6->Text +
					 "\nId1mm=" + Edit22->Text +
					 "\nId2mm=" + Edit7->Text +
					 "\ninfKol=" + Edit18->Text +
					 "\ntochnost=" + Edit19->Text +
					 "\nFocDevice=" + Edit45->Text +
					 "\nFocCol=" + Edit46->Text +
					 "\nPixSize=" + Edit50->Text +
					 "\nBaric=" + EditBaric->Text +
					 "\nBaricCheck=" + BoolToStr(CheckBoxBaric->Checked) +
					 "\nSlopeParam=" + IntToStr(ComboBoxMainPoint->ItemIndex) +

					 "\n[Graphic-options]\nminXGraf=" + Edit27->Text +
					 "\nmaxXGraf=" + Edit29->Text +
					 "\nCheckGrafX=" + BoolToStr(CheckBox6->Checked) +
					 "\nminYGraf=" + Edit30->Text +
					 "\nmaxYGraf=" + Edit31->Text +
					 "\nCheckGrafY=" + BoolToStr(CheckBox7->Checked) +
					 "\nHeightGraf=" + Edit40->Text +
					 "\nWidthGraf=" + Edit41->Text +
					 "\nStep=" + Edit53->Text +
					 "\nStepGrafX=" + Edit38->Text +
					 "\nStepGrafY=" + Edit39->Text +
					 "\nCheckStep=" + BoolToStr(CheckBox12->Checked) +
					 "\nSizeBubble=" + Edit44->Text +
					 "\nFixSizeBubble=" + BoolToStr(CheckBox11->Checked) +
					 "\nBrGist=" + Edit56->Text +
					 "\nCheckBrGist=" + BoolToStr(CheckBox16->Checked) +

					 "\noptSigma1=" + Edit20->Text +
					 "\noptSigma2=" + Edit21->Text +
					 "\ndopSigma1=" + Edit47->Text +
					 "\ndopSigma2=" + Edit48->Text +
					 "\nCheckDopSigma=" + BoolToStr(CheckBox9->Checked) +
					 "\nMu=" + Edit49->Text +
					 "\nCheckMu=" + BoolToStr(CheckBox10->Checked) +
					 "\nDeviationCenter=" + Edit55->Text +
					 "\nCheckDC=" + BoolToStr(CheckBox15->Checked) +
					 "\nSizeFrag=" + Edit54->Text +
					 "\nNFrameMAX=" + EditNFmax->Text +
					 "\nNFrameMIN=" + EditNFmin->Text +
					 "\nCheckNF=" + BoolToStr(CheckBoxNF->Checked) +
					 "\nCheckNFgraf=" + BoolToStr(CheckBoxNframe->Checked) +

					 "\n[Sky-options]\nCheckSkyGraf=" + BoolToStr(CheckBox5->Checked) +
					 "\nMinStarsInSector=" + Edit51->Text);

//		AddParagraph("[Overall]\nX=" + Edit1->Text +
//					 "\nY=" + Edit2->Text +
//					 "\nDir=" + Edit3->Text +
//					 "\nMask=" + Edit4->Text +
//					 "\nCheckChoice=" + BoolToStr(CheckBox1->Checked) +
//					 "\nSaveDir=" + Edit23->Text);
//
//		AddParagraph("[LOC-options]\nKofFilt=" + Edit10->Text +
//					 "\nCheckFilt=" + BoolToStr(CheckBox3->Checked) +
//					 "\nkofSKO1=" + Edit17->Text +
//					 "\npixMin=" + Edit11->Text +
//					 "\npixMax=" + Edit12->Text +
//					 "\nIsMin=" + Edit13->Text +
//					 "\nIsMax=" + Edit14->Text);
//
//		AddParagraph("[LOC-options]\nKofFilt=" + Edit10->Text +
//					 "\nCheckFilt=" + BoolToStr(CheckBox3->Checked) +
//					 "\nkofSKO1=" + Edit17->Text +
//					 "\npixMin=" + Edit11->Text +
//					 "\npixMax=" + Edit12->Text +
//					 "\nIsMin=" + Edit13->Text +
//					 "\nIsMax=" + Edit14->Text);
//
//		AddParagraph("[Sigma-options]\nframe=" + Edit9->Text +
//					 "\nKofBin=" + Edit5->Text +
//					 "\nCheckBin=" + BoolToStr(CheckBox2->Checked) +
//					 "\nkofSKO2=" + Edit28->Text +
//					 "\npixMaxIs=" + Edit15->Text +
//					 "\npixMinIs=" + Edit16->Text +
//					 "\nCheckMostLight=" + BoolToStr(CheckBox4->Checked) +
//					 "\nforLG_step=" + Edit24->Text +
//					 "\nforLG_iter=" + Edit25->Text +
//					 "\nforLG_emptyPix=" + Edit26->Text);
//
//		AddParagraph("[Report-options]\nNameDevice=" + Edit32->Text +
//					 "\nNomberDevice=" + Edit33->Text +
//					 "\nXmatr=" + Edit34->Text +
//					 "\nYmatr=" + Edit43->Text +
//					 "\nNameShooter=" + Edit35->Text +
//					 "\nDataShoot=" + Edit36->Text +
//					 "\nNameReporter=" + Edit37->Text +
//					 "\nComment=" + Memo2->Text);
//
//		AddParagraph("[Research-options]\nId1ms=" + Edit8->Text +
//					 "\nId2ms=" + Edit6->Text +
//					 "\nId1mm=" + Edit22->Text +
//					 "\nId2mm=" + Edit7->Text +
//					 "\ninfKol=" + Edit18->Text +
//					 "\ntochnost=" + Edit19->Text +
//					 "\nFocDevice=" + Edit45->Text +
//					 "\nFocCol=" + Edit46->Text +
//					 "\nPixSize=" + Edit50->Text);
//
//		AddParagraph("[Graphic-options]\nminXGraf=" + Edit27->Text +
//					 "\nmaxXGraf=" + Edit29->Text +
//					 "\nCheckGrafX=" + BoolToStr(CheckBox6->Checked) +
//					 "\nminYGraf=" + Edit30->Text +
//					 "\nmaxYGraf=" + Edit31->Text +
//					 "\nCheckGrafY=" + BoolToStr(CheckBox7->Checked) +
//					 "\nHeightGraf=" + Edit40->Text +
//					 "\nWidthGraf=" + Edit41->Text +
//					 "\nStepGrafX=" + Edit38->Text +
//					 "\nStepGrafY=" + Edit39->Text +
//					 "\nCheckStep=" + BoolToStr(CheckBox12->Checked) +
//					 "\nSizeBubble=" + Edit44->Text +
//					 "\nFixSizeBubble=" + BoolToStr(CheckBox11->Checked) +
//					 "\noptSigma1=" + Edit20->Text +
//					 "\noptSigma2=" + Edit21->Text +
//					 "\ndopSigma1=" + Edit47->Text +
//					 "\ndopSigma2=" + Edit48->Text +
//					 "\nCheckDopSigma=" + BoolToStr(CheckBox9->Checked) +
//					 "\nMu=" + Edit49->Text +
//					 "\nCheckMu=" + BoolToStr(CheckBox10->Checked));
//
//		AddParagraph("[Sky-options]\nCheckSkyGraf=" + BoolToStr(CheckBox5->Checked) + "\nMinStarsInSector=" + Edit51->Text);

		//----------------------------------------------------
		ReportName = Edit23->Text+"\\Sigma - " + bufftime + " - SigmaGraph Sky.doc";
		SaveDoc(ReportName);
		CloseDoc();
		CloseWord();
		N3->Enabled = true;
		ProgressBar1->Position++;
		delete [] BoxXY;
		delete [] BoxMS;
		delete [] BoxMM;
		BoxXY = NULL;
		BoxMS = NULL;
		BoxMM = NULL;
	}
	else { //Обработка по ИОЗ            ---------------------------------------------------------------------------------------------------------------------------------иоз
		SBoxXY *BoxXY = new SBoxXY[Meter];
		SBoxXY *BoxMS = new SBoxXY[Meter];
		SBoxXY *BoxMM = new SBoxXY[Meter];
		//Обнаружение идентификаторов
		for (ifile = 0; ifile < Meter; ifile++) {
			IdentifFromName(AnsiString(FullSigma[ifile].Names).c_str(), AnsiString(Edit8->Text).c_str() , AnsiString(Edit6->Text).c_str(), &FullSigma[ifile].ms);
			IdentifFromName(AnsiString(FullSigma[ifile].Names).c_str(), AnsiString(Edit22->Text).c_str(), AnsiString(Edit7->Text).c_str(), &FullSigma[ifile].mm);
		}

		//Массивы по координатам и мс и мм
		//Очистка
		for (ifile = 0; ifile < Meter; ifile++)
		{
			BoxXY[ifile].x = 0;
			BoxXY[ifile].y = 0;
			BoxXY[ifile].N = 0;
			BoxMS[ifile].ms = 0;
		}

		//Присвоение первого значения
		BoxXY[0].x = FullSigma[0].x;
		BoxXY[0].y = FullSigma[0].y;
		BoxMS[0].ms = FullSigma[0].ms;
		BoxMM[0].mm = FullSigma[0].mm;
		BoxXY[0].N++;


		float pixSize = StrToFloat(Edit50->Text);
		float InfCol = StrToFloat(Edit18->Text);
		float FocBkz = StrToFloat(Edit45->Text);
		float FocCol = StrToFloat(Edit46->Text);

		int Xmatr = StrToInt(Edit34->Text);
		int Ymatr = StrToInt(Edit43->Text);
		int Sector =  StrToInt(Edit19->Text);
		float SigmaInfX, SigmaInfY;
		float BubbleSize = StrToFloat(Edit44->Text);
		
//		NomBoxXY = 0;
//		int StepForBoxX, StepForBoxY = PorogPx/2;  //Создание сетки секторов
//		for (i = 0; i < ((StrToInt(Form1->Edit43->Text)+PorogPx-1)/PorogPx); i++) {
//		StepForBoxX = PorogPx/2;
//			for (j = 0; j < ((StrToInt(Form1->Edit34->Text)+PorogPx-1)/PorogPx); j++) {
//				BoxXY[NomBoxXY].x = StepForBoxX;
//				StepForBoxX += PorogPx;
//				BoxXY[NomBoxXY++].y = StepForBoxY;
//			}
//			StepForBoxY += PorogPx;
//		}
//		NomBoxXY--;
		//Заполнение массивов  BoxXY  BoxMS  BoxMM
		ReportSort(FullSigma, Meter, PorogPx, BoxXY, &NomBoxXY, BoxMS, &NomBoxMS, BoxMM, &NomBoxMM);

		//Рандомное присвоение цветов
		for (i = 0; i < NomBoxMS; i++) {
			BoxMS[i].ColGraf = (TColor) RGB (rand()%255,rand()%255,rand()%255);
		}

		String NameGraf2;

		//Заполнение документа Ворд
		OpenWord(false);
		AddDoc();
		SetTextFormat(14, 1, 0, 0, 1, 1);
		AddParagraph("Отчет по параметрам фокусировки\n");

		//AddParagraph("\f"); Переход на страницу
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label27->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1);AddParagraph("  " + Edit32->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label28->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1);AddParagraph("  " + Edit33->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label29->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit34->Text + "x" + Edit43->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph("Бесконечность коллиматора: "); 	SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit18->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label43->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit45->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label44->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit46->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph("Оптимальная сигма:"); 			SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  от " + Edit20->Text + "  до " + Edit21->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label32->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Memo2->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph("Название первого кадра:"); 		SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + FullSigma[0].Names);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph("Название файлов для отчета:");

		SetTextFormat(12, 0, 0, 0, 0, 0.1);
		for (i = 0; i < OpenDialog3 -> Files -> Count; i++)
		AddParagraph("  " + IntToStr(i+1) + ". " + OpenDialog3 -> Files -> Strings[i]);

		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraphNo("\n"+Label30->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit35->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraphNo(Label33->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit36->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraphNo(Label31->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit37->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraphNo("Дата формирования отчета:"); 	SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + buffDate);

		//Параметры графика для пузырей
		Chart2->Visible = false;
		Chart2->Walls->Visible = false;
		Chart2->Width = StrToFloat(Edit41->Text);
		Chart2->Height = Chart2->Width*StrToFloat(Edit2->Text)/StrToFloat(Edit1->Text);

		Chart2->Axes->Left->Minimum    = -10000;
		Chart2->Axes->Bottom->Minimum  = -10000;

		Chart2->Axes->Left->Maximum = StrToFloat(Edit2->Text);
		Chart2->Axes->Left->Minimum = 0;
		Chart2->Axes->Bottom->Maximum = StrToFloat(Edit1->Text);
		Chart2->Axes->Bottom->Minimum = 0;

		bm2->PixelFormat = pf32bit;
		bm2->Width = Chart2->Width;
		bm2->Height = Chart2->Height;

		float schet3;
		String MarksStr[50];
		float ConverMM;

		SetLineUnitAfter(1);

		//Аппроксимация  --------------------------------------------------------
		#pragma region Аппроксимация

		float BorderMin;
		float BorderMax;

		if (Edit53->Text == "")
		{
        	Edit53->Text = 1;
        }

		if (CheckBox6->Checked == true)
		{
			BorderMin = 10000;
			BorderMax = -10000;
			for (ifile = 0; ifile < Meter; ifile++)
			{
				if (FullSigma[ifile].mm < BorderMin) BorderMin = FullSigma[ifile].mm;
				if (FullSigma[ifile].mm > BorderMax) BorderMax = FullSigma[ifile].mm;
			}
		}
		else
		{
			BorderMin = StrToFloat(Edit27->Text);
			BorderMax = StrToFloat(Edit29->Text);
        }

		const float SigmaMin = StrToFloat(Edit20->Text);
		const float SigmaMax = StrToFloat(Edit21->Text);

		//Создание структуры для хранения результатов
		struct AprocRezStr
		{
			//При ax2+bx+c
			double A;
			double B;
			double C;

			double* K;
			double ExtX;
			double ExtY;

			double PerV1;
			double PerV2;
			double PerN1;
			double PerN2;

			double Rez;
			double RezMeanL;
			double RezMeanR;

			double X;

			double* massive;
		};

		AprocRezStr *AprocRezX = new AprocRezStr[NomBoxXY];  //Структура результатов для сигмы Х
		AprocRezStr *AprocRezY = new AprocRezStr[NomBoxXY];  //Структура результатов для сигмы У

		int MNK_deg = 2;
		if (EditMNK->Text != "")
		{
			MNK_deg = StrToInt(EditMNK->Text); //Степень полинома аппроксимации
		}
		else
		{
			 ShowMessage("Степень полинома аппроксимации не задана :(");
		}
		float SpetApproc = 0.01;
		int SizeNewMas = (BorderMax -  BorderMin) / SpetApproc;

		for (i = 0; i < NomBoxXY; i++)
		{
			AprocRezX[i].K = new double[MNK_deg+1];
			AprocRezY[i].K = new double[MNK_deg+1];

			AprocRezX[i].massive = new double[SizeNewMas];
			AprocRezY[i].massive = new double[SizeNewMas];
		}

		//Цикл МНК
		for (i = 0; i < NomBoxXY; i++)
		{

			double *AprocMasMM = new double[Meter];
			double *AprocMasX  = new double[Meter];
			double *AprocMasY  = new double[Meter];

			int schA = 0;   //Переписываем в новые массивы данные по каждой координате для расчета МНК
			for (ifile = 0; ifile < Meter; ifile++)
			if ( ((FullSigma[ifile].x + PorogPx) >= BoxXY[i].x) && ((FullSigma[ifile].x - PorogPx) <= BoxXY[i].x) &&
				 ((FullSigma[ifile].y + PorogPx) >= BoxXY[i].y) && ((FullSigma[ifile].y - PorogPx) <= BoxXY[i].y))
			{
				AprocMasMM[schA]  = FullSigma[ifile].mm;
				AprocMasX[schA]   = FullSigma[ifile].SigX;
				AprocMasY[schA++] = FullSigma[ifile].SigY;
			}

			//МНК Любой степени, получаем значения а, б и с для ax2+bx+c
			MNK (AprocMasMM, AprocMasX, MNK_deg, schA, AprocRezX[i].K);
			MNK (AprocMasMM, AprocMasY, MNK_deg, schA, AprocRezY[i].K);

			//Расчет результатов (нахождение экстремума для параболы)

			MakeMassive(BorderMin, SizeNewMas, MNK_deg, SpetApproc, AprocRezX[i].K, AprocRezX[i].massive);
			MakeMassive(BorderMin, SizeNewMas, MNK_deg, SpetApproc, AprocRezY[i].K, AprocRezY[i].massive);
			MinimumMas(AprocRezX[i].massive, SizeNewMas, BorderMin, SpetApproc, &AprocRezX[i].ExtY, &AprocRezX[i].ExtX);
			MinimumMas(AprocRezY[i].massive, SizeNewMas, BorderMin, SpetApproc, &AprocRezY[i].ExtY, &AprocRezY[i].ExtX);

			AprocRezY[i].RezMeanL = 1000;
			AprocRezY[i].RezMeanR = 1000;
			AprocRezX[i].RezMeanL = 1000;
			AprocRezX[i].RezMeanR = 1000;
			AprocRezX[i].X = 1000;
			AprocRezY[i].X = 1000;

			//(нахождение пересечений: PerV1,PerV2 - с верхней линией; PerN1,PerN2 - с нижней линией
			FindRootsOfEquation(AprocRezX[i].K, MNK_deg, SigmaMin, BorderMin, BorderMax, &AprocRezX[i].PerN1, &AprocRezX[i].PerN2);
			FindRootsOfEquation(AprocRezY[i].K, MNK_deg, SigmaMin, BorderMin, BorderMax, &AprocRezY[i].PerN1, &AprocRezY[i].PerN2);

			FindRootsOfEquation(AprocRezX[i].K, MNK_deg, SigmaMax, BorderMin, BorderMax, &AprocRezX[i].PerV1, &AprocRezX[i].PerV2);
			FindRootsOfEquation(AprocRezY[i].K, MNK_deg, SigmaMax, BorderMin, BorderMax, &AprocRezY[i].PerV1, &AprocRezY[i].PerV2);

			delete [] AprocMasMM; AprocMasMM= NULL;
			delete [] AprocMasX;  AprocMasX = NULL;
			delete [] AprocMasY;  AprocMasY = NULL;

			AprocRezX[i].RezMeanL = MeanABorAC(AprocRezX[i].PerV2, AprocRezX[i].PerN2, AprocRezX[i].ExtX, 1000);
			AprocRezY[i].RezMeanL = MeanABorAC(AprocRezY[i].PerV2, AprocRezY[i].PerN2, AprocRezY[i].ExtX, 1000);
			AprocRezX[i].RezMeanR = MeanABorAC(AprocRezX[i].PerV1, AprocRezX[i].PerN1, AprocRezX[i].ExtX, -1000);
			AprocRezY[i].RezMeanR = MeanABorAC(AprocRezY[i].PerV1, AprocRezY[i].PerN1, AprocRezY[i].ExtX, -1000);
			
//			if (AprocRezX[i].PerV2 != 1000 && AprocRezX[i].PerN2 != 1000)  AprocRezX[i].RezMeanL = (AprocRezX[i].PerV2+AprocRezX[i].PerN2)/2 ;
//			else  	if (AprocRezX[i].PerV2 != 1000)           			   AprocRezX[i].RezMeanL = (AprocRezX[i].PerV2+AprocRezX[i].ExtX) /2 ;
//
//			if (AprocRezY[i].PerV2 != 1000 && AprocRezY[i].PerN2 != 1000)  AprocRezY[i].RezMeanL = (AprocRezY[i].PerV2+AprocRezY[i].PerN2)/2 ;
//			else  	if (AprocRezY[i].PerV2 != 1000)           			   AprocRezY[i].RezMeanL = (AprocRezY[i].PerV2+AprocRezY[i].ExtX) /2 ;
//
//			if (AprocRezX[i].PerV1 != -1000 && AprocRezX[i].PerN1 != -1000)  AprocRezX[i].RezMeanR = (AprocRezX[i].PerV1+AprocRezX[i].PerN1)/2 ;
//			else  	if (AprocRezX[i].PerV1 != -1000)           			   AprocRezX[i].RezMeanR = (AprocRezX[i].PerV1+AprocRezX[i].ExtX) /2 ;
//
//			if (AprocRezY[i].PerV1 != -1000 && AprocRezY[i].PerN1 != -1000)  AprocRezY[i].RezMeanR = (AprocRezY[i].PerV1+AprocRezY[i].PerN1)/2 ;
//			else  	if (AprocRezY[i].PerV1 != -1000)           			   AprocRezY[i].RezMeanR = (AprocRezY[i].PerV1+AprocRezY[i].ExtX) /2 ;

//			AprocRezX[i].Rez = AprocRezX[i].RezMeanL*AprocRezX[i].RezMeanL*AprocRezX[i].A +AprocRezX[i].B*AprocRezX[i].RezMeanL + AprocRezX[i].C;
//			AprocRezY[i].Rez = AprocRezY[i].RezMeanL*AprocRezY[i].RezMeanL*AprocRezY[i].A +AprocRezY[i].B*AprocRezY[i].RezMeanL + AprocRezY[i].C;

		} //Цикл МНК - конец



//		delete [] schetBx; schetBx = NULL;  //Да-Нет вхождения бесконечности в 100%
//		delete [] schetBy; schetBy = NULL;  //Да-Нет вхождения бесконечности в 100%

		double *AprocLineX  = new double[NomBoxXY];
		double *AprocLineY  = new double[NomBoxXY];
		int Chuwi;
		double ChuwiAx, ChuwiBx, ChuwiAy, ChuwiBy;  //x,y - обозначение оси
		double ForYaxisMax = -1000, ForYaxisMin = 1000;


		float RelizXrez, RelizYrez, RelizXYrez;
		for (i = 0; i < NomBoxXY; i++)
		{
			switch(ComboBoxMainPoint->ItemIndex)
			{
				case 0:
					AprocRezX[i].X = AprocRezX[i].PerV2;
					AprocRezY[i].X = AprocRezY[i].PerV2;
				break;
			
				case 1:
					AprocRezX[i].X = AprocRezX[i].ExtX;
					AprocRezY[i].X = AprocRezY[i].ExtX;
				break;
			
				case 2:
					AprocRezX[i].X = AprocRezX[i].RezMeanL;
					AprocRezY[i].X = AprocRezY[i].RezMeanL;
				break;
			}
		}

		AddParagraph("Параметр наклона: " + ComboBoxMainPoint->Items->Strings[ComboBoxMainPoint->ItemIndex]); 
		
		Chuwi = 0;
		for (i = 0; i < NomBoxXY; i++)
		if (AprocRezY[i].X != 1000 && AprocRezX[i].X != 1000
										  && (BoxXY[i].y >= ((StrToInt(Edit43->Text)/2) - StrToInt(Edit19->Text)))
										  && (BoxXY[i].y <= ((StrToInt(Edit43->Text)/2) + StrToInt(Edit19->Text))) ) {
			AprocLineX[Chuwi] = BoxXY[i].x * StrToFloat(Edit50->Text);
			AprocLineY[Chuwi] = ConvMM((AprocRezX[i].X+AprocRezY[i].X)/2);

//			if (ForYaxisMax < ConvMM(AprocRezX[i].RezMeanL)) ForYaxisMax = ConvMM(AprocRezX[i].RezMeanL);  //Для левой шкалы макс/мин значения
//			if (ForYaxisMax < ConvMM(AprocRezY[i].RezMeanL)) ForYaxisMax = ConvMM(AprocRezY[i].RezMeanL);
//			if (ForYaxisMin > ConvMM(AprocRezX[i].RezMeanL)) ForYaxisMin = ConvMM(AprocRezX[i].RezMeanL);
//			if (ForYaxisMin > ConvMM(AprocRezY[i].RezMeanL)) ForYaxisMin = ConvMM(AprocRezY[i].RezMeanL);
			Chuwi++;
		}
		int MNK1ProvX = MNK1pow (AprocLineX, AprocLineY, 0, Chuwi-1, &ChuwiBx, &ChuwiAx); //МНК (линейный) по оси Х

		Chuwi = 0;
		for (i = 0; i < NomBoxXY; i++)
		if (AprocRezY[i].X != 1000 && AprocRezX[i].X != 1000
										  && (BoxXY[i].x >= ((StrToInt(Edit34->Text)/2) - StrToInt(Edit19->Text)))
										  && (BoxXY[i].x <= ((StrToInt(Edit34->Text)/2) + StrToInt(Edit19->Text))) ) {
			AprocLineX[Chuwi] = BoxXY[i].y * StrToFloat(Edit50->Text);
			AprocLineY[Chuwi] = ConvMM((AprocRezX[i].X+AprocRezY[i].X)/2);

//			if (ForYaxisMax < ConvMM(AprocRezX[i].RezMeanL)) ForYaxisMax = ConvMM(AprocRezX[i].RezMeanL);  //Для левой шкалы макс/мин значения
//			if (ForYaxisMax < ConvMM(AprocRezY[i].RezMeanL)) ForYaxisMax = ConvMM(AprocRezY[i].RezMeanL);
//			if (ForYaxisMin > ConvMM(AprocRezX[i].RezMeanL)) ForYaxisMin = ConvMM(AprocRezX[i].RezMeanL);
//			if (ForYaxisMin > ConvMM(AprocRezY[i].RezMeanL)) ForYaxisMin = ConvMM(AprocRezY[i].RezMeanL);
			Chuwi++;
		}
		int MNK1ProvY = MNK1pow (AprocLineX, AprocLineY, 0, Chuwi-1, &ChuwiBy, &ChuwiAy); //МНК (линейный) по оси У

		delete [] AprocLineX; AprocLineX = NULL;
		delete [] AprocLineY; AprocLineY = NULL;

		for (i = 0; i < NomBoxXY; i++) {
			if (AprocRezY[i].X != 1000 && ((BoxXY[i].x >= ((StrToInt(Edit34->Text)/2) - StrToInt(Edit19->Text)))
											  && (BoxXY[i].x <= ((StrToInt(Edit34->Text)/2) + StrToInt(Edit19->Text)))
											  || (BoxXY[i].y >= ((StrToInt(Edit43->Text)/2) - StrToInt(Edit19->Text)))
											  && (BoxXY[i].y <= ((StrToInt(Edit43->Text)/2) + StrToInt(Edit19->Text)))  )) {
				if (ForYaxisMax < ConvMM(AprocRezY[i].X)) ForYaxisMax = ConvMM(AprocRezY[i].X);
				if (ForYaxisMin > ConvMM(AprocRezY[i].X)) ForYaxisMin = ConvMM(AprocRezY[i].X);
			}
			if (AprocRezX[i].X != 1000 && ((BoxXY[i].x >= ((StrToInt(Edit34->Text)/2) - StrToInt(Edit19->Text)))
											  && (BoxXY[i].x <= ((StrToInt(Edit34->Text)/2) + StrToInt(Edit19->Text)))
											  || (BoxXY[i].y >= ((StrToInt(Edit43->Text)/2) - StrToInt(Edit19->Text)))
											  && (BoxXY[i].y <= ((StrToInt(Edit43->Text)/2) + StrToInt(Edit19->Text)))  )) {
				if (ForYaxisMax < ConvMM(AprocRezX[i].X)) ForYaxisMax = ConvMM(AprocRezX[i].X);  //Для левой шкалы макс/мин значения
				if (ForYaxisMin > ConvMM(AprocRezX[i].X)) ForYaxisMin = ConvMM(AprocRezX[i].X);
			}
		}

		//Создание бмп рисунка
		bm->PixelFormat = pf32bit;
		bm->Width = Chart1->Width;
		bm->Height = Chart1->Height;

		//Настройки графика    ?????? почему  Chart1
		TPointSeries *PointSeries[10];
		Chart1->Visible = false;
		Chart1->Walls->Visible = false;
		Chart1->Width = StrToInt(Edit41->Text);
		Chart1->Height = StrToInt(Edit40->Text);

		ProgressBar1->Max = NomBoxXY*Meter;
		SetBreak();
		Chart3->Legend->Visible = true;
		Chart3->Visible = false;
		Chart3->Walls->Visible = false;
		Chart3->Width = StrToInt(Edit41->Text);
		Chart3->Height = StrToInt(Edit40->Text);
		Chart3->Series[0]->Clear();
		Chart3->Series[1]->Clear();
		Chart3->Series[2]->Clear();
		Chart3->Series[3]->Clear();
		Chart3->Refresh();

		//Заполнение графиков наклона матрицы по Х
		for (i = 0; i < NomBoxXY; i++)
		{
			Parameters(AprocRezX[i].X, AprocRezY[i].X, &RelizXrez, &RelizYrez, &RelizXYrez, InfCol, FocBkz, FocCol);

			if (AprocRezX[i].X != 1000 && (BoxXY[i].y >= (Ymatr/2) - Sector) && (BoxXY[i].y <= (Ymatr/2) + Sector) )
			Chart3->Series[0]->AddXY(BoxXY[i].x * pixSize,RelizXrez, FloatToStr(FixRoundTo(BoxXY[i].x,-1))+"\r\n"+FloatToStr(FixRoundTo(BoxXY[i].x * pixSize,-2)), RGB (30,144,255));

			if (AprocRezY[i].X != 1000 && (BoxXY[i].y >= (Ymatr/2) - Sector) && (BoxXY[i].y <= (Ymatr/2) + Sector) )
			Chart3->Series[1]->AddXY(BoxXY[i].x * pixSize,RelizYrez, FloatToStr(FixRoundTo(BoxXY[i].x,-1))+"\r\n"+FloatToStr(FixRoundTo(BoxXY[i].x * pixSize,-2)), RGB (185,78,72));

			if (AprocRezY[i].X != 1000 && AprocRezX[i].X != 1000 && (BoxXY[i].y >= (Ymatr/2) - Sector) && (BoxXY[i].y <= (Ymatr/2) + Sector) )
			Chart3->Series[2]->AddXY(BoxXY[i].x * pixSize,RelizXYrez, FloatToStr(FixRoundTo(BoxXY[i].x,-1))+"\r\n"+FloatToStr(FixRoundTo(BoxXY[i].x * pixSize,-2)), RGB (2,174,36));
		}

		//Заполнение графиков наклона матрицы по Х
//		for (i = 0; i < NomBoxXY; i++)
//		{
//			if (AprocRezX[i].RezMeanL != 1000 && (BoxXY[i].y >= (StrToInt(Edit43->Text)/2) - StrToInt(Edit19->Text))
//											  && (BoxXY[i].y <= (StrToInt(Edit43->Text)/2) + StrToInt(Edit19->Text)) )
//			Chart3->Series[0]->AddXY(BoxXY[i].x * StrToFloat(Edit50->Text),(AprocRezX[i].RezMeanL - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)), FloatToStr(RoundTo(BoxXY[i].x,-1))+"\r\n"+FloatToStr(RoundTo(BoxXY[i].x * StrToFloat(Edit50->Text),-2)), RGB (30,144,255));
//
//			if (AprocRezY[i].RezMeanL != 1000 && (BoxXY[i].y >= (StrToInt(Edit43->Text)/2) - StrToInt(Edit19->Text))
//											  && (BoxXY[i].y <= (StrToInt(Edit43->Text)/2) + StrToInt(Edit19->Text)) )
//			Chart3->Series[1]->AddXY(BoxXY[i].x * StrToFloat(Edit50->Text),(AprocRezY[i].RezMeanL - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)), FloatToStr(RoundTo(BoxXY[i].x,-1))+"\r\n"+FloatToStr(RoundTo(BoxXY[i].x * StrToFloat(Edit50->Text),-2)), RGB (185,78,72));
//
//			if (AprocRezY[i].RezMeanL != 1000 && AprocRezX[i].RezMeanL != 1000
//											  && (BoxXY[i].y >= (StrToInt(Edit43->Text)/2) - StrToInt(Edit19->Text))
//											  && (BoxXY[i].y <= (StrToInt(Edit43->Text)/2) + StrToInt(Edit19->Text)) )
//			Chart3->Series[2]->AddXY(BoxXY[i].x * StrToFloat(Edit50->Text),((AprocRezX[i].RezMeanL+AprocRezY[i].RezMeanL)/2 - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)), FloatToStr(RoundTo(BoxXY[i].x,-1))+"\r\n"+FloatToStr(RoundTo(BoxXY[i].x * StrToFloat(Edit50->Text),-2)), RGB (2,174,36));
//		}

		//Прямая аппроксимации по Х
		for (i = 0; i < StrToInt(Edit34->Text) * StrToFloat(Edit50->Text); i++)
			Chart3->Series[3]->AddXY(i,ChuwiAx*i+ChuwiBx);

		Chart3->Axes->Bottom->AutomaticMaximum = false;
		Chart3->Axes->Bottom->AutomaticMinimum = false;
		Chart3->Axes->Left->AutomaticMaximum = false;
		Chart3->Axes->Left->AutomaticMinimum = false;

		if (ForYaxisMax != (-1000) && ForYaxisMin != 1000)
		{
			Chart3->Axes->Bottom->Minimum = -10000;
			Chart3->Axes->Bottom->Maximum = StrToFloat(Edit34->Text) * StrToFloat(Edit50->Text);
			Chart3->Axes->Bottom->Minimum = 0;

			Chart3->Axes->Left->Minimum = -10000;
			Chart3->Axes->Left->Maximum = ForYaxisMax+0.01;
			Chart3->Axes->Left->Minimum = ForYaxisMin-0.01;
			//Chart3->Axes->Left->Increment = StrToFloat(Edit53->Text);
		}

		Chart3->Axes->Bottom->Title->Caption = "\r\nКоордината по оси X";
		Chart3->Refresh();
		Application->ProcessMessages();
		Chart3->Title->Text->Text = AnsiString("   Наклон матрицы по оси X  " + Edit32->Text + " №" + Edit33->Text + "  ("+ buffDate +")").c_str();
//		Chart3->PaintTo(bm->Canvas->Handle,0,0);
		String NameGraf = (Edit23->Text+"\\Sigma - " + bufftime + " - Graf RezX.bmp");

		Chart3->SaveToBitmapFile(NameGraf);
		AddPicture(NameGraf);

		if ( MNK1ProvX == 0 ) {
			AddParagraph("Y = "+FloatToStr(FixRoundTo(ChuwiAx,-8))+"*X + "+FloatToStr(FixRoundTo(ChuwiBx,-8)));
			AddParagraph("acrtg(A) = "+FloatToStr(FixRoundTo(atan(ChuwiAx)* 180.0 / M_PI,-5))+"    ("+FloatToStr(FixRoundTo(60*atan(ChuwiAx)* 180.0 / M_PI,-3))+" угл.мин)");
			AddParagraph("Расстояние по центру до нуля = " + FloatToStr(FixRoundTo(ChuwiAx*((StrToInt(Edit34->Text) * StrToFloat(Edit50->Text))/2)+ChuwiBx,-4)) + " мм");
		}

		//SaveDoc(ReportName);
		Chart3->Series[0]->Clear();
		Chart3->Series[1]->Clear();
		Chart3->Series[2]->Clear();
		Chart3->Series[3]->Clear();
		Chart3->Refresh();

		//Заполнение графиков наклона матрицы по У
		for (i = 0; i < NomBoxXY; i++)
		{
			Parameters(AprocRezX[i].X, AprocRezY[i].X, &RelizXrez, &RelizYrez, &RelizXYrez, InfCol, FocBkz, FocCol);

			if (AprocRezX[i].X != 1000 && (BoxXY[i].x >= (Xmatr/2) - Sector) && (BoxXY[i].x <= (Xmatr/2) + Sector) )
			Chart3->Series[0]->AddXY(BoxXY[i].y * pixSize,RelizXrez, FloatToStr(FixRoundTo(BoxXY[i].y,-1))+"\r\n"+FloatToStr(FixRoundTo(BoxXY[i].y * pixSize,-2)), RGB (30,144,255));

			if (AprocRezY[i].X != 1000 && (BoxXY[i].x >= (Xmatr/2) - Sector) && (BoxXY[i].x <= (Xmatr/2) + Sector) )
			Chart3->Series[1]->AddXY(BoxXY[i].y * pixSize,RelizYrez, FloatToStr(FixRoundTo(BoxXY[i].y,-1))+"\r\n"+FloatToStr(FixRoundTo(BoxXY[i].y * pixSize,-2)), RGB (185,78,72));

			if (AprocRezY[i].X != 1000 && AprocRezX[i].X != 1000 && (BoxXY[i].x >= (Xmatr/2) - Sector) && (BoxXY[i].x <= (Xmatr/2) + Sector) )
			Chart3->Series[2]->AddXY(BoxXY[i].y * pixSize,RelizXYrez, FloatToStr(FixRoundTo(BoxXY[i].y,-1))+"\r\n"+FloatToStr(FixRoundTo(BoxXY[i].y * pixSize,-2)), RGB (2,174,36));
		}

		
//		for (i = 0; i < NomBoxXY; i++)
//		{
//			if (AprocRezX[i].RezMeanL != 1000 && (BoxXY[i].x >= (StrToInt(Edit34->Text)/2) - StrToInt(Edit19->Text))
//											  && (BoxXY[i].x <= (StrToInt(Edit34->Text)/2) + StrToInt(Edit19->Text)) )
//			Chart3->Series[0]->AddXY(BoxXY[i].y * StrToFloat(Edit50->Text),(AprocRezX[i].RezMeanL - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)), FloatToStr(RoundTo(BoxXY[i].y,-1))+"\r\n"+FloatToStr(RoundTo(BoxXY[i].y * StrToFloat(Edit50->Text),-2)), RGB (30,144,255));
//
//			if (AprocRezY[i].RezMeanL != 1000 && (BoxXY[i].x >= (StrToInt(Edit34->Text)/2) - StrToInt(Edit19->Text))
//											  && (BoxXY[i].x <= (StrToInt(Edit34->Text)/2) + StrToInt(Edit19->Text)) )
//			Chart3->Series[1]->AddXY(BoxXY[i].y * StrToFloat(Edit50->Text),(AprocRezY[i].RezMeanL - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)), FloatToStr(RoundTo(BoxXY[i].y,-1))+"\r\n"+FloatToStr(RoundTo(BoxXY[i].y * StrToFloat(Edit50->Text),-2)), RGB (185,78,72));
//
//			if (AprocRezY[i].RezMeanL != 1000 && AprocRezX[i].RezMeanL != 1000
//										 && (BoxXY[i].x >= (StrToInt(Edit34->Text)/2) - StrToInt(Edit19->Text))
//										 && (BoxXY[i].x <= (StrToInt(Edit34->Text)/2) + StrToInt(Edit19->Text)) )
//			Chart3->Series[2]->AddXY(BoxXY[i].y * StrToFloat(Edit50->Text),((AprocRezX[i].RezMeanL+AprocRezY[i].RezMeanL)/2 - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)), FloatToStr(RoundTo(BoxXY[i].y,-1))+"\r\n"+FloatToStr(RoundTo(BoxXY[i].y * StrToFloat(Edit50->Text),-2)), RGB (2,174,36));
//		}
		//Прямая аппроксимации по У
		for (i = 0; i < StrToInt(Edit43->Text) * StrToFloat(Edit50->Text); i++)
		Chart3->Series[3]->AddXY(i,ChuwiAy*i+ChuwiBy);
		Chart3->Refresh();

		Chart3->Axes->Bottom->AutomaticMaximum = false;
		Chart3->Axes->Bottom->AutomaticMinimum = false;
		Chart3->Axes->Left->AutomaticMaximum = false;
		Chart3->Axes->Left->AutomaticMinimum = false;

		if (ForYaxisMax != (-1000) && ForYaxisMin != 1000)
		{
			Chart3->Axes->Bottom->Minimum = -10000;
			Chart3->Axes->Bottom->Maximum = StrToFloat(Edit43->Text) * StrToFloat(Edit50->Text);
			Chart3->Axes->Bottom->Minimum = 0;

			Chart3->Axes->Left->Minimum = -10000;
			Chart3->Axes->Left->Maximum = ForYaxisMax+0.01;
			Chart3->Axes->Left->Minimum = ForYaxisMin-0.01;
//			Chart3->Axes->Left->Increment = StrToFloat(Edit53->Text);
		}

		Chart3->Axes->Bottom->Title->Caption = "\r\nКоордината по оси Y";
		Application->ProcessMessages();
		Chart3->Title->Text->Text = AnsiString("    Наклон матрицы по оси Y  " + Edit32->Text + " №" + Edit33->Text + "  ("+ buffDate +")").c_str();
		Chart3->Refresh();
		Application->ProcessMessages();

//		Chart3->PaintTo(bm->Canvas->Handle,0,0);
		Chart3->Refresh();
		NameGraf = (Edit23->Text+"\\Sigma - " + bufftime + " - Graf RezY.bmp");
		Chart3->SaveToBitmapFile(NameGraf);

		AddPicture(NameGraf);

		if ( MNK1ProvY == 0 ) {
			AddParagraph("Y = "+FloatToStr(FixRoundTo(ChuwiAy,-8))+"*X + "+FloatToStr(FixRoundTo(ChuwiBy,-8)));
			AddParagraph("acrtg(A) = "+FloatToStr(FixRoundTo(atan(ChuwiAy)* 180.0 / M_PI,-5))+"    ("+FloatToStr(FixRoundTo(60*atan(ChuwiAy)* 180.0 / M_PI,-3))+" угл.мин)");
			AddParagraph("Расстояние по центру до нуля = " + FloatToStr(FixRoundTo(ChuwiAy*((StrToInt(Edit43->Text) * StrToFloat(Edit50->Text))/2)+ChuwiBy,-4)) + " мм");
		}
		//SaveDoc(ReportName);
		#pragma end_region

		//Картинка по Х
		Chart2->Series[0]->Clear();
		Chart2->Series[1]->Clear();
		Series3->Marks->Visible=true;
		Series4->Marks->Visible=true;
		Chart2->Title->Text->Text = AnsiString("Sigma X "+Edit32->Text + " №" + Edit33->Text + "  ("+ buffDate +")").c_str();

//		for (i = 0; i < NomBoxXY; i++)
//		{
//			if ((StrToFloat(Edit20->Text)<=(StrToFloat(Edit18->Text)*StrToFloat(Edit18->Text)*AprocRezX[i].A +AprocRezX[i].B*StrToFloat(Edit18->Text) + AprocRezX[i].C))
//			 && (StrToFloat(Edit21->Text)>=(StrToFloat(Edit18->Text)*StrToFloat(Edit18->Text)*AprocRezX[i].A +AprocRezX[i].B*StrToFloat(Edit18->Text) + AprocRezX[i].C)))
//			Series3->AddBubble(BoxXY[i].x,BoxXY[i].y,(StrToFloat(Edit18->Text)*StrToFloat(Edit18->Text)*AprocRezX[i].A +AprocRezX[i].B*StrToFloat(Edit18->Text) + AprocRezX[i].C)*StrToFloat(Edit44->Text), FloatToStr(RoundTo((AprocRezX[i].RezMeanL - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)) +"\n"+ ,RGB (0,255,0));
//			else
//				if (AprocRezX[i].RezMeanL == 1000)
//				Series4->AddBubble(BoxXY[i].x,BoxXY[i].y,(StrToFloat(Edit18->Text)*StrToFloat(Edit18->Text)*AprocRezX[i].A +AprocRezX[i].B*StrToFloat(Edit18->Text) + AprocRezX[i].C)*StrToFloat(Edit44->Text),"-",RGB (255,0,0));
//				else
//				Series4->AddBubble(BoxXY[i].x,BoxXY[i].y,(StrToFloat(Edit18->Text)*StrToFloat(Edit18->Text)*AprocRezX[i].A +AprocRezX[i].B*StrToFloat(Edit18->Text) + AprocRezX[i].C)*StrToFloat(Edit44->Text), FloatToStr(RoundTo((AprocRezX[i].RezMeanL - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)),RGB (255,0,0));
//		}

		for (i = 0; i < NomBoxXY; i++)
		{
			SigmaInfX = FindOneRootOfEquation(InfCol, MNK_deg, AprocRezX[i].K);
			double bubbleRadius = fabs(SigmaInfX) * BubbleSize;

			if ( (SigmaMin <= SigmaInfX) && (SigmaMax >= SigmaInfX) )
			{
				Series3->AddBubble(BoxXY[i].x,
									BoxXY[i].y,
									bubbleRadius,
									FloatToStr(FixRoundTo((AprocRezX[i].X - InfCol)*(FocBkz*FocBkz)/(FocCol*FocCol),-3))
									+ "\r\n" + FloatToStr(FixRoundTo(SigmaInfX,-2)),RGB (0,204,0));
			}
			else
			{
				if (AprocRezX[i].X == 1000)
					Series4->AddBubble(BoxXY[i].x, BoxXY[i].y, bubbleRadius,"-",RGB (255,77,0));
				else
					Series4->AddBubble(BoxXY[i].x,BoxXY[i].y, bubbleRadius, FloatToStr(FixRoundTo((AprocRezX[i].X - InfCol)*(FocBkz*FocBkz)/(FocCol*FocCol),-3)) + "\r\n" + FloatToStr(FixRoundTo(SigmaInfX,-2)),RGB (255,77,0));
			}
		}

		SetTextFormat(12, 1, 1, 0, 0, 1);
		AddParagraph("Результат расчета для сигмы X (график)");
		SetTextFormat(12, 0, 0, 0, 0, 0.5);
		AddParagraph("Размер окружности отражает значение сигмы на бесконечности (пересечение с аппроксимацией)\nЗеленый цвет - значение сигмы попадает в оптимальный промежуток\nКрасный цвет - значение сигмы выходит за границы оптимума\nПодпись - среднее значение в мм для достижения оптимума: \n\"0\" идеал (оптимум на бесконечности коллиматора)\n\"-\" сточить\n\"+\" нарастить");
		AddParagraph("Оптимальное значение сигмы: " + Edit20->Text + " - " + Edit21->Text + "\nБесконечность коллиматора:  " + Edit18->Text);

//		Chart2->PaintTo(bm2->Canvas->Handle,0,0);
		NameGraf2 = Edit23->Text+"\\Sigma - " + bufftime + " - Graf BUBBLE X.bmp";
		Chart2->SaveToBitmapFile(NameGraf2);
		AddPicture(NameGraf2);
		AddParagraph("\f");
		//SaveDoc(ReportName);

		//Картинка по Y
		Chart2->Series[0]->Clear();
		Chart2->Series[1]->Clear();
		Chart2->Refresh();
		Application->ProcessMessages();
		Series3->Marks->Visible=true;
		Series4->Marks->Visible=true;
		Chart2->Title->Text->Text = AnsiString("Sigma Y "+Edit32->Text + " №" + Edit33->Text + "  ("+ buffDate +")").c_str();

//		for (i = 0; i < NomBoxXY; i++)
//		{
//			if ((StrToFloat(Edit20->Text)<=(StrToFloat(Edit18->Text)*StrToFloat(Edit18->Text)*AprocRezY[i].A +AprocRezY[i].B*StrToFloat(Edit18->Text) + AprocRezY[i].C))
//			 && (StrToFloat(Edit21->Text)>=(StrToFloat(Edit18->Text)*StrToFloat(Edit18->Text)*AprocRezY[i].A +AprocRezY[i].B*StrToFloat(Edit18->Text) + AprocRezY[i].C)))
//			Series3->AddBubble(BoxXY[i].x,BoxXY[i].y,(StrToFloat(Edit18->Text)*StrToFloat(Edit18->Text)*AprocRezY[i].A +AprocRezY[i].B*StrToFloat(Edit18->Text) + AprocRezY[i].C)*StrToFloat(Edit44->Text), FloatToStr(RoundTo((AprocRezY[i].RezMeanL - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)),RGB (0,255,0));
//			else
//				if (AprocRezY[i].RezMeanL == 1000)
//				Series4->AddBubble(BoxXY[i].x,BoxXY[i].y,(StrToFloat(Edit18->Text)*StrToFloat(Edit18->Text)*AprocRezY[i].A +AprocRezY[i].B*StrToFloat(Edit18->Text) + AprocRezY[i].C)*StrToFloat(Edit44->Text), "-",RGB (255,0,0));
//				else
//				Series4->AddBubble(BoxXY[i].x,BoxXY[i].y,(StrToFloat(Edit18->Text)*StrToFloat(Edit18->Text)*AprocRezY[i].A +AprocRezY[i].B*StrToFloat(Edit18->Text) + AprocRezY[i].C)*StrToFloat(Edit44->Text), FloatToStr(RoundTo((AprocRezY[i].RezMeanL - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)),RGB (255,0,0));
//		}


		for (i = 0; i < NomBoxXY; i++)
		{
			SigmaInfY = FindOneRootOfEquation(InfCol, MNK_deg, AprocRezY[i].K);
			double bubbleRadius = fabs(SigmaInfY) * BubbleSize;

			if ( (SigmaMin <= SigmaInfY) && (SigmaMax >= SigmaInfY) )
			{
				Series3->AddBubble(BoxXY[i].x,BoxXY[i].y, bubbleRadius, FloatToStr(FixRoundTo((AprocRezY[i].X - InfCol)*(FocBkz*FocBkz)/(FocCol*FocCol),-3)) + "\r\n" + FloatToStr(FixRoundTo(SigmaInfY,-2)),RGB (0,204,0));
			}
			else
			{
				if (AprocRezY[i].X == 1000)
					Series4->AddBubble(BoxXY[i].x, BoxXY[i].y, bubbleRadius,"-",RGB (255,77,0));
				else
					Series4->AddBubble(BoxXY[i].x,BoxXY[i].y, bubbleRadius, FloatToStr(FixRoundTo((AprocRezY[i].X - InfCol)*(FocBkz*FocBkz)/(FocCol*FocCol),-3)) + "\r\n" + FloatToStr(FixRoundTo(SigmaInfY,-2)),RGB (255,77,0));
			}
		}


		Chart2->Refresh();
		Application->ProcessMessages();
		SetTextFormat(12, 1, 1, 0, 0, 1);
		AddParagraph("Результат расчета для сигмы Y (график)");
		SetTextFormat(12, 0, 0, 0, 0, 0.5);
		AddParagraph("Размер окружности отражает значение сигмы на бесконечности (пересечение с аппроксимацией)\nЗеленый цвет - значение сигмы попадает в оптимальный промежуток\nКрасный цвет - значение сигмы выходит за границы оптимума\nПодпись - среднее значение в мм для достижения оптимума:\n\"0\" идеал (оптимум на бесконечности коллиматора)\n\"-\" сточить\n\"+\" нарастить");
		AddParagraph("Оптимальное значение сигмы: " + Edit20->Text + " - " + Edit21->Text + "\nБесконечность коллиматора:  " + Edit18->Text);

//		Chart2->PaintTo(bm2->Canvas->Handle,0,0);
		NameGraf2 = Edit23->Text+"\\Sigma - " + bufftime + " - Graf BUBBLE Y.bmp";
		Chart2->SaveToBitmapFile(NameGraf2);
		AddPicture(NameGraf2);

		ReportName = Edit23->Text+"\\Sigma - " + bufftime + " - SigmaGraph.doc";
		//------------------------------------
		AddParagraph( "\f");
		//SaveDoc(ReportName);
		//Обычные графики по каждой координате с таблицей результатов
		for (i = 0; i < NomBoxXY; i++)
		{
			Chart1->Series[0]->Clear();
			Chart1->Series[1]->Clear();
			Chart1->Series[2]->Clear();
			Chart1->Series[3]->Clear();
			Chart1->Series[4]->Clear();
			Chart1->Series[5]->Clear();
			Chart1->Series[6]->Clear();
			Chart1->Series[7]->Clear();
			Chart1->Series[8]->Clear();
			Chart1->Series[9]->Clear();
			Chart1->Refresh();

			Chart1->Series[2]->AddXY(0,  StrToFloat(Edit20->Text), 0, RGB (123,0,28));
			Chart1->Series[2]->AddXY(1000,StrToFloat(Edit20->Text), "", RGB (123,0,28)); //StrToFloat(Edit29->Text)

			Chart1->Series[3]->AddXY(0,  StrToFloat(Edit21->Text), 0, RGB (123,0,28));
			Chart1->Series[3]->AddXY(1000,StrToFloat(Edit21->Text), "", RGB (123,0,28));

			Chart1->Series[4]->AddXY(StrToFloat(Edit18->Text),0, "", RGB (83,55,122));
			Chart1->Series[4]->AddXY(StrToFloat(Edit18->Text),StrToFloat(Edit31->Text),"", RGB (83,55,122));

			if (CheckBoxBaric -> Checked == true)
			{
				Chart1->Series[8]->AddXY(StrToFloat(EditBaric->Text), 0, (EditBaric->Text));
				Chart1->Series[8]->AddXY(StrToFloat(EditBaric->Text),StrToFloat(Edit31->Text), (EditBaric->Text));
			}

//			SetTextFormat(11, 0, 0, 0, 1, 0.2);
//            SetCellFormat(1, 1, 11, 0, 0, 0);

//			for (ifile = StrToFloat(Edit27->Text); ifile < StrToFloat(Edit29->Text)+1; ifile++)
//			{
//				Chart1->Series[5]->AddXY(ifile,AprocRezX[i].C+AprocRezX[i].B*ifile+AprocRezX[i].A*ifile*ifile,"", RGB (48,100,169));
//				Chart1->Series[6]->AddXY(ifile,AprocRezY[i].C+AprocRezY[i].B*ifile+AprocRezY[i].A*ifile*ifile,"", RGB (191,43,69));

//				double polynomX = 0;
//				double polynomY = 0;
//				double Xstepen = 1;
//				for (int deg = 0; deg < MNK_deg+1; deg++)
//				{
//					if (deg > 0)
//					{
//						Xstepen *= ifile;
//					}
//					polynomX += AprocRezX[i].K[deg] * Xstepen;
//					polynomY += AprocRezY[i].K[deg] * Xstepen;
//				}
//
//				Chart1->Series[5]->AddXY(ifile,polynomX,"", RGB (48,100,169));
//				Chart1->Series[6]->AddXY(ifile,polynomY,"", RGB (191,43,69));
//			}                                                                /
			float StepOne = BorderMin;
			for (ifile = 0; ifile < SizeNewMas; ifile++)
			{
				Chart1->Series[5]->AddXY(StepOne,AprocRezX[i].massive[ifile],"", RGB (48,100,169));
				Chart1->Series[6]->AddXY(StepOne,AprocRezY[i].massive[ifile],"", RGB (191,43,69));
				StepOne += SpetApproc;
			}

			float MinChart1 = 10000;
			float MaxChart1 = -10000;
			for (ifile = 0; ifile < Meter; ifile++)
			{
				if ( ((FullSigma[ifile].x + PorogPx) >= BoxXY[i].x) && ((FullSigma[ifile].x - PorogPx) <= BoxXY[i].x) &&
					 ((FullSigma[ifile].y + PorogPx) >= BoxXY[i].y) && ((FullSigma[ifile].y - PorogPx) <= BoxXY[i].y))
				{
					for (j = 0; j < NomBoxMS; j++) {
						if (FullSigma[ifile].ms < (BoxMS[j].ms+1) && FullSigma[ifile].ms > (BoxMS[j].ms-1))
						{

						ConverMM = (FullSigma[ifile].mm - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text));
						Chart1->Series[0]->AddXY(FullSigma[ifile].mm,FullSigma[ifile].SigX, FloatToStr(FixRoundTo(FullSigma[ifile].mm,-3))+"\r\n"+FloatToStr(FixRoundTo(ConverMM,-3)), BoxMS[j].ColGraf);
						Chart1->Series[1]->AddXY(FullSigma[ifile].mm,FullSigma[ifile].SigY, FloatToStr(FixRoundTo(FullSigma[ifile].mm,-3))+"\r\n"+FloatToStr(FixRoundTo(ConverMM,-3)), BoxMS[j].ColGraf);

						if (FullSigma[ifile].mm < MinChart1) MinChart1 = FullSigma[ifile].mm;
						if (FullSigma[ifile].mm > MaxChart1) MaxChart1 = FullSigma[ifile].mm;
//						PointSeries[0]->AddXY(FullSigma[ifile].mm,FullSigma[ifile].SigX, FullSigma[ifile].mm, (int)FullSigma[ifile].ms*300);
//						PointSeries[0]->AddXY(FullSigma[ifile].mm,FullSigma[ifile].SigY, FullSigma[ifile].mm, (int)FullSigma[ifile].ms*10);
//						Chart1->Series[5]->AddXY(FullSigma[ifile].mm,AprocRezX[i].C+AprocRezX[i].B*FullSigma[ifile].mm+AprocRezX[i].A*FullSigma[ifile].mm*FullSigma[ifile].mm, FloatToStr(FullSigma[ifile].mm)+"\r\n"+FloatToStr(RoundTo(ConverMM,-3)), RGB (48,100,169));
//						Chart1->Series[6]->AddXY(FullSigma[ifile].mm,AprocRezY[i].C+AprocRezY[i].B*FullSigma[ifile].mm+AprocRezY[i].A*FullSigma[ifile].mm*FullSigma[ifile].mm, FloatToStr(FullSigma[ifile].mm)+"\r\n"+FloatToStr(RoundTo(ConverMM,-3)), RGB (191,43,69));
						}
					}
				}
				ProgressBar1->Position++;
				StatusBar1->Panels->Items[0]->Text = IntToStr(ProgressBar1->Position) + " / " + IntToStr(ProgressBar1->Max);
			}

		Chart1->Series[1]->AddXY(MaxChart1 + 0.5, 0, "", 0);
		Chart1->Series[1]->AddXY(MinChart1 - 0.5, 0, "", 0);


			Chart1->Axes->Bottom->Title->Caption = "\r\nfк (df)";
			Chart1->Axes->Left->Title->Caption = "Sigma";
//			Chart1->Axes->Bottom->Items->Format->Font->Style << fsBold;
			//Значения по осям max min
			// Bottom - X
			if (CheckBox6->Checked == false && Edit29->Text != "" && Edit27->Text != "")
			{
				Chart1->Axes->Bottom->AutomaticMaximum = false;
				Chart1->Axes->Bottom->AutomaticMinimum = false;
				Chart1->Axes->Bottom->Minimum = -10000;
				Chart1->Axes->Bottom->Maximum = StrToFloat(Edit29->Text);
				Chart1->Axes->Bottom->Minimum = StrToFloat(Edit27->Text);
			}
			else
			{
				Chart1->Axes->Bottom->AutomaticMaximum = false;
				Chart1->Axes->Bottom->AutomaticMinimum = false;
				Chart1->Axes->Bottom->Minimum = -10000;
				Chart1->Axes->Bottom->Maximum = MaxChart1 + 0.5;
				Chart1->Axes->Bottom->Minimum = MinChart1 - 0.5;
			}

			//  Left - Y
			if (CheckBox7->Checked == false && Edit30->Text != "" && Edit31->Text != "")
			{
				Chart1->Axes->Left->AutomaticMaximum = false;
				Chart1->Axes->Left->AutomaticMinimum = false;
				Chart1->Axes->Left->Minimum = -10000;
				Chart1->Axes->Left->Maximum = StrToFloat(Edit31->Text);
				Chart1->Axes->Left->Minimum = StrToFloat(Edit30->Text);
			}
			else
			{
				Chart1->Axes->Left->AutomaticMaximum = true;
				Chart1->Axes->Left->AutomaticMinimum = true;
			}

			if (CheckBox12->Checked == false)
			{
				Chart1->Axes->Bottom->Increment = StrToFloat(Edit38->Text);
				Chart1->Axes->Left->Increment 	= StrToFloat(Edit39->Text);


//				if (StrToFloat(Edit38->Text) != 0)
//				{
//					float nom1 = 0;
//					for (int nom = 0; nom < (1000/StrToFloat(Edit38->Text)); nom++)
//					{
//						nom1 = nom1 + StrToFloat(Edit38->Text);
//						Chart1->Series[7]->AddX(nom1,FloatToStr(RoundTo(nom1,-3)));
//					}
//					nom1 = 0;
//					for (int nom = 0; nom < (1000/StrToFloat(Edit38->Text)); nom++)
//					{
//						nom1 = nom1 - StrToFloat(Edit38->Text);
//						Chart1->Series[7]->AddX(nom1,FloatToStr(RoundTo(nom1,-3)));
//					}
//				}
			}
			Chart1->Refresh();
			Application->ProcessMessages();


			Chart1->Title->Text->Text = AnsiString(FloatToStr(((int)(BoxXY[i].x*1000))/1000) + "x"
								+ FloatToStr(((int)(BoxXY[i].y*1000))/1000)
								+ "   " + Edit32->Text + " №"
								+ Edit33->Text + "  ("+ buffDate +")").c_str();

			//Chart1->PaintTo(bm->Canvas->Handle,0,0);
			NameGraf = (Edit23->Text+"\\Graf_"+IntToStr(i+1)+"_"+FloatToStr(((int)(BoxXY[i].x*1000))/1000) + "x"+ FloatToStr(((int)(BoxXY[i].y*1000))/1000)+ " " + bufftime + ".bmp");
			Chart1->SaveToBitmapFile(NameGraf);
			AddPicture(NameGraf);

			//График по N_frame   ------------------------------------------------------------
			if (CheckBoxNframe->Checked)
			{
				Chart1->Series[0]->Clear();
				Chart1->Series[1]->Clear();
				Chart1->Series[2]->Clear();
				Chart1->Series[3]->Clear();
				Chart1->Series[4]->Clear();
				Chart1->Series[5]->Clear();
				Chart1->Series[6]->Clear();
				Chart1->Series[7]->Clear();
				Chart1->Series[8]->Clear();
				Chart1->Series[9]->Clear();
				Chart1->Refresh();
	//			Chart1->Series[2]->AddXY(0,  StrToFloat(Edit20->Text), 0, RGB (123,0,28));
	//			Chart1->Series[2]->AddXY(StrToFloat(Edit29->Text),StrToFloat(Edit20->Text), "", RGB (123,0,28));
	//
	//			Chart1->Series[3]->AddXY(0,  StrToFloat(Edit21->Text), 0, RGB (123,0,28));
	//			Chart1->Series[3]->AddXY(StrToFloat(Edit29->Text),StrToFloat(Edit21->Text), "", RGB (123,0,28));

				int NframeMAX = FullSigma[0].Nframe;
				int NframeMIN = FullSigma[0].Nframe;

				if (CheckBoxNF->Checked == true)
				{
					for (ifile = 0; ifile < Meter; ifile++)
					{
						if (FullSigma[ifile].Nframe > NframeMAX)	NframeMAX = FullSigma[ifile].Nframe;
						if (FullSigma[ifile].Nframe < NframeMIN)	NframeMIN = FullSigma[ifile].Nframe;
					}
				}
				else
				{
					NframeMAX = StrToFloat(EditNFmax->Text);
					NframeMIN = StrToFloat(EditNFmin->Text);
				}

				Chart1->Series[4]->AddXY(StrToFloat(Edit18->Text),0, "", RGB (83,55,122));
				Chart1->Series[4]->AddXY(StrToFloat(Edit18->Text),NframeMAX+10,"", RGB (83,55,122));

				if (CheckBoxBaric -> Checked == true)
				{
					Chart1->Series[8]->AddXY(StrToFloat(EditBaric->Text), 0, (EditBaric->Text));
					Chart1->Series[8]->AddXY(StrToFloat(EditBaric->Text),NframeMAX+10, (EditBaric->Text));
				}

	//			SetTextFormat(11, 0, 0, 0, 1, 0.2);
	//            SetCellFormat(1, 1, 11, 0, 0, 0);
	//			for (ifile = StrToFloat(Edit27->Text); ifile < StrToFloat(Edit29->Text)+1; ifile++)
	//			{
	//				Chart1->Series[5]->AddXY(ifile,AprocRezX[i].C+AprocRezX[i].B*ifile+AprocRezX[i].A*ifile*ifile,"", RGB (48,100,169));
	//				Chart1->Series[6]->AddXY(ifile,AprocRezY[i].C+AprocRezY[i].B*ifile+AprocRezY[i].A*ifile*ifile,"", RGB (191,43,69));
	//			}

				for (ifile = 0; ifile < Meter; ifile++)
				{
					if ( ((FullSigma[ifile].x + PorogPx) >= BoxXY[i].x) && ((FullSigma[ifile].x - PorogPx) <= BoxXY[i].x) &&
						 ((FullSigma[ifile].y + PorogPx) >= BoxXY[i].y) && ((FullSigma[ifile].y - PorogPx) <= BoxXY[i].y))
					{
						for (j = 0; j < NomBoxMS; j++)
						if (FullSigma[ifile].ms < (BoxMS[j].ms+1) && FullSigma[ifile].ms > (BoxMS[j].ms-1))
						{

							ConverMM = (FullSigma[ifile].mm - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text));
							Chart1->Series[9]->AddXY(FullSigma[ifile].mm,FullSigma[ifile].Nframe, FloatToStr(FixRoundTo(FullSigma[ifile].mm,-3))+"\r\n"+FloatToStr(FixRoundTo(ConverMM,-3)), BoxMS[j].ColGraf);

	//						PointSeries[0]->AddXY(FullSigma[ifile].mm,FullSigma[ifile].SigX, FullSigma[ifile].mm, (int)FullSigma[ifile].ms*300);
	//						PointSeries[0]->AddXY(FullSigma[ifile].mm,FullSigma[ifile].SigY, FullSigma[ifile].mm, (int)FullSigma[ifile].ms*10);
	//						Chart1->Series[5]->AddXY(FullSigma[ifile].mm,AprocRezX[i].C+AprocRezX[i].B*FullSigma[ifile].mm+AprocRezX[i].A*FullSigma[ifile].mm*FullSigma[ifile].mm, FloatToStr(FullSigma[ifile].mm)+"\r\n"+FloatToStr(RoundTo(ConverMM,-3)), RGB (48,100,169));
	//						Chart1->Series[6]->AddXY(FullSigma[ifile].mm,AprocRezY[i].C+AprocRezY[i].B*FullSigma[ifile].mm+AprocRezY[i].A*FullSigma[ifile].mm*FullSigma[ifile].mm, FloatToStr(FullSigma[ifile].mm)+"\r\n"+FloatToStr(RoundTo(ConverMM,-3)), RGB (191,43,69));
						}
					}
					ProgressBar1->Position++;
					StatusBar1->Panels->Items[0]->Text = IntToStr(ProgressBar1->Position) + " / " + IntToStr(ProgressBar1->Max);
				}

				Chart1->Axes->Bottom->Title->Caption = "\r\nfк (df)";
				Chart1->Axes->Left->Title->Caption = "N_frame";
	//			Chart1->Axes->Bottom->Items->Format->Font->Style << fsBold;
				//Значения по осям max min
				// Bottom - X
				if (CheckBox6->Checked == false && Edit29->Text != "" && Edit27->Text != "")
				{
					Chart1->Axes->Bottom->AutomaticMaximum = false;
					Chart1->Axes->Bottom->AutomaticMinimum = false;
					Chart1->Axes->Bottom->Minimum = -10000;
					Chart1->Axes->Bottom->Maximum = StrToFloat(Edit29->Text);
					Chart1->Axes->Bottom->Minimum = StrToFloat(Edit27->Text);
				}
				else
				{
					Chart1->Axes->Bottom->AutomaticMaximum = FullSigma[ifile-1].mm;
					Chart1->Axes->Bottom->AutomaticMinimum = FullSigma[0].mm;
				}

				if (CheckBoxNF->Checked == false && Edit29->Text != "" && Edit27->Text != "")
				{
					Chart1->Axes->Left->AutomaticMaximum = false;
					Chart1->Axes->Left->AutomaticMinimum = false;
					Chart1->Axes->Left->Minimum = -10000;
					Chart1->Axes->Left->Maximum = StrToFloat(EditNFmax->Text);
					Chart1->Axes->Left->Minimum = StrToFloat(EditNFmin->Text);
				}
				else
				{
					Chart1->Axes->Left->AutomaticMaximum = false;
					Chart1->Axes->Left->AutomaticMinimum = false;
					Chart1->Axes->Left->Minimum = -10000;
					Chart1->Axes->Left->Maximum = NframeMAX+5;
					Chart1->Axes->Left->Minimum = NframeMIN-5;
	//				Chart1->Axes->Left->AutomaticMaximum = true;
	//				Chart1->Axes->Left->AutomaticMinimum = true;
				}

				if (CheckBoxNF->Checked == false && Edit29->Text != "" && Edit27->Text != "")
				{
					int IncrementY = (int)((StrToFloat(EditNFmax->Text) - StrToFloat(EditNFmin->Text)) / 16 + 0.5);
					if (IncrementY % 2 != 0)
						IncrementY++;

					Chart1->Axes->Left->Increment = IncrementY;
				}


				if (CheckBox12->Checked == false)
				{
					Chart1->Axes->Bottom->Increment = StrToFloat(Edit38->Text);

	//				if (StrToFloat(Edit38->Text) != 0)
	//				{
	//					float nom1 = 0;
	//					for (int nom = 0; nom < (1000/StrToFloat(Edit38->Text)); nom++)
	//					{
	//						nom1 = nom1 + StrToFloat(Edit38->Text);
	//						Chart1->Series[7]->AddX(nom1,FloatToStr(RoundTo(nom1,-3)));
	//					}
	//					nom1 = 0;
	//					for (int nom = 0; nom < (1000/StrToFloat(Edit38->Text)); nom++)
	//					{
	//						nom1 = nom1 - StrToFloat(Edit38->Text);
	//						Chart1->Series[7]->AddX(nom1,FloatToStr(RoundTo(nom1,-3)));
	//					}
	//				}
				}

				Chart1->Title->Text->Clear();
				Chart1->Refresh();
				Application->ProcessMessages();
				Chart1->Title->Text->Text = AnsiString(FloatToStr(((int)(BoxXY[i].x*1000))/1000) + "x"+ FloatToStr(((int)(BoxXY[i].y*1000))/1000) + "   " + Edit32->Text + " №" + Edit33->Text + "  ("+ buffDate +")").c_str();
				Chart1->PaintTo(bm->Canvas->Handle,0,0);
				NameGraf = (Edit23->Text+"\\Graf-Nframe_"+IntToStr(i+1)+"_"+FloatToStr(((int)(BoxXY[i].x*1000))/1000) + "x"+ FloatToStr(((int)(BoxXY[i].y*1000))/1000)+ " " + bufftime + ".bmp");
				Chart1->SaveToBitmapFile(NameGraf);
				AddPicture(NameGraf);
			}
			//   end of "График по N_frame"

			AddParagraph( "\n");

			SetTextFormat(11, 0, 0, 0, 1, 0.2);
			AddTable(12, 7);
			UnionCell(2, 1, 3, 1);
			UnionCell(2, 1, 4, 1);
			UnionCell(2, 1, 5, 1);
			UnionCell(2, 1, 6, 1);

			UnionCell(7, 1, 8, 1);
			UnionCell(7, 1, 9, 1);
			UnionCell(7, 1, 10, 1);
			UnionCell(7, 1, 11, 1);

			UnionCell(2, 3, 2, 4);
			UnionCell(2, 4, 2, 5);

			UnionCell(7, 3, 7, 4);
			UnionCell(7, 4, 7, 5);

			UnionCell(5, 3, 5, 4);
			UnionCell(5, 4, 5, 5);
			UnionCell(10, 3, 10, 4);
			UnionCell(10, 4, 10, 5);

			UnionCell(12, 1, 12, 2);


			SetCell(1, 3, "fk\n<-" );
			SetCell(1, 4, "fk\n->");
			SetCell(1, 5, "df\n<-");
			SetCell(1, 6, "df\n->");
			SetCell(1, 7, "Sigma");

			SetCell(2, 1, "Sigma X");
			SetCell(7, 1, "Sigma Y");

			SetCell(2, 2, "inf");
			SetCell(3, 2, "верх");
			SetCell(4, 2, "низ");
			SetCell(5, 2, "MIN");
			SetCell(6, 2, "среднее");
			SetCell(7, 2, "inf");
			SetCell(8, 2, "верх");
			SetCell(9, 2, "низ");
			SetCell(10, 2, "MIN");
			SetCell(11, 2, "среднее");
			SetCell(12, 1, "ИТОГ");
			SetCellFormat(6, 5, 11, 1, 0, 0);
			SetCellFormat(6, 6, 11, 1, 0, 0);
			SetCellFormat(11, 5, 11, 1, 0, 0);
			SetCellFormat(11, 6, 11, 1, 0, 0);
			SetCellFormat(12, 4, 11, 1, 0, 0);
			SetCellFormat(12, 5, 11, 1, 0, 0);
			SetCellFormat(2, 5, 11, 1, 0, 0);
			SetCellFormat(7, 5, 11, 1, 0, 0);

			SigmaInfX = FindOneRootOfEquation(InfCol, MNK_deg, AprocRezX[i].K);
			SigmaInfY = FindOneRootOfEquation(InfCol, MNK_deg, AprocRezY[i].K);

			//Бесконечность
			SetCell(2, 3, Edit18->Text);
			SetCell(2, 4, "0");
			SetCell(2, 5, FloatToStr(FixRoundTo( SigmaInfX,-2)) );
			SetCell(7, 3, Edit18->Text);
			SetCell(7, 4, "0");
			SetCell(7, 5, FloatToStr(FixRoundTo( SigmaInfY,-2)) );

			if (AprocRezX[i].PerV2 != 1000) SetCell(3, 3, FloatToStr(FixRoundTo(AprocRezX[i].PerV2,-2)));  else SetCell(3, 3, "-");
			if (AprocRezX[i].PerV1 != -1000) SetCell(3, 4, FloatToStr(FixRoundTo(AprocRezX[i].PerV1,-2)));  else SetCell(3, 4, "-");
			if (AprocRezX[i].PerN2 != 1000) SetCell(4, 3, FloatToStr(FixRoundTo(AprocRezX[i].PerN2,-2)));  else SetCell(4, 3, "-");
			if (AprocRezX[i].PerN1 != -1000) SetCell(4, 4, FloatToStr(FixRoundTo(AprocRezX[i].PerN1,-2)));  else SetCell(4, 4, "-");

			SetCell(5, 3, FloatToStr(FixRoundTo(AprocRezX[i].ExtX,-2)));
			SetCell(5, 5, FloatToStr(FixRoundTo(AprocRezX[i].ExtY,-2)));

			if (AprocRezY[i].PerV2 != 1000) SetCell(8, 3, FloatToStr(FixRoundTo(AprocRezY[i].PerV2,-2)));  else SetCell(8, 3, "-");
			if (AprocRezY[i].PerV1 != -1000) SetCell(8, 4, FloatToStr(FixRoundTo(AprocRezY[i].PerV1,-2)));  else SetCell(8, 4, "-");
			if (AprocRezY[i].PerN2 != 1000) SetCell(9, 3, FloatToStr(FixRoundTo(AprocRezY[i].PerN2,-2)));  else SetCell(9, 3, "-");
			if (AprocRezY[i].PerN1 != -1000) SetCell(9, 4, FloatToStr(FixRoundTo(AprocRezY[i].PerN1,-2)));  else SetCell(9, 4, "-");

			SetCell(10, 3, FloatToStr(FixRoundTo(AprocRezY[i].ExtX,-2)));
			SetCell(10, 5, FloatToStr(FixRoundTo(AprocRezY[i].ExtY,-2)));

			SetCell(3, 7, (Edit21->Text) );
			SetCell(4, 7, (Edit20->Text) );
			SetCell(8, 7, (Edit21->Text) );
			SetCell(9, 7, (Edit20->Text) );

			if (AprocRezX[i].PerV2 != 1000) SetCell(3, 5, FloatToStr(FixRoundTo((AprocRezX[i].PerV2 - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)));  else SetCell(3, 5, "-");
			if (AprocRezX[i].PerV1 != -1000) SetCell(3, 6, FloatToStr(FixRoundTo((AprocRezX[i].PerV1 - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)));  else SetCell(3, 6, "-");
			if (AprocRezX[i].PerN2 != 1000) SetCell(4, 5, FloatToStr(FixRoundTo((AprocRezX[i].PerN2 - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)));  else SetCell(4, 5, "-");
			if (AprocRezX[i].PerN1 != -1000) SetCell(4, 6, FloatToStr(FixRoundTo((AprocRezX[i].PerN1 - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)));  else SetCell(4, 6, "-");

			if (AprocRezY[i].PerV2 != 1000) SetCell(8, 5, FloatToStr(FixRoundTo((AprocRezY[i].PerV2 - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)));  else SetCell(8, 5, "-");
			if (AprocRezY[i].PerV1 != -1000) SetCell(8, 6, FloatToStr(FixRoundTo((AprocRezY[i].PerV1 - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)));  else SetCell(8, 6, "-");
			if (AprocRezY[i].PerN2 != 1000) SetCell(9, 5, FloatToStr(FixRoundTo((AprocRezY[i].PerN2 - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)));  else SetCell(9, 5, "-");
			if (AprocRezY[i].PerN1 != -1000) SetCell(9, 6, FloatToStr(FixRoundTo((AprocRezY[i].PerN1 - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)));  else SetCell(9, 6, "-");

			SetCell(5,  4, FloatToStr(FixRoundTo((AprocRezX[i].ExtX - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)));
			SetCell(10, 4, FloatToStr(FixRoundTo((AprocRezY[i].ExtX - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)));

			SigmaInfX = FindOneRootOfEquation(AprocRezX[i].RezMeanL, MNK_deg, AprocRezX[i].K);
			if (AprocRezX[i].RezMeanL != 1000) SetCell(6, 3, FloatToStr(FixRoundTo( AprocRezX[i].RezMeanL,-2)) );                                                                                                                                      else SetCell(6, 3, "-");
			if (AprocRezX[i].RezMeanL != 1000) SetCell(6, 5, FloatToStr(FixRoundTo((AprocRezX[i].RezMeanL - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)) );  else SetCell(6, 5, "-");
			if (AprocRezX[i].RezMeanL != 1000) SetCell(6, 7, FloatToStr(FixRoundTo( SigmaInfX,-2)) );                                          else SetCell(6, 7, "-");

			SigmaInfX = FindOneRootOfEquation(AprocRezX[i].RezMeanR, MNK_deg, AprocRezX[i].K);
			if (AprocRezX[i].RezMeanR != 1000) SetCell(6, 4, FloatToStr(FixRoundTo( AprocRezX[i].RezMeanR,-2)) );                                                                                                                                      else SetCell(6, 4, "-");
			if (AprocRezX[i].RezMeanR != 1000) SetCell(6, 6, FloatToStr(FixRoundTo((AprocRezX[i].RezMeanR - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)) );  else SetCell(6, 6, "-");
//			SetCell(5, 7, " - " + FloatToStr(RoundTo( SigmaInfX,-3)) );

			SigmaInfY = FindOneRootOfEquation(AprocRezY[i].RezMeanL, MNK_deg, AprocRezY[i].K);
			if (AprocRezY[i].RezMeanL != 1000) SetCell(11, 3, FloatToStr(FixRoundTo( AprocRezY[i].RezMeanL,-2)) );                                                                                                                                     else SetCell(11, 3, "-");
			if (AprocRezY[i].RezMeanL != 1000) SetCell(11, 5, FloatToStr(FixRoundTo((AprocRezY[i].RezMeanL - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)) ); else SetCell(11, 5, "-");
			if (AprocRezY[i].RezMeanL != 1000) SetCell(11, 7, FloatToStr(FixRoundTo( SigmaInfY,-2)) );                                         else SetCell(11, 7, "-");

			SigmaInfY = FindOneRootOfEquation(AprocRezY[i].RezMeanR, MNK_deg, AprocRezY[i].K);
			if (AprocRezY[i].RezMeanR != 1000) SetCell(11, 4, FloatToStr(FixRoundTo( AprocRezY[i].RezMeanR,-2)) );                                                                                                                                     else SetCell(11, 4, "-");
			if (AprocRezY[i].RezMeanR != 1000) SetCell(11, 6, FloatToStr(FixRoundTo((AprocRezY[i].RezMeanR - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)) ); else SetCell(11, 6, "-");
//			SetCell(9, 7, " - " + FloatToStr(RoundTo( SigmaInfY,-3)) );

			if (AprocRezX[i].RezMeanL != 1000) SetCell(12, 2, FloatToStr(FixRoundTo((AprocRezX[i].RezMeanL+AprocRezY[i].RezMeanL)/2,-2))); else SetCell(12, 2, "-");
			if (AprocRezX[i].RezMeanR != 1000) SetCell(12, 3, FloatToStr(FixRoundTo((AprocRezX[i].RezMeanR+AprocRezY[i].RezMeanR)/2,-2))); else SetCell(12, 3, "-");

			if (AprocRezX[i].RezMeanL != 1000) SetCell(12, 4, FloatToStr(FixRoundTo(((AprocRezX[i].RezMeanL+AprocRezY[i].RezMeanL)/2 - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)) ); else SetCell(12, 4, "-");
			if (AprocRezX[i].RezMeanR != 1000) SetCell(12, 5, FloatToStr(FixRoundTo(((AprocRezX[i].RezMeanR+AprocRezY[i].RezMeanR)/2 - StrToFloat(Edit18->Text))*(StrToFloat(Edit45->Text)*StrToFloat(Edit45->Text))/(StrToFloat(Edit46->Text)*StrToFloat(Edit46->Text)),-3)) ); else SetCell(12, 5, "-");



			float Sx1 = FindOneRootOfEquation((AprocRezX[i].RezMeanL+AprocRezY[i].RezMeanL)/2, MNK_deg, AprocRezX[i].K);
			float Sx2 = FindOneRootOfEquation((AprocRezX[i].RezMeanR+AprocRezY[i].RezMeanR)/2, MNK_deg, AprocRezX[i].K);
			float Sy1 = FindOneRootOfEquation((AprocRezX[i].RezMeanL+AprocRezY[i].RezMeanL)/2, MNK_deg, AprocRezY[i].K);
			float Sy2 = FindOneRootOfEquation((AprocRezX[i].RezMeanR+AprocRezY[i].RezMeanR)/2, MNK_deg, AprocRezY[i].K);

			if (AprocRezX[i].RezMeanL != 1000) SetCell(12, 6,   "Sx: " + FloatToStr(FixRoundTo(Sx1,-2)) + " / " + FloatToStr(FixRoundTo(Sx2,-2)));  else SetCell(12, 6, "-");
			if (AprocRezX[i].RezMeanL != 1000) SetCell(12, 6, "\nSy: " + FloatToStr(FixRoundTo(Sy1,-2)) + " / " + FloatToStr(FixRoundTo(Sy2,-2)));  else SetCell(12, 6, "-");

//			if (AprocRezX[i].RezMeanL != 1000) SetCell(12, 6,   "Sx: " + FloatToStr(RoundTo( ((AprocRezX[i].RezMeanL+AprocRezY[i].RezMeanL)/2)*((AprocRezX[i].RezMeanL+AprocRezY[i].RezMeanL)/2)*AprocRezX[i].A +AprocRezX[i].B*((AprocRezX[i].RezMeanL+AprocRezY[i].RezMeanL)/2) + AprocRezX[i].C,-2)) + " / " + FloatToStr(RoundTo( ((AprocRezX[i].RezMeanR+AprocRezY[i].RezMeanR)/2)*((AprocRezX[i].RezMeanR+AprocRezY[i].RezMeanR)/2)*AprocRezX[i].A +AprocRezX[i].B*((AprocRezX[i].RezMeanR+AprocRezY[i].RezMeanR)/2) + AprocRezX[i].C,-2)));  else SetCell(12, 6, "-");
//			if (AprocRezX[i].RezMeanL != 1000) SetCell(12, 6, "\nSy: " + FloatToStr(RoundTo( ((AprocRezX[i].RezMeanL+AprocRezY[i].RezMeanL)/2)*((AprocRezX[i].RezMeanL+AprocRezY[i].RezMeanL)/2)*AprocRezY[i].A +AprocRezY[i].B*((AprocRezX[i].RezMeanL+AprocRezY[i].RezMeanL)/2) + AprocRezY[i].C,-2)) + " / " + FloatToStr(RoundTo( ((AprocRezX[i].RezMeanR+AprocRezY[i].RezMeanR)/2)*((AprocRezX[i].RezMeanR+AprocRezY[i].RezMeanR)/2)*AprocRezY[i].A +AprocRezY[i].B*((AprocRezX[i].RezMeanR+AprocRezY[i].RezMeanR)/2) + AprocRezY[i].C,-2)));  else SetCell(12, 6, "-");
//			AddParagraph( "Экстремум по Х: " + FloatToStr(RoundTo(AprocRezX[i].ExtX,-4)) + "  " + FloatToStr(RoundTo(AprocRezX[i].ExtY,-4)) );
//			AddParagraph( "Экстремум по Y: " + FloatToStr(RoundTo(AprocRezY[i].ExtX,-4)) + "  " + FloatToStr(RoundTo(AprocRezY[i].ExtY,-4)) );
//
//			AddParagraph( "Пересечения по Х (верх): " + FloatToStr(RoundTo(AprocRezX[i].PerV1,-4)) + "  " + FloatToStr(RoundTo(AprocRezX[i].PerV2,-4)) );
//			AddParagraph( "Пересечения по Х (низ): "  + FloatToStr(RoundTo(AprocRezX[i].PerN1,-4)) + "  " + FloatToStr(RoundTo(AprocRezX[i].PerN2,-4)) + "\n");
//			AddParagraph( "Пересечения по Y (верх): " + FloatToStr(RoundTo(AprocRezY[i].PerV1,-4)) + "  " + FloatToStr(RoundTo(AprocRezY[i].PerV2,-4)) );
//			AddParagraph( "Пересечения по Y (низ): "  + FloatToStr(RoundTo(AprocRezY[i].PerN1,-4)) + "  " + FloatToStr(RoundTo(AprocRezY[i].PerN2,-4)) );
			AddParagraph( "\f");

//		AddParagraph("График "+IntToStr(i+1) + "    Координаты: " +FloatToStr((float)(((int)(BoxXY[i].x*1000))/1000)) + "x"+ FloatToStr((float)(((int)(BoxXY[i].y*1000))/1000)));
//		bm->SaveToFile(Edit23->Text+"\\BGraf_"+IntToStr(i+1)+".bmp");
//		PointSeries[i]->Active=false;
//		PointSeries[0]->Clear();
//		delete PointSeries[0];

//		Chart1->ClearChart(); //Вывести все настройки в код программы
//		Chart1->Refresh();
//		Chart1->SaveToBitmapFile(Edit23->Text+"\\Graf2_"+IntToStr(i+1)+".bmp");
			//SaveDoc(ReportName);
		}
		//----------------------------------------------------

		//легенда мс (цветной текст с сортировкой от мин к макс)
		SetTextFormat(14, 1, 0, 0, 1, 0.05);
		float MINms;
		for (i = 0; i < NomBoxMS; i++)
		BoxMS[i].mm = BoxMS[i].ms;

		for (j = 0; j < NomBoxMS; j++)
		for (i = 0; i < NomBoxMS; i++)
		if (BoxMS[j].mm < BoxMS[i].mm) {
			MINms = BoxMS[j].mm;
			BoxMS[j].mm = BoxMS[i].mm;
			BoxMS[i].mm = MINms;
		}
		for (i = 0; i < NomBoxMS; i++)
		{
			SetTextColor(BoxMS[i].ColGraf);
			AddParagraph(FloatToStr(FixRoundTo(BoxMS[i].mm,-3)) + " мс");
		}
		SetTextColor((TColor)RGB(0,0,0));

		//INI
		SetTextFormat(8, 0, 0, 0, 1, 0.1);
		AddTable(2, 2);
		UnionCell(1, 1, 1, 2);
		SetCell(1, 1, "INI файл");
		SetCell(2, 1,"[Overall]\nX=" + Edit1->Text +
					 "\nY=" + Edit2->Text +
					 "\nDir=" + Edit3->Text +
					 "\nMask=" + Edit4->Text +
					 "\nCheckChoice=" + BoolToStr(CheckBox1->Checked) +
					 "\nSaveDir=" + Edit23->Text +
					 "\nFormatFrame=" + IntToStr(RadioGroup1->ItemIndex) +

					 "\n[LOC-options]\nKofFilt=" + Edit10->Text +
					 "\nCheckFilt=" + BoolToStr(CheckBox3->Checked) +
					 "\nkofSKO1=" + Edit17->Text +
					 "\npixMin=" + Edit11->Text +
					 "\npixMax=" + Edit12->Text +
					 "\nIsMin=" + Edit13->Text +
					 "\nIsMax=" + Edit14->Text +

					 "\n[Sigma-options]\nframe=" + Edit9->Text +
					 "\nKofBin=" + Edit5->Text +
					 "\nCheckBin=" + BoolToStr(CheckBox2->Checked) +
					 "\nkofSKO2=" + Edit28->Text +
					 "\npixMaxIs=" + Edit15->Text +
					 "\npixMinIs=" + Edit16->Text +
					 "\nCheckMostLight=" + BoolToStr(CheckBox4->Checked) +
					 "\nCheck_Is-pix=" + IntToStr(ComboBoxPixIs->ItemIndex) +
					 "\nforLG_step=" + Edit24->Text +
					 "\nforLG_iter=" + Edit25->Text +
					 "\nforLG_emptyPix=" + Edit26->Text +
					 "\nforLG_MinPix=" + Edit52->Text +
					 "\nCheckSector=" + BoolToStr(CheckBox14->Checked) +

					 "\n[Report-options]\nNameDevice=" + Edit32->Text +
					 "\nNomberDevice=" + Edit33->Text +
					 "\nXmatr=" + Edit34->Text +
					 "\nYmatr=" + Edit43->Text +
					 "\nNameShooter=" + Edit35->Text +
					 "\nDataShoot=" + Edit36->Text +
					 "\nNameReporter=" + Edit37->Text +
					 "\nComment=" + Memo2->Text +

					 "\n[Photogrammetry-options]\nIsStar6=" + Edit_IOZ6->Text +
					 "\nIsStar5=" + Edit_IOZ5->Text +
					 "\nIsStar4=" + Edit_IOZ4->Text +
					 "\nPhotometryCheck=" + BoolToStr(CheckBoxPhotometry->Checked));

		SetCell(2, 2,"[Research-options]\nId1ms=" + Edit8->Text +
					 "\nId2ms=" + Edit6->Text +
					 "\nId1mm=" + Edit22->Text +
					 "\nId2mm=" + Edit7->Text +
					 "\ninfKol=" + Edit18->Text +
					 "\ntochnost=" + Edit19->Text +
					 "\nFocDevice=" + Edit45->Text +
					 "\nFocCol=" + Edit46->Text +
					 "\nPixSize=" + Edit50->Text +
					 "\nBaric=" + EditBaric->Text +
					 "\nBaricCheck=" + BoolToStr(CheckBoxBaric->Checked) +
					 "\nSlopeParam=" + IntToStr(ComboBoxMainPoint->ItemIndex) +

					 "\n[Graphic-options]\nminXGraf=" + Edit27->Text +
					 "\nmaxXGraf=" + Edit29->Text +
					 "\nCheckGrafX=" + BoolToStr(CheckBox6->Checked) +
					 "\nminYGraf=" + Edit30->Text +
					 "\nmaxYGraf=" + Edit31->Text +
					 "\nCheckGrafY=" + BoolToStr(CheckBox7->Checked) +
					 "\nHeightGraf=" + Edit40->Text +
					 "\nWidthGraf=" + Edit41->Text +
					 "\nStep=" + Edit53->Text +
					 "\nStepGrafX=" + Edit38->Text +
					 "\nStepGrafY=" + Edit39->Text +
					 "\nCheckStep=" + BoolToStr(CheckBox12->Checked) +
					 "\nSizeBubble=" + Edit44->Text +
					 "\nFixSizeBubble=" + BoolToStr(CheckBox11->Checked) +
					 "\nBrGist=" + Edit56->Text +
					 "\nCheckBrGist=" + BoolToStr(CheckBox16->Checked) +

					 "\noptSigma1=" + Edit20->Text +
					 "\noptSigma2=" + Edit21->Text +
					 "\ndopSigma1=" + Edit47->Text +
					 "\ndopSigma2=" + Edit48->Text +
					 "\nCheckDopSigma=" + BoolToStr(CheckBox9->Checked) +
					 "\nMu=" + Edit49->Text +
					 "\nCheckMu=" + BoolToStr(CheckBox10->Checked) +
					 "\nDeviationCenter=" + Edit55->Text +
					 "\nCheckDC=" + BoolToStr(CheckBox15->Checked) +
					 "\nSizeFrag=" + Edit54->Text +
					 "\nNFrameMAX=" + EditNFmax->Text +
					 "\nNFrameMIN=" + EditNFmin->Text +
					 "\nCheckNF=" + BoolToStr(CheckBoxNF->Checked) +
					 "\nPolinomDegree=" + EditMNK->Text +
					 "\nCheckNFgraf=" + BoolToStr(CheckBoxNframe->Checked) +

					 "\n[Sky-options]\nCheckSkyGraf=" + BoolToStr(CheckBox5->Checked) +
					 "\nMinStarsInSector=" + Edit51->Text);

//		AddParagraph("[Overall]\nX=" + Edit1->Text +
//					 "\nY=" + Edit2->Text +
//					 "\nDir=" + Edit3->Text +
//					 "\nMask=" + Edit4->Text +
//					 "\nCheckChoice=" + BoolToStr(CheckBox1->Checked) +
//					 "\nSaveDir=" + Edit23->Text);
//
//		AddParagraph("[LOC-options]\nKofFilt=" + Edit10->Text +
//					 "\nCheckFilt=" + BoolToStr(CheckBox3->Checked) +
//					 "\nkofSKO1=" + Edit17->Text +
//					 "\npixMin=" + Edit11->Text +
//					 "\npixMax=" + Edit12->Text +
//					 "\nIsMin=" + Edit13->Text +
//					 "\nIsMax=" + Edit14->Text);
//
//		AddParagraph("[LOC-options]\nKofFilt=" + Edit10->Text +
//					 "\nCheckFilt=" + BoolToStr(CheckBox3->Checked) +
//					 "\nkofSKO1=" + Edit17->Text +
//					 "\npixMin=" + Edit11->Text +
//					 "\npixMax=" + Edit12->Text +
//					 "\nIsMin=" + Edit13->Text +
//					 "\nIsMax=" + Edit14->Text);
//
//		AddParagraph("[Sigma-options]\nframe=" + Edit9->Text +
//					 "\nKofBin=" + Edit5->Text +
//					 "\nCheckBin=" + BoolToStr(CheckBox2->Checked) +
//					 "\nkofSKO2=" + Edit28->Text +
//					 "\npixMaxIs=" + Edit15->Text +
//					 "\npixMinIs=" + Edit16->Text +
//					 "\nCheckMostLight=" + BoolToStr(CheckBox4->Checked) +
//					 "\nforLG_step=" + Edit24->Text +
//					 "\nforLG_iter=" + Edit25->Text +
//					 "\nforLG_emptyPix=" + Edit26->Text);
//
//		AddParagraph("[Report-options]\nNameDevice=" + Edit32->Text +
//					 "\nNomberDevice=" + Edit33->Text +
//					 "\nXmatr=" + Edit34->Text +
//					 "\nYmatr=" + Edit43->Text +
//					 "\nNameShooter=" + Edit35->Text +
//					 "\nDataShoot=" + Edit36->Text +
//					 "\nNameReporter=" + Edit37->Text +
//					 "\nComment=" + Memo2->Text);
//
//		AddParagraph("[Research-options]\nId1ms=" + Edit8->Text +
//					 "\nId2ms=" + Edit6->Text +
//					 "\nId1mm=" + Edit22->Text +
//					 "\nId2mm=" + Edit7->Text +
//					 "\ninfKol=" + Edit18->Text +
//					 "\ntochnost=" + Edit19->Text +
//					 "\nFocDevice=" + Edit45->Text +
//					 "\nFocCol=" + Edit46->Text +
//					 "\nPixSize=" + Edit50->Text);
//
//		AddParagraph("[Graphic-options]\nminXGraf=" + Edit27->Text +
//					 "\nmaxXGraf=" + Edit29->Text +
//					 "\nCheckGrafX=" + BoolToStr(CheckBox6->Checked) +
//					 "\nminYGraf=" + Edit30->Text +
//					 "\nmaxYGraf=" + Edit31->Text +
//					 "\nCheckGrafY=" + BoolToStr(CheckBox7->Checked) +
//					 "\nHeightGraf=" + Edit40->Text +
//					 "\nWidthGraf=" + Edit41->Text +
//					 "\nStepGrafX=" + Edit38->Text +
//					 "\nStepGrafY=" + Edit39->Text +
//					 "\nCheckStep=" + BoolToStr(CheckBox12->Checked) +
//					 "\nSizeBubble=" + Edit44->Text +
//					 "\nFixSizeBubble=" + BoolToStr(CheckBox11->Checked) +
//					 "\noptSigma1=" + Edit20->Text +
//					 "\noptSigma2=" + Edit21->Text +
//					 "\ndopSigma1=" + Edit47->Text +
//					 "\ndopSigma2=" + Edit48->Text +
//					 "\nCheckDopSigma=" + BoolToStr(CheckBox9->Checked) +
//					 "\nMu=" + Edit49->Text +
//					 "\nCheckMu=" + BoolToStr(CheckBox10->Checked));
//
//		AddParagraph("[Sky-options]\nCheckSkyGraf=" + BoolToStr(CheckBox5->Checked) + "\nMinStarsInSector=" + Edit51->Text);

		SaveDoc(ReportName);
		CloseDoc();
		CloseWord();
		N3->Enabled = true;
		delete [] AprocRezX;
		delete [] AprocRezY;
		AprocRezX = NULL;
		AprocRezY = NULL;

		delete [] BoxXY;
		delete [] BoxMS;
		delete [] BoxMM;
		BoxXY = NULL;
		BoxMS = NULL;
		BoxMM = NULL;
		}

		delete bm2;
		delete bm;
		delete [] FullSigma;

		bm2 = NULL;
		bm = NULL;
		FullSigma = NULL;

		FlagClose2 = false;

	Form1->Memo1->Lines->Add("Done");
	}
	else ShowMessage("Файлы не выбраны");
	}
	else    ShowMessage("Заполните все ячейки!");
	}
	else    ShowMessage("Ошибочно заданы параметры сигмы ( min > max )");
	}
	else    ShowMessage("Папка для сохранения результатов не существует!");
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button5MouseEnter(TObject *Sender)
{
	Edit1 -> Color = clInfoBk;
	Edit2 -> Color = clInfoBk;
	Edit9 -> Color = clInfoBk;
	Edit23 -> Color = clInfoBk;
	Edit15 -> Color = clInfoBk;
	Edit16 -> Color = clInfoBk;
	Edit28 -> Color = clInfoBk;
	Edit24 -> Color = clInfoBk;
	Edit25 -> Color = clInfoBk;
	Edit26 -> Color = clInfoBk;
	Edit52 -> Color = clInfoBk;

	Label60->Font->Color = (TColor)RGB(229,190,1);
	LabelBFstate->Font->Color = (TColor)RGB(229,190,1);

	if (CheckBox2->Checked) Edit5 -> Color = clInfoBk;
	if (CheckBox4->Checked) ComboBoxPixIs -> Color = clInfoBk;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button5MouseLeave(TObject *Sender)
{
	Edit1 -> Color = clWindow;
	Edit2 -> Color = clWindow;
	Edit9 -> Color = clWindow;
	Edit23 -> Color = clWindow;
	Edit15 -> Color = clWindow;
	Edit16 -> Color = clWindow;
	Edit28 -> Color = clWindow;
	Edit24 -> Color = clWindow;
	Edit25 -> Color = clWindow;
	Edit26 -> Color = clWindow;
	Edit52 -> Color = clWindow;

	Label60->Font->Color = clWindowText;
	LabelBFstate->Font->Color = clWindowText;

	if (CheckBox2->Checked) Edit5 -> Color = clWindow;
	if (CheckBox4->Checked) ComboBoxPixIs -> Color = clWindow;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button6MouseEnter(TObject *Sender)
{

	Edit19 -> Color = clInfoBk;
	Edit23 -> Color = clInfoBk;
	Edit32 -> Color = clInfoBk;
	Edit33 -> Color = clInfoBk;
	Edit34 -> Color = clInfoBk;
	Edit35 -> Color = clInfoBk;
	Edit36 -> Color = clInfoBk;
	Edit37 -> Color = clInfoBk;
	Edit38 -> Color = clInfoBk;

	Edit20 -> Color = clInfoBk;
	Edit21 -> Color = clInfoBk;
	Edit39 -> Color = clInfoBk;
	Edit40 -> Color = clInfoBk;
	Edit41 -> Color = clInfoBk;
	Edit42 -> Color = clInfoBk;
	Edit43 -> Color = clInfoBk;

	Edit44 -> Color = clInfoBk;
	Edit45 -> Color = clInfoBk;
	Edit50 -> Color = clInfoBk;

	Edit_IOZ4 -> Color = clInfoBk;
	Edit_IOZ5 -> Color = clInfoBk;
	Edit_IOZ6 -> Color = clInfoBk;
	Edit53 -> Color = clInfoBk;

	Memo2 -> Color = clInfoBk;

	if (CheckBox5->Checked == false)
	{
		Edit46 -> Color = clInfoBk;
		Edit6  -> Color = clInfoBk;
		Edit7  -> Color = clInfoBk;
		Edit8  -> Color = clInfoBk;
		Edit22 -> Color = clInfoBk;
		Edit18 -> Color = clInfoBk;
		ComboBoxMainPoint -> Color = clInfoBk;
	}

	if (CheckBox9->Checked == false)
	{
		Edit47 -> Color = clInfoBk;
		Edit48 -> Color = clInfoBk;
	}

	if (CheckBox10->Checked == false)
		Edit49 -> Color = clInfoBk;

	if (CheckBox15->Checked == false) {
		Edit54 -> Color = clInfoBk;
		Edit55 -> Color = clInfoBk;   }

	if (CheckBox5->Checked == true)
	{
		Edit51 -> Color = clInfoBk;
	}
	else
	{
		if (CheckBoxNF->Checked == false)
		{
			EditNFmin -> Color = clInfoBk;
			EditNFmax -> Color = clInfoBk;
		}
		if (CheckBox6->Checked == false)
		{
			Edit27 -> Color = clInfoBk;
			Edit29 -> Color = clInfoBk;
		}
		if (CheckBox7->Checked == false)
		{
			Edit30 -> Color = clInfoBk;
			Edit31 -> Color = clInfoBk;
		}
		EditMNK -> Color = clInfoBk;
	}

	if (CheckBoxBaric->Checked == true)
		EditBaric -> Color = clInfoBk;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button6MouseLeave(TObject *Sender)
{
	Edit19 -> Color = clWindow;
	Edit23 -> Color = clWindow;

	Edit32 -> Color = clWindow;
	Edit33 -> Color = clWindow;
	Edit34 -> Color = clWindow;
	Edit35 -> Color = clWindow;
	Edit36 -> Color = clWindow;
	Edit37 -> Color = clWindow;
	Edit38 -> Color = clWindow;

	Edit20 -> Color = clWindow;
	Edit21 -> Color = clWindow;
	Edit39 -> Color = clWindow;
	Edit40 -> Color = clWindow;
	Edit41 -> Color = clWindow;
	Edit42 -> Color = clWindow;
	Edit43 -> Color = clWindow;

	Edit44 -> Color = clWindow;
	Edit45 -> Color = clWindow;
	Edit50 -> Color = clWindow;

	Edit_IOZ4 -> Color = clWindow;
	Edit_IOZ5 -> Color = clWindow;
	Edit_IOZ6 -> Color = clWindow;
	Edit53 -> Color = clWindow;

	Memo2 -> Color = clWindow;

	if (CheckBox5->Checked == false)
	{
		Edit46 -> Color = clWindow;
		Edit6  -> Color = clWindow;
		Edit7  -> Color = clWindow;
		Edit8  -> Color = clWindow;
		Edit22 -> Color = clWindow;
		Edit18 -> Color = clWindow;
		ComboBoxMainPoint -> Color = clWindow;
	}

	if (CheckBox9->Checked == false)
	{
		Edit47 -> Color = clWindow;
		Edit48 -> Color = clWindow;
	}

	if (CheckBox10->Checked == false)
		Edit49 -> Color = clWindow;

	if (CheckBox15->Checked == false)  {
		Edit54 -> Color = clWindow;
		Edit55 -> Color = clWindow;    }

	if (CheckBox5->Checked == true)
	{
		Edit51 -> Color = clWindow;
	}
	else
	{
		if (CheckBoxNF->Checked == false)
		{
			EditNFmin -> Color = clWindow;
			EditNFmax -> Color = clWindow;
		}
		if (CheckBox6->Checked == false)
		{
			Edit27 -> Color = clWindow;
			Edit29 -> Color = clWindow;
		}
		if (CheckBox7->Checked == false)
		{
			Edit30 -> Color = clWindow;
			Edit31 -> Color = clWindow;
		}
		EditMNK -> Color = clWindow;
    }

	if (CheckBoxBaric->Checked == true)
		EditBaric -> Color = clWindow;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button2Click(TObject *Sender)
{
//	if (DirectoryListBox2->Visible == true) {
//		DirectoryListBox2->Visible = false;
//		Button8->Visible = false;
//		Button3->Caption = "Обзор...";
//	}
//	if (DirectoryListBox1->Visible == false) {
//		DirectoryListBox1->Visible = true;
//		Button2->Caption = "Закрыть";
//		Button7->Visible = true;
//	}
//	else
//	{
//		DirectoryListBox1->Visible = false;
//		Button7->Visible = false;
//		Button2->Caption = "Обзор...";
//	}

	if (FileOpenDialog1->Execute())
	{
		Edit3->Text = FileOpenDialog1->FileName;
	}
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button3Click(TObject *Sender)
{
//	if (DirectoryListBox1->Visible == true) {
//		DirectoryListBox1->Visible = false;
//		Button7->Visible = false;
//		Button2->Caption = "Обзор...";
//	}
//	if (DirectoryListBox2->Visible == false) {
//		DirectoryListBox2->Visible = true;
//		Button3->Caption = "Закрыть";
//		Button8->Visible = true;
//	}
//	else
//	{
//		DirectoryListBox2->Visible = false;
//		Button8->Visible = false;
//		Button3->Caption = "Обзор...";
//	}
	if (FileOpenDialog2->Execute())
	{
		Edit23->Text = FileOpenDialog2->FileName;
	}
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button7Click(TObject *Sender)
{
	DirectoryListBox1->Visible = false;
	Button7->Visible = false;
	Button2->Caption = "Обзор...";
	Edit3->Text = DirectoryListBox1->Directory;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button8Click(TObject *Sender)
{
	DirectoryListBox2->Visible = false;
	Button8->Visible = false;
	Button3->Caption = "Обзор...";
	Edit23->Text = DirectoryListBox2->Directory;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::IniSave(String dir)
{
	TIniFile *iniS = new TIniFile(dir+".ini");

	iniS->WriteString("Overall","X",Edit1->Text);
	iniS->WriteString("Overall","Y",Edit2->Text);
	iniS->WriteString("Overall","Dir",Edit3->Text);
	iniS->WriteString("Overall","Mask",Edit4->Text);
	iniS->WriteBool  ("Overall","CheckChoice",CheckBox1->Checked);
	iniS->WriteString("Overall","SaveDir",Edit23->Text);
	iniS->WriteInteger("Overall","FormatFrame",RadioGroup1->ItemIndex);

	iniS->WriteString("LOC-options","KofFilt",Edit10->Text);
	iniS->WriteBool  ("LOC-options","CheckFilt",CheckBox3->Checked);
	iniS->WriteString("LOC-options","kofSKO1",Edit17->Text);
	iniS->WriteString("LOC-options","pixMin",Edit11->Text);
	iniS->WriteString("LOC-options","pixMax",Edit12->Text);
	iniS->WriteString("LOC-options","IsMin",Edit13->Text);
	iniS->WriteString("LOC-options","IsMax",Edit14->Text);

	iniS->WriteString("Sigma-options","frame",Edit9->Text);
	iniS->WriteString("Sigma-options","KofBin",Edit5->Text);
	iniS->WriteBool  ("Sigma-options","CheckBin",CheckBox2->Checked);
	iniS->WriteString("Sigma-options","kofSKO2",Edit28->Text);
	iniS->WriteString("Sigma-options","pixMaxIs",Edit15->Text);
	iniS->WriteString("Sigma-options","pixMinIs",Edit16->Text);
	iniS->WriteBool  ("Sigma-options","CheckMostLight",CheckBox4->Checked);
	iniS->WriteInteger("Sigma-options","Check_Is-pix",ComboBoxPixIs->ItemIndex);
	iniS->WriteString("Sigma-options","forLG_step",Edit24->Text);
	iniS->WriteString("Sigma-options","forLG_iter",Edit25->Text);
	iniS->WriteString("Sigma-options","forLG_emptyPix",Edit26->Text);
	iniS->WriteString("Sigma-options","forLG_MinPix",Edit52->Text);
	iniS->WriteBool  ("Sigma-options","CheckSector",CheckBox14->Checked);

	iniS->WriteString("Report-options","NameDevice",Edit32->Text);
	iniS->WriteString("Report-options","NomberDevice",Edit33->Text);
	iniS->WriteString("Report-options","Xmatr",Edit34->Text);
	iniS->WriteString("Report-options","Ymatr",Edit43->Text);
	iniS->WriteString("Report-options","NameShooter",Edit35->Text);
	iniS->WriteString("Report-options","DataShoot",Edit36->Text);
	iniS->WriteString("Report-options","NameReporter",Edit37->Text);
	iniS->WriteString("Report-options","Comment",Memo2->Text);

	iniS->WriteString("Photogrammetry-options","IsStar6",Edit_IOZ6->Text);
	iniS->WriteString("Photogrammetry-options","IsStar5",Edit_IOZ5->Text);
	iniS->WriteString("Photogrammetry-options","IsStar4",Edit_IOZ4->Text);
	iniS->WriteBool  ("Photogrammetry-options","PhotometryCheck",CheckBoxPhotometry->Checked);

	iniS->WriteString("Research-options","Id1ms",Edit8->Text);
	iniS->WriteString("Research-options","Id2ms",Edit6->Text);
	iniS->WriteString("Research-options","Id1mm",Edit22->Text);
	iniS->WriteString("Research-options","Id2mm",Edit7->Text);
	iniS->WriteString("Research-options","infKol",Edit18->Text);
	iniS->WriteString("Research-options","tochnost",Edit19->Text);
	iniS->WriteString("Research-options","FocDevice",Edit45->Text);
	iniS->WriteString("Research-options","FocCol",Edit46->Text);
	iniS->WriteString("Research-options","PixSize",Edit50->Text);
	iniS->WriteString("Research-options","Baric",EditBaric->Text);
	iniS->WriteBool  ("Research-options","BaricCheck",CheckBoxBaric->Checked);
	iniS->WriteInteger("Research-options","SlopeParam",ComboBoxMainPoint->ItemIndex);

	iniS->WriteString("Graphic-options","minXGraf",Edit27->Text);
	iniS->WriteString("Graphic-options","maxXGraf",Edit29->Text);
	iniS->WriteBool  ("Graphic-options","CheckGrafX",CheckBox6->Checked);
	iniS->WriteString("Graphic-options","minYGraf",Edit30->Text);
	iniS->WriteString("Graphic-options","maxYGraf",Edit31->Text);
	iniS->WriteBool  ("Graphic-options","CheckGrafY",CheckBox7->Checked);
	iniS->WriteString("Graphic-options","HeightGraf",Edit40->Text);
	iniS->WriteString("Graphic-options","WidthGraf",Edit41->Text);
	iniS->WriteString("Graphic-options","Step",Edit53->Text);
	iniS->WriteString("Graphic-options","StepGrafX",Edit38->Text);
	iniS->WriteString("Graphic-options","StepGrafY",Edit39->Text);
	iniS->WriteBool  ("Graphic-options","CheckStep",CheckBox12->Checked);
	iniS->WriteString("Graphic-options","SizeBubble",Edit44->Text);
	iniS->WriteBool  ("Graphic-options","FixSizeBubble",CheckBox11->Checked);
	iniS->WriteString("Graphic-options","BrGist",Edit56->Text);
	iniS->WriteBool  ("Graphic-options","CheckBrGist",CheckBox16->Checked);
	iniS->WriteString("Graphic-options","optSigma1",Edit20->Text);
	iniS->WriteString("Graphic-options","optSigma2",Edit21->Text);
	iniS->WriteString("Graphic-options","dopSigma1",Edit47->Text);
	iniS->WriteString("Graphic-options","dopSigma2",Edit48->Text);
	iniS->WriteBool  ("Graphic-options","CheckDopSigma",CheckBox9->Checked);
	iniS->WriteString("Graphic-options","Mu",Edit49->Text);
	iniS->WriteBool  ("Graphic-options","CheckMu",CheckBox10->Checked);
	iniS->WriteString("Graphic-options","DeviationCenter",Edit55->Text);
	iniS->WriteBool  ("Graphic-options","CheckDC",CheckBox15->Checked);
	iniS->WriteString("Graphic-options","SizeFrag",Edit54->Text);
	iniS->WriteString("Graphic-options","NFrameMAX",EditNFmax->Text);
	iniS->WriteString("Graphic-options","NFrameMIN",EditNFmin->Text);
	iniS->WriteBool  ("Graphic-options","CheckNF",CheckBoxNF->Checked);
	iniS->WriteBool  ("Graphic-options","CheckNFgraf",CheckBoxNframe->Checked);
	iniS->WriteString("Graphic-options","PolinomDegree",EditMNK->Text);

	iniS->WriteBool  ("Sky-options","CheckSkyGraf",CheckBox5->Checked);
	iniS->WriteString("Sky-options","MinStarsInSector",Edit51->Text);

	delete iniS; iniS = NULL;

}

void __fastcall TForm1::N2Click(TObject *Sender)
{
	if (Form1->SaveDialog1->Execute())
	{
		IniSave(SaveDialog1->FileName);
	}
	else ShowMessage("Файл ini не выбран");
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ini2Click(TObject *Sender)
{
	if (Form1->OpenDialog5->Execute())  {
		TIniFile *iniR = new TIniFile(OpenDialog5->FileName);

		Edit1->Text=iniR->ReadString("Overall","X","");
		Edit2->Text=iniR->ReadString("Overall","Y","");
		Edit3->Text=iniR->ReadString("Overall","Dir","");
		Edit4->Text=iniR->ReadString("Overall","Mask","");
		CheckBox1->Checked=iniR->ReadBool("Overall","CheckChoice","");
		Edit23->Text=iniR->ReadString("Overall","SaveDir","");
		if (iniR->ValueExists("Overall","FormatFrame"))
			RadioGroup1->ItemIndex = iniR->ReadInteger("Overall","FormatFrame",0);

		Edit10->Text=iniR->ReadString("LOC-options","KofFilt","");
		CheckBox3->Checked=iniR->ReadBool("LOC-options","CheckFilt","");
		Edit17->Text=iniR->ReadString("LOC-options","kofSKO1","");
		Edit11->Text=iniR->ReadString("LOC-options","pixMin","");
		Edit12->Text=iniR->ReadString("LOC-options","pixMax","");
		Edit13->Text=iniR->ReadString("LOC-options","IsMin","");
		Edit14->Text=iniR->ReadString("LOC-options","IsMax","");

		Edit9->Text=iniR->ReadString("Sigma-options","frame","");
		Edit5->Text=iniR->ReadString("Sigma-options","KofBin","");
		CheckBox2->Checked=iniR->ReadBool  ("Sigma-options","CheckBin","");
		Edit28->Text=iniR->ReadString("Sigma-options","kofSKO2","");
		Edit15->Text=iniR->ReadString("Sigma-options","pixMaxIs","");
		Edit16->Text=iniR->ReadString("Sigma-options","pixMinIs","");
		CheckBox4->Checked=iniR->ReadBool("Sigma-options","CheckMostLight","");
		if (iniR->ValueExists("Sigma-options","Check_Is-pix"))
			ComboBoxPixIs->ItemIndex = iniR->ReadInteger("Sigma-options","Check_Is-pix",0);
		Edit24->Text=iniR->ReadString("Sigma-options","forLG_step","");
		Edit25->Text=iniR->ReadString("Sigma-options","forLG_iter","");
		Edit52->Text=iniR->ReadString("Sigma-options","forLG_MinPix","");
        Edit25->Text=iniR->ReadString("Sigma-options","forLG_iter","");
		CheckBox14->Checked=iniR->ReadBool("Sigma-options","CheckSector","");

		Edit32->Text=iniR->ReadString("Report-options","NameDevice","");
		Edit33->Text=iniR->ReadString("Report-options","NomberDevice","");
		Edit34->Text=iniR->ReadString("Report-options","Xmatr","");
		Edit43->Text=iniR->ReadString("Report-options","Ymatr","");
		Edit35->Text=iniR->ReadString("Report-options","NameShooter","");
		Edit36->Text=iniR->ReadString("Report-options","DataShoot","");
		Edit37->Text=iniR->ReadString("Report-options","NameReporter","");
		Memo2->Text=iniR->ReadString("Report-options","Comment","");

		Edit_IOZ6->Text=iniR->ReadString("Photogrammetry-options","IsStar6","");
		Edit_IOZ5->Text=iniR->ReadString("Photogrammetry-options","IsStar5","");
		Edit_IOZ4->Text=iniR->ReadString("Photogrammetry-options","IsStar4","");
		CheckBoxPhotometry->Checked=iniR->ReadBool("Photogrammetry-options","PhotometryCheck","");

		Edit8->Text=iniR->ReadString("Research-options","Id1ms","");
		Edit6->Text=iniR->ReadString("Research-options","Id2ms","");
		Edit22->Text=iniR->ReadString("Research-options","Id1mm","");
		Edit7->Text=iniR->ReadString("Research-options","Id2mm","");
		Edit18->Text=iniR->ReadString("Research-options","infKol","");
		Edit19->Text=iniR->ReadString("Research-options","tochnost","");
		Edit45->Text=iniR->ReadString("Research-options","FocDevice","");
		Edit46->Text=iniR->ReadString("Research-options","FocCol","");
		Edit50->Text=iniR->ReadString("Research-options","PixSize","");
		EditBaric->Text=iniR->ReadString("Research-options","Baric","");
		CheckBoxBaric->Checked = iniR->ReadBool("Research-options","BaricCheck","");
		ComboBoxMainPoint->ItemIndex= iniR->ReadInteger("Research-options","SlopeParam",0);

		Edit27->Text=iniR->ReadString("Graphic-options","minXGraf","");
		Edit29->Text=iniR->ReadString("Graphic-options","maxXGraf","");
		CheckBox6->Checked=iniR->ReadBool  ("Graphic-options","CheckGrafX","");
		Edit30->Text=iniR->ReadString("Graphic-options","minYGraf","");
		Edit31->Text=iniR->ReadString("Graphic-options","maxYGraf","");
		CheckBox7->Checked=iniR->ReadBool  ("Graphic-options","CheckGrafY","");
		Edit40->Text=iniR->ReadString("Graphic-options","HeightGraf","");
		Edit41->Text=iniR->ReadString("Graphic-options","WidthGraf","");
		Edit53->Text=iniR->ReadString("Graphic-options","Step","");
		Edit38->Text=iniR->ReadString("Graphic-options","StepGrafX","");
		Edit39->Text=iniR->ReadString("Graphic-options","StepGrafY","");
		CheckBox12->Checked=iniR->ReadBool  ("Graphic-options","CheckStep","");
		Edit44->Text=iniR->ReadString("Graphic-options","SizeBubble","");
		CheckBox11->Checked=iniR->ReadBool  ("Graphic-options","FixSizeBubble","");
		Edit56->Text=iniR->ReadString("Graphic-options","BrGist","");
		CheckBox16->Checked=iniR->ReadBool  ("Graphic-options","CheckBrGist","");
		Edit20->Text=iniR->ReadString("Graphic-options","optSigma1","");
		Edit21->Text=iniR->ReadString("Graphic-options","optSigma2","");
		Edit47->Text=iniR->ReadString("Graphic-options","dopSigma1","");
		Edit48->Text=iniR->ReadString("Graphic-options","dopSigma2","");
		CheckBox9->Checked=iniR->ReadBool  ("Graphic-options","CheckDopSigma","");
		Edit49->Text=iniR->ReadString("Graphic-options","Mu","");
		CheckBox10->Checked=iniR->ReadBool  ("Graphic-options","CheckMu","");

		Edit55->Text=iniR->ReadString("Graphic-options","DeviationCenter","");
		CheckBox15->Checked=iniR->ReadBool  ("Graphic-options","CheckDC","");
		Edit54->Text=iniR->ReadString("Graphic-options","SizeFrag","");
		EditNFmax->Text=iniR->ReadString("Graphic-options","NFrameMAX","");
		EditNFmin->Text=iniR->ReadString("Graphic-options","NFrameMIN","");
		CheckBoxNF->Checked=iniR->ReadBool  ("Graphic-options","CheckNF","");
		CheckBoxNframe->Checked=iniR->ReadBool  ("Graphic-options","CheckNFgraf","");
		EditMNK->Text = iniR->ReadString("Graphic-options","PolinomDegree","");

		CheckBox5->Checked=iniR->ReadBool  ("Sky-options","CheckSkyGraf","");
		Edit51->Text=iniR->ReadString("Sky-options","MinStarsInSector","");


//		Edit1->Text=iniR->ReadString("Edits","Edit1-X","");
//		Edit2->Text=iniR->ReadString("Edits","Edit2-Y","");
//		Edit3->Text=iniR->ReadString("Edits","Edit3-Dir","");
//		Edit4->Text=iniR->ReadString("Edits","Edit4-Mask","");
//		Edit5->Text=iniR->ReadString("Edits","Edit5-Bin","");
//		Edit8->Text=iniR->ReadString("Edits","Edit8-Id1ms","");
//		Edit6->Text=iniR->ReadString("Edits","Edit6-Id2ms","");
//		Edit22->Text=iniR->ReadString("Edits","Edit22-Id1mm","");
//		Edit7->Text=iniR->ReadString("Edits","Edit7-Id2mm","");
//
//		Edit9->Text=iniR->ReadString("Edits","Edit9-frame","");
//		Edit10->Text=iniR->ReadString("Edits","Edit10-filt","");
//		Edit11->Text=iniR->ReadString("Edits","Edit11-pixMin","");
//		Edit12->Text=iniR->ReadString("Edits","Edit12-pixMax","");
//		Edit13->Text=iniR->ReadString("Edits","Edit13-IsMin","");
//		Edit14->Text=iniR->ReadString("Edits","Edit14-IsMax","");
//		Edit15->Text=iniR->ReadString("Edits","Edit15-pixMaxIs","");
//		Edit16->Text=iniR->ReadString("Edits","Edit16-pixMinIs","");
//		Edit17->Text=iniR->ReadString("Edits","Edit17-kofSKO1","");
//		Edit18->Text=iniR->ReadString("Edits","Edit18-infKol","");
//		Edit19->Text=iniR->ReadString("Edits","Edit19-tochnost","");
//		Edit20->Text=iniR->ReadString("Edits","Edit20-optSigma1","");
//		Edit21->Text=iniR->ReadString("Edits","Edit21-optSigma2","");
//
//		Edit23->Text=iniR->ReadString("Edits","Edit23-SaveDir","");
//		Edit24->Text=iniR->ReadString("Edits","Edit24-forLG_step","");
//		Edit25->Text=iniR->ReadString("Edits","Edit25-forLG_iter","");
//		Edit26->Text=iniR->ReadString("Edits","Edit26-emptyPix","");
//		Edit27->Text=iniR->ReadString("Edits","Edit27-maxYGraf","");
//		Edit28->Text=iniR->ReadString("Edits","Edit28-kofSKO2","");
//
//		Edit29->Text=iniR->ReadString("Edits","Edit29","");
//		Edit30->Text=iniR->ReadString("Edits","Edit30","");
//		Edit31->Text=iniR->ReadString("Edits","Edit31","");
//		Edit32->Text=iniR->ReadString("Edits","Edit32","");
//		Edit33->Text=iniR->ReadString("Edits","Edit33","");
//		Edit34->Text=iniR->ReadString("Edits","Edit34","");
//		Edit35->Text=iniR->ReadString("Edits","Edit35","");
//		Edit36->Text=iniR->ReadString("Edits","Edit36","");
//		Edit37->Text=iniR->ReadString("Edits","Edit37","");
//		Edit38->Text=iniR->ReadString("Edits","Edit38","");
//		Edit39->Text=iniR->ReadString("Edits","Edit39","");
//		Edit40->Text=iniR->ReadString("Edits","Edit40","");
//		Edit41->Text=iniR->ReadString("Edits","Edit41","");
//		Edit42->Text=iniR->ReadString("Edits","Edit42","");
//		Edit43->Text=iniR->ReadString("Edits","Edit43","");
//		Edit44->Text=iniR->ReadString("Edits","Edit44","");
//		Edit45->Text=iniR->ReadString("Edits","Edit45","");
//		Edit46->Text=iniR->ReadString("Edits","Edit46","");
//
//		Edit47->Text=iniR->ReadString("Edits","Edit47","");
//		Edit48->Text=iniR->ReadString("Edits","Edit48","");
//		Edit49->Text=iniR->ReadString("Edits","Edit49","");
//		Edit50->Text=iniR->ReadString("Edits","Edit50","");
//		Edit51->Text=iniR->ReadString("Edits","Edit51","");
//
//		CheckBox1->Checked=iniR->ReadBool("CheckBoxes","CheckBox1","");
//		CheckBox2->Checked=iniR->ReadBool("CheckBoxes","CheckBox2","");
//		CheckBox3->Checked=iniR->ReadBool("CheckBoxes","CheckBox3","");
//		CheckBox4->Checked=iniR->ReadBool("CheckBoxes","CheckBox4","");
//		CheckBox5->Checked=iniR->ReadBool("CheckBoxes","CheckBox5","");
//		CheckBox6->Checked=iniR->ReadBool("CheckBoxes","CheckBox6","");
//		CheckBox7->Checked=iniR->ReadBool("CheckBoxes","CheckBox7","");
//
//		CheckBox9->Checked=iniR->ReadBool("CheckBoxes","CheckBox9","");
//		CheckBox10->Checked=iniR->ReadBool("CheckBoxes","CheckBox10","");
//
//		Memo2->Text=iniR->ReadString("Memo","Memo2","");
		delete iniR; iniR = NULL;
	}
	else ShowMessage("Файл ini не выбран");
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormClick(TObject *Sender)
{
	if (DirectoryListBox2->Visible == true) {
		DirectoryListBox2->Visible = false;
		Button8->Visible = false;
		Button3->Caption = "Обзор...";
	}
	if (DirectoryListBox1->Visible == true) {
		DirectoryListBox1->Visible = false;
		Button7->Visible = false;
		Button2->Caption = "Обзор...";
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TabSheet2Show(TObject *Sender)
{
	if (DirectoryListBox2->Visible == true) {
		DirectoryListBox2->Visible = false;
		Button8->Visible = false;
		Button3->Caption = "Обзор...";
	}
	if (DirectoryListBox1->Visible == true) {
		DirectoryListBox1->Visible = false;
		Button7->Visible = false;
		Button2->Caption = "Обзор...";
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Memo1Click(TObject *Sender)
{
	if (DirectoryListBox2->Visible == true) {
		DirectoryListBox2->Visible = false;
		Button8->Visible = false;
		Button3->Caption = "Обзор...";
	}
	if (DirectoryListBox1->Visible == true) {
		DirectoryListBox1->Visible = false;
		Button7->Visible = false;
		Button2->Caption = "Обзор...";
	}
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button9Click(TObject *Sender)
{
	Ostanov = true;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::CheckBox6Click(TObject *Sender)
{
	if (CheckBox6 -> Checked)
	{
		Edit27 -> Enabled = false;
		Edit29 -> Enabled = false;
		Edit27 -> Color = cl3DLight;
		Edit29 -> Color = cl3DLight;
	}
	else
	{
		Edit27 -> Enabled = true;
		Edit29 -> Enabled = true;
		Edit27 -> Color = clWindow;
		Edit29 -> Color = clWindow;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBox7Click(TObject *Sender)
{
	if (CheckBox7 -> Checked)
	{
		Edit30 -> Enabled = false;
		Edit31 -> Enabled = false;
		Edit30 -> Color = cl3DLight;
		Edit31 -> Color = cl3DLight;
	}
	else
	{
		Edit30 -> Enabled = true;
		Edit31 -> Enabled = true;
		Edit30 -> Color = clWindow;
		Edit31 -> Color = clWindow;
	}
}
//---------------------------------------------------------------------------



void __fastcall TForm1::Edit1Change(TObject *Sender)
{
	Edit34->Text = Edit1->Text;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit2Change(TObject *Sender)
{
	Edit43->Text = Edit2->Text;
}
//---------------------------------------------------------------------------



void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
	if (FlagClose) {

	ini = new TIniFile(ExtractFilePath(Application->ExeName)+"INI_Sigma.ini");

	ini->WriteString("Overall","X",Edit1->Text);
	ini->WriteString("Overall","Y",Edit2->Text);
	ini->WriteString("Overall","Dir",Edit3->Text);
	ini->WriteString("Overall","Mask",Edit4->Text);
	ini->WriteBool  ("Overall","CheckChoice",CheckBox1->Checked);
	ini->WriteString("Overall","SaveDir",Edit23->Text);
	ini->WriteInteger("Overall","FormatFrame",RadioGroup1->ItemIndex);

	ini->WriteString("LOC-options","KofFilt",Edit10->Text);
	ini->WriteBool  ("LOC-options","CheckFilt",CheckBox3->Checked);
	ini->WriteString("LOC-options","kofSKO1",Edit17->Text);
	ini->WriteString("LOC-options","pixMin",Edit11->Text);
	ini->WriteString("LOC-options","pixMax",Edit12->Text);
	ini->WriteString("LOC-options","IsMin",Edit13->Text);
	ini->WriteString("LOC-options","IsMax",Edit14->Text);

	ini->WriteString("Sigma-options","frame",Edit9->Text);
	ini->WriteString("Sigma-options","KofBin",Edit5->Text);
	ini->WriteBool  ("Sigma-options","CheckBin",CheckBox2->Checked);
	ini->WriteString("Sigma-options","kofSKO2",Edit28->Text);
	ini->WriteString("Sigma-options","pixMaxIs",Edit15->Text);
	ini->WriteString("Sigma-options","pixMinIs",Edit16->Text);
	ini->WriteBool  ("Sigma-options","CheckMostLight",CheckBox4->Checked);
	ini->WriteInteger("Sigma-options","Check_Is-pix",ComboBoxPixIs->ItemIndex);
	ini->WriteString("Sigma-options","forLG_step",Edit24->Text);
	ini->WriteString("Sigma-options","forLG_iter",Edit25->Text);
	ini->WriteString("Sigma-options","forLG_emptyPix",Edit26->Text);
	ini->WriteString("Sigma-options","forLG_MinPix",Edit52->Text);
	ini->WriteBool  ("Sigma-options","CheckSector",CheckBox14->Checked);

	ini->WriteString("Report-options","NameDevice",Edit32->Text);
	ini->WriteString("Report-options","NomberDevice",Edit33->Text);
	ini->WriteString("Report-options","Xmatr",Edit34->Text);
	ini->WriteString("Report-options","Ymatr",Edit43->Text);
	ini->WriteString("Report-options","NameShooter",Edit35->Text);
	ini->WriteString("Report-options","DataShoot",Edit36->Text);
	ini->WriteString("Report-options","NameReporter",Edit37->Text);
	ini->WriteString("Report-options","Comment",Memo2->Text);

	ini->WriteString("Photogrammetry-options","IsStar6",Edit_IOZ6->Text);
	ini->WriteString("Photogrammetry-options","IsStar5",Edit_IOZ5->Text);
	ini->WriteString("Photogrammetry-options","IsStar4",Edit_IOZ4->Text);
	ini->WriteBool  ("Photogrammetry-options","PhotometryCheck",CheckBoxPhotometry->Checked);

	ini->WriteString("Research-options","Id1ms",Edit8->Text);
	ini->WriteString("Research-options","Id2ms",Edit6->Text);
	ini->WriteString("Research-options","Id1mm",Edit22->Text);
	ini->WriteString("Research-options","Id2mm",Edit7->Text);
	ini->WriteString("Research-options","infKol",Edit18->Text);
	ini->WriteString("Research-options","tochnost",Edit19->Text);
	ini->WriteString("Research-options","FocDevice",Edit45->Text);
	ini->WriteString("Research-options","FocCol",Edit46->Text);
	ini->WriteString("Research-options","PixSize",Edit50->Text);
	ini->WriteString("Research-options","Baric",EditBaric->Text);
	ini->WriteBool  ("Research-options","BaricCheck",CheckBoxBaric->Checked);
	ini->WriteInteger("Research-options","SlopeParam",ComboBoxMainPoint->ItemIndex);

	ini->WriteString("Graphic-options","minXGraf",Edit27->Text);
	ini->WriteString("Graphic-options","maxXGraf",Edit29->Text);
	ini->WriteBool  ("Graphic-options","CheckGrafX",CheckBox6->Checked);
	ini->WriteString("Graphic-options","minYGraf",Edit30->Text);
	ini->WriteString("Graphic-options","maxYGraf",Edit31->Text);
	ini->WriteBool  ("Graphic-options","CheckGrafY",CheckBox7->Checked);
	ini->WriteString("Graphic-options","HeightGraf",Edit40->Text);
	ini->WriteString("Graphic-options","WidthGraf",Edit41->Text);
	ini->WriteString("Graphic-options","Step",Edit53->Text);
	ini->WriteString("Graphic-options","StepGrafX",Edit38->Text);
	ini->WriteString("Graphic-options","StepGrafY",Edit39->Text);
	ini->WriteBool  ("Graphic-options","CheckStep",CheckBox12->Checked);
	ini->WriteString("Graphic-options","SizeBubble",Edit44->Text);
	ini->WriteBool  ("Graphic-options","FixSizeBubble",CheckBox11->Checked);
	ini->WriteString("Graphic-options","BrGist",Edit56->Text);
	ini->WriteBool  ("Graphic-options","CheckBrGist",CheckBox16->Checked);
	ini->WriteString("Graphic-options","optSigma1",Edit20->Text);
	ini->WriteString("Graphic-options","optSigma2",Edit21->Text);
	ini->WriteString("Graphic-options","dopSigma1",Edit47->Text);
	ini->WriteString("Graphic-options","dopSigma2",Edit48->Text);
	ini->WriteBool  ("Graphic-options","CheckDopSigma",CheckBox9->Checked);
	ini->WriteString("Graphic-options","Mu",Edit49->Text);
	ini->WriteBool  ("Graphic-options","CheckMu",CheckBox10->Checked);
	ini->WriteString("Graphic-options","DeviationCenter",Edit55->Text);
	ini->WriteBool  ("Graphic-options","CheckDC",CheckBox15->Checked);
	ini->WriteString("Graphic-options","SizeFrag",Edit54->Text);
	ini->WriteString("Graphic-options","NFrameMAX",EditNFmax->Text);
	ini->WriteString("Graphic-options","NFrameMIN",EditNFmin->Text);
	ini->WriteBool  ("Graphic-options","CheckNF",CheckBoxNF->Checked);
	ini->WriteBool  ("Graphic-options","CheckNFgraf",CheckBoxNframe->Checked);
	ini->WriteString("Graphic-options","PolinomDegree",EditMNK->Text);

	ini->WriteBool  ("Sky-options","CheckSkyGraf",CheckBox5->Checked);
	ini->WriteString("Sky-options","MinStarsInSector",Edit51->Text);

//	ini->WriteString("Edits","Edit1-X",Edit1->Text);
//	ini->WriteString("Edits","Edit2-Y",Edit2->Text);
//	ini->WriteString("Edits","Edit3-Dir",Edit3->Text);
//	ini->WriteString("Edits","Edit4-Mask",Edit4->Text);
//	ini->WriteString("Edits","Edit5-Bin",Edit5->Text);
//	ini->WriteString("Edits","Edit8-Id1ms",Edit8->Text);
//	ini->WriteString("Edits","Edit6-Id2ms",Edit6->Text);
//	ini->WriteString("Edits","Edit22-Id1mm",Edit22->Text);
//	ini->WriteString("Edits","Edit7-Id2mm",Edit7->Text);
//
//	ini->WriteString("Edits","Edit9-frame",Edit9->Text);
//	ini->WriteString("Edits","Edit10-filt",Edit10->Text);
//	ini->WriteString("Edits","Edit11-pixMin",Edit11->Text);
//	ini->WriteString("Edits","Edit12-pixMax",Edit12->Text);
//	ini->WriteString("Edits","Edit13-IsMin",Edit13->Text);
//	ini->WriteString("Edits","Edit14-IsMax",Edit14->Text);
//	ini->WriteString("Edits","Edit15-pixMaxIs",Edit15->Text);
//	ini->WriteString("Edits","Edit16-pixMinIs",Edit16->Text);
//	ini->WriteString("Edits","Edit17-kofSKO1",Edit17->Text);
//	ini->WriteString("Edits","Edit18-infKol",Edit18->Text);
//	ini->WriteString("Edits","Edit19-tochnost",Edit19->Text);
//	ini->WriteString("Edits","Edit20-optSigma1",Edit20->Text);
//	ini->WriteString("Edits","Edit21-optSigma2",Edit21->Text);
//
//	ini->WriteString("Edits","Edit23-SaveDir",Edit23->Text);
//	ini->WriteString("Edits","Edit24-forLG_step",Edit24->Text);
//	ini->WriteString("Edits","Edit25-forLG_iter",Edit25->Text);
//	ini->WriteString("Edits","Edit26-emptyPix",Edit26->Text);
//	ini->WriteString("Edits","Edit27-maxYGraf",Edit27->Text);
//	ini->WriteString("Edits","Edit28-kofSKO2",Edit28->Text);
//
//	ini->WriteString("Edits","Edit29",Edit29->Text);
//	ini->WriteString("Edits","Edit30",Edit30->Text);
//	ini->WriteString("Edits","Edit31",Edit31->Text);
//	ini->WriteString("Edits","Edit32",Edit32->Text);
//	ini->WriteString("Edits","Edit33",Edit33->Text);
//	ini->WriteString("Edits","Edit34",Edit34->Text);
//	ini->WriteString("Edits","Edit35",Edit35->Text);
//	ini->WriteString("Edits","Edit36",Edit36->Text);
//	ini->WriteString("Edits","Edit37",Edit37->Text);
//	ini->WriteString("Edits","Edit38",Edit38->Text);
//	ini->WriteString("Edits","Edit39",Edit39->Text);
//	ini->WriteString("Edits","Edit40",Edit40->Text);
//	ini->WriteString("Edits","Edit41",Edit41->Text);
//	ini->WriteString("Edits","Edit42",Edit42->Text);
//	ini->WriteString("Edits","Edit43",Edit43->Text);
//	ini->WriteString("Edits","Edit44",Edit44->Text);
//	ini->WriteString("Edits","Edit45",Edit45->Text);
//	ini->WriteString("Edits","Edit46",Edit46->Text);
//
//	ini->WriteString("Edits","Edit47",Edit47->Text);
//	ini->WriteString("Edits","Edit48",Edit48->Text);
//	ini->WriteString("Edits","Edit49",Edit49->Text);
//	ini->WriteString("Edits","Edit50",Edit50->Text);
//	ini->WriteString("Edits","Edit51",Edit51->Text);
//
//	ini->WriteBool("CheckBoxes","CheckBox1",CheckBox1->Checked);
//	ini->WriteBool("CheckBoxes","CheckBox2",CheckBox2->Checked);
//	ini->WriteBool("CheckBoxes","CheckBox3",CheckBox3->Checked);
//	ini->WriteBool("CheckBoxes","CheckBox4",CheckBox4->Checked);
//	ini->WriteBool("CheckBoxes","CheckBox5",CheckBox5->Checked);
//	ini->WriteBool("CheckBoxes","CheckBox6",CheckBox6->Checked);
//	ini->WriteBool("CheckBoxes","CheckBox7",CheckBox7->Checked);
//
//	ini->WriteBool("CheckBoxes","CheckBox9",CheckBox9->Checked);
//	ini->WriteBool("CheckBoxes","CheckBox10",CheckBox10->Checked);
//
//	ini->WriteString("Memo","Memo2",Memo2->Text);

	delete ini; ini = NULL;
	}

	if (FlagClose2)
	CloseDoc();

	if (BKflag)
	{
		for (int i = 0; i < StrToInt(Form1->Edit2->Text); i++)
		delete [] BlackKadr[i];
		delete [] BlackKadr;

		for (int i = 0; i < StrToInt(Form1->Edit9->Text); i++)
		delete [] BlackKadrWindow[i];
		delete [] BlackKadrWindow;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N3Click(TObject *Sender)
{
	CloseDoc();
	OpenWord(true);
	OpenW(true, ReportName);
	FlagClose2 = true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBox9Click(TObject *Sender)
{
	if (CheckBox9 -> Checked)
	{
		Edit47 -> Enabled = false;
		Edit48 -> Enabled = false;
		Edit47 -> Color = cl3DLight;
		Edit48 -> Color = cl3DLight;
	}
	else
	{
		Edit47 -> Enabled = true;
		Edit48 -> Enabled = true;
		Edit47 -> Color = clWindow;
		Edit48 -> Color = clWindow;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBox10Click(TObject *Sender)
{
	if (CheckBox10 -> Checked)
	{
		Edit49 -> Enabled = false;
		Edit49 -> Color = cl3DLight;
	}
	else
	{
		Edit49 -> Enabled = true;
		Edit49 -> Color = clWindow;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBox5Click(TObject *Sender)
{
	if (CheckBox5 -> Checked)
	{
		Edit51 -> Enabled = true;
		Edit51 -> Color = clWindow;

		Edit6  -> Enabled = false;
		Edit7  -> Enabled = false;
		Edit8  -> Enabled = false;
		Edit18 -> Enabled = false;
		Edit22 -> Enabled = false;
		Edit46 -> Enabled = false;
		Edit6  -> Color = cl3DLight;
		Edit7  -> Color = cl3DLight;
		Edit8  -> Color = cl3DLight;
		Edit18 -> Color = cl3DLight;
		Edit22 -> Color = cl3DLight;
		Edit46 -> Color = cl3DLight;

		EditNFmin -> Enabled = false;
		EditNFmax -> Enabled = false;
		EditMNK   -> Enabled = false;
		EditNFmin -> Color = cl3DLight;
		EditNFmax -> Color = cl3DLight;
		EditMNK   -> Color = cl3DLight;

		Edit27-> Enabled = false;
		Edit29-> Enabled = false;
		Edit30-> Enabled = false;
		Edit31-> Enabled = false;
		Edit27-> Color = cl3DLight;
		Edit29-> Color = cl3DLight;
		Edit30-> Color = cl3DLight;
		Edit31-> Color = cl3DLight;

		ComboBoxMainPoint -> Enabled = false;
		ComboBoxMainPoint -> Color = cl3DLight;
	}
	else
	{
		Edit51 -> Enabled = false;
		Edit51 -> Color = cl3DLight;

		Edit6  -> Enabled = true;
		Edit7  -> Enabled = true;
		Edit8  -> Enabled = true;
		Edit18 -> Enabled = true;
		Edit22 -> Enabled = true;
		Edit46 -> Enabled = true;
		Edit6  -> Color = clWindow;
		Edit7  -> Color = clWindow;
		Edit8  -> Color = clWindow;
		Edit18 -> Color = clWindow;
		Edit22 -> Color = clWindow;
		Edit46 -> Color = clWindow;

		EditMNK   -> Enabled = true;
		EditMNK   -> Color = clWindow;

		ComboBoxMainPoint -> Enabled = true;
		ComboBoxMainPoint -> Color = clWindow;

		if (CheckBoxNF->Checked == false)
		{
			EditNFmin -> Enabled = true;
			EditNFmax -> Enabled = true;

			EditNFmin -> Color = clWindow;
			EditNFmax -> Color = clWindow;
		}

		if (CheckBox6->Checked == false)
		{
			Edit27 -> Enabled = true;
			Edit29 -> Enabled = true;

			Edit27 -> Color = clWindow;
			Edit29 -> Color = clWindow;
		}

		if (CheckBox7->Checked == false)
		{
			Edit30 -> Enabled = true;
			Edit31 -> Enabled = true;

			Edit30 -> Color = clWindow;
			Edit31 -> Color = clWindow;
		}

	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit11Change(TObject *Sender)
{
	Edit52->Text = Edit11->Text;
}
//---------------------------------------------------------------------------



void __fastcall TForm1::Edit9Change(TObject *Sender)
{
	Edit54->Text = Edit9->Text;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBox15Click(TObject *Sender)
{
	if (CheckBox15 -> Checked)
	{
		Edit54 -> Enabled = false;
		Edit55 -> Enabled = false;
		Edit54 -> Color = cl3DLight;
		Edit55 -> Color = cl3DLight;
	}
	else
	{
		Edit54 -> Enabled = true;
		Edit55 -> Enabled = true;
		Edit54 -> Color = clWindow;
		Edit55 -> Color = clWindow;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBox16Click(TObject *Sender)
{
	if (CheckBox16 -> Checked)
	{
		Edit56 -> Enabled = false;
		Edit56 -> Color = cl3DLight;
	}
	else
	{
		Edit56 -> Enabled = true;
		Edit56 -> Color = clWindow;
	}
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Edit_IOZ6Change(TObject *Sender)
{
	if (Edit_IOZ6->Text != "" && StrToInt(Edit_IOZ6->Text) != 0)
	{
		Edit_IOZ5->Text = "0";
		Edit_IOZ4->Text = "0";
		Label_IOZ6->Caption = Edit_IOZ6->Text;
		Label_IOZ5->Caption = IntToStr((int)(StrToInt(Edit_IOZ6->Text) * 2.512));
		Label_IOZ4->Caption = IntToStr((int)(StrToInt(Edit_IOZ6->Text) * 2.512 * 2.512));
	}
	else
	{
		Edit_IOZ6->Text = "0";
		Label_IOZ6->Caption = "";
		Label_IOZ5->Caption = "";
		Label_IOZ4->Caption = "";
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit_IOZ5Change(TObject *Sender)
{
	if (Edit_IOZ5->Text != "" && StrToInt(Edit_IOZ5->Text) != 0)
	{
		Edit_IOZ6->Text = "0";
		Edit_IOZ4->Text = "0";
		Label_IOZ6->Caption = IntToStr((int)(StrToInt(Edit_IOZ5->Text) / 2.512));
		Label_IOZ5->Caption = Edit_IOZ5->Text;
		Label_IOZ4->Caption = IntToStr((int)(StrToInt(Edit_IOZ5->Text) * 2.512));
	}
	else
	{
		Edit_IOZ5->Text = "0";
		Label_IOZ6->Caption = "";
		Label_IOZ5->Caption = "";
		Label_IOZ4->Caption = "";
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit_IOZ4Change(TObject *Sender)
{
	if (Edit_IOZ4->Text != "" && StrToInt(Edit_IOZ4->Text) != 0)
	{
		Edit_IOZ6->Text = "0";
		Edit_IOZ5->Text = "0";
		Label_IOZ6->Caption = IntToStr((int)(StrToInt(Edit_IOZ4->Text) / 2.512 / 2.512));
		Label_IOZ5->Caption = IntToStr((int)(StrToInt(Edit_IOZ4->Text) / 2.512));
		Label_IOZ4->Caption = Edit_IOZ4->Text;
	}
	else
	{
		Edit_IOZ4->Text = "0";
		Label_IOZ6->Caption = "";
		Label_IOZ5->Caption = "";
		Label_IOZ4->Caption = "";
	}
}
//---------------------------------------------------------------------------


void __fastcall TForm1::CheckBoxBaricClick(TObject *Sender)
{
	if (CheckBoxBaric -> Checked)
	{
		EditBaric -> Enabled = true;
		EditBaric -> Color = clWindow;
	}
	else
	{
		EditBaric -> Enabled = false;
		EditBaric -> Color = cl3DLight;
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBoxPhotometryClick(TObject *Sender)
{
	if (CheckBoxPhotometry)
	{
		if (Label_IOZ5->Caption == "")
		{
			Edit_IOZ6->Text = "0";
			Edit_IOZ5->Text = "1000";
			Edit_IOZ4->Text = "0";
			Label_IOZ6->Caption = IntToStr((int)(StrToInt(Edit_IOZ5->Text) / 2.512));
			Label_IOZ5->Caption = Edit_IOZ5->Text;
			Label_IOZ4->Caption = IntToStr((int)(StrToInt(Edit_IOZ5->Text) * 2.512));
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ButtonBlackFrameClick(TObject *Sender)  //Создание усредненного темнового кадра для Фотометрии
{
    //Прочтение и усреднение
	if (OpenDialog6->Execute())
	{
		FILE * fileBF;

		ProgressBar1->Max = OpenDialog6->Files->Count;
		ProgressBar1->Position = 0;
		StatusBar1->Panels->Items[0]->Text = IntToStr(0) + " / " + IntToStr(OpenDialog6->Files->Count);
		Application->ProcessMessages();

		//Выделение памяти
		BlackKadrWindow = new WORD*[StrToInt(Form1->Edit9->Text)];
		for (int i = 0; i < StrToInt(Form1->Edit9->Text); i++)
			BlackKadrWindow[i] = new WORD[StrToInt(Form1->Edit9->Text)];

			   BlackKadr = new WORD*[StrToInt(Form1->Edit2->Text)];
		int  **BufBlKadr = new int* [StrToInt(Form1->Edit2->Text)];

		for (int i = 0; i < StrToInt(Form1->Edit2->Text); i++)
		{
			BlackKadr[i] = new WORD[StrToInt(Form1->Edit1->Text)];
			BufBlKadr[i] = new int [StrToInt(Form1->Edit1->Text)];
		}

		//Очищение кадров
		for (int i = 0; i < StrToInt(Form1->Edit2->Text); i++)
		for (int j = 0; j < StrToInt(Form1->Edit1->Text); j++)
		{
			BlackKadr[i][j] = 0;
			BufBlKadr[i][j] = 0;
		}

		StatusBar1->Panels->Items[2]->Text = "";
		StatusBar1->Panels->Items[3]->Text = "";

		for (int ia = 0; ia < OpenDialog6->Files->Count; ia++)
		{
			fileBF = fopen(AnsiString(OpenDialog6->Files->Strings[ia]).c_str(), "rb");
			for (int j = 0; j < StrToInt(Form1->Edit2->Text); j++)
				fread(BlackKadr[j], sizeof(WORD), StrToInt(Form1->Edit1->Text), fileBF);
			fclose(fileBF);

			for (int i = 0; i < StrToInt(Form1->Edit2->Text); i++)
			for (int j = 0; j < StrToInt(Form1->Edit1->Text); j++)
			{
				BufBlKadr[i][j] += BlackKadr[i][j];
				BlackKadr[i][j] = 0;
			}
			ProgressBar1->Position = ia+1;
			StatusBar1->Panels->Items[0]->Text = IntToStr(ia+1) + " / " + IntToStr(OpenDialog6->Files->Count);
			Application->ProcessMessages();
		}

		for (int i = 0; i < StrToInt(Form1->Edit2->Text); i++)
		for (int j = 0; j < StrToInt(Form1->Edit1->Text); j++)
		{
			BlackKadr[i][j] = (WORD) ( (float)((BufBlKadr[i][j] / (float) OpenDialog6->Files->Count) + 0.5) );
		}

		BKflag = true;
		LabelBFstate->Caption = "Выбрано файлов:  " + IntToStr(OpenDialog6->Files->Count);
		CheckBoxCloseBF->Checked = true;
		CheckBoxCloseBF->Visible = true;
		ButtonBFsave->Visible = true;

		for (int i = 0; i < StrToInt(Form1->Edit2->Text); i++)
		delete [] BufBlKadr[i];
		delete [] BufBlKadr;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBoxCloseBFClick(TObject *Sender)
{
	if (CheckBoxCloseBF->Checked)
	{
		BKflag = true;
		LabelBFstate->Enabled = true;
	}
	else
	{
		BKflag = false;
		LabelBFstate->Enabled = false;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ButtonBFsaveClick(TObject *Sender)
{
	if (SaveDialog2->Execute())
	{
		FILE * fileSave = fopen(AnsiString(SaveDialog2->FileName+".cmv").c_str(), "wb");
		for (int j = 0; j < StrToInt(Form1->Edit1->Text); j++)
		{
			fwrite(BlackKadr[j], sizeof(WORD), StrToInt(Form1->Edit1->Text), fileSave);
		}
		fclose(fileSave);
	}
}
//---------------------------------------------------------------------------


void __fastcall TForm1::CheckBox4Click(TObject *Sender)
{
	if (CheckBox4->Checked)
	{
		ComboBoxPixIs->Color = clWindow;
		ComboBoxPixIs->Enabled = true;
	}
	else
	{
		ComboBoxPixIs->Color = cl3DLight;
		ComboBoxPixIs->Enabled = false;
	}
}
//---------------------------------------------------------------------------



void __fastcall TForm1::ButtonNOPClick(TObject *Sender)
{
	if (OpenDialogNOP->Execute())
	{
		FILE *fileNOP = fopen(AnsiString(OpenDialogNOP->FileName).c_str(), "r");
		char liles3[300];
		bool flag = false;
		int pos = 0;

		while (feof(fileNOP) == 0 && pos == 0)
		{
			fgets(liles3, 300, fileNOP);
			String ddd= liles3;
			pos = ddd.Pos ("BV");
			if (pos > 0 && flag == false)
			{
				flag = true;
				pos = 0;
			}
		}

		if (pos > 0)
		{
			float bufF;
			char bufC[2];
			int  bufI;
			fgets(liles3, 300, fileNOP);
			sscanf(liles3, "       %d	%d	%d	%f	%c%c	%d	%d	%f	%f	%f	%f	%f	%f	%f", &bufI, &bufI, &bufI, &bufF, &bufC[0], &bufC[1],
								&bufI, &bufI, &bufF, &bufF, &bufF, &bufF, &bufF, &bufF, &bufF);
			float fg = 6 * bufF;
		}
	}
}
//---------------------------------------------------------------------------


void __fastcall TForm1::ini3Click(TObject *Sender)
{
	if (ini3->Checked == true) {
		ini3->Checked = false;
	}
	else
	if (ini3->Checked == false) {
		ini3->Checked = true;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::EditBaricChange(TObject *Sender)
{
	if (EditBaric->Text == "") {
		EditBaric->Text = 0;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBoxNFClick(TObject *Sender)
{
	if (CheckBoxNF -> Checked)
	{
		EditNFmin -> Enabled = false;
		EditNFmax -> Enabled = false;
		EditNFmin -> Color = cl3DLight;
		EditNFmax -> Color = cl3DLight;
	}
	else
	{
		EditNFmin -> Enabled = true;
		EditNFmax -> Enabled = true;
		EditNFmin -> Color = clWindow;
		EditNFmax -> Color = clWindow;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBox12Click(TObject *Sender)
{
	if (CheckBox12 -> Checked)
	{
		Edit38 -> Enabled = false;
		Edit39 -> Enabled = false;
		Edit38 -> Color = cl3DLight;
		Edit39 -> Color = cl3DLight;
	}
	else
	{
		Edit38 -> Enabled = true;
		Edit39 -> Enabled = true;
		Edit38 -> Color = clWindow;
		Edit39 -> Color = clWindow;
	}
}
//---------------------------------------------------------------------------


void __fastcall TForm1::SelectDarkFramesBtnClick(TObject *Sender)
{
    //Прочтение и усреднение
	if (OpenDialog6->Execute())
	{
		FILE * fileBF;

		ProgressBar1->Max = OpenDialog6->Files->Count;
		ProgressBar1->Position = 0;
		StatusBar1->Panels->Items[0]->Text = IntToStr(0) + " / " + IntToStr(OpenDialog6->Files->Count);
		Application->ProcessMessages();

		//Выделение памяти
		BlackKadrWindow = new WORD*[StrToInt(Form1->Edit9->Text)];
		for (int i = 0; i < StrToInt(Form1->Edit9->Text); i++)
			BlackKadrWindow[i] = new WORD[StrToInt(Form1->Edit9->Text)];

		BlackKadr = new WORD*[StrToInt(Form1->Edit2->Text)];
		int  **BufBlKadr = new int* [StrToInt(Form1->Edit2->Text)];

		for (int i = 0; i < StrToInt(Form1->Edit2->Text); i++) {
			BlackKadr[i] = new WORD[StrToInt(Form1->Edit1->Text)];
			BufBlKadr[i] = new int [StrToInt(Form1->Edit1->Text)];
		}

		//Очищение кадров
		for (int i = 0; i < StrToInt(Form1->Edit2->Text); i++)
			for (int j = 0; j < StrToInt(Form1->Edit1->Text); j++) {
				BlackKadr[i][j] = 0;
				BufBlKadr[i][j] = 0;
			}

		StatusBar1->Panels->Items[2]->Text = "";
		StatusBar1->Panels->Items[3]->Text = "";

		for (int ia = 0; ia < OpenDialog6->Files->Count; ia++)
		{
			fileBF = fopen(AnsiString(OpenDialog6->Files->Strings[ia]).c_str(), "rb");

			for (int j = 0; j < StrToInt(Form1->Edit2->Text); j++)
				fread(BlackKadr[j], sizeof(WORD), StrToInt(Form1->Edit1->Text), fileBF);
			fclose(fileBF);

			for (int i = 0; i < StrToInt(Form1->Edit2->Text); i++)
				for (int j = 0; j < StrToInt(Form1->Edit1->Text); j++) {
					BufBlKadr[i][j] += BlackKadr[i][j];
					BlackKadr[i][j] = 0;
				}
			ProgressBar1->Position = ia+1;
			StatusBar1->Panels->Items[0]->Text = IntToStr(ia+1) + " / " + IntToStr(OpenDialog6->Files->Count);
			Application->ProcessMessages();
		}

		for (int i = 0; i < StrToInt(Form1->Edit2->Text); i++)
			for (int j = 0; j < StrToInt(Form1->Edit1->Text); j++) {
				BlackKadr[i][j] = (WORD) ( (float)((BufBlKadr[i][j] / (float) OpenDialog6->Files->Count) + 0.5) );
			}

		BKflag = true;
		LabelBFstate->Caption = "Выбрано файлов:  " + IntToStr(OpenDialog6->Files->Count);
		CheckBoxCloseBF->Checked = true;
		CheckBoxCloseBF->Visible = true;
		ButtonBFsave->Visible = true;

		for (int i = 0; i < StrToInt(Form1->Edit2->Text); i++)
			delete [] BufBlKadr[i];
		delete [] BufBlKadr;
	}
}
//---------------------------------------------------------------------------

bool __fastcall TForm1::localizationStep(AnsiString resultFileName[], int* size){
	int i,j, mainFOR, Gsch = 0, Bsch = 0;
	FILE *streG;
	FILE *Buono, *Brutto, *Cattivo;
	Label40->Visible = true;
	StatusBar1->Panels->Items[0]->Text = "Поиск файлов...";
	Application->ProcessMessages();
	FilesCount = 0;
	//Выбор файлов по папкам или вручную, получаем все пути к файлам в FileNames[FilesCount]
	SelectingFilesFrames();

	if (DirectoryListBox2->Visible == true) {
		DirectoryListBox2->Visible = false;
		Button8->Visible = false;
		Button3->Caption = "Обзор...";
	}
	if (DirectoryListBox1->Visible == true) {
		DirectoryListBox1->Visible = false;
		Button7->Visible = false;
		Button2->Caption = "Обзор...";
	}

	struct tm *local;
	time_t t = time(NULL);
	local = localtime(&t);
	char bufftime [80];        // строка, в которой будет храниться текущее время
	strftime(bufftime,80,"%Y.%m.%d %H-%M-%S",local);

	if (DirectoryExists(AnsiString(Edit23->Text).c_str()) == true) {
		if (FilesCount > 0) {
			MemoryForFrame(); //Выделение памяти для кадра SigmaSet.Kadr[i][j]

			Buono = fopen(AnsiString(Edit23->Text + "\\Sigma - " + bufftime
					+ " - OneStar.txt").c_str(), "a");   //Good
			Brutto = fopen(AnsiString(Edit23->Text + "\\Sigma - " + bufftime
					+ " - AllStars.txt").c_str(), "a"); //Bad
			Cattivo = fopen(AnsiString(Edit23->Text + "\\Sigma - " + bufftime
					+ " - AllStarsList.txt").c_str(), "a");

			resultFileName[0]  = AnsiString(Edit23->Text + "\\Sigma - " + bufftime
					+ " - OneStar.txt");
			resultFileName[1]  = AnsiString(Edit23->Text + "\\Sigma - " + bufftime
					+ " - AllStars.txt");
            *size = 2;

			String TabSpace = "	";
			switch(ComboBox1->ItemIndex) {
				case 0:
				TabSpace = "	";
				break;

				case 1:
				TabSpace = "		";
				break;

				case 2:
				TabSpace = "			";
				break;

				case 3:
				TabSpace = "				";
				break;

				case 4:
				TabSpace = "					";
				break;
			}

			ProgressBar1->Position = 0;
			ProgressBar1->Max = FilesCount;
			StatusBar1->Panels->Items[0]->Text = IntToStr(ProgressBar1->Position) + " / " + IntToStr(ProgressBar1->Max);
			Form1->Memo1->Lines->Add("Локализация - Начало");
			Form1->Memo1->Lines->Add("№" + TabSpace + "Время" + TabSpace + "Порог" + TabSpace + "Лок-но" + TabSpace + "Вывод" + TabSpace + "Имя файла");
			//Цикл по файлам
			for (mainFOR = 0; mainFOR < FilesCount; mainFOR++) {
				for (i = 0; i < SigmaSet.Height; i++)
					for (j = 0; j < SigmaSet.Width; j++) {
						SigmaSet.Kadr[i][j] 	= 0;
						SigmaSet.FiltKadr[i][j] = 0;
					}

				bool resultReadFiles = false;
				ReadFilesFrames(mainFOR, &resultReadFiles);  //Считываем файл Стандартный или Новый (IKI), получаем SigmaSet.Kadr[i][j]
				if (resultReadFiles == false)
					Ostanov = true;

				if (Ostanov == false) {
					if (CheckBox3->Checked)
						FilteringMovingAverage(); //Фильтруем заданным окном и вычитаем среднее+Edit17*СКО
					else
						FilteringMovingAverage();

					// Результаты локализации при 3х случаях
					if (Nobj > -1) {
						switch (Nobj) {
							case 0:
								Form1->Memo1->Lines->Add(IntToStr(mainFOR+1) + TabSpace + TimeToStr(Time()) + TabSpace +  IntToStr(MeanPor) + TabSpace + IntToStr(Nobj) + TabSpace + "ПЛОХ" + TabSpace+ FileNames[mainFOR]);
								fprintf(Cattivo, "%s	%d\n",  FileNames[mainFOR], Nobj);
								Bsch++;
							break;

							case 1:
								if (CheckBox8->Checked == false) //Можно выводить не только ошибки
								Form1->Memo1->Lines->Add(IntToStr(mainFOR+1) + TabSpace + TimeToStr(Time()) + TabSpace +  IntToStr(MeanPor) + TabSpace + IntToStr(Nobj) + TabSpace + "ХОР" + TabSpace + FileNames[mainFOR]);
								fprintf(Cattivo, "%s	%d\n",  FileNames[mainFOR], Nobj);
								Gsch++;

								SelectMaxObject(SigmaSet.FiltKadr, SigmaSet.Width, SigmaSet.Height, Xloc[0], Yloc[0], false);
								fprintf(Buono, "%d	%f	%f	%d	%d	%d	%s\n", Nobj, Xloc[0], Yloc[0], BRloc[0], NelLoc[0], SigmaSet.Ispic, FileNames[mainFOR]);
							break;

						   default:
								if (CheckBox8->Checked == false) //Можно выводить не только ошибки
								Form1->Memo1->Lines->Add(IntToStr(mainFOR+1) + TabSpace + TimeToStr(Time()) + TabSpace +  IntToStr(MeanPor) + TabSpace + IntToStr(Nobj) + TabSpace + "ХОР" + TabSpace + FileNames[mainFOR]);
								fprintf(Cattivo, "%s	%d\n",  FileNames[mainFOR], Nobj);
								Gsch++;

								for (i = 0; i < Nobj; i++) {
									SelectMaxObject(SigmaSet.FiltKadr, SigmaSet.Width, SigmaSet.Height, Xloc[i], Yloc[i], false);
									fprintf(Brutto, "%d	%f	%f	%d	%d	%d	%s\n", Nobj, Xloc[i], Yloc[i], BRloc[i], NelLoc[i], SigmaSet.Ispic, FileNames[mainFOR]);
								}
						   break;
						}
						if (ini3->Checked == true)
							IniSave(AnsiString(Edit23->Text + "\\INI_Sigma - " + bufftime + " - AllStarsList").c_str());
					}


					ProgressBar1->Position++;
					StatusBar1->Panels->Items[0]->Text = IntToStr(mainFOR+1) + " / " + IntToStr(FilesCount);
					StatusBar1->Panels->Items[2]->Text = "G: " + IntToStr(Gsch) + " / " + IntToStr(FilesCount);
					StatusBar1->Panels->Items[3]->Text = "B: " + IntToStr(Bsch) + " / " + IntToStr(FilesCount);
					Application->ProcessMessages();

					if (Ostanov == true)
						mainFOR = FilesCount;
				} //Ostanov
				else
					mainFOR = FilesCount;
			}
			ProgressBar1->Position = ProgressBar1->Max;
			if (Ostanov == false)
			Form1->Memo1->Lines->Add("№" + TabSpace + "Время" + TabSpace + "Порог"
				+ TabSpace + "Лок-но" + TabSpace + "Вывод" + TabSpace + "Имя файла");
			Form1->Memo1->Lines->Add("Локализация - Завершено");
			fclose(Cattivo);
			fclose(Brutto);
			fclose(Buono);

			for (i = 0; i < StrToInt(Form1->Edit2->Text); i++) delete [] SigmaSet.FiltKadr[i];
				delete [] SigmaSet.FiltKadr;

			for (i = 0; i < StrToInt(Form1->Edit2->Text); i++) delete [] SigmaSet.Kadr[i];
				delete [] SigmaSet.Kadr;

			for (i = 0; i < StrToInt(Form1->Edit9->Text); i++) delete [] SigmaSet.Frame[i];
				delete [] SigmaSet.Frame;
			if (RadioGroup1->ItemIndex == 1)
				delete ikimg;
		}
		else  {
			ShowMessage("Не выбраны файлы");
			return false;
		}
	}
	else {
		ShowMessage("Папка для сохранения результатов не существует!");
		return false;
	}

	if (Ostanov == true){
		Form1->Memo1->Lines->Add("---Преждевременноое завершение---");
		return false;
	}

	Ostanov = false;
	Label40->Visible = false;
	return true;
}

bool __fastcall TForm1::sigmaStep(AnsiString resultFileNames[], int size, AnsiString* sigmaResultFileName) {
	int ifile, i, j, goool = 1, Gsch = 0, Bsch = 0;
	FILE* Strim, * Rezu;
	int SigKol, prov;
	const int SigmaNameFi = 200;
	char SigmaNameF[SigmaNameFi];
	bool flagOnceTitle = true;

	Form1->OpenDialog2->Options.Clear();
	Form1->OpenDialog2->Options << ofAllowMultiSelect << ofFileMustExist;

	int CMmMs;
	BoxMmMsStr* BoxMmMs = new BoxMmMsStr[MAX_FILE];

	if (DirectoryListBox2->Visible == true) {
		DirectoryListBox2->Visible = false;
		Button8->Visible = false;
		Button3->Caption = "Обзор...";
	}
	if (DirectoryListBox1->Visible == true) {
		DirectoryListBox1->Visible = false;
		Button7->Visible = false;
		Button2->Caption = "Обзор...";
	}

	struct tm* local;
	time_t t = time(NULL);
	local = localtime(&t);
	char bufftime[80];        // строка, в которой будет храниться текущее время
	strftime(bufftime, 80, "%Y.%m.%d %H-%M-%S", local);

	unsigned int start_time, end_time;
	start_time = clock();
	String MemoText = "Start Sigma - ";
	MemoText += TimeToStr(Time());
	Form1->Memo1->Lines->Add(MemoText);

	//Оценка для термометра и выделение памяти
	ProgressBar1->Position = 0;
	ProgressBar1->Max = 0;
	for (ifile = 0; ifile < size; ifile++) {
		char BufForProgress[300];
		Strim = fopen(resultFileNames[ifile].c_str(), "r");
		while (feof(Strim) == 0) {
			prov = fscanf(Strim, "%s	%s	%s	%s	%s	%s	%[^\n]s", &BufForProgress, &BufForProgress, &BufForProgress, &BufForProgress, &BufForProgress, &BufForProgress, &BufForProgress);
			ProgressBar1->Max++;
			StatusBar1->Panels->Items[0]->Text = "Оценка... " + IntToStr(ProgressBar1->Max);
			Application->ProcessMessages();

			if (flagOnceTitle && feof(Strim) == 0) {
				for (i = 0; i < SigmaNameFi; i++) SigmaNameF[i] = '\0';
				prov = fscanf(Strim, "%s	%s	%s	%s	%s	%s	%[^\n]s", &BufForProgress, &BufForProgress, &BufForProgress, &BufForProgress, &BufForProgress, &BufForProgress, &SigmaNameF);
				ProgressBar1->Max++;
				StatusBar1->Panels->Items[0]->Text = "Оценка... " + IntToStr(ProgressBar1->Max);
				Application->ProcessMessages();

				FileNames[0] = SigmaNameF;
				MemoryForFrame(); //Выделение памяти для кадра SigmaSet.Kadr[i][j]
				flagOnceTitle = false;
			}
		}
		fclose(Strim);
		ProgressBar1->Max--;
	}

	flagOnceTitle = true;
	StatusBar1->Panels->Items[0]->Text = IntToStr(ProgressBar1->Position) + " / " + IntToStr(ProgressBar1->Max);
	Application->ProcessMessages();

	Label41->Visible = true;

	String TabSpace = "	";
	switch (ComboBox1->ItemIndex) {
	case 0:
		TabSpace = "	";
		break;

	case 1:
		TabSpace = "		";
		break;

	case 2:
		TabSpace = "			";
		break;

	case 3:
		TabSpace = "				";
		break;

	case 4:
		TabSpace = "					";
		break;
	}

	Form1->Memo1->Lines->Add("№" + TabSpace + "Время" + TabSpace + "Вывод"
		+ TabSpace + "Х-Y" + TabSpace + "SigmaX" + TabSpace + "SigmaY" + TabSpace
		+ "Mu" + TabSpace + "Имя файла");

	//Цикл по выбранным текстовым файлам
	for (ifile = 0; ifile < size; ifile++) {
		Strim = fopen(resultFileNames[ifile].c_str(), "r");

		if (DirectoryExists(AnsiString(Edit23->Text).c_str()) == true) {
			Rezu = fopen(AnsiString(Edit23->Text + "\\Sigma - " + bufftime
				+ " - REZ.txt").c_str(), "a");

			*sigmaResultFileName = AnsiString(Edit23->Text + "\\Sigma - " + bufftime
				+ " - REZ.txt");
		}
		else {
			ShowMessage("Папка для сохранения результатов не существует!");
			return false;
		}


		CMmMs = 0;
		int rarr = 1;
		//Обработка каждого выбранного текстового файла до конца построчно
		while (feof(Strim) == 0) {
			if (flagOnceTitle) {
				fprintf(Rezu, "N_Stars	X	Y	Is_loc	N_loc	CenterPix	Is_frame	N_frame	Mean_Th	SKO_Th	Th	Is_fot	Mu	X_Gauss	Y_Gauss	X_CenterPixGauss	Y_CenterPixGauss	SigmaX	SigmaY	NameFile\n");
				flagOnceTitle = false;
			}
			FileNames[0] = "";
			for (i = 0; i < SigmaNameFi; i++)
				SigmaNameF[i] = '\0';

			SigKol = 0;
			SigmaSet.x = 0;
			SigmaSet.y = 0;
			SigmaSet.Is = 0;
			SigmaSet.N = 0;
			SigmaSet.Is2 = 0;
			SigmaSet.N2 = 0;
			SigmaSet.Ispic = 0;
			prov = fscanf(Strim, "%d	%f	%f	%d	%d	%d	%[^\n]s", &SigKol,
				&SigmaSet.x, &SigmaSet.y, &SigmaSet.Is, &SigmaSet.N, &SigmaSet.Ispic, &SigmaNameF);
			FileNames[0] = SigmaNameF;

			//Если стоит обработка самого яркого элемента на кадре
			//Первая колонка в текстовом файле - количество локализованных объектов
			if (CheckBox4->Checked && SigKol > 1 && prov == 7) {
				switch (ComboBoxPixIs->ItemIndex) {
					case 0:  // по яркости
						for (i = 0; i < SigKol - 1; i++) {
							int BufIs, BufN, BufKol, BufIsM;
							float BufX, BufY;
							prov = fscanf(Strim, "%d	%f	%f	%d	%d	%d	%[^\n]s",
								&BufKol, &BufX, &BufY, &BufIs, &BufN, &BufIsM, &SigmaNameF);

							if (BufIs > SigmaSet.Is && SigKol == BufKol && prov == 7) {
								FileNames[0] = SigmaNameF;
								SigKol = BufKol;
								SigmaSet.x = BufX;
								SigmaSet.y = BufY;
								SigmaSet.Is = BufIs;
								SigmaSet.N = BufN;
								SigmaSet.Ispic = BufIsM;
							}
						}
						break;

					case 1:  //по ярчайшему пикселю в объекте
						for (i = 0; i < SigKol - 1; i++) {
							int BufIs, BufN, BufKol, BufIsM;
							float BufX, BufY;
							prov = fscanf(Strim, "%d	%f	%f	%d	%d	%d	%[^\n]s",
								&BufKol, &BufX, &BufY, &BufIs, &BufN, &BufIsM, &SigmaNameF);

							if (BufIsM > SigmaSet.Ispic && SigKol == BufKol && prov == 7) {
								FileNames[0] = SigmaNameF;
								SigKol = BufKol;
								SigmaSet.x = BufX;
								SigmaSet.y = BufY;
								SigmaSet.Is = BufIs;
								SigmaSet.N = BufN;
								SigmaSet.Ispic = BufIsM;
							}
						}
						break;
					}
			}

			if (Ostanov == false) {
				if (FileNames[0] != "" && prov == 7) {
					if (SigmaSet.Ispic < StrToInt(Edit15->Text) || SigmaSet.Ispic > StrToInt(Edit16->Text))
					{
						Form1->Memo1->Lines->Add(IntToStr(ProgressBar1->Position + 1)
							+ TabSpace + TimeToStr(Time()) + TabSpace + "ЯРЧ!" + TabSpace + FloatToStr((int)SigmaSet.x) + "x" + FloatToStr((int)SigmaSet.y) + TabSpace + "-" + TabSpace + "-" + TabSpace + "-" + TabSpace + FileNames[0]);
						Bsch++;
					}
					else {
						bool resultReadFiles = false;
						//Считываем файл Стандартный или Новый (IKI), получаем SigmaSet.Kadr[i][j]
						ReadFilesFrames(0, &resultReadFiles);
						if (resultReadFiles == false)
							if (MessageDlg("Продолжить выполнение?", mtConfirmation, TMsgDlgButtons() << mbYes << mbNo, 0) == mrNo)
								Ostanov = true;

						SigmaSet.HWframe = StrToInt(Edit9->Text);

						if (CheckBox2->Checked)
							BinFrameSigma();  //Бинирование
						else {
							//Вырезание окошка
							for (i = 0; i < SigmaSet.HWframe; i++)
								for (j = 0; j < SigmaSet.HWframe; j++)
									if ((int)(i + SigmaSet.y - SigmaSet.HWframe / 2) >= 0 && (int)(i + SigmaSet.y - SigmaSet.HWframe / 2) < SigmaSet.Height &&
										(int)(j + SigmaSet.x - SigmaSet.HWframe / 2) >= 0 && (int)(j + SigmaSet.x - SigmaSet.HWframe / 2) < SigmaSet.Width)
										SigmaSet.Frame[i][j] = SigmaSet.Kadr[(int)(i + SigmaSet.y - SigmaSet.HWframe / 2)][(int)(j + SigmaSet.x - SigmaSet.HWframe / 2)];
									else
										SigmaSet.Frame[i][j] = 0;
						}

						if (BKflag) {
							for (i = 0; i < SigmaSet.HWframe; i++)
								for (j = 0; j < SigmaSet.HWframe; j++)
									BlackKadrWindow[i][j] = BlackKadr[(int)(i + SigmaSet.y - SigmaSet.HWframe / 2)][(int)(j + SigmaSet.x - SigmaSet.HWframe / 2)];
						}

						float IYasa = 0;
						BufLOC1 = 0;

						if (CheckBox13->Checked) {
							int jLOC = 0;
							for (j = 0; j < 4; j++) {
								for (i = 0; i < SigmaSet.HWframe; i++)
									BufLOC1 += SigmaSet.Frame[jLOC][i];

								if (j == 0)
									jLOC++;
								else
									jLOC = SigmaSet.HWframe - j;
							}

							BufLOC1 = BufLOC1 / (SigmaSet.HWframe * 4);

							for (i = 0; i < SigmaSet.HWframe; i++)
								for (j = 0; j < SigmaSet.HWframe; j++)
									IYasa += SigmaSet.Frame[i][j] - BufLOC1;
						}

						F4Lines = 0;

						if (CheckBox14->Checked) { // расчет сигмы по сектору, не выделяя звезду
							BufLOCdSKO = 0;
							int jLOC = 0;
							for (j = 0; j < 4; j++) {
								for (i = 0; i < SigmaSet.HWframe; i++)
									BufLOCdSKO += SigmaSet.Frame[jLOC][i];

								if (j == 0)
									jLOC++;
								else
									jLOC = SigmaSet.HWframe - j;
							}

							BufLOCdSKO = BufLOCdSKO / (SigmaSet.HWframe * 4);
							Form1->Edit26->Text = "10";
						}
						else
							Filter4Lines(StrToInt(Form1->Edit28->Text), SigmaSet.Frame, SigmaSet.HWframe, SigmaSet.HWframe); //Фильтрация по 4м строкам

						SelectMaxObject(SigmaSet.Frame, SigmaSet.HWframe, SigmaSet.HWframe, SigmaSet.HWframe / 2, SigmaSet.HWframe / 2, true);

						if (SigmaSet.N2 >= StrToInt(Edit52->Text)) {
							fprintf(Rezu, "%d	%f	%f	%d	%d	%d", SigKol, SigmaSet.x, SigmaSet.y, SigmaSet.Is, SigmaSet.N, SigmaSet.Ispic);
							//Вычитаем F4Lines обратно
							fprintf(Rezu, "	%d	%d	%5.2f	%5.2f	%d	%d", SigmaSet.Is2, SigmaSet.N2, BufLOC1, BufLOCdSKO, F4Lines, Is_fot);

							double Mug = 0, SigX = 0, SigY = 0, Sx0 = 0, Sy0 = 0, xi_G = 0, yi_G = 0;
							//Сигма
							if (true) {
								int* pix_X = new int[SigmaSet.HWframe * SigmaSet.HWframe];
								int* pix_Y = new int[SigmaSet.HWframe * SigmaSet.HWframe];
								int* pix_val = new int[SigmaSet.HWframe * SigmaSet.HWframe];

								int numMeas = 0;

								for (i = 0; i < SigmaSet.HWframe; i++) {
									SigmaSet.Frame[i][0] = 0;
									SigmaSet.Frame[i][SigmaSet.HWframe - 1] = 0;
									SigmaSet.Frame[0][i] = 0;
									SigmaSet.Frame[SigmaSet.HWframe - 1][i] = 0;
								}

								int Fl3 = 0;
								for (i = 1; i < SigmaSet.HWframe - 1; i++)
									for (j = 1; j < SigmaSet.HWframe - 1; j++) {
										if (SigmaSet.Frame[i][j] == 0) {
											if (SigmaSet.Frame[i - 1][j] > 0) Fl3++;
											if (SigmaSet.Frame[i + 1][j] > 0) Fl3++;
											if (SigmaSet.Frame[i][j - 1] > 0) Fl3++;
											if (SigmaSet.Frame[i][j + 1] > 0) Fl3++;

											if (SigmaSet.Frame[i - 1][j - 1] > 0) Fl3++;
											if (SigmaSet.Frame[i + 1][j + 1] > 0) Fl3++;
											if (SigmaSet.Frame[i + 1][j - 1] > 0) Fl3++;
											if (SigmaSet.Frame[i - 1][j + 1] > 0) Fl3++;

											if (Fl3 >= StrToInt(Form1->Edit26->Text)) {
												pix_val[numMeas] = 1;
												pix_X[numMeas] = j;
												pix_Y[numMeas++] = i;
											}

											Fl3 = 0;
										}
										else
											if (SigmaSet.Frame[i][j] > 0) {
												if (CheckBox14->Checked)
													pix_val[numMeas] = SigmaSet.Frame[i][j] - BufLOCdSKO;//+ 1 + BufLOCdSKO*StrToInt(Form1->Edit28->Text); // DELETE NOW
												else
													pix_val[numMeas] = SigmaSet.Frame[i][j] + 1 + BufLOCdSKO * StrToInt(Form1->Edit28->Text); // DELETE NOW

												pix_X[numMeas] = j;
												pix_Y[numMeas++] = i;
											}
									}

								LocalGauss(pix_X, pix_Y, pix_val, numMeas, StrToInt(Form1->Edit25->Text), StrToInt(Form1->Edit24->Text), 2, 0.5, &Sx0, &Sy0, &SigX, &SigY, &Mug, &xi_G, &yi_G);
								fprintf(Rezu, "	%f	%f	%f	%f	%f	%f	%f	%s\n", Mug, Sx0, Sy0, xi_G, yi_G, SigX, SigY, FileNames[0]);

								delete[] pix_X;
								delete[] pix_Y;
								delete[] pix_val;
								Gsch++;

								if (CheckBox13->Checked) {
									float BufReport10 = 0;
									IdentifFromName(AnsiString(FileNames[0]).c_str(), AnsiString(Edit8->Text).c_str(), AnsiString(Edit6->Text).c_str(), &BufReport10);
									SetCell(ProgressBar1->Position + 3, 2, FloatToStr(FixRoundTo(BufReport10, -3)));
									BufReport10 = 0;
									IdentifFromName(AnsiString(FileNames[0]).c_str(), AnsiString(Edit22->Text).c_str(), AnsiString(Edit7->Text).c_str(), &BufReport10);
									SetCell(ProgressBar1->Position + 3, 1, FloatToStr(BufReport10));

									SetCell(ProgressBar1->Position + 3, 3, IntToStr(SigmaSet.Is2));
									SetCell(ProgressBar1->Position + 3, 4, FloatToStr(FixRoundTo(IYasa, -1)));
									SetCell(ProgressBar1->Position + 3, 5, IntToStr(SigmaSet.N2));
									SetCell(ProgressBar1->Position + 3, 6, FloatToStr(FixRoundTo(SigX, -3)));
									SetCell(ProgressBar1->Position + 3, 7, FloatToStr(FixRoundTo(SigY, -3)));
									SetCell(ProgressBar1->Position + 3, 8, FloatToStr(FixRoundTo(BufLOC1, -1)));
									SetCell(ProgressBar1->Position + 3, 9, FloatToStr(FixRoundTo(BufLOCdSKO, -1)));
									SetCell(ProgressBar1->Position + 3, 10, IntToStr(F4Lines));
									SetCell(ProgressBar1->Position + 3, 11, IntToStr(SigmaSet.Ispic));
									SetCell(ProgressBar1->Position + 3, 12, FloatToStr(FixRoundTo(pow(2.512f, BufReport10) * IYasa, -1)));
									SetCell(ProgressBar1->Position + 3, 13, FloatToStr(FixRoundTo(pow(2.512f, BufReport10) * SigmaSet.Is2, -1)));
								}
							}

							if (Mug > 10)
								Form1->Memo1->Lines->Add(IntToStr(ProgressBar1->Position + 1) + TabSpace + TimeToStr(Time()) + TabSpace + "Плохое уравнивание по Мю" + TabSpace + FloatToStr((int)SigmaSet.x) + "x" + FloatToStr((int)SigmaSet.y) + TabSpace + FloatToStr(FixRoundTo(SigX, -3)) + TabSpace + FloatToStr(FixRoundTo(SigY, -3)) + TabSpace + FloatToStr(FixRoundTo(Mug, -3)) + TabSpace + FileNames[0]);
							else
								if (CheckBox8->Checked == false)
									Form1->Memo1->Lines->Add(IntToStr(ProgressBar1->Position + 1) + TabSpace + TimeToStr(Time()) + TabSpace + "ОК" + TabSpace + FloatToStr((int)SigmaSet.x) + "x" + FloatToStr((int)SigmaSet.y) + TabSpace + FloatToStr(FixRoundTo(SigX, -3)) + TabSpace + FloatToStr(FixRoundTo(SigY, -3)) + TabSpace + FloatToStr(FixRoundTo(Mug, -3)) + TabSpace + FileNames[0]);

						}
						else {
							Form1->Memo1->Lines->Add(IntToStr(ProgressBar1->Position + 1) + TabSpace + TimeToStr(Time()) + TabSpace + "ПИКС!" + TabSpace + FloatToStr((int)SigmaSet.x) + "x" + FloatToStr((int)SigmaSet.y) + TabSpace + "-" + TabSpace + "-" + TabSpace + "-" + TabSpace + FileNames[0]);
							Bsch++;
						}

					}
				}
			}

			ProgressBar1->Position++;
			StatusBar1->Panels->Items[0]->Text = IntToStr(ProgressBar1->Position) + " / " + IntToStr(ProgressBar1->Max);
			StatusBar1->Panels->Items[2]->Text = "G: " + IntToStr(Gsch) + " / " + IntToStr(ProgressBar1->Max);
			StatusBar1->Panels->Items[3]->Text = "B: " + IntToStr(Bsch) + " / " + IntToStr(ProgressBar1->Max);
			Application->ProcessMessages();
		}
		fclose(Rezu);
		fclose(Strim);

		if (ini3->Checked == true)
			IniSave(AnsiString(Edit23->Text + "\\INI_Sigma - " + bufftime + " - REZ").c_str());
	}

	end_time = clock();

	MemoText = "Done Sigma - ";
	MemoText += TimeToStr(Time());
	Form1->Memo1->Lines->Add(MemoText);
	MemoText = "Total Time: ";
	MemoText += FloatToStr((end_time - start_time) / 1000.0);
	Form1->Memo1->Lines->Add(MemoText);

	for (i = 0; i < SigmaSet.Height; i++)
		delete[] SigmaSet.FiltKadr[i];

	delete[] SigmaSet.FiltKadr;

	for (i = 0; i < SigmaSet.Height; i++)
		delete[] SigmaSet.Kadr[i];

	delete[] SigmaSet.Kadr;

	for (i = 0; i < StrToInt(Form1->Edit9->Text); i++)
		delete[] SigmaSet.Frame[i];

	delete[] SigmaSet.Frame;
	delete[] BoxMmMs;

	if (RadioGroup1->ItemIndex == 1)
		delete ikimg;

	Label41->Visible = false;

	if (Ostanov == true) {
		Form1->Memo1->Lines->Add("Stoppage");
		return false;
	}

	Ostanov = false;
	return true;
}

bool __fastcall TForm1::reportResult(AnsiString sigmaResultFileName) {
	int ifile, i, j;
	FILE* Strim, * Rezu;
	int prov;
	FormatSettings.DecimalSeparator = '.';
	Form1->OpenDialog3->Options.Clear();
	Form1->OpenDialog3->Options << ofAllowMultiSelect << ofFileMustExist;
	int MaxIsForAxis = 0;

	if (DirectoryListBox2->Visible == true) {
		DirectoryListBox2->Visible = false;
		Button8->Visible = false;
		Button3->Caption = "Обзор...";
	}
	if (DirectoryListBox1->Visible == true) {
		DirectoryListBox1->Visible = false;
		Button7->Visible = false;
		Button2->Caption = "Обзор...";
	}
	//Время
	struct tm* local;
	time_t t = time(NULL);
	local = localtime(&t);
	char bufftime[80];        // строка, в которой будет храниться текущее время
	strftime(bufftime, 80, "%d.%m.%Y", local);
	String buffDate = bufftime;

	strftime(bufftime, 80, "%Y.%m.%d %H-%M-%S", local);

	if (DirectoryExists(AnsiString(Edit23->Text).c_str()) == false) {
		ShowMessage("Папка для сохранения результатов не существует!");
		return false;
	}
	bool correctSigmaCondition = StrToFloat(Edit20->Text) <= StrToFloat(Edit21->Text)
					&& StrToFloat(Edit47->Text) <= StrToFloat(Edit48->Text);
	if (!correctSigmaCondition) {
		ShowMessage("Ошибочно заданы параметры сигмы ( min > max )");
		return false;
	}

	bool correctParamsCondition = (Edit19->Text != "" && Edit20->Text != "" && Edit21->Text != "" && Edit18->Text != "" &&
		Edit38->Text != "" && Edit39->Text != "" && Edit40->Text != "" && Edit41->Text != "" && Edit42->Text != "");
	if (!correctParamsCondition) {
		ShowMessage("Заполните все ячейки!");
		return false;
	}

	String MemoText = "Start - ";
	MemoText += TimeToStr(Time());
	Form1->Memo1->Lines->Add(MemoText);

	//Оценка для термометра
	ProgressBar1->Position = 0;
	ProgressBar1->Max = 0;

	char BuFP0[300];
	Strim = fopen(sigmaResultFileName.c_str(), "r");
	//Пустые проходы для оценки количества ячеек в FullSigma
	while (feof(Strim) == 0) {
		prov = fscanf(Strim, "%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%[^\n]s",
		&BuFP0, &BuFP0, &BuFP0, &BuFP0, &BuFP0, &BuFP0, &BuFP0, &BuFP0, &BuFP0, &BuFP0, &BuFP0, &BuFP0, &BuFP0, &BuFP0, &BuFP0, &BuFP0, &BuFP0, &BuFP0, &BuFP0, &BuFP0);
		if (prov == 20)
			ProgressBar1->Max++;
		StatusBar1->Panels->Items[0]->Text = "Оценка... " + IntToStr(ProgressBar1->Max);
		Application->ProcessMessages();
	}
	fclose(Strim);
	ProgressBar1->Max--;

	StatusBar1->Panels->Items[0]->Text = IntToStr(ProgressBar1->Position) + " / " + IntToStr(ProgressBar1->Max);
	Application->ProcessMessages();

	//Место возможной ошибки
	FullSigmaTable* FullSigma = new FullSigmaTable[ProgressBar1->Max + 1];

	int Meter = 0, BadMeter = 1;
	const SigmaNameFi = 200;
	char SigmaNameF[SigmaNameFi];
	bool Condition1;

	//считывание данных из файлов (перемешивание)
	bool flagOnceTitle = true;
	char BuFP[300];
	Strim = fopen(sigmaResultFileName.c_str(), "r");
	while (feof(Strim) == 0) {
		for (i = 0; i < SigmaNameFi; i++) SigmaNameF[i] = '\0';
			FullSigma[Meter].Names = "";

		//Считывание строки названий
		char BuFP[300];
		if (flagOnceTitle) {
			prov = fscanf(Strim, "%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%s	%[^\n]s", &BuFP, &BuFP, &BuFP, &BuFP, &BuFP, &BuFP, &BuFP, &BuFP, &BuFP, &BuFP, &BuFP, &BuFP, &BuFP, &BuFP, &BuFP, &BuFP, &BuFP, &BuFP, &BuFP, &BuFP);
			flagOnceTitle = false;
		}
		prov = fscanf(Strim, "%s	%f	%f	%s	%s	%s	%d	%d	%s	%s	%s	%d	%f	%f	%f	%f	%f	%f	%f	%[^\n]s", &BuFP, &FullSigma[Meter].x, &FullSigma[Meter].y, &BuFP, &BuFP, &BuFP, &FullSigma[Meter].Is, &FullSigma[Meter].Nframe, &BuFP, &BuFP, &BuFP, &FullSigma[Meter].IsF, &FullSigma[Meter].Mu, &FullSigma[Meter].xS, &FullSigma[Meter].yS, &BuFP, &BuFP, &FullSigma[Meter].SigX, &FullSigma[Meter].SigY, &SigmaNameF);
		FullSigma[Meter].Names = SigmaNameF;

		if (FullSigma[Meter].Names != "" && prov == 20) {
			Condition1 = true;
			if (CheckBox9->Checked == false)
				if (StrToFloat(Edit47->Text) > FullSigma[Meter].SigX || StrToFloat(Edit48->Text) < FullSigma[Meter].SigX ||
					StrToFloat(Edit47->Text) > FullSigma[Meter].SigY || StrToFloat(Edit48->Text) < FullSigma[Meter].SigY)
					Condition1 = false;

			if (CheckBox10->Checked == false)
				if (StrToFloat(Edit49->Text) < FullSigma[Meter].Mu)
					Condition1 = false;

			if (CheckBox15->Checked == false)
				if (FullSigma[Meter].xS < ((StrToFloat(Edit54->Text) / 2) - StrToFloat(Edit55->Text)) ||
					FullSigma[Meter].xS >((StrToFloat(Edit54->Text) / 2) + StrToFloat(Edit55->Text)) ||
					FullSigma[Meter].yS < ((StrToFloat(Edit54->Text) / 2) - StrToFloat(Edit55->Text)) ||
					FullSigma[Meter].yS >((StrToFloat(Edit54->Text) / 2) + StrToFloat(Edit55->Text)))
					Condition1 = false;

			if (Condition1) {
				Meter++;
				StatusBar1->Panels->Items[2]->Text = "G: " + IntToStr(Meter) + " / " + IntToStr(ProgressBar1->Max);
			}
		}

		else
			StatusBar1->Panels->Items[3]->Text = "B: " + IntToStr(BadMeter++) + " / " + IntToStr(ProgressBar1->Max);
		Application->ProcessMessages();
	}
	fclose(Strim);

	// Meter - расчетное значение значимых строк
	//На этом этапе все данные получены
	//Вывод полученных данных в FullSigma
	Strim = fopen(AnsiString(Edit23->Text + "\\Sigma - " + bufftime + " - report.txt").c_str(), "a");
	fprintf(Strim, "X	Y	Is_w	Mu	X_w	Y_w	SigmaX	SigmaY\n");
	for (i = 0; i < Meter; i++)
		fprintf(Strim, "%f	%f	%d	%f	%f	%f	%f	%f\n", FullSigma[i].x, FullSigma[i].y, FullSigma[i].Is, FullSigma[i].Mu, FullSigma[i].xS, FullSigma[i].yS, FullSigma[i].SigX, FullSigma[i].SigY);
	fclose(Strim);

	int PorogPx = StrToInt(Form1->Edit19->Text);
	if (PorogPx % 2 != 0)
		PorogPx++;   //нужно, чтобы оно было четным

	const int BoxRazm = ((StrToInt(Form1->Edit34->Text) + PorogPx - 1) / PorogPx) * ((StrToInt(Form1->Edit43->Text) + PorogPx - 1) / PorogPx) + 1;
	int NomBoxXY = 1, NomBoxMS = 1, NomBoxMM = 1;
	bool fl5 = false, fl6;

	Graphics::TBitmap* bm = new Graphics::TBitmap;
	Graphics::TBitmap* bm2 = new Graphics::TBitmap;
	if (ini3->Checked == true)
		IniSave(AnsiString(Edit23->Text + "\\INI_Sigma - " + bufftime + " - report").c_str());

	//Обработка по небу
	if (CheckBox5->Checked) {
		SBoxXY* BoxXY = new SBoxXY[BoxRazm];
		SBoxXY* BoxMS = new SBoxXY[BoxRazm];
		SBoxXY* BoxMM = new SBoxXY[BoxRazm];

		ProgressBar1->Max = 16;
		for (ifile = 0; ifile < BoxRazm; ifile++) {
			BoxXY[ifile].x = 0;
			BoxXY[ifile].y = 0;
			BoxXY[ifile].N = 0;
			BoxMS[ifile].ms = 0;
		}

		BoxXY[0].x = FullSigma[0].x;
		BoxXY[0].y = FullSigma[0].y;
		BoxMS[0].ms = FullSigma[0].ms;
		BoxMM[0].mm = FullSigma[0].mm;
		BoxXY[0].N++;
		FullSigma[0].mark = 0;

		NomBoxXY = 0;
		int StepForBoxX, StepForBoxY = PorogPx / 2;  //Создание сетки секторов
		for (i = 0; i < ((StrToInt(Form1->Edit43->Text) + PorogPx - 1) / PorogPx); i++) {
			StepForBoxX = PorogPx / 2;
			for (j = 0; j < ((StrToInt(Form1->Edit34->Text) + PorogPx - 1) / PorogPx); j++) {
				BoxXY[NomBoxXY].x = StepForBoxX;
				StepForBoxX += PorogPx;
				BoxXY[NomBoxXY++].y = StepForBoxY;
			}
			StepForBoxY += PorogPx;
		}

		ProgressBar1->Position++;
		//Сортировка по координатам (массив наборов, например 5 наборов по 35 (BoxXY[i].N) точек)
		for (ifile = 0; ifile < Meter; ifile++) {
			fl5 = false;
			for (i = 0; i < NomBoxXY; i++) {
				if (((FullSigma[ifile].x) < (BoxXY[i].x + PorogPx / 2)) && ((FullSigma[ifile].x) >= (BoxXY[i].x - PorogPx / 2)) &&
					((FullSigma[ifile].y) < (BoxXY[i].y + PorogPx / 2)) && ((FullSigma[ifile].y) >= (BoxXY[i].y - PorogPx / 2)))
				{
					BoxXY[i].N++;
					fl5 = true;
					FullSigma[ifile].mark = i;
				}
				else {
					if ((i + 1) == NomBoxXY && fl5 == false) {
						BoxXY[NomBoxXY].N++;
						FullSigma[ifile].mark = NomBoxXY;
					}
				}
			}
		}
		ProgressBar1->Position++;


		//Картинка
		Chart2->Visible = false;
		Chart2->Walls->Visible = false;
		Chart2->Width = StrToFloat(Edit41->Text);
		Chart2->Height = Chart2->Width * StrToFloat(Edit43->Text) / StrToFloat(Edit34->Text);

		Chart2->Axes->Left->Minimum = -10000;
		Chart2->Axes->Left->Maximum = StrToFloat(Edit2->Text);
		Chart2->Axes->Left->Minimum = 0;

		Chart2->Axes->Bottom->Minimum = -10000;
		Chart2->Axes->Bottom->Maximum = StrToFloat(Edit1->Text);
		Chart2->Axes->Bottom->Minimum = 0;

		bm2->PixelFormat = pf32bit;
		bm2->Width = Chart2->Width;
		bm2->Height = Chart2->Height;
		Chart2->Series[0]->Clear();
		Chart2->Series[1]->Clear();

		float SigMaxOne, SigMinOne;
		float SigMaxTwo, SigMinTwo;
		for (ifile = 0; ifile < NomBoxXY; ifile++) {
			SigMaxOne = 0;
			SigMinOne = 100;
			for (i = 0; i < Meter; i++) {
				if (FullSigma[i].mark == ifile) {
					if (SigMaxOne < FullSigma[i].SigX)
						SigMaxOne = FullSigma[i].SigX;
					if (SigMinOne > FullSigma[i].SigX)
						SigMinOne = FullSigma[i].SigX;
				}
			}

			if (SigMinOne != 100 && SigMaxOne != 0) {
				if ((StrToFloat(Edit20->Text) <= SigMaxOne) && (StrToFloat(Edit21->Text) >= SigMaxOne))
					Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, SigMaxOne * StrToFloat(Edit44->Text), "", RGB(0, 255, 0));
				else
					Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, SigMaxOne * StrToFloat(Edit44->Text), "", RGB(255, 0, 0));

				if ((StrToFloat(Edit20->Text) <= SigMinOne) && (StrToFloat(Edit21->Text) >= SigMinOne))
					Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, SigMinOne * StrToFloat(Edit44->Text), "", RGB(0, 255, 0));
				else
					Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, SigMinOne * StrToFloat(Edit44->Text), "", RGB(255, 0, 0));
			}
		}

		Chart2->Title->Text->Text = AnsiString("MIN/MAX\r\n" + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")").c_str();
		Chart2->Refresh();
		Application->ProcessMessages();
		String NameGraf3 = (Edit23->Text + "\\Sigma - " + bufftime + " - Graf BUBBLE Sky1.bmp");
		Chart2->SaveToBitmapFile(NameGraf3);
		ProgressBar1->Position++;

		OpenWord(false);
		AddDoc();

		SetTextFormat(14, 1, 0, 0, 1, 1.5);
		AddParagraph("Отчет по параметрам фокусировки по небу");

		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label27->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit32->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label28->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit33->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label29->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit34->Text + "x" + Edit43->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label43->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit45->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph("Оптимальная сигма:"); 			SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  от " + Edit20->Text + "  до " + Edit21->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label32->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Memo2->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph("Название первого кадра:"); 		SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + FullSigma[0].Names);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph("Название файлов для отчета:");

		SetTextFormat(12, 0, 0, 0, 0, 0.1);
		for (i = 0; i < OpenDialog3->Files->Count; i++)
			AddParagraph("  " + IntToStr(i + 1) + ". " + OpenDialog3->Files->Strings[i]);

		SetTextFormat(12, 1, 1, 0, 0, 0.02); AddParagraphNo("\n" + Label30->Caption); 			SetTextFormat(12, 0, 0, 0, 0, 0.02); AddParagraph("  " + Edit35->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.02); AddParagraphNo(Label33->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 0.02); AddParagraph("  " + Edit36->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.02); AddParagraphNo(Label31->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 0.02); AddParagraph("  " + Edit37->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.02); AddParagraphNo("Дата формирования отчета:"); 	SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + buffDate);

		//	AddParagraph("____________________________________________");
		SetTextFormat(14, 1, 0, 0, 1, 1);
		AddParagraph("Результат");
		SetTextFormat(12, 0, 0, 0, 0, 0);

		// Расчет оптимальных
		int AllGoodPointX = 0, AllGoodPointY = 0;
		for (i = 0; i < Meter; i++) {
			if ((StrToFloat(Edit20->Text) <= FullSigma[i].SigX) && (StrToFloat(Edit21->Text) >= FullSigma[i].SigX))
				AllGoodPointX++;
			if ((StrToFloat(Edit20->Text) <= FullSigma[i].SigY) && (StrToFloat(Edit21->Text) >= FullSigma[i].SigY))
				AllGoodPointY++;
		}

		SetTextFormat(12, 0, 0, 0, 0, 0.02);
		AddParagraph("Всего объектов:	" + IntToStr(Meter) + "\nИз них оптимальных:\nпо оси Х -	" + IntToStr(AllGoodPointX) + "	( " + FloatToStr(FixRoundTo((float)(AllGoodPointX * 100) / Meter, -3)) + " % )");
		SetTextFormat(12, 0, 0, 0, 0, 1);
		AddParagraph("по оси Y -	" + IntToStr(AllGoodPointY) + "	( " + FloatToStr(FixRoundTo((float)(AllGoodPointY * 100) / Meter, -3)) + " % )");
		SetTextFormat(12, 0, 0, 0, 0, 0.02);
		// Расчет медианы
		float* MassSortX = new float[Meter];
		float* MassSortY = new float[Meter];
		for (i = 0; i < Meter; i++) {
			MassSortX[i] = FullSigma[i].SigX;
			MassSortY[i] = FullSigma[i].SigY;
		}

		qsort(MassSortY, Meter, sizeof(float), qfloatX);
		AddParagraph("Медиана (значение параметра Sigma):");
		if (Meter % 2 == 0)  //четное
			AddParagraph("по оси Х -	" + FloatToStr(FixRoundTo((float)(MassSortX[Meter / 2] + MassSortX[Meter / 2 - 1]) / 2, -3)) + "\nпо оси Y -	" + FloatToStr(FixRoundTo((float)(MassSortY[Meter / 2] + MassSortY[Meter / 2 - 1]) / 2, -3)));
		else   //не четное
			AddParagraph("по оси Х -	" + FloatToStr(FixRoundTo((float)MassSortX[Meter / 2], -3)) + "\nпо оси Y -	" + FloatToStr(FixRoundTo((float)MassSortY[Meter / 2], -3)));

		delete[] MassSortX;
		delete[] MassSortY;

		AddParagraph("\f");
		FlagClose2 = true;
		ProgressBar1->Position++;
		Chart2->PaintTo(bm2->Canvas->Handle, 0, 0);

		//----------------------------------------------------
		Chart2->Series[0]->Clear();
		Chart2->Series[1]->Clear();
		for (ifile = 0; ifile < NomBoxXY; ifile++) {
			SigMaxOne = 0;
			SigMinOne = 100;
			for (i = 0; i < Meter; i++) {
				if (FullSigma[i].mark == ifile) {
					if (SigMaxOne < FullSigma[i].SigX)
						SigMaxOne = FullSigma[i].SigX;
					if (SigMinOne > FullSigma[i].SigX)
						SigMinOne = FullSigma[i].SigX;
				}
			}

			SigMaxTwo = 0;
			SigMinTwo = 100;
			for (i = 0; i < Meter; i++) {
				if (FullSigma[i].mark == ifile) {
					if (SigMaxTwo < FullSigma[i].SigX && SigMaxOne != FullSigma[i].SigX && BoxXY[ifile].N>2)
						SigMaxTwo = FullSigma[i].SigX;
					if (SigMinTwo > FullSigma[i].SigX && SigMinOne != FullSigma[i].SigX && BoxXY[ifile].N > 2)
						SigMinTwo = FullSigma[i].SigX;
				}
			}

			if (SigMinTwo != 100 && SigMaxTwo != 0) {
				if ((StrToFloat(Edit20->Text) <= SigMaxTwo) && (StrToFloat(Edit21->Text) >= SigMaxTwo))
					Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, SigMaxTwo * StrToFloat(Edit44->Text), "", RGB(0, 255, 0));
				else
					Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, SigMaxTwo * StrToFloat(Edit44->Text), "", RGB(255, 0, 0));

				if ((StrToFloat(Edit20->Text) <= SigMinTwo) && (StrToFloat(Edit21->Text) >= SigMinTwo))
					Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, SigMinTwo * StrToFloat(Edit44->Text), "", RGB(0, 255, 0));
				else
					Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, SigMinTwo * StrToFloat(Edit44->Text), "", RGB(255, 0, 0));
			}
		}

		Chart2->Title->Text->Text = AnsiString("MIN/MAX без крайних значений по каждому сектору\r\n" + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")").c_str();
		Chart2->Refresh();
		Application->ProcessMessages();
		NameGraf3 = (Edit23->Text + "\\Sigma - " + bufftime + " - Graf BUBBLE Sky3.bmp");
		Chart2->SaveToBitmapFile(NameGraf3);

		FlagClose2 = true;
		ProgressBar1->Position++;
		//----------------------------------------------------

		//----------------------------------------------------
		AddParagraph("Графики отражающие среднее значение параметра сигма по центральным осям Х и У, частичное представление предыдущего графика.\n");
		Chart3->Visible = false;
		Chart3->Walls->Visible = false;
		Chart3->Width = StrToInt(Edit41->Text);
		Chart3->Height = StrToInt(Edit40->Text);
		Chart3->Series[0]->Clear();
		Chart3->Series[1]->Clear();
		Chart3->Series[2]->Clear();
		Chart3->Series[3]->Clear();

		Chart3->Series[2]->AddXY(-20, StrToFloat(Edit20->Text), 0, RGB(123, 0, 28));
		Chart3->Series[2]->AddXY(StrToInt(Form1->Edit34->Text) + 20, StrToFloat(Edit20->Text), "", RGB(123, 0, 28));

		Chart3->Series[3]->AddXY(-20, StrToFloat(Edit21->Text), 0, RGB(123, 0, 28));
		Chart3->Series[3]->AddXY(StrToInt(Form1->Edit34->Text) + 20, StrToFloat(Edit21->Text), "", RGB(123, 0, 28));

		Chart3->Legend->Visible = false;
		Chart3->Refresh();

		for (ifile = 0; ifile < NomBoxXY; ifile++) {
			SigMaxOne = 0;
			for (i = 0; i < Meter; i++)
				if (FullSigma[i].mark == ifile)
					SigMaxOne += FullSigma[i].SigX;

			if (((StrToInt(Form1->Edit43->Text) / 2) < (BoxXY[ifile].y + PorogPx / 2)) && ((StrToInt(Form1->Edit43->Text) / 2) >= (BoxXY[ifile].y - PorogPx / 2)) && BoxXY[ifile].N >= StrToInt(Form1->Edit51->Text))
				if (BoxXY[ifile].x > StrToInt(Form1->Edit34->Text))
					Chart3->Series[0]->AddXY(StrToInt(Form1->Edit34->Text) - 1, SigMaxOne / BoxXY[ifile].N, StrToInt(Form1->Edit34->Text), RGB(30, 144, 255));
				else
					Chart3->Series[0]->AddXY(BoxXY[ifile].x, SigMaxOne / BoxXY[ifile].N, BoxXY[ifile].x, RGB(30, 144, 255));
		}

		Chart3->Title->Text->Text = AnsiString("Значение сигмы по центру матрицы (ось X)\r\n" + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")\r\n ").c_str();
		Chart3->Axes->Bottom->AutomaticMaximum = false;
		Chart3->Axes->Bottom->Minimum = -10000;
		Chart3->Axes->Bottom->Maximum = StrToInt(Form1->Edit34->Text);

		Chart3->Axes->Bottom->AutomaticMinimum = false;
		Chart3->Axes->Bottom->Minimum = 0;

		Chart3->Axes->Left->AutomaticMaximum = false;
		Chart3->Axes->Left->AutomaticMinimum = false;
		Chart3->Axes->Left->Minimum = -10000;
		Chart3->Axes->Left->Maximum = StrToFloat(Edit31->Text);
		Chart3->Axes->Left->Minimum = StrToFloat(Edit30->Text);
		Chart3->Axes->Left->Increment = 0.1;
		Chart3->Axes->Bottom->Title->Caption = "\r\nКоордината по оси X";
		Chart3->Axes->Left->Title->Caption = "Sigma";

		Chart3->Refresh();
		Application->ProcessMessages();
		NameGraf3 = (Edit23->Text + "\\Sigma - " + bufftime + " - Graf Sky5.bmp");
		Chart3->SaveToBitmapFile(NameGraf3);

		AddPicture(NameGraf3);
		ProgressBar1->Position++;
		//----------------------------------------------------

		Chart3->Series[0]->Clear();
		Chart3->Series[2]->Clear();
		Chart3->Series[3]->Clear();
		Chart3->Refresh();

		Chart3->Series[2]->AddXY(-20, StrToFloat(Edit20->Text), 0, RGB(123, 0, 28));
		Chart3->Series[2]->AddXY(StrToInt(Form1->Edit43->Text) + 20, StrToFloat(Edit20->Text), "", RGB(123, 0, 28));

		Chart3->Series[3]->AddXY(-20, StrToFloat(Edit21->Text), 0, RGB(123, 0, 28));
		Chart3->Series[3]->AddXY(StrToInt(Form1->Edit43->Text) + 20, StrToFloat(Edit21->Text), "", RGB(123, 0, 28));

		Application->ProcessMessages();

		for (ifile = 0; ifile < NomBoxXY; ifile++)
		{
			SigMaxOne = 0;
			for (i = 0; i < Meter; i++)
				if (FullSigma[i].mark == ifile)
					SigMaxOne += FullSigma[i].SigX;

			if (((StrToInt(Form1->Edit34->Text) / 2) < (BoxXY[ifile].x + PorogPx / 2)) && ((StrToInt(Form1->Edit34->Text) / 2) >= (BoxXY[ifile].x - PorogPx / 2)) && BoxXY[ifile].N >= StrToInt(Form1->Edit51->Text))
				if (BoxXY[ifile].y > StrToInt(Form1->Edit43->Text))
					Chart3->Series[0]->AddXY(StrToInt(Form1->Edit43->Text) - 1, SigMaxOne / BoxXY[ifile].N, StrToInt(Form1->Edit43->Text), RGB(30, 144, 255));
				else
					Chart3->Series[0]->AddXY(BoxXY[ifile].y, SigMaxOne / BoxXY[ifile].N, BoxXY[ifile].y, RGB(30, 144, 255));
		}

		Chart3->Title->Text->Text = AnsiString("Значение сигмы по центру матрицы (ось Y)\r\n" + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")\r\n ").c_str();
		Chart3->Axes->Bottom->AutomaticMaximum = false;
		Chart3->Axes->Bottom->Minimum = -10000;
		Chart3->Axes->Bottom->Maximum = StrToInt(Form1->Edit43->Text);

		Chart3->Axes->Bottom->AutomaticMinimum = false;
		Chart3->Axes->Bottom->Minimum = 0;

		Chart3->Axes->Left->AutomaticMaximum = false;
		Chart3->Axes->Left->AutomaticMinimum = false;

		Chart3->Axes->Left->Minimum = -10000;
		Chart3->Axes->Left->Maximum = StrToFloat(Edit31->Text);
		Chart3->Axes->Left->Minimum = StrToFloat(Edit30->Text);

		Chart3->Axes->Left->Increment = 0.1;
		Chart3->Axes->Bottom->Title->Caption = "\r\nКоордината по оси Y";
		Chart3->Axes->Left->Title->Caption = "Sigma";
		NameGraf3 = (Edit23->Text + "\\Sigma - " + bufftime + " - Graf Sky6.bmp");
		Chart3->SaveToBitmapFile(NameGraf3);

		AddPicture(NameGraf3);
		ProgressBar1->Position++;
		//---------------------------------------------------- 1
		//Гистограммы!

		int IsMax = 0;
		int IsMin = 10000000;

		if (CheckBox16->Checked == true) {
			for (i = 0; i < Meter; i++) {
				if (FullSigma[i].Is > IsMax)
					IsMax = FullSigma[i].Is;
				if (FullSigma[i].Is < IsMin)
					IsMin = FullSigma[i].Is;
			}
			IsMax = (IsMax - IsMin) / 4;
		}
		else
			IsMax = StrToInt(Edit56->Text);

		Chart6->Walls->Visible = false;
		Chart6->Width = StrToInt(Edit41->Text);
		Chart6->Height = StrToInt(Edit40->Text);
		Chart6->Refresh();

		TColor CColor[4] = { RGB(32,159,223), RGB(153,202,83), RGB(246,166,37), RGB(191,89,62) };
		TBarSeries** BarSeries = new TBarSeries * [4];
		for (int i = 0; i < 4; i++) {
			BarSeries[i] = new TBarSeries(Chart6);
			Chart6->AddSeries(BarSeries[i]);
			BarSeries[i]->Marks->Visible = false;
			BarSeries[i]->Color = CColor[i];
		}

		int Ustep = IsMax;
		int Dstep = 1;

		int SIGstepInt = 0;
		float SIGstep = (float)SIGstepInt / 10;

		bool Flag4For = true;
		int NewMeter = 0;
		int Shet1 = 0;
		int Savesh = 0; //Против зацикливания на всякий случай
		while (Flag4For && SIGstep < StrToFloat(Edit48->Text)) {
			if (SIGstep >= StrToFloat(Edit47->Text)) {
				for (j = 0; j < 4; j++) {
					for (i = 0; i < Meter; i++) {
						if ((FullSigma[i].SigX >= SIGstep) && (FullSigma[i].SigX < (SIGstep + 0.1))) {
							if (j != 3) {
								if ((FullSigma[i].Is >= Dstep) && (FullSigma[i].Is < Ustep)) //Ограничения с двух сторон
								{
									NewMeter++;
									Shet1++;
								}
							}
							else
								if ((FullSigma[i].Is >= Dstep))      //Ограничение для последнего значения яркости с одной стороны
								{
									NewMeter++;
									Shet1++;
								}
						}
					}

					BarSeries[j]->AddXY(SIGstep, Shet1);
					Dstep = Ustep;

					if (CheckBox16->Checked == true)
						Ustep = Ustep + IsMax;
					else
						Ustep = Ustep * 2;

					Shet1 = 0;
					Savesh++;
					if (Savesh > 100) break;
				}
			}
			Flag4For = false;

			if (NewMeter < Meter)
			{
				Flag4For = true;
				SIGstepInt++;     //Приращение значения сигмы (в чистом виде значение int делится на 10)
				SIGstep = (float)SIGstepInt / 10;
				Ustep = IsMax;
				Dstep = 1;
			}

		}

		Ustep = IsMax;
		if (CheckBox16->Checked == true) {
			BarSeries[0]->Title = "< " + IntToStr(Ustep);
			BarSeries[1]->Title = IntToStr(Ustep) + " - " + IntToStr(Ustep * 2);
			BarSeries[2]->Title = IntToStr(Ustep * 2) + " - " + IntToStr(Ustep * 3);
			BarSeries[3]->Title = "> " + IntToStr(Ustep * 3);
		}
		else {
			BarSeries[0]->Title = "< " + IntToStr(Ustep);
			BarSeries[1]->Title = IntToStr(Ustep) + " - " + IntToStr(Ustep * 2);
			BarSeries[2]->Title = IntToStr(Ustep * 2) + " - " + IntToStr(Ustep * 4);
			BarSeries[3]->Title = "> " + IntToStr(Ustep * 4);
		}
		Chart6->Axes->Bottom->Minimum = -10000;
		Chart6->Axes->Bottom->Maximum = StrToFloat(Edit48->Text) + 0.05;
		Chart6->Axes->Bottom->Minimum = StrToFloat(Edit47->Text) - 0.05;
		Chart6->Refresh();

		Application->ProcessMessages();
		Chart6->LeftAxis->Title->Caption = AnsiString("Интегральная яркость, град. АЦП").c_str();
		Chart6->Title->Text->Text = AnsiString("Гистограмма параметра SigmaX по интегральной яркости\r\n" + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")\r\n ").c_str();
		NameGraf3 = (Edit23->Text + "\\Sigma - " + bufftime + " - Graf SkyGX.bmp");
		Chart6->SaveToBitmapFile(NameGraf3);
		AddPicture(NameGraf3);

		for (int i = 0; i < 4; i++)
			BarSeries[i]->Clear();
		ProgressBar1->Position++;
		//---------------------------------------- Гистограмма № 2 по У
		Chart6->Walls->Visible = false;
		Chart6->Width = StrToInt(Edit41->Text);
		Chart6->Height = StrToInt(Edit40->Text);
		Chart6->Refresh();

		Ustep = IsMax;
		Dstep = 1;

		SIGstepInt = 0;
		SIGstep = (float)SIGstepInt / 10;

		Flag4For = true;
		NewMeter = Shet1 = Savesh = 0;
		while (Flag4For && SIGstep < StrToFloat(Edit48->Text))
		{
			if (SIGstep >= StrToFloat(Edit47->Text)) {
				for (j = 0; j < 4; j++)
				{
					for (i = 0; i < Meter; i++) {
						if ((FullSigma[i].SigY >= SIGstep) && (FullSigma[i].SigY < (SIGstep + 0.1))) {
							if (j != 3) {
								if ((FullSigma[i].Is >= Dstep) && (FullSigma[i].Is < Ustep)) //Ограничения с двух сторон
								{
									NewMeter++;
									Shet1++;
								}
							}
							else
								if ((FullSigma[i].Is >= Dstep))      //Ограничение для последнего значения яркости с одной стороны
								{
									NewMeter++;
									Shet1++;
								}
						}
					}

					BarSeries[j]->AddXY(SIGstep, Shet1);
					Dstep = Ustep;

					if (CheckBox16->Checked == true)
						Ustep = Ustep + IsMax;
					else
						Ustep = Ustep * 2;

					Shet1 = 0;
					Savesh++;
					if (Savesh > 100) break;
				}
			}

			Flag4For = false;
			if (NewMeter < Meter)
			{
				Flag4For = true;
				SIGstepInt++;     //Приращение значения сигмы (в чистом виде значение int делится на 10)
				SIGstep = (float)SIGstepInt / 10;
				Ustep = IsMax;
				Dstep = 1;
			}
		}

		Ustep = IsMax;
		if (CheckBox16->Checked == true) {
			BarSeries[0]->Title = "< " + IntToStr(Ustep);
			BarSeries[1]->Title = IntToStr(Ustep) + " - " + IntToStr(Ustep * 2);
			BarSeries[2]->Title = IntToStr(Ustep * 2) + " - " + IntToStr(Ustep * 3);
			BarSeries[3]->Title = "> " + IntToStr(Ustep * 3);
		}
		else {
			BarSeries[0]->Title = "< " + IntToStr(Ustep);
			BarSeries[1]->Title = IntToStr(Ustep) + " - " + IntToStr(Ustep * 2);
			BarSeries[2]->Title = IntToStr(Ustep * 2) + " - " + IntToStr(Ustep * 4);
			BarSeries[3]->Title = "> " + IntToStr(Ustep * 4);
		}
		Chart6->Axes->Bottom->Minimum = -10000;
		Chart6->Axes->Bottom->Maximum = StrToFloat(Edit48->Text) + 0.05;
		Chart6->Axes->Bottom->Minimum = StrToFloat(Edit47->Text) - 0.05;
		Chart6->Refresh();

		Application->ProcessMessages();
		Chart6->LeftAxis->Title->Caption = AnsiString("Интегральная яркость, град. АЦП").c_str();
		Chart6->Title->Text->Text = AnsiString("Гистограмма параметра SigmaY по интегральной яркости\r\n" + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")\r\n ").c_str();
		NameGraf3 = (Edit23->Text + "\\Sigma - " + bufftime + " - Graf SkyGY.bmp");
		Chart6->SaveToBitmapFile(NameGraf3);
		AddPicture(NameGraf3);

		for (int i = 0; i < 4; i++)
			BarSeries[i]->Clear();
		ProgressBar1->Position++;
		//---------------------------------------- 3
		Chart6->Walls->Visible = false;
		Chart6->Width = StrToInt(Edit41->Text);
		Chart6->Height = StrToInt(Edit40->Text);
		Chart6->Refresh();

		Chart6->RemoveAllSeries();
		for (int i = 0; i < 2; i++) {
			Chart6->AddSeries(BarSeries[i]);
			BarSeries[i]->Marks->Visible = false;
		}

		SIGstepInt = 0;
		SIGstep = (float)SIGstepInt / 10;

		int Shet2;
		Shet1 = Shet2 = 0;

		BarSeries[0]->Color = CColor[0];
		BarSeries[1]->Color = CColor[3];

		while (SIGstep < StrToFloat(Edit48->Text)) {
			if (SIGstep >= StrToFloat(Edit47->Text)) {
				for (i = 0; i < Meter; i++) {
					if ((FullSigma[i].SigX >= SIGstep) && (FullSigma[i].SigX < (SIGstep + 0.1)))  Shet1++;
					if ((FullSigma[i].SigY >= SIGstep) && (FullSigma[i].SigY < (SIGstep + 0.1)))  Shet2++;
				}

				BarSeries[0]->AddXY(SIGstep, Shet1);
				BarSeries[1]->AddXY(SIGstep, Shet2);
				Shet1 = Shet2 = 0;
			}
			SIGstepInt++;
			SIGstep = (float)SIGstepInt / 10;
		}

		BarSeries[0]->Title = "Значения по Х";
		BarSeries[1]->Title = "Значения по Y";
		Chart6->Refresh();

		Application->ProcessMessages();
		Chart6->Axes->Bottom->Minimum = -10000;
		Chart6->Axes->Bottom->Maximum = StrToFloat(Edit48->Text) + 0.05;
		Chart6->Axes->Bottom->Minimum = StrToFloat(Edit47->Text) - 0.05;
		Chart6->LeftAxis->Title->Caption = AnsiString("").c_str();
		Chart6->Title->Text->Text = AnsiString("Количество значений SigmaХ/SigmaY\r\n" + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")\r\n ").c_str();
		NameGraf3 = (Edit23->Text + "\\Sigma - " + bufftime + " - Graf SkyXY.bmp");
		Chart6->SaveToBitmapFile(NameGraf3);
		AddPicture(NameGraf3);
		Chart6->RemoveAllSeries();

		delete[] BarSeries;
		ProgressBar1->Position++;
		//----------------------------------------------------

		Chart4->Walls->Visible = false;
		Chart4->Width = StrToInt(Edit41->Text);
		Chart4->Height = StrToInt(Edit40->Text);
		Chart4->Series[0]->Clear();
		Chart4->Series[1]->Clear();
		Chart4->Refresh();
		Application->ProcessMessages();

		MaxIsForAxis = 0;
		for (i = 0; i < Meter; i++) {
			if (MaxIsForAxis < FullSigma[i].Is)  MaxIsForAxis = FullSigma[i].Is;

			if (StrToFloat(Edit20->Text) <= FullSigma[i].SigX && StrToFloat(Edit21->Text) >= FullSigma[i].SigX)
				Chart4->Series[0]->AddXY(FullSigma[i].Is, FullSigma[i].SigX);
			else
				Chart4->Series[1]->AddXY(FullSigma[i].Is, FullSigma[i].SigX);
		}
		Chart4->Title->Text->Text = AnsiString("Значение параметра sigma X от интегральной яркости\r\n" + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")\r\n ").c_str();

		Chart4->Axes->Left->Increment = 0.1;
		Chart4->Axes->Bottom->Title->Caption = "\r\nИнтегральная яркость, град. АЦП";
		Chart4->Axes->Left->Title->Caption = "Sigma";
		Chart4->Axes->Left->AutomaticMaximum = false;
		Chart4->Axes->Left->Maximum = StrToFloat(Edit48->Text);

		Chart4->Axes->Left->AutomaticMinimum = false;
		Chart4->Axes->Left->Minimum = 0;

		Chart4->Axes->Bottom->AutomaticMaximum = false;
		Chart4->Axes->Bottom->Maximum = MaxIsForAxis;

		Chart4->Axes->Bottom->AutomaticMinimum = false;
		Chart4->Axes->Bottom->Minimum = 0;

		Chart4->Axes->Bottom->Increment = 100;

		Application->ProcessMessages();

		NameGraf3 = (Edit23->Text + "\\Sigma - " + bufftime + " - Graf Sky7.bmp");
		Chart4->SaveToBitmapFile(NameGraf3);

		AddPicture(NameGraf3);
		ProgressBar1->Position++;

		//----------------------------------------------------
		Chart4->Walls->Visible = false;
		Chart4->Width = StrToInt(Edit41->Text);
		Chart4->Height = StrToInt(Edit40->Text);
		Chart4->Series[0]->Clear();
		Chart4->Series[1]->Clear();
		Chart4->Refresh();
		Application->ProcessMessages();

		MaxIsForAxis = 0;
		for (i = 0; i < Meter; i++) {
			if (MaxIsForAxis < FullSigma[i].Is)  MaxIsForAxis = FullSigma[i].Is;

			if (StrToFloat(Edit20->Text) <= FullSigma[i].SigY && StrToFloat(Edit21->Text) >= FullSigma[i].SigY)
				Chart4->Series[0]->AddXY(FullSigma[i].Is, FullSigma[i].SigY);
			else
				Chart4->Series[1]->AddXY(FullSigma[i].Is, FullSigma[i].SigY);
		}
		Chart4->Title->Text->Text = AnsiString("Значение параметра sigma Y от интегральной яркости\r\n" + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")\r\n ").c_str();

		Chart4->Axes->Left->Increment = 0.1;
		Chart4->Axes->Bottom->Title->Caption = "\r\nИнтегральная яркость, град. АЦП";
		Chart4->Axes->Left->Title->Caption = "Sigma";
		Chart4->Axes->Left->AutomaticMaximum = false;
		Chart4->Axes->Left->Maximum = StrToFloat(Edit48->Text);

		Chart4->Axes->Left->AutomaticMinimum = false;
		Chart4->Axes->Left->Minimum = 0;

		Chart4->Axes->Bottom->AutomaticMaximum = false;
		Chart4->Axes->Bottom->Maximum = MaxIsForAxis;

		Chart4->Axes->Bottom->AutomaticMinimum = false;
		Chart4->Axes->Bottom->Minimum = 0;

		Chart4->Axes->Bottom->Increment = 100;

		Application->ProcessMessages();

		NameGraf3 = (Edit23->Text + "\\Sigma - " + bufftime + " - Graf Sky8.bmp");
		Chart4->SaveToBitmapFile(NameGraf3);

		AddPicture(NameGraf3);
		AddParagraph("\f");
		ProgressBar1->Position++;
		//----------------------------------------------------
		if (CheckBoxPhotometry->Checked == true) {
			ChartPhotogrammetry->Walls->Visible = false;
			ChartPhotogrammetry->Width = StrToInt(Edit41->Text);
			ChartPhotogrammetry->Height = StrToInt(Edit40->Text);
			ChartPhotogrammetry->Series[0]->Clear();
			ChartPhotogrammetry->Series[1]->Clear();
			ChartPhotogrammetry->Series[2]->Clear();
			ChartPhotogrammetry->Series[3]->Clear();
			ChartPhotogrammetry->Refresh();
			Application->ProcessMessages();

			//Отрисовка 3х линий с яркостями
			ChartPhotogrammetry->Series[1]->AddXY(StrToInt(Label_IOZ4->Caption), 0);
			ChartPhotogrammetry->Series[1]->AddXY(StrToInt(Label_IOZ4->Caption), 20);

			ChartPhotogrammetry->Series[2]->AddXY(StrToInt(Label_IOZ5->Caption), 0);
			ChartPhotogrammetry->Series[2]->AddXY(StrToInt(Label_IOZ5->Caption), 20);

			ChartPhotogrammetry->Series[3]->AddXY(StrToInt(Label_IOZ6->Caption), 0);
			ChartPhotogrammetry->Series[3]->AddXY(StrToInt(Label_IOZ6->Caption), 20);

			for (i = 0; i < Meter; i++)
				ChartPhotogrammetry->Series[0]->AddXY(FullSigma[i].IsF, FullSigma[i].SigX);
			ChartPhotogrammetry->Title->Text->Text = AnsiString("Значение параметра sigma X от интегральной яркости IsF\r\n" + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")\r\n ").c_str();

			ChartPhotogrammetry->Axes->Left->Increment = 0.1;
			ChartPhotogrammetry->Axes->Bottom->Title->Caption = "\r\nИнтегральная яркость IsF, град. АЦП";
			ChartPhotogrammetry->Axes->Left->Title->Caption = "Sigma";
			ChartPhotogrammetry->Axes->Left->AutomaticMaximum = false;
			ChartPhotogrammetry->Axes->Left->Maximum = StrToFloat(Edit48->Text);

			ChartPhotogrammetry->Axes->Left->AutomaticMinimum = false;
			ChartPhotogrammetry->Axes->Left->Minimum = 0;

			ChartPhotogrammetry->Axes->Bottom->AutomaticMaximum = false;
			ChartPhotogrammetry->Axes->Bottom->Maximum = StrToInt(Label_IOZ4->Caption) + StrToInt(Label_IOZ6->Caption);

			ChartPhotogrammetry->Axes->Bottom->AutomaticMinimum = false;
			ChartPhotogrammetry->Axes->Bottom->Minimum = 0;

			ChartPhotogrammetry->Axes->Bottom->Increment = 100;

			Application->ProcessMessages();

			NameGraf3 = (Edit23->Text + "\\Sigma - " + bufftime + " - Graf Sky IsF.bmp");
			ChartPhotogrammetry->SaveToBitmapFile(NameGraf3);

			AddPicture(NameGraf3);
			ProgressBar1->Position++;
		}

		//----------------------------------------------------

		Chart2->Series[0]->Clear();
		Chart2->Series[1]->Clear();

		for (i = 0; i < Meter; i++)
			if (FullSigma[i].SigX >= (StrToFloat(Edit21->Text) + 0.4))
				Series3->AddBubble(BoxXY[FullSigma[i].mark].x, BoxXY[FullSigma[i].mark].y, FullSigma[i].SigX * StrToFloat(Edit44->Text), "", RGB(255, 0, 0));

		for (i = 0; i < Meter; i++)
			if (FullSigma[i].SigX < (StrToFloat(Edit21->Text) + 0.4) && FullSigma[i].SigX >= (StrToFloat(Edit21->Text) + 0.2))
				Series3->AddBubble(BoxXY[FullSigma[i].mark].x, BoxXY[FullSigma[i].mark].y, FullSigma[i].SigX * StrToFloat(Edit44->Text), "", RGB(255, 165, 0));

		for (i = 0; i < Meter; i++)
			if (FullSigma[i].SigX < (StrToFloat(Edit21->Text) + 0.2) && FullSigma[i].SigX >(StrToFloat(Edit21->Text)))
				Series3->AddBubble(BoxXY[FullSigma[i].mark].x, BoxXY[FullSigma[i].mark].y, FullSigma[i].SigX * StrToFloat(Edit44->Text), "", RGB(255, 255, 0));

		for (i = 0; i < Meter; i++)
			if (FullSigma[i].SigX <= (StrToFloat(Edit21->Text)) && FullSigma[i].SigX >= (StrToFloat(Edit20->Text)))
				Series3->AddBubble(BoxXY[FullSigma[i].mark].x, BoxXY[FullSigma[i].mark].y, FullSigma[i].SigX * StrToFloat(Edit44->Text), "", RGB(0, 255, 0));

		for (i = 0; i < Meter; i++)
			if (FullSigma[i].SigX < (StrToFloat(Edit20->Text)) && FullSigma[i].SigX >= (StrToFloat(Edit20->Text) - 0.1))
				Series3->AddBubble(BoxXY[FullSigma[i].mark].x, BoxXY[FullSigma[i].mark].y, FullSigma[i].SigX * StrToFloat(Edit44->Text), "", RGB(66, 170, 255));

		for (i = 0; i < Meter; i++)
			if (FullSigma[i].SigX < (StrToFloat(Edit20->Text) - 0.1) && FullSigma[i].SigX >= (StrToFloat(Edit20->Text) - 0.2))
				Series3->AddBubble(BoxXY[FullSigma[i].mark].x, BoxXY[FullSigma[i].mark].y, FullSigma[i].SigX * StrToFloat(Edit44->Text), "", RGB(0, 87, 250));

		for (i = 0; i < Meter; i++)
			if (FullSigma[i].SigX < (StrToFloat(Edit20->Text) - 0.2))
				Series3->AddBubble(BoxXY[FullSigma[i].mark].x, BoxXY[FullSigma[i].mark].y, FullSigma[i].SigX * StrToFloat(Edit44->Text), "", RGB(153, 50, 204));

		Chart2->Title->Text->Text = AnsiString("Все значения по каждому сектору\r\n" + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")").c_str();
		Chart2->Refresh();
		Application->ProcessMessages();
		NameGraf3 = (Edit23->Text + "\\Sigma - " + bufftime + " - Graf BUBBLE Sky4.bmp");
		Chart2->SaveToBitmapFile(NameGraf3);

		FlagClose2 = true;
		AddParagraph("Все значения попавшие в сектор");
		AddPicture(NameGraf3);
		AddParagraph("\f");
		ProgressBar1->Position++;
		//----------------------------------------------------
		Chart2->Series[0]->Clear();
		Chart2->Series[1]->Clear();

		//Тут SigMaxOne используется как сумма
		int GoodPoint;
		Series3->Marks->Visible = true;
		for (ifile = 0; ifile < NomBoxXY; ifile++) {
			SigMaxOne = 0;
			GoodPoint = 0;
			for (i = 0; i < Meter; i++) {
				if (FullSigma[i].mark == ifile)
				{
					SigMaxOne += FullSigma[i].SigX;
					if ((StrToFloat(Edit20->Text) <= FullSigma[i].SigX) && (StrToFloat(Edit21->Text) >= FullSigma[i].SigX))
						GoodPoint++;
				}
			}

			if (SigMaxOne != 0 && BoxXY[ifile].N >= StrToInt(Edit51->Text))
				if (CheckBox11->Checked) {
					if (SigMaxOne / BoxXY[ifile].N < (StrToFloat(Edit20->Text) - 0.2))
						Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, StrToFloat(Edit44->Text), FloatToStr(FixRoundTo(SigMaxOne / BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint) + "/" + IntToStr(BoxXY[ifile].N), RGB(153, 50, 204));
					else
						if (SigMaxOne / BoxXY[ifile].N < (StrToFloat(Edit20->Text) - 0.1))
							Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, StrToFloat(Edit44->Text), FloatToStr(FixRoundTo(SigMaxOne / BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint) + "/" + IntToStr(BoxXY[ifile].N), RGB(0, 87, 250));
						else
							if (SigMaxOne / BoxXY[ifile].N < (StrToFloat(Edit20->Text)))
								Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, StrToFloat(Edit44->Text), FloatToStr(FixRoundTo(SigMaxOne / BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint) + "/" + IntToStr(BoxXY[ifile].N), RGB(66, 170, 255));
							else
								if (SigMaxOne / BoxXY[ifile].N <= (StrToFloat(Edit21->Text)))
									Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, StrToFloat(Edit44->Text), FloatToStr(FixRoundTo(SigMaxOne / BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint) + "/" + IntToStr(BoxXY[ifile].N), RGB(0, 255, 0));
								else
									if (SigMaxOne / BoxXY[ifile].N < (StrToFloat(Edit21->Text) + 0.2))
										Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, StrToFloat(Edit44->Text), FloatToStr(FixRoundTo(SigMaxOne / BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint) + "/" + IntToStr(BoxXY[ifile].N), RGB(255, 255, 0));
									else
										if (SigMaxOne / BoxXY[ifile].N < (StrToFloat(Edit21->Text) + 0.4))
											Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, StrToFloat(Edit44->Text), FloatToStr(FixRoundTo(SigMaxOne / BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint) + "/" + IntToStr(BoxXY[ifile].N), RGB(255, 165, 0));
										else
											if (SigMaxOne / BoxXY[ifile].N >= (StrToFloat(Edit21->Text) + 0.4))
												Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, StrToFloat(Edit44->Text), FloatToStr(FixRoundTo(SigMaxOne / BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint) + "/" + IntToStr(BoxXY[ifile].N), RGB(255, 0, 0));
				}
				else {
					if (SigMaxOne / BoxXY[ifile].N < (StrToFloat(Edit20->Text) - 0.2))
						Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, SigMaxOne * StrToFloat(Edit44->Text) / BoxXY[ifile].N, FloatToStr(FixRoundTo(SigMaxOne / BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint) + "/" + IntToStr(BoxXY[ifile].N), RGB(153, 50, 204));
					else
						if (SigMaxOne / BoxXY[ifile].N < (StrToFloat(Edit20->Text) - 0.1))
							Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, SigMaxOne * StrToFloat(Edit44->Text) / BoxXY[ifile].N, FloatToStr(FixRoundTo(SigMaxOne / BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint) + "/" + IntToStr(BoxXY[ifile].N), RGB(0, 87, 250));
						else
							if (SigMaxOne / BoxXY[ifile].N < (StrToFloat(Edit20->Text)))
								Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, SigMaxOne * StrToFloat(Edit44->Text) / BoxXY[ifile].N, FloatToStr(FixRoundTo(SigMaxOne / BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint) + "/" + IntToStr(BoxXY[ifile].N), RGB(66, 170, 255));
							else
								if (SigMaxOne / BoxXY[ifile].N <= (StrToFloat(Edit21->Text)))
									Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, SigMaxOne * StrToFloat(Edit44->Text) / BoxXY[ifile].N, FloatToStr(FixRoundTo(SigMaxOne / BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint) + "/" + IntToStr(BoxXY[ifile].N), RGB(0, 255, 0));
								else
									if (SigMaxOne / BoxXY[ifile].N < (StrToFloat(Edit21->Text) + 0.2))
										Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, SigMaxOne * StrToFloat(Edit44->Text) / BoxXY[ifile].N, FloatToStr(FixRoundTo(SigMaxOne / BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint) + "/" + IntToStr(BoxXY[ifile].N), RGB(255, 255, 0));
									else
										if (SigMaxOne / BoxXY[ifile].N < (StrToFloat(Edit21->Text) + 0.4))
											Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, SigMaxOne * StrToFloat(Edit44->Text) / BoxXY[ifile].N, FloatToStr(FixRoundTo(SigMaxOne / BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint) + "/" + IntToStr(BoxXY[ifile].N), RGB(255, 165, 0));
										else
											if (SigMaxOne / BoxXY[ifile].N >= (StrToFloat(Edit21->Text) + 0.4))
												Series3->AddBubble(BoxXY[ifile].x, BoxXY[ifile].y, SigMaxOne * StrToFloat(Edit44->Text) / BoxXY[ifile].N, FloatToStr(FixRoundTo(SigMaxOne / BoxXY[ifile].N, -3)) + "\r\n" + IntToStr(GoodPoint) + "/" + IntToStr(BoxXY[ifile].N), RGB(255, 0, 0));
				}

		}

		Chart2->Title->Text->Text = AnsiString("Среднее значение по сектору (оптимум/всего)\r\n" + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")\r\n\r\n\r\n").c_str();
		Chart2->Refresh();
		Application->ProcessMessages();
		NameGraf3 = (Edit23->Text + "\\Sigma - " + bufftime + " - Graf BUBBLE Sky5.bmp");
		Chart2->SaveToBitmapFile(NameGraf3);

		FlagClose2 = true;
		AddParagraph("Расчет среднего значения по каждому сектору");

		AddPicture(NameGraf3);
		ProgressBar1->Position++;
		//----------------------------------------------------
		//Легенда
		Chart2->Axes->Visible = false;
		Chart2->Width = 700;
		Chart2->Height = 150;
		Chart2->Series[0]->Clear();
		Chart2->Series[1]->Clear();
		Chart2->Refresh();
		Chart2->Title->Text->Text = AnsiString("Легенда\r\n").c_str();

		int FirstStepBubble = 5;
		int StepBubble = 15;
		int SBiter = FirstStepBubble;

		Series3->AddBubble(SBiter, 10, 2, (Edit47->Text) + " - " + FloatToStr(StrToFloat(Edit20->Text) - 0.2) + "\r", RGB(153, 50, 204));
		Series3->AddBubble(SBiter += StepBubble, 10, 2, FloatToStr(StrToFloat(Edit20->Text) - 0.2) + " - " + FloatToStr(StrToFloat(Edit20->Text) - 0.1) + "\r", RGB(0, 87, 250));
		Series3->AddBubble(SBiter += StepBubble, 10, 2, FloatToStr(StrToFloat(Edit20->Text) - 0.1) + " - " + (Edit20->Text) + "\r", RGB(66, 170, 255));
		Series3->AddBubble(SBiter += StepBubble, 10, 2, (Edit20->Text) + " - " + (Edit21->Text) + "\r", RGB(0, 255, 0));
		Series3->AddBubble(SBiter += StepBubble, 10, 2, (Edit21->Text) + " - " + FloatToStr(StrToFloat(Edit21->Text) + 0.2) + "\r", RGB(255, 255, 0));
		Series3->AddBubble(SBiter += StepBubble, 10, 2, FloatToStr(StrToFloat(Edit21->Text) + 0.2) + " - " + FloatToStr(StrToFloat(Edit21->Text) + 0.4) + "\r", RGB(255, 165, 0));
		Series3->AddBubble(SBiter += StepBubble, 10, 2, FloatToStr(StrToFloat(Edit21->Text) + 0.4) + " - " + (Edit48->Text) + "\r", RGB(255, 0, 0));

		Series3->AddBubble(1500, 10, 8, "0", RGB(255, 0, 0));

		Chart2->Axes->Bottom->AutomaticMaximum = false;
		Chart2->Axes->Bottom->AutomaticMinimum = false;
		Chart2->Axes->Bottom->Minimum = 0;
		Chart2->Axes->Bottom->Maximum = 100;
		Chart2->Axes->Left->AutomaticMaximum = true;
		Chart2->Axes->Left->AutomaticMinimum = true;

		Chart2->Refresh();
		Application->ProcessMessages();
		//		Chart2->PaintTo(bm2->Canvas->Handle,0,0);
		NameGraf3 = (Edit23->Text + "\\Sigma - " + bufftime + " - Graf Leg.bmp");
		Chart2->SaveToBitmapFile(NameGraf3);
		AddPicture(NameGraf3);

		Chart2->Width = StrToFloat(Edit41->Text);
		Chart2->Height = Chart2->Width * StrToFloat(Edit43->Text) / StrToFloat(Edit34->Text);
		Chart2->Axes->Visible = true;
		Chart2->Refresh();
		AddParagraph("\f");
		Series3->Marks->Visible = false;
		ProgressBar1->Position++;
		//----------------------------------------------------
		//INI
		SetTextFormat(8, 0, 0, 0, 0, 0.1);
		AddTable(2, 2);
		UnionCell(1, 1, 1, 2);
		SetCell(1, 1, "INI файл");
		SetCell(2, 1, "[Overall]\nX=" + Edit1->Text +
			"\nY=" + Edit2->Text +
			"\nDir=" + Edit3->Text +
			"\nMask=" + Edit4->Text +
			"\nCheckChoice=" + BoolToStr(CheckBox1->Checked) +
			"\nSaveDir=" + Edit23->Text +
			"\nFormatFrame=" + IntToStr(RadioGroup1->ItemIndex) +

			"\n[LOC-options]\nKofFilt=" + Edit10->Text +
			"\nCheckFilt=" + BoolToStr(CheckBox3->Checked) +
			"\nkofSKO1=" + Edit17->Text +
			"\npixMin=" + Edit11->Text +
			"\npixMax=" + Edit12->Text +
			"\nIsMin=" + Edit13->Text +
			"\nIsMax=" + Edit14->Text +

			"\n[Sigma-options]\nframe=" + Edit9->Text +
			"\nKofBin=" + Edit5->Text +
			"\nCheckBin=" + BoolToStr(CheckBox2->Checked) +
			"\nkofSKO2=" + Edit28->Text +
			"\npixMaxIs=" + Edit15->Text +
			"\npixMinIs=" + Edit16->Text +
			"\nCheckMostLight=" + BoolToStr(CheckBox4->Checked) +
			"\nCheck_Is-pix=" + IntToStr(ComboBoxPixIs->ItemIndex) +
			"\nforLG_step=" + Edit24->Text +
			"\nforLG_iter=" + Edit25->Text +
			"\nforLG_emptyPix=" + Edit26->Text +
			"\nforLG_MinPix=" + Edit52->Text +
			"\nCheckSector=" + BoolToStr(CheckBox14->Checked) +

			"\n[Report-options]\nNameDevice=" + Edit32->Text +
			"\nNomberDevice=" + Edit33->Text +
			"\nXmatr=" + Edit34->Text +
			"\nYmatr=" + Edit43->Text +
			"\nNameShooter=" + Edit35->Text +
			"\nDataShoot=" + Edit36->Text +
			"\nNameReporter=" + Edit37->Text +
			"\nComment=" + Memo2->Text +

			"\n[Photogrammetry-options]\nIsStar6=" + Edit_IOZ6->Text +
			"\nIsStar5=" + Edit_IOZ5->Text +
			"\nIsStar4=" + Edit_IOZ4->Text +
			"\nPhotometryCheck=" + BoolToStr(CheckBoxPhotometry->Checked));

		SetCell(2, 2, "[Research-options]\nId1ms=" + Edit8->Text +
			"\nId2ms=" + Edit6->Text +
			"\nId1mm=" + Edit22->Text +
			"\nId2mm=" + Edit7->Text +
			"\ninfKol=" + Edit18->Text +
			"\ntochnost=" + Edit19->Text +
			"\nFocDevice=" + Edit45->Text +
			"\nFocCol=" + Edit46->Text +
			"\nPixSize=" + Edit50->Text +
			"\nBaric=" + EditBaric->Text +
			"\nBaricCheck=" + BoolToStr(CheckBoxBaric->Checked) +
			"\nSlopeParam=" + IntToStr(ComboBoxMainPoint->ItemIndex) +

			"\n[Graphic-options]\nminXGraf=" + Edit27->Text +
			"\nmaxXGraf=" + Edit29->Text +
			"\nCheckGrafX=" + BoolToStr(CheckBox6->Checked) +
			"\nminYGraf=" + Edit30->Text +
			"\nmaxYGraf=" + Edit31->Text +
			"\nCheckGrafY=" + BoolToStr(CheckBox7->Checked) +
			"\nHeightGraf=" + Edit40->Text +
			"\nWidthGraf=" + Edit41->Text +
			"\nStep=" + Edit53->Text +
			"\nStepGrafX=" + Edit38->Text +
			"\nStepGrafY=" + Edit39->Text +
			"\nCheckStep=" + BoolToStr(CheckBox12->Checked) +
			"\nSizeBubble=" + Edit44->Text +
			"\nFixSizeBubble=" + BoolToStr(CheckBox11->Checked) +
			"\nBrGist=" + Edit56->Text +
			"\nCheckBrGist=" + BoolToStr(CheckBox16->Checked) +

			"\noptSigma1=" + Edit20->Text +
			"\noptSigma2=" + Edit21->Text +
			"\ndopSigma1=" + Edit47->Text +
			"\ndopSigma2=" + Edit48->Text +
			"\nCheckDopSigma=" + BoolToStr(CheckBox9->Checked) +
			"\nMu=" + Edit49->Text +
			"\nCheckMu=" + BoolToStr(CheckBox10->Checked) +
			"\nDeviationCenter=" + Edit55->Text +
			"\nCheckDC=" + BoolToStr(CheckBox15->Checked) +
			"\nSizeFrag=" + Edit54->Text +
			"\nNFrameMAX=" + EditNFmax->Text +
			"\nNFrameMIN=" + EditNFmin->Text +
			"\nCheckNF=" + BoolToStr(CheckBoxNF->Checked) +
			"\nCheckNFgraf=" + BoolToStr(CheckBoxNframe->Checked) +

			"\n[Sky-options]\nCheckSkyGraf=" + BoolToStr(CheckBox5->Checked) +
			"\nMinStarsInSector=" + Edit51->Text);

		//----------------------------------------------------
		ReportName = Edit23->Text + "\\Sigma - " + bufftime + " - SigmaGraph Sky.doc";
		SaveDoc(ReportName);
		CloseDoc();
		CloseWord();
		N3->Enabled = true;
		ProgressBar1->Position++;
		delete[] BoxXY;
		delete[] BoxMS;
		delete[] BoxMM;
		BoxXY = NULL;
		BoxMS = NULL;
		BoxMM = NULL;
	}
	else { //Обработка по ИОЗ            ---------------------------------------------------------------------------------------------------------------------------------иоз
		SBoxXY* BoxXY = new SBoxXY[Meter];
		SBoxXY* BoxMS = new SBoxXY[Meter];
		SBoxXY* BoxMM = new SBoxXY[Meter];
		//Обнаружение идентификаторов
		for (ifile = 0; ifile < Meter; ifile++) {
			IdentifFromName(AnsiString(FullSigma[ifile].Names).c_str(), AnsiString(Edit8->Text).c_str(), AnsiString(Edit6->Text).c_str(), &FullSigma[ifile].ms);
			IdentifFromName(AnsiString(FullSigma[ifile].Names).c_str(), AnsiString(Edit22->Text).c_str(), AnsiString(Edit7->Text).c_str(), &FullSigma[ifile].mm);
		}

		//Массивы по координатам и мс и мм
		//Очистка
		for (ifile = 0; ifile < Meter; ifile++) {
			BoxXY[ifile].x = 0;
			BoxXY[ifile].y = 0;
			BoxXY[ifile].N = 0;
			BoxMS[ifile].ms = 0;
		}

		//Присвоение первого значения
		BoxXY[0].x = FullSigma[0].x;
		BoxXY[0].y = FullSigma[0].y;
		BoxMS[0].ms = FullSigma[0].ms;
		BoxMM[0].mm = FullSigma[0].mm;
		BoxXY[0].N++;


		float pixSize = StrToFloat(Edit50->Text);
		float InfCol = StrToFloat(Edit18->Text);
		float FocBkz = StrToFloat(Edit45->Text);
		float FocCol = StrToFloat(Edit46->Text);

		int Xmatr = StrToInt(Edit34->Text);
		int Ymatr = StrToInt(Edit43->Text);
		int Sector = StrToInt(Edit19->Text);
		float SigmaInfX, SigmaInfY;
		float BubbleSize = StrToFloat(Edit44->Text);

		//Заполнение массивов  BoxXY  BoxMS  BoxMM
		ReportSort(FullSigma, Meter, PorogPx, BoxXY, &NomBoxXY, BoxMS, &NomBoxMS, BoxMM, &NomBoxMM);

		//Рандомное присвоение цветов
		for (i = 0; i < NomBoxMS; i++)
			BoxMS[i].ColGraf = (TColor)RGB(rand() % 255, rand() % 255, rand() % 255);

		String NameGraf2;

		//Заполнение документа Ворд
		OpenWord(false);
		AddDoc();
		SetTextFormat(14, 1, 0, 0, 1, 1);
		AddParagraph("Отчет по параметрам фокусировки\n");

		//AddParagraph("\f"); Переход на страницу
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label27->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit32->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label28->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit33->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label29->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit34->Text + "x" + Edit43->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph("Бесконечность коллиматора: "); 	SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit18->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label43->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit45->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label44->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit46->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph("Оптимальная сигма:"); 			SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  от " + Edit20->Text + "  до " + Edit21->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph(Label32->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Memo2->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph("Название первого кадра:"); 		SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + FullSigma[0].Names);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraph("Название файлов для отчета:");

		SetTextFormat(12, 0, 0, 0, 0, 0.1);
		for (i = 0; i < OpenDialog3->Files->Count; i++)
			AddParagraph("  " + IntToStr(i + 1) + ". " + OpenDialog3->Files->Strings[i]);

		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraphNo("\n" + Label30->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit35->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraphNo(Label33->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit36->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraphNo(Label31->Caption); 				SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + Edit37->Text);
		SetTextFormat(12, 1, 1, 0, 0, 0.1); AddParagraphNo("Дата формирования отчета:"); 	SetTextFormat(12, 0, 0, 0, 0, 1); AddParagraph("  " + buffDate);

		//Параметры графика для пузырей
		Chart2->Visible = false;
		Chart2->Walls->Visible = false;
		Chart2->Width = StrToFloat(Edit41->Text);
		Chart2->Height = Chart2->Width * StrToFloat(Edit2->Text) / StrToFloat(Edit1->Text);

		Chart2->Axes->Left->Minimum = -10000;
		Chart2->Axes->Bottom->Minimum = -10000;

		Chart2->Axes->Left->Maximum = StrToFloat(Edit2->Text);
		Chart2->Axes->Left->Minimum = 0;
		Chart2->Axes->Bottom->Maximum = StrToFloat(Edit1->Text);
		Chart2->Axes->Bottom->Minimum = 0;

		bm2->PixelFormat = pf32bit;
		bm2->Width = Chart2->Width;
		bm2->Height = Chart2->Height;

		float schet3;
		String MarksStr[50];
		float ConverMM;

		SetLineUnitAfter(1);

		//Аппроксимация  --------------------------------------------------------
#pragma region Аппроксимация

		float BorderMin;
		float BorderMax;

		if (Edit53->Text == "")
			Edit53->Text = 1;

		if (CheckBox6->Checked == true) {
			BorderMin = 10000;
			BorderMax = -10000;
			for (ifile = 0; ifile < Meter; ifile++) {
				if (FullSigma[ifile].mm < BorderMin)
					BorderMin = FullSigma[ifile].mm;

				if (FullSigma[ifile].mm > BorderMax)
					BorderMax = FullSigma[ifile].mm;
			}
		}
		else {
			BorderMin = StrToFloat(Edit27->Text);
			BorderMax = StrToFloat(Edit29->Text);
		}

		const float SigmaMin = StrToFloat(Edit20->Text);
		const float SigmaMax = StrToFloat(Edit21->Text);

		//Создание структуры для хранения результатов
		struct AprocRezStr
		{
			//При ax2+bx+c
			double A;
			double B;
			double C;

			double* K;
			double ExtX;
			double ExtY;

			double PerV1;
			double PerV2;
			double PerN1;
			double PerN2;

			double Rez;
			double RezMeanL;
			double RezMeanR;

			double X;

			double* massive;
		};

		AprocRezStr* AprocRezX = new AprocRezStr[NomBoxXY];  //Структура результатов для сигмы Х
		AprocRezStr* AprocRezY = new AprocRezStr[NomBoxXY];  //Структура результатов для сигмы У

		int MNK_deg = 2;
		if (EditMNK->Text != "")
			MNK_deg = StrToInt(EditMNK->Text); //Степень полинома аппроксимации
		else
			ShowMessage("Степень полинома аппроксимации не задана :(");

		float SpetApproc = 0.01;
		int SizeNewMas = (BorderMax - BorderMin) / SpetApproc;

		for (i = 0; i < NomBoxXY; i++) {
			AprocRezX[i].K = new double[MNK_deg + 1];
			AprocRezY[i].K = new double[MNK_deg + 1];

			AprocRezX[i].massive = new double[SizeNewMas];
			AprocRezY[i].massive = new double[SizeNewMas];
		}

		//Цикл МНК
		for (i = 0; i < NomBoxXY; i++) {

			double* AprocMasMM = new double[Meter];
			double* AprocMasX = new double[Meter];
			double* AprocMasY = new double[Meter];

			int schA = 0;   //Переписываем в новые массивы данные по каждой координате для расчета МНК
			for (ifile = 0; ifile < Meter; ifile++)
				if (((FullSigma[ifile].x + PorogPx) >= BoxXY[i].x) && ((FullSigma[ifile].x - PorogPx) <= BoxXY[i].x) &&
					((FullSigma[ifile].y + PorogPx) >= BoxXY[i].y) && ((FullSigma[ifile].y - PorogPx) <= BoxXY[i].y))
				{
					AprocMasMM[schA] = FullSigma[ifile].mm;
					AprocMasX[schA] = FullSigma[ifile].SigX;
					AprocMasY[schA++] = FullSigma[ifile].SigY;
				}

			//МНК Любой степени, получаем значения а, б и с для ax2+bx+c
			MNK(AprocMasMM, AprocMasX, MNK_deg, schA, AprocRezX[i].K);
			MNK(AprocMasMM, AprocMasY, MNK_deg, schA, AprocRezY[i].K);

			//Расчет результатов (нахождение экстремума для параболы)

			MakeMassive(BorderMin, SizeNewMas, MNK_deg, SpetApproc, AprocRezX[i].K, AprocRezX[i].massive);
			MakeMassive(BorderMin, SizeNewMas, MNK_deg, SpetApproc, AprocRezY[i].K, AprocRezY[i].massive);
			MinimumMas(AprocRezX[i].massive, SizeNewMas, BorderMin, SpetApproc, &AprocRezX[i].ExtY, &AprocRezX[i].ExtX);
			MinimumMas(AprocRezY[i].massive, SizeNewMas, BorderMin, SpetApproc, &AprocRezY[i].ExtY, &AprocRezY[i].ExtX);

			AprocRezY[i].RezMeanL = 1000;
			AprocRezY[i].RezMeanR = 1000;
			AprocRezX[i].RezMeanL = 1000;
			AprocRezX[i].RezMeanR = 1000;
			AprocRezX[i].X = 1000;
			AprocRezY[i].X = 1000;

			//(нахождение пересечений: PerV1,PerV2 - с верхней линией; PerN1,PerN2 - с нижней линией
			FindRootsOfEquation(AprocRezX[i].K, MNK_deg, SigmaMin, BorderMin, BorderMax, &AprocRezX[i].PerN1, &AprocRezX[i].PerN2);
			FindRootsOfEquation(AprocRezY[i].K, MNK_deg, SigmaMin, BorderMin, BorderMax, &AprocRezY[i].PerN1, &AprocRezY[i].PerN2);

			FindRootsOfEquation(AprocRezX[i].K, MNK_deg, SigmaMax, BorderMin, BorderMax, &AprocRezX[i].PerV1, &AprocRezX[i].PerV2);
			FindRootsOfEquation(AprocRezY[i].K, MNK_deg, SigmaMax, BorderMin, BorderMax, &AprocRezY[i].PerV1, &AprocRezY[i].PerV2);

			delete[] AprocMasMM; AprocMasMM = NULL;
			delete[] AprocMasX;  AprocMasX = NULL;
			delete[] AprocMasY;  AprocMasY = NULL;

			AprocRezX[i].RezMeanL = MeanABorAC(AprocRezX[i].PerV2, AprocRezX[i].PerN2, AprocRezX[i].ExtX, 1000);
			AprocRezY[i].RezMeanL = MeanABorAC(AprocRezY[i].PerV2, AprocRezY[i].PerN2, AprocRezY[i].ExtX, 1000);
			AprocRezX[i].RezMeanR = MeanABorAC(AprocRezX[i].PerV1, AprocRezX[i].PerN1, AprocRezX[i].ExtX, -1000);
			AprocRezY[i].RezMeanR = MeanABorAC(AprocRezY[i].PerV1, AprocRezY[i].PerN1, AprocRezY[i].ExtX, -1000);
		} //Цикл МНК - конец

		double* AprocLineX = new double[NomBoxXY];
		double* AprocLineY = new double[NomBoxXY];
		int Chuwi;
		double ChuwiAx, ChuwiBx, ChuwiAy, ChuwiBy;  //x,y - обозначение оси
		double ForYaxisMax = -1000, ForYaxisMin = 1000;


		float RelizXrez, RelizYrez, RelizXYrez;
		for (i = 0; i < NomBoxXY; i++) {
			switch (ComboBoxMainPoint->ItemIndex)
			{
			case 0:
				AprocRezX[i].X = AprocRezX[i].PerV2;
				AprocRezY[i].X = AprocRezY[i].PerV2;
				break;

			case 1:
				AprocRezX[i].X = AprocRezX[i].ExtX;
				AprocRezY[i].X = AprocRezY[i].ExtX;
				break;

			case 2:
				AprocRezX[i].X = AprocRezX[i].RezMeanL;
				AprocRezY[i].X = AprocRezY[i].RezMeanL;
				break;
			}
		}

		AddParagraph("Параметр наклона: " + ComboBoxMainPoint->Items->Strings[ComboBoxMainPoint->ItemIndex]);

		Chuwi = 0;
		for (i = 0; i < NomBoxXY; i++)
			if (AprocRezY[i].X != 1000 && AprocRezX[i].X != 1000
				&& (BoxXY[i].y >= ((StrToInt(Edit43->Text) / 2) - StrToInt(Edit19->Text)))
				&& (BoxXY[i].y <= ((StrToInt(Edit43->Text) / 2) + StrToInt(Edit19->Text)))) {
				AprocLineX[Chuwi] = BoxXY[i].x * StrToFloat(Edit50->Text);
				AprocLineY[Chuwi] = ConvMM((AprocRezX[i].X + AprocRezY[i].X) / 2);
				Chuwi++;
			}
		int MNK1ProvX = MNK1pow(AprocLineX, AprocLineY, 0, Chuwi - 1, &ChuwiBx, &ChuwiAx); //МНК (линейный) по оси Х

		Chuwi = 0;
		for (i = 0; i < NomBoxXY; i++)
			if (AprocRezY[i].X != 1000 && AprocRezX[i].X != 1000
				&& (BoxXY[i].x >= ((StrToInt(Edit34->Text) / 2) - StrToInt(Edit19->Text)))
				&& (BoxXY[i].x <= ((StrToInt(Edit34->Text) / 2) + StrToInt(Edit19->Text)))) {
				AprocLineX[Chuwi] = BoxXY[i].y * StrToFloat(Edit50->Text);
				AprocLineY[Chuwi] = ConvMM((AprocRezX[i].X + AprocRezY[i].X) / 2);
				Chuwi++;
			}
		int MNK1ProvY = MNK1pow(AprocLineX, AprocLineY, 0, Chuwi - 1, &ChuwiBy, &ChuwiAy); //МНК (линейный) по оси У

		delete[] AprocLineX; AprocLineX = NULL;
		delete[] AprocLineY; AprocLineY = NULL;

		for (i = 0; i < NomBoxXY; i++) {
			if (AprocRezY[i].X != 1000 && ((BoxXY[i].x >= ((StrToInt(Edit34->Text) / 2) - StrToInt(Edit19->Text)))
				&& (BoxXY[i].x <= ((StrToInt(Edit34->Text) / 2) + StrToInt(Edit19->Text)))
				|| (BoxXY[i].y >= ((StrToInt(Edit43->Text) / 2) - StrToInt(Edit19->Text)))
				&& (BoxXY[i].y <= ((StrToInt(Edit43->Text) / 2) + StrToInt(Edit19->Text))))) {
				if (ForYaxisMax < ConvMM(AprocRezY[i].X)) ForYaxisMax = ConvMM(AprocRezY[i].X);
				if (ForYaxisMin > ConvMM(AprocRezY[i].X)) ForYaxisMin = ConvMM(AprocRezY[i].X);
			}
			if (AprocRezX[i].X != 1000 && ((BoxXY[i].x >= ((StrToInt(Edit34->Text) / 2) - StrToInt(Edit19->Text)))
				&& (BoxXY[i].x <= ((StrToInt(Edit34->Text) / 2) + StrToInt(Edit19->Text)))
				|| (BoxXY[i].y >= ((StrToInt(Edit43->Text) / 2) - StrToInt(Edit19->Text)))
				&& (BoxXY[i].y <= ((StrToInt(Edit43->Text) / 2) + StrToInt(Edit19->Text))))) {
				if (ForYaxisMax < ConvMM(AprocRezX[i].X)) ForYaxisMax = ConvMM(AprocRezX[i].X);  //Для левой шкалы макс/мин значения
				if (ForYaxisMin > ConvMM(AprocRezX[i].X)) ForYaxisMin = ConvMM(AprocRezX[i].X);
			}
		}

		//Создание бмп рисунка
		bm->PixelFormat = pf32bit;
		bm->Width = Chart1->Width;
		bm->Height = Chart1->Height;

		//Настройки графика    ?????? почему  Chart1
		TPointSeries* PointSeries[10];
		Chart1->Visible = false;
		Chart1->Walls->Visible = false;
		Chart1->Width = StrToInt(Edit41->Text);
		Chart1->Height = StrToInt(Edit40->Text);

		ProgressBar1->Max = NomBoxXY * Meter;
		SetBreak();
		Chart3->Legend->Visible = true;
		Chart3->Visible = false;
		Chart3->Walls->Visible = false;
		Chart3->Width = StrToInt(Edit41->Text);
		Chart3->Height = StrToInt(Edit40->Text);
		Chart3->Series[0]->Clear();
		Chart3->Series[1]->Clear();
		Chart3->Series[2]->Clear();
		Chart3->Series[3]->Clear();
		Chart3->Refresh();

		//Заполнение графиков наклона матрицы по Х
		for (i = 0; i < NomBoxXY; i++) {
			Parameters(AprocRezX[i].X, AprocRezY[i].X, &RelizXrez, &RelizYrez, &RelizXYrez, InfCol, FocBkz, FocCol);

			if (AprocRezX[i].X != 1000 && (BoxXY[i].y >= (Ymatr / 2) - Sector) && (BoxXY[i].y <= (Ymatr / 2) + Sector))
				Chart3->Series[0]->AddXY(BoxXY[i].x * pixSize, RelizXrez, FloatToStr(FixRoundTo(BoxXY[i].x, -1)) + "\r\n" + FloatToStr(FixRoundTo(BoxXY[i].x * pixSize, -2)), RGB(30, 144, 255));

			if (AprocRezY[i].X != 1000 && (BoxXY[i].y >= (Ymatr / 2) - Sector) && (BoxXY[i].y <= (Ymatr / 2) + Sector))
				Chart3->Series[1]->AddXY(BoxXY[i].x * pixSize, RelizYrez, FloatToStr(FixRoundTo(BoxXY[i].x, -1)) + "\r\n" + FloatToStr(FixRoundTo(BoxXY[i].x * pixSize, -2)), RGB(185, 78, 72));

			if (AprocRezY[i].X != 1000 && AprocRezX[i].X != 1000 && (BoxXY[i].y >= (Ymatr / 2) - Sector) && (BoxXY[i].y <= (Ymatr / 2) + Sector))
				Chart3->Series[2]->AddXY(BoxXY[i].x * pixSize, RelizXYrez, FloatToStr(FixRoundTo(BoxXY[i].x, -1)) + "\r\n" + FloatToStr(FixRoundTo(BoxXY[i].x * pixSize, -2)), RGB(2, 174, 36));
		}

		//Прямая аппроксимации по Х
		for (i = 0; i < StrToInt(Edit34->Text) * StrToFloat(Edit50->Text); i++)
			Chart3->Series[3]->AddXY(i, ChuwiAx * i + ChuwiBx);

		Chart3->Axes->Bottom->AutomaticMaximum = false;
		Chart3->Axes->Bottom->AutomaticMinimum = false;
		Chart3->Axes->Left->AutomaticMaximum = false;
		Chart3->Axes->Left->AutomaticMinimum = false;

		if (ForYaxisMax != (-1000) && ForYaxisMin != 1000) {
			Chart3->Axes->Bottom->Minimum = -10000;
			Chart3->Axes->Bottom->Maximum = StrToFloat(Edit34->Text) * StrToFloat(Edit50->Text);
			Chart3->Axes->Bottom->Minimum = 0;

			Chart3->Axes->Left->Minimum = -10000;
			Chart3->Axes->Left->Maximum = ForYaxisMax + 0.01;
			Chart3->Axes->Left->Minimum = ForYaxisMin - 0.01;
		}

		Chart3->Axes->Bottom->Title->Caption = "\r\nКоордината по оси X";
		Chart3->Refresh();
		Application->ProcessMessages();
		Chart3->Title->Text->Text = AnsiString("   Наклон матрицы по оси X  " + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")").c_str();
		String NameGraf = (Edit23->Text + "\\Sigma - " + bufftime + " - Graf RezX.bmp");

		Chart3->SaveToBitmapFile(NameGraf);
		AddPicture(NameGraf);

		if (MNK1ProvX == 0) {
			AddParagraph("Y = " + FloatToStr(FixRoundTo(ChuwiAx, -8)) + "*X + " + FloatToStr(FixRoundTo(ChuwiBx, -8)));
			AddParagraph("acrtg(A) = " + FloatToStr(FixRoundTo(atan(ChuwiAx) * 180.0 / M_PI, -5)) + "    (" + FloatToStr(FixRoundTo(60 * atan(ChuwiAx) * 180.0 / M_PI, -3)) + " угл.мин)");
			AddParagraph("Расстояние по центру до нуля = " + FloatToStr(FixRoundTo(ChuwiAx * ((StrToInt(Edit34->Text) * StrToFloat(Edit50->Text)) / 2) + ChuwiBx, -4)) + " мм");
		}

		//SaveDoc(ReportName);
		Chart3->Series[0]->Clear();
		Chart3->Series[1]->Clear();
		Chart3->Series[2]->Clear();
		Chart3->Series[3]->Clear();
		Chart3->Refresh();

		//Заполнение графиков наклона матрицы по У
		for (i = 0; i < NomBoxXY; i++) {
			Parameters(AprocRezX[i].X, AprocRezY[i].X, &RelizXrez, &RelizYrez, &RelizXYrez, InfCol, FocBkz, FocCol);

			if (AprocRezX[i].X != 1000 && (BoxXY[i].x >= (Xmatr / 2) - Sector) && (BoxXY[i].x <= (Xmatr / 2) + Sector))
				Chart3->Series[0]->AddXY(BoxXY[i].y * pixSize, RelizXrez, FloatToStr(FixRoundTo(BoxXY[i].y, -1)) + "\r\n" + FloatToStr(FixRoundTo(BoxXY[i].y * pixSize, -2)), RGB(30, 144, 255));

			if (AprocRezY[i].X != 1000 && (BoxXY[i].x >= (Xmatr / 2) - Sector) && (BoxXY[i].x <= (Xmatr / 2) + Sector))
				Chart3->Series[1]->AddXY(BoxXY[i].y * pixSize, RelizYrez, FloatToStr(FixRoundTo(BoxXY[i].y, -1)) + "\r\n" + FloatToStr(FixRoundTo(BoxXY[i].y * pixSize, -2)), RGB(185, 78, 72));

			if (AprocRezY[i].X != 1000 && AprocRezX[i].X != 1000 && (BoxXY[i].x >= (Xmatr / 2) - Sector) && (BoxXY[i].x <= (Xmatr / 2) + Sector))
				Chart3->Series[2]->AddXY(BoxXY[i].y * pixSize, RelizXYrez, FloatToStr(FixRoundTo(BoxXY[i].y, -1)) + "\r\n" + FloatToStr(FixRoundTo(BoxXY[i].y * pixSize, -2)), RGB(2, 174, 36));
		}

		//Прямая аппроксимации по У
		for (i = 0; i < StrToInt(Edit43->Text) * StrToFloat(Edit50->Text); i++)
			Chart3->Series[3]->AddXY(i, ChuwiAy * i + ChuwiBy);
		Chart3->Refresh();

		Chart3->Axes->Bottom->AutomaticMaximum = false;
		Chart3->Axes->Bottom->AutomaticMinimum = false;
		Chart3->Axes->Left->AutomaticMaximum = false;
		Chart3->Axes->Left->AutomaticMinimum = false;

		if (ForYaxisMax != (-1000) && ForYaxisMin != 1000) {
			Chart3->Axes->Bottom->Minimum = -10000;
			Chart3->Axes->Bottom->Maximum = StrToFloat(Edit43->Text) * StrToFloat(Edit50->Text);
			Chart3->Axes->Bottom->Minimum = 0;

			Chart3->Axes->Left->Minimum = -10000;
			Chart3->Axes->Left->Maximum = ForYaxisMax + 0.01;
			Chart3->Axes->Left->Minimum = ForYaxisMin - 0.01;
		}

		Chart3->Axes->Bottom->Title->Caption = "\r\nКоордината по оси Y";
		Application->ProcessMessages();
		Chart3->Title->Text->Text = AnsiString("    Наклон матрицы по оси Y  " + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")").c_str();
		Chart3->Refresh();
		Application->ProcessMessages();

		Chart3->Refresh();
		NameGraf = (Edit23->Text + "\\Sigma - " + bufftime + " - Graf RezY.bmp");
		Chart3->SaveToBitmapFile(NameGraf);

		AddPicture(NameGraf);

		if (MNK1ProvY == 0) {
			AddParagraph("Y = " + FloatToStr(FixRoundTo(ChuwiAy, -8)) + "*X + " + FloatToStr(FixRoundTo(ChuwiBy, -8)));
			AddParagraph("acrtg(A) = " + FloatToStr(FixRoundTo(atan(ChuwiAy) * 180.0 / M_PI, -5)) + "    (" + FloatToStr(FixRoundTo(60 * atan(ChuwiAy) * 180.0 / M_PI, -3)) + " угл.мин)");
			AddParagraph("Расстояние по центру до нуля = " + FloatToStr(FixRoundTo(ChuwiAy * ((StrToInt(Edit43->Text) * StrToFloat(Edit50->Text)) / 2) + ChuwiBy, -4)) + " мм");
		}
#pragma end_region

		//Картинка по Х
		Chart2->Series[0]->Clear();
		Chart2->Series[1]->Clear();
		Series3->Marks->Visible = true;
		Series4->Marks->Visible = true;
		Chart2->Title->Text->Text = AnsiString("Sigma X " + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")").c_str();

		for (i = 0; i < NomBoxXY; i++) {
			SigmaInfX = FindOneRootOfEquation(InfCol, MNK_deg, AprocRezX[i].K);
			double bubbleRadius = fabs(SigmaInfX) * BubbleSize;

			if ((SigmaMin <= SigmaInfX) && (SigmaMax >= SigmaInfX)) {
				Series3->AddBubble(BoxXY[i].x,
					BoxXY[i].y,
					bubbleRadius,
					FloatToStr(FixRoundTo((AprocRezX[i].X - InfCol) * (FocBkz * FocBkz) / (FocCol * FocCol), -3))
					+ "\r\n" + FloatToStr(FixRoundTo(SigmaInfX, -2)), RGB(0, 204, 0));
			}
			else {
				if (AprocRezX[i].X == 1000)
					Series4->AddBubble(BoxXY[i].x, BoxXY[i].y, bubbleRadius, "-", RGB(255, 77, 0));
				else
					Series4->AddBubble(BoxXY[i].x, BoxXY[i].y, bubbleRadius, FloatToStr(FixRoundTo((AprocRezX[i].X - InfCol) * (FocBkz * FocBkz) / (FocCol * FocCol), -3)) + "\r\n" + FloatToStr(FixRoundTo(SigmaInfX, -2)), RGB(255, 77, 0));
			}
		}

		SetTextFormat(12, 1, 1, 0, 0, 1);
		AddParagraph("Результат расчета для сигмы X (график)");
		SetTextFormat(12, 0, 0, 0, 0, 0.5);
		AddParagraph("Размер окружности отражает значение сигмы на бесконечности (пересечение с аппроксимацией)\nЗеленый цвет - значение сигмы попадает в оптимальный промежуток\nКрасный цвет - значение сигмы выходит за границы оптимума\nПодпись - среднее значение в мм для достижения оптимума: \n\"0\" идеал (оптимум на бесконечности коллиматора)\n\"-\" сточить\n\"+\" нарастить");
		AddParagraph("Оптимальное значение сигмы: " + Edit20->Text + " - " + Edit21->Text + "\nБесконечность коллиматора:  " + Edit18->Text);

		NameGraf2 = Edit23->Text + "\\Sigma - " + bufftime + " - Graf BUBBLE X.bmp";
		Chart2->SaveToBitmapFile(NameGraf2);
		AddPicture(NameGraf2);
		AddParagraph("\f");

		//Картинка по Y
		Chart2->Series[0]->Clear();
		Chart2->Series[1]->Clear();
		Chart2->Refresh();
		Application->ProcessMessages();
		Series3->Marks->Visible = true;
		Series4->Marks->Visible = true;
		Chart2->Title->Text->Text = AnsiString("Sigma Y " + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")").c_str();

		for (i = 0; i < NomBoxXY; i++) {
			SigmaInfY = FindOneRootOfEquation(InfCol, MNK_deg, AprocRezY[i].K);
			double bubbleRadius = fabs(SigmaInfY) * BubbleSize;

			if ((SigmaMin <= SigmaInfY) && (SigmaMax >= SigmaInfY)) {
				Series3->AddBubble(BoxXY[i].x, BoxXY[i].y, bubbleRadius, FloatToStr(FixRoundTo((AprocRezY[i].X - InfCol) * (FocBkz * FocBkz) / (FocCol * FocCol), -3)) + "\r\n" + FloatToStr(FixRoundTo(SigmaInfY, -2)), RGB(0, 204, 0));
			}
			else {
				if (AprocRezY[i].X == 1000)
					Series4->AddBubble(BoxXY[i].x, BoxXY[i].y, bubbleRadius, "-", RGB(255, 77, 0));
				else
					Series4->AddBubble(BoxXY[i].x, BoxXY[i].y, bubbleRadius, FloatToStr(FixRoundTo((AprocRezY[i].X - InfCol) * (FocBkz * FocBkz) / (FocCol * FocCol), -3)) + "\r\n" + FloatToStr(FixRoundTo(SigmaInfY, -2)), RGB(255, 77, 0));
			}
		}


		Chart2->Refresh();
		Application->ProcessMessages();
		SetTextFormat(12, 1, 1, 0, 0, 1);
		AddParagraph("Результат расчета для сигмы Y (график)");
		SetTextFormat(12, 0, 0, 0, 0, 0.5);
		AddParagraph("Размер окружности отражает значение сигмы на бесконечности (пересечение с аппроксимацией)\nЗеленый цвет - значение сигмы попадает в оптимальный промежуток\nКрасный цвет - значение сигмы выходит за границы оптимума\nПодпись - среднее значение в мм для достижения оптимума:\n\"0\" идеал (оптимум на бесконечности коллиматора)\n\"-\" сточить\n\"+\" нарастить");
		AddParagraph("Оптимальное значение сигмы: " + Edit20->Text + " - " + Edit21->Text + "\nБесконечность коллиматора:  " + Edit18->Text);

		NameGraf2 = Edit23->Text + "\\Sigma - " + bufftime + " - Graf BUBBLE Y.bmp";
		Chart2->SaveToBitmapFile(NameGraf2);
		AddPicture(NameGraf2);

		ReportName = Edit23->Text + "\\Sigma - " + bufftime + " - SigmaGraph.doc";
		//------------------------------------
		AddParagraph("\f");

		//Обычные графики по каждой координате с таблицей результатов
		for (i = 0; i < NomBoxXY; i++) {
			Chart1->Series[0]->Clear();
			Chart1->Series[1]->Clear();
			Chart1->Series[2]->Clear();
			Chart1->Series[3]->Clear();
			Chart1->Series[4]->Clear();
			Chart1->Series[5]->Clear();
			Chart1->Series[6]->Clear();
			Chart1->Series[7]->Clear();
			Chart1->Series[8]->Clear();
			Chart1->Series[9]->Clear();
			Chart1->Refresh();

			Chart1->Series[2]->AddXY(0, StrToFloat(Edit20->Text), 0, RGB(123, 0, 28));
			Chart1->Series[2]->AddXY(1000, StrToFloat(Edit20->Text), "", RGB(123, 0, 28)); //StrToFloat(Edit29->Text)

			Chart1->Series[3]->AddXY(0, StrToFloat(Edit21->Text), 0, RGB(123, 0, 28));
			Chart1->Series[3]->AddXY(1000, StrToFloat(Edit21->Text), "", RGB(123, 0, 28));

			Chart1->Series[4]->AddXY(StrToFloat(Edit18->Text), 0, "", RGB(83, 55, 122));
			Chart1->Series[4]->AddXY(StrToFloat(Edit18->Text), StrToFloat(Edit31->Text), "", RGB(83, 55, 122));

			if (CheckBoxBaric->Checked == true) {
				Chart1->Series[8]->AddXY(StrToFloat(EditBaric->Text), 0, (EditBaric->Text));
				Chart1->Series[8]->AddXY(StrToFloat(EditBaric->Text), StrToFloat(Edit31->Text), (EditBaric->Text));
			}

			float StepOne = BorderMin;
			for (ifile = 0; ifile < SizeNewMas; ifile++) {
				Chart1->Series[5]->AddXY(StepOne, AprocRezX[i].massive[ifile], "", RGB(48, 100, 169));
				Chart1->Series[6]->AddXY(StepOne, AprocRezY[i].massive[ifile], "", RGB(191, 43, 69));
				StepOne += SpetApproc;
			}

			float MinChart1 = 10000;
			float MaxChart1 = -10000;
			for (ifile = 0; ifile < Meter; ifile++) {
				if (((FullSigma[ifile].x + PorogPx) >= BoxXY[i].x) && ((FullSigma[ifile].x - PorogPx) <= BoxXY[i].x) &&
					((FullSigma[ifile].y + PorogPx) >= BoxXY[i].y) && ((FullSigma[ifile].y - PorogPx) <= BoxXY[i].y))
				{
					for (j = 0; j < NomBoxMS; j++) {
						if (FullSigma[ifile].ms < (BoxMS[j].ms + 1) && FullSigma[ifile].ms >(BoxMS[j].ms - 1))
						{

							ConverMM = (FullSigma[ifile].mm - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text));
							Chart1->Series[0]->AddXY(FullSigma[ifile].mm, FullSigma[ifile].SigX, FloatToStr(FixRoundTo(FullSigma[ifile].mm, -3)) + "\r\n" + FloatToStr(FixRoundTo(ConverMM, -3)), BoxMS[j].ColGraf);
							Chart1->Series[1]->AddXY(FullSigma[ifile].mm, FullSigma[ifile].SigY, FloatToStr(FixRoundTo(FullSigma[ifile].mm, -3)) + "\r\n" + FloatToStr(FixRoundTo(ConverMM, -3)), BoxMS[j].ColGraf);

							if (FullSigma[ifile].mm < MinChart1) MinChart1 = FullSigma[ifile].mm;
							if (FullSigma[ifile].mm > MaxChart1) MaxChart1 = FullSigma[ifile].mm;
						}
					}
				}
				ProgressBar1->Position++;
				StatusBar1->Panels->Items[0]->Text = IntToStr(ProgressBar1->Position) + " / " + IntToStr(ProgressBar1->Max);
			}

			Chart1->Series[1]->AddXY(MaxChart1 + 0.5, 0, "", 0);
			Chart1->Series[1]->AddXY(MinChart1 - 0.5, 0, "", 0);


			Chart1->Axes->Bottom->Title->Caption = "\r\nfк (df)";
			Chart1->Axes->Left->Title->Caption = "Sigma";

			//Значения по осям max min
			// Bottom - X
			if (CheckBox6->Checked == false && Edit29->Text != "" && Edit27->Text != "") {
				Chart1->Axes->Bottom->AutomaticMaximum = false;
				Chart1->Axes->Bottom->AutomaticMinimum = false;
				Chart1->Axes->Bottom->Minimum = -10000;
				Chart1->Axes->Bottom->Maximum = StrToFloat(Edit29->Text);
				Chart1->Axes->Bottom->Minimum = StrToFloat(Edit27->Text);
			}
			else {
				Chart1->Axes->Bottom->AutomaticMaximum = false;
				Chart1->Axes->Bottom->AutomaticMinimum = false;
				Chart1->Axes->Bottom->Minimum = -10000;
				Chart1->Axes->Bottom->Maximum = MaxChart1 + 0.5;
				Chart1->Axes->Bottom->Minimum = MinChart1 - 0.5;
			}

			//  Left - Y
			if (CheckBox7->Checked == false && Edit30->Text != "" && Edit31->Text != "") {
				Chart1->Axes->Left->AutomaticMaximum = false;
				Chart1->Axes->Left->AutomaticMinimum = false;
				Chart1->Axes->Left->Minimum = -10000;
				Chart1->Axes->Left->Maximum = StrToFloat(Edit31->Text);
				Chart1->Axes->Left->Minimum = StrToFloat(Edit30->Text);
			}
			else {
				Chart1->Axes->Left->AutomaticMaximum = true;
				Chart1->Axes->Left->AutomaticMinimum = true;
			}

			if (CheckBox12->Checked == false) {
				Chart1->Axes->Bottom->Increment = StrToFloat(Edit38->Text);
				Chart1->Axes->Left->Increment = StrToFloat(Edit39->Text);
			}
			Chart1->Refresh();
			Application->ProcessMessages();


			Chart1->Title->Text->Text = AnsiString(FloatToStr(((int)(BoxXY[i].x * 1000)) / 1000) + "x"
				+ FloatToStr(((int)(BoxXY[i].y * 1000)) / 1000)
				+ "   " + Edit32->Text + " №"
				+ Edit33->Text + "  (" + buffDate + ")").c_str();


			NameGraf = (Edit23->Text + "\\Graf_" + IntToStr(i + 1) + "_" + FloatToStr(((int)(BoxXY[i].x * 1000)) / 1000) + "x" + FloatToStr(((int)(BoxXY[i].y * 1000)) / 1000) + " " + bufftime + ".bmp");
			Chart1->SaveToBitmapFile(NameGraf);
			AddPicture(NameGraf);

			//График по N_frame   ------------------------------------------------------------
			if (CheckBoxNframe->Checked) {
				Chart1->Series[0]->Clear();
				Chart1->Series[1]->Clear();
				Chart1->Series[2]->Clear();
				Chart1->Series[3]->Clear();
				Chart1->Series[4]->Clear();
				Chart1->Series[5]->Clear();
				Chart1->Series[6]->Clear();
				Chart1->Series[7]->Clear();
				Chart1->Series[8]->Clear();
				Chart1->Series[9]->Clear();
				Chart1->Refresh();

				int NframeMAX = FullSigma[0].Nframe;
				int NframeMIN = FullSigma[0].Nframe;

				if (CheckBoxNF->Checked == true) {
					for (ifile = 0; ifile < Meter; ifile++) {
						if (FullSigma[ifile].Nframe > NframeMAX)	NframeMAX = FullSigma[ifile].Nframe;
						if (FullSigma[ifile].Nframe < NframeMIN)	NframeMIN = FullSigma[ifile].Nframe;
					}
				}
				else {
					NframeMAX = StrToFloat(EditNFmax->Text);
					NframeMIN = StrToFloat(EditNFmin->Text);
				}

				Chart1->Series[4]->AddXY(StrToFloat(Edit18->Text), 0, "", RGB(83, 55, 122));
				Chart1->Series[4]->AddXY(StrToFloat(Edit18->Text), NframeMAX + 10, "", RGB(83, 55, 122));

				if (CheckBoxBaric->Checked == true) {
					Chart1->Series[8]->AddXY(StrToFloat(EditBaric->Text), 0, (EditBaric->Text));
					Chart1->Series[8]->AddXY(StrToFloat(EditBaric->Text), NframeMAX + 10, (EditBaric->Text));
				}

				for (ifile = 0; ifile < Meter; ifile++) {
					if (((FullSigma[ifile].x + PorogPx) >= BoxXY[i].x) && ((FullSigma[ifile].x - PorogPx) <= BoxXY[i].x) &&
						((FullSigma[ifile].y + PorogPx) >= BoxXY[i].y) && ((FullSigma[ifile].y - PorogPx) <= BoxXY[i].y))
					{
						for (j = 0; j < NomBoxMS; j++)
							if (FullSigma[ifile].ms < (BoxMS[j].ms + 1) && FullSigma[ifile].ms >(BoxMS[j].ms - 1))
							{

								ConverMM = (FullSigma[ifile].mm - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text));
								Chart1->Series[9]->AddXY(FullSigma[ifile].mm, FullSigma[ifile].Nframe, FloatToStr(FixRoundTo(FullSigma[ifile].mm, -3)) + "\r\n" + FloatToStr(FixRoundTo(ConverMM, -3)), BoxMS[j].ColGraf);
							}
					}
					ProgressBar1->Position++;
					StatusBar1->Panels->Items[0]->Text = IntToStr(ProgressBar1->Position) + " / " + IntToStr(ProgressBar1->Max);
				}

				Chart1->Axes->Bottom->Title->Caption = "\r\nfк (df)";
				Chart1->Axes->Left->Title->Caption = "N_frame";
				//Значения по осям max min
				// Bottom - X
				if (CheckBox6->Checked == false && Edit29->Text != "" && Edit27->Text != "") {
					Chart1->Axes->Bottom->AutomaticMaximum = false;
					Chart1->Axes->Bottom->AutomaticMinimum = false;
					Chart1->Axes->Bottom->Minimum = -10000;
					Chart1->Axes->Bottom->Maximum = StrToFloat(Edit29->Text);
					Chart1->Axes->Bottom->Minimum = StrToFloat(Edit27->Text);
				}
				else {
					Chart1->Axes->Bottom->AutomaticMaximum = FullSigma[ifile - 1].mm;
					Chart1->Axes->Bottom->AutomaticMinimum = FullSigma[0].mm;
				}

				if (CheckBoxNF->Checked == false && Edit29->Text != "" && Edit27->Text != "") {
					Chart1->Axes->Left->AutomaticMaximum = false;
					Chart1->Axes->Left->AutomaticMinimum = false;
					Chart1->Axes->Left->Minimum = -10000;
					Chart1->Axes->Left->Maximum = StrToFloat(EditNFmax->Text);
					Chart1->Axes->Left->Minimum = StrToFloat(EditNFmin->Text);
				}
				else {
					Chart1->Axes->Left->AutomaticMaximum = false;
					Chart1->Axes->Left->AutomaticMinimum = false;
					Chart1->Axes->Left->Minimum = -10000;
					Chart1->Axes->Left->Maximum = NframeMAX + 5;
					Chart1->Axes->Left->Minimum = NframeMIN - 5;
				}

				if (CheckBoxNF->Checked == false && Edit29->Text != "" && Edit27->Text != "") {
					int IncrementY = (int)((StrToFloat(EditNFmax->Text) - StrToFloat(EditNFmin->Text)) / 16 + 0.5);
					if (IncrementY % 2 != 0)
						IncrementY++;

					Chart1->Axes->Left->Increment = IncrementY;
				}


				if (CheckBox12->Checked == false)
					Chart1->Axes->Bottom->Increment = StrToFloat(Edit38->Text);

				Chart1->Title->Text->Clear();
				Chart1->Refresh();
				Application->ProcessMessages();
				Chart1->Title->Text->Text = AnsiString(FloatToStr(((int)(BoxXY[i].x * 1000)) / 1000) + "x" + FloatToStr(((int)(BoxXY[i].y * 1000)) / 1000) + "   " + Edit32->Text + " №" + Edit33->Text + "  (" + buffDate + ")").c_str();
				Chart1->PaintTo(bm->Canvas->Handle, 0, 0);
				NameGraf = (Edit23->Text + "\\Graf-Nframe_" + IntToStr(i + 1) + "_" + FloatToStr(((int)(BoxXY[i].x * 1000)) / 1000) + "x" + FloatToStr(((int)(BoxXY[i].y * 1000)) / 1000) + " " + bufftime + ".bmp");
				Chart1->SaveToBitmapFile(NameGraf);
				AddPicture(NameGraf);
			}
			//   end of "График по N_frame"

			AddParagraph("\n");

			SetTextFormat(11, 0, 0, 0, 1, 0.2);
			AddTable(12, 7);
			UnionCell(2, 1, 3, 1);
			UnionCell(2, 1, 4, 1);
			UnionCell(2, 1, 5, 1);
			UnionCell(2, 1, 6, 1);

			UnionCell(7, 1, 8, 1);
			UnionCell(7, 1, 9, 1);
			UnionCell(7, 1, 10, 1);
			UnionCell(7, 1, 11, 1);

			UnionCell(2, 3, 2, 4);
			UnionCell(2, 4, 2, 5);

			UnionCell(7, 3, 7, 4);
			UnionCell(7, 4, 7, 5);

			UnionCell(5, 3, 5, 4);
			UnionCell(5, 4, 5, 5);
			UnionCell(10, 3, 10, 4);
			UnionCell(10, 4, 10, 5);

			UnionCell(12, 1, 12, 2);


			SetCell(1, 3, "fk\n<-");
			SetCell(1, 4, "fk\n->");
			SetCell(1, 5, "df\n<-");
			SetCell(1, 6, "df\n->");
			SetCell(1, 7, "Sigma");

			SetCell(2, 1, "Sigma X");
			SetCell(7, 1, "Sigma Y");

			SetCell(2, 2, "inf");
			SetCell(3, 2, "верх");
			SetCell(4, 2, "низ");
			SetCell(5, 2, "MIN");
			SetCell(6, 2, "среднее");
			SetCell(7, 2, "inf");
			SetCell(8, 2, "верх");
			SetCell(9, 2, "низ");
			SetCell(10, 2, "MIN");
			SetCell(11, 2, "среднее");
			SetCell(12, 1, "ИТОГ");
			SetCellFormat(6, 5, 11, 1, 0, 0);
			SetCellFormat(6, 6, 11, 1, 0, 0);
			SetCellFormat(11, 5, 11, 1, 0, 0);
			SetCellFormat(11, 6, 11, 1, 0, 0);
			SetCellFormat(12, 4, 11, 1, 0, 0);
			SetCellFormat(12, 5, 11, 1, 0, 0);
			SetCellFormat(2, 5, 11, 1, 0, 0);
			SetCellFormat(7, 5, 11, 1, 0, 0);

			SigmaInfX = FindOneRootOfEquation(InfCol, MNK_deg, AprocRezX[i].K);
			SigmaInfY = FindOneRootOfEquation(InfCol, MNK_deg, AprocRezY[i].K);

			//Бесконечность
			SetCell(2, 3, Edit18->Text);
			SetCell(2, 4, "0");
			SetCell(2, 5, FloatToStr(FixRoundTo(SigmaInfX, -2)));
			SetCell(7, 3, Edit18->Text);
			SetCell(7, 4, "0");
			SetCell(7, 5, FloatToStr(FixRoundTo(SigmaInfY, -2)));

			if (AprocRezX[i].PerV2 != 1000) SetCell(3, 3, FloatToStr(FixRoundTo(AprocRezX[i].PerV2, -2)));  else SetCell(3, 3, "-");
			if (AprocRezX[i].PerV1 != -1000) SetCell(3, 4, FloatToStr(FixRoundTo(AprocRezX[i].PerV1, -2)));  else SetCell(3, 4, "-");
			if (AprocRezX[i].PerN2 != 1000) SetCell(4, 3, FloatToStr(FixRoundTo(AprocRezX[i].PerN2, -2)));  else SetCell(4, 3, "-");
			if (AprocRezX[i].PerN1 != -1000) SetCell(4, 4, FloatToStr(FixRoundTo(AprocRezX[i].PerN1, -2)));  else SetCell(4, 4, "-");

			SetCell(5, 3, FloatToStr(FixRoundTo(AprocRezX[i].ExtX, -2)));
			SetCell(5, 5, FloatToStr(FixRoundTo(AprocRezX[i].ExtY, -2)));

			if (AprocRezY[i].PerV2 != 1000) SetCell(8, 3, FloatToStr(FixRoundTo(AprocRezY[i].PerV2, -2)));  else SetCell(8, 3, "-");
			if (AprocRezY[i].PerV1 != -1000) SetCell(8, 4, FloatToStr(FixRoundTo(AprocRezY[i].PerV1, -2)));  else SetCell(8, 4, "-");
			if (AprocRezY[i].PerN2 != 1000) SetCell(9, 3, FloatToStr(FixRoundTo(AprocRezY[i].PerN2, -2)));  else SetCell(9, 3, "-");
			if (AprocRezY[i].PerN1 != -1000) SetCell(9, 4, FloatToStr(FixRoundTo(AprocRezY[i].PerN1, -2)));  else SetCell(9, 4, "-");

			SetCell(10, 3, FloatToStr(FixRoundTo(AprocRezY[i].ExtX, -2)));
			SetCell(10, 5, FloatToStr(FixRoundTo(AprocRezY[i].ExtY, -2)));

			SetCell(3, 7, (Edit21->Text));
			SetCell(4, 7, (Edit20->Text));
			SetCell(8, 7, (Edit21->Text));
			SetCell(9, 7, (Edit20->Text));

			if (AprocRezX[i].PerV2 != 1000) SetCell(3, 5, FloatToStr(FixRoundTo((AprocRezX[i].PerV2 - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text)), -3)));  else SetCell(3, 5, "-");
			if (AprocRezX[i].PerV1 != -1000) SetCell(3, 6, FloatToStr(FixRoundTo((AprocRezX[i].PerV1 - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text)), -3)));  else SetCell(3, 6, "-");
			if (AprocRezX[i].PerN2 != 1000) SetCell(4, 5, FloatToStr(FixRoundTo((AprocRezX[i].PerN2 - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text)), -3)));  else SetCell(4, 5, "-");
			if (AprocRezX[i].PerN1 != -1000) SetCell(4, 6, FloatToStr(FixRoundTo((AprocRezX[i].PerN1 - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text)), -3)));  else SetCell(4, 6, "-");

			if (AprocRezY[i].PerV2 != 1000) SetCell(8, 5, FloatToStr(FixRoundTo((AprocRezY[i].PerV2 - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text)), -3)));  else SetCell(8, 5, "-");
			if (AprocRezY[i].PerV1 != -1000) SetCell(8, 6, FloatToStr(FixRoundTo((AprocRezY[i].PerV1 - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text)), -3)));  else SetCell(8, 6, "-");
			if (AprocRezY[i].PerN2 != 1000) SetCell(9, 5, FloatToStr(FixRoundTo((AprocRezY[i].PerN2 - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text)), -3)));  else SetCell(9, 5, "-");
			if (AprocRezY[i].PerN1 != -1000) SetCell(9, 6, FloatToStr(FixRoundTo((AprocRezY[i].PerN1 - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text)), -3)));  else SetCell(9, 6, "-");

			SetCell(5, 4, FloatToStr(FixRoundTo((AprocRezX[i].ExtX - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text)), -3)));
			SetCell(10, 4, FloatToStr(FixRoundTo((AprocRezY[i].ExtX - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text)), -3)));

			SigmaInfX = FindOneRootOfEquation(AprocRezX[i].RezMeanL, MNK_deg, AprocRezX[i].K);
			if (AprocRezX[i].RezMeanL != 1000) SetCell(6, 3, FloatToStr(FixRoundTo(AprocRezX[i].RezMeanL, -2)));                                                                                                                                      else SetCell(6, 3, "-");
			if (AprocRezX[i].RezMeanL != 1000) SetCell(6, 5, FloatToStr(FixRoundTo((AprocRezX[i].RezMeanL - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text)), -3)));  else SetCell(6, 5, "-");
			if (AprocRezX[i].RezMeanL != 1000) SetCell(6, 7, FloatToStr(FixRoundTo(SigmaInfX, -2)));                                          else SetCell(6, 7, "-");

			SigmaInfX = FindOneRootOfEquation(AprocRezX[i].RezMeanR, MNK_deg, AprocRezX[i].K);
			if (AprocRezX[i].RezMeanR != 1000) SetCell(6, 4, FloatToStr(FixRoundTo(AprocRezX[i].RezMeanR, -2)));                                                                                                                                      else SetCell(6, 4, "-");
			if (AprocRezX[i].RezMeanR != 1000) SetCell(6, 6, FloatToStr(FixRoundTo((AprocRezX[i].RezMeanR - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text)), -3)));  else SetCell(6, 6, "-");
			//			SetCell(5, 7, " - " + FloatToStr(RoundTo( SigmaInfX,-3)) );

			SigmaInfY = FindOneRootOfEquation(AprocRezY[i].RezMeanL, MNK_deg, AprocRezY[i].K);
			if (AprocRezY[i].RezMeanL != 1000) SetCell(11, 3, FloatToStr(FixRoundTo(AprocRezY[i].RezMeanL, -2)));                                                                                                                                     else SetCell(11, 3, "-");
			if (AprocRezY[i].RezMeanL != 1000) SetCell(11, 5, FloatToStr(FixRoundTo((AprocRezY[i].RezMeanL - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text)), -3))); else SetCell(11, 5, "-");
			if (AprocRezY[i].RezMeanL != 1000) SetCell(11, 7, FloatToStr(FixRoundTo(SigmaInfY, -2)));                                         else SetCell(11, 7, "-");

			SigmaInfY = FindOneRootOfEquation(AprocRezY[i].RezMeanR, MNK_deg, AprocRezY[i].K);
			if (AprocRezY[i].RezMeanR != 1000) SetCell(11, 4, FloatToStr(FixRoundTo(AprocRezY[i].RezMeanR, -2)));                                                                                                                                     else SetCell(11, 4, "-");
			if (AprocRezY[i].RezMeanR != 1000) SetCell(11, 6, FloatToStr(FixRoundTo((AprocRezY[i].RezMeanR - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text)), -3))); else SetCell(11, 6, "-");
			//			SetCell(9, 7, " - " + FloatToStr(RoundTo( SigmaInfY,-3)) );

			if (AprocRezX[i].RezMeanL != 1000) SetCell(12, 2, FloatToStr(FixRoundTo((AprocRezX[i].RezMeanL + AprocRezY[i].RezMeanL) / 2, -2))); else SetCell(12, 2, "-");
			if (AprocRezX[i].RezMeanR != 1000) SetCell(12, 3, FloatToStr(FixRoundTo((AprocRezX[i].RezMeanR + AprocRezY[i].RezMeanR) / 2, -2))); else SetCell(12, 3, "-");

			if (AprocRezX[i].RezMeanL != 1000) SetCell(12, 4, FloatToStr(FixRoundTo(((AprocRezX[i].RezMeanL + AprocRezY[i].RezMeanL) / 2 - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text)), -3))); else SetCell(12, 4, "-");
			if (AprocRezX[i].RezMeanR != 1000) SetCell(12, 5, FloatToStr(FixRoundTo(((AprocRezX[i].RezMeanR + AprocRezY[i].RezMeanR) / 2 - StrToFloat(Edit18->Text)) * (StrToFloat(Edit45->Text) * StrToFloat(Edit45->Text)) / (StrToFloat(Edit46->Text) * StrToFloat(Edit46->Text)), -3))); else SetCell(12, 5, "-");



			float Sx1 = FindOneRootOfEquation((AprocRezX[i].RezMeanL + AprocRezY[i].RezMeanL) / 2, MNK_deg, AprocRezX[i].K);
			float Sx2 = FindOneRootOfEquation((AprocRezX[i].RezMeanR + AprocRezY[i].RezMeanR) / 2, MNK_deg, AprocRezX[i].K);
			float Sy1 = FindOneRootOfEquation((AprocRezX[i].RezMeanL + AprocRezY[i].RezMeanL) / 2, MNK_deg, AprocRezY[i].K);
			float Sy2 = FindOneRootOfEquation((AprocRezX[i].RezMeanR + AprocRezY[i].RezMeanR) / 2, MNK_deg, AprocRezY[i].K);

			if (AprocRezX[i].RezMeanL != 1000) SetCell(12, 6, "Sx: " + FloatToStr(FixRoundTo(Sx1, -2)) + " / " + FloatToStr(FixRoundTo(Sx2, -2)));  else SetCell(12, 6, "-");
			if (AprocRezX[i].RezMeanL != 1000) SetCell(12, 6, "\nSy: " + FloatToStr(FixRoundTo(Sy1, -2)) + " / " + FloatToStr(FixRoundTo(Sy2, -2)));  else SetCell(12, 6, "-");

			AddParagraph("\f");

		}
		//----------------------------------------------------

		//легенда мс (цветной текст с сортировкой от мин к макс)
		SetTextFormat(14, 1, 0, 0, 1, 0.05);
		float MINms;
		for (i = 0; i < NomBoxMS; i++)
			BoxMS[i].mm = BoxMS[i].ms;

		for (j = 0; j < NomBoxMS; j++)
			for (i = 0; i < NomBoxMS; i++)
				if (BoxMS[j].mm < BoxMS[i].mm) {
					MINms = BoxMS[j].mm;
					BoxMS[j].mm = BoxMS[i].mm;
					BoxMS[i].mm = MINms;
				}
		for (i = 0; i < NomBoxMS; i++) {
			SetTextColor(BoxMS[i].ColGraf);
			AddParagraph(FloatToStr(FixRoundTo(BoxMS[i].mm, -3)) + " мс");
		}
		SetTextColor((TColor)RGB(0, 0, 0));

		//INI
		SetTextFormat(8, 0, 0, 0, 1, 0.1);
		AddTable(2, 2);
		UnionCell(1, 1, 1, 2);
		SetCell(1, 1, "INI файл");
		SetCell(2, 1, "[Overall]\nX=" + Edit1->Text +
			"\nY=" + Edit2->Text +
			"\nDir=" + Edit3->Text +
			"\nMask=" + Edit4->Text +
			"\nCheckChoice=" + BoolToStr(CheckBox1->Checked) +
			"\nSaveDir=" + Edit23->Text +
			"\nFormatFrame=" + IntToStr(RadioGroup1->ItemIndex) +

			"\n[LOC-options]\nKofFilt=" + Edit10->Text +
			"\nCheckFilt=" + BoolToStr(CheckBox3->Checked) +
			"\nkofSKO1=" + Edit17->Text +
			"\npixMin=" + Edit11->Text +
			"\npixMax=" + Edit12->Text +
			"\nIsMin=" + Edit13->Text +
			"\nIsMax=" + Edit14->Text +

			"\n[Sigma-options]\nframe=" + Edit9->Text +
			"\nKofBin=" + Edit5->Text +
			"\nCheckBin=" + BoolToStr(CheckBox2->Checked) +
			"\nkofSKO2=" + Edit28->Text +
			"\npixMaxIs=" + Edit15->Text +
			"\npixMinIs=" + Edit16->Text +
			"\nCheckMostLight=" + BoolToStr(CheckBox4->Checked) +
			"\nCheck_Is-pix=" + IntToStr(ComboBoxPixIs->ItemIndex) +
			"\nforLG_step=" + Edit24->Text +
			"\nforLG_iter=" + Edit25->Text +
			"\nforLG_emptyPix=" + Edit26->Text +
			"\nforLG_MinPix=" + Edit52->Text +
			"\nCheckSector=" + BoolToStr(CheckBox14->Checked) +

			"\n[Report-options]\nNameDevice=" + Edit32->Text +
			"\nNomberDevice=" + Edit33->Text +
			"\nXmatr=" + Edit34->Text +
			"\nYmatr=" + Edit43->Text +
			"\nNameShooter=" + Edit35->Text +
			"\nDataShoot=" + Edit36->Text +
			"\nNameReporter=" + Edit37->Text +
			"\nComment=" + Memo2->Text +

			"\n[Photogrammetry-options]\nIsStar6=" + Edit_IOZ6->Text +
			"\nIsStar5=" + Edit_IOZ5->Text +
			"\nIsStar4=" + Edit_IOZ4->Text +
			"\nPhotometryCheck=" + BoolToStr(CheckBoxPhotometry->Checked));

		SetCell(2, 2, "[Research-options]\nId1ms=" + Edit8->Text +
			"\nId2ms=" + Edit6->Text +
			"\nId1mm=" + Edit22->Text +
			"\nId2mm=" + Edit7->Text +
			"\ninfKol=" + Edit18->Text +
			"\ntochnost=" + Edit19->Text +
			"\nFocDevice=" + Edit45->Text +
			"\nFocCol=" + Edit46->Text +
			"\nPixSize=" + Edit50->Text +
			"\nBaric=" + EditBaric->Text +
			"\nBaricCheck=" + BoolToStr(CheckBoxBaric->Checked) +
			"\nSlopeParam=" + IntToStr(ComboBoxMainPoint->ItemIndex) +

			"\n[Graphic-options]\nminXGraf=" + Edit27->Text +
			"\nmaxXGraf=" + Edit29->Text +
			"\nCheckGrafX=" + BoolToStr(CheckBox6->Checked) +
			"\nminYGraf=" + Edit30->Text +
			"\nmaxYGraf=" + Edit31->Text +
			"\nCheckGrafY=" + BoolToStr(CheckBox7->Checked) +
			"\nHeightGraf=" + Edit40->Text +
			"\nWidthGraf=" + Edit41->Text +
			"\nStep=" + Edit53->Text +
			"\nStepGrafX=" + Edit38->Text +
			"\nStepGrafY=" + Edit39->Text +
			"\nCheckStep=" + BoolToStr(CheckBox12->Checked) +
			"\nSizeBubble=" + Edit44->Text +
			"\nFixSizeBubble=" + BoolToStr(CheckBox11->Checked) +
			"\nBrGist=" + Edit56->Text +
			"\nCheckBrGist=" + BoolToStr(CheckBox16->Checked) +

			"\noptSigma1=" + Edit20->Text +
			"\noptSigma2=" + Edit21->Text +
			"\ndopSigma1=" + Edit47->Text +
			"\ndopSigma2=" + Edit48->Text +
			"\nCheckDopSigma=" + BoolToStr(CheckBox9->Checked) +
			"\nMu=" + Edit49->Text +
			"\nCheckMu=" + BoolToStr(CheckBox10->Checked) +
			"\nDeviationCenter=" + Edit55->Text +
			"\nCheckDC=" + BoolToStr(CheckBox15->Checked) +
			"\nSizeFrag=" + Edit54->Text +
			"\nNFrameMAX=" + EditNFmax->Text +
			"\nNFrameMIN=" + EditNFmin->Text +
			"\nCheckNF=" + BoolToStr(CheckBoxNF->Checked) +
			"\nPolinomDegree=" + EditMNK->Text +
			"\nCheckNFgraf=" + BoolToStr(CheckBoxNframe->Checked) +

			"\n[Sky-options]\nCheckSkyGraf=" + BoolToStr(CheckBox5->Checked) +
			"\nMinStarsInSector=" + Edit51->Text);
		SaveDoc(ReportName);
		CloseDoc();
		CloseWord();
		N3->Enabled = true;
		delete[] AprocRezX;
		delete[] AprocRezY;
		AprocRezX = NULL;
		AprocRezY = NULL;

		delete[] BoxXY;
		delete[] BoxMS;
		delete[] BoxMM;
		BoxXY = NULL;
		BoxMS = NULL;
		BoxMM = NULL;
	}

	delete bm2;
	delete bm;
	delete[] FullSigma;

	bm2 = NULL;
	bm = NULL;
	FullSigma = NULL;

	FlagClose2 = false;

	Form1->Memo1->Lines->Add("Done");
	return true;
}

void __fastcall TForm1::allTogetherButtonClick(TObject *Sender) {
	AnsiString resultFileNames[2];
    int size = 0;
	bool operationResult =  localizationStep(resultFileNames, &size);

	if(!operationResult)
		return;

	AnsiString sigmaResultFileName;
	operationResult =  sigmaStep(resultFileNames, size, &sigmaResultFileName);

	if(!operationResult)
		return;

    operationResult =  reportResult(sigmaResultFileName);
	if(!operationResult)
		ShowMessage("Ошибка при формировании отчета");
}

//---------------------------------------------------------------------------

