//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "LocalGaussForm.h"
#include "SigmaU1.h"
#include "FilterLib.h"
#include "HeadSigma.h"
#include <iostream.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm2 *Form2;
void FilteringMovingAverage();
void BinFrameSigma();
//---------------------------------------------------------------------------
__fastcall TForm2::TForm2(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm2::N1Click(TObject *Sender)
{
	Form2 -> Close();
}
//---------------------------------------------------------------------------
void __fastcall TForm2::FormShow(TObject *Sender)
{
	Form2->Edit7->Text = Form1->Edit1->Text;
	Form2->Edit8->Text = Form1->Edit2->Text;
	Form2->Edit9->Text = Form1->Edit9->Text;
	Form2->Edit10->Text = Form1->Edit5->Text;
	Form2->CheckBox1 -> Checked = Form1->CheckBox2 -> Checked;
	if (CheckBox1 -> Checked)
	{
		Edit10 -> Enabled = true;
		Edit10 -> Color = clWindow;
	}
	else
	{
		Edit10 -> Enabled = false;
		Edit10 -> Color = cl3DLight;
	}
	Form2->Edit1->Text = Form1->Edit23->Text;

	if (StrToInt(Form1->Edit24->Text)-1>0) 	Form2->Edit2->Text = IntToStr(StrToInt(Form1->Edit24->Text)-1);
	else Form2->Edit2->Text = "0";
	Form2->Edit3->Text = IntToStr(StrToInt(Form1->Edit24->Text)+1);
	if (StrToInt(Form1->Edit25->Text)-1>0) 	Form2->Edit4->Text = IntToStr(StrToInt(Form1->Edit25->Text)-1);
	else Form2->Edit4->Text = "0";
	Form2->Edit5->Text = IntToStr(StrToInt(Form1->Edit25->Text)+1);
}
//---------------------------------------------------------------------------
void __fastcall TForm2::Button1Click(TObject *Sender)
{
	int i,j;
	FILE *stream;
	unsigned int start_time, end_time;

	//�������� �������
	if (Form2->OpenDialog1->Execute()) {

		//��������� ������ ��� ���� �������� � ������������
		SigmaSet.Kadr = new WORD*[StrToInt(Form2->Edit8->Text)];
		SigmaSet.FiltKadr = new WORD*[StrToInt(Form2->Edit8->Text)];

		for (i = 0; i < StrToInt(Form2->Edit8->Text); i++) {
			SigmaSet.Kadr[i] = new WORD[StrToInt(Form2->Edit7->Text)];
			SigmaSet.FiltKadr[i] = new WORD[StrToInt(Form2->Edit7->Text)];
		}
		SigmaSet.Height = StrToInt(Form2->Edit8->Text);
		SigmaSet.Width  = StrToInt(Form2->Edit7->Text);

		ProgressBar1->Position = 0;
		ProgressBar1->Max = (StrToInt(Form2->Edit3->Text)-StrToInt(Form2->Edit2->Text))*(StrToInt(Form2->Edit5->Text)-StrToInt(Form2->Edit4->Text));

		stream = fopen(AnsiString(Form2->OpenDialog1->Files->Strings[0]).c_str(), "rb");
		for (i = 0; i < SigmaSet.Height; i++)
		fread(SigmaSet.Kadr[i], sizeof(WORD), SigmaSet.Width, stream);
		fclose(stream);

		FilteringMovingAverage(); //��������� �������� ����� � �������� �������+Edit17*���
		SigmaSet.x = Xloc[0];
		SigmaSet.y = Yloc[0];

		SigmaSet.HWframe = StrToInt(Form1->Edit9->Text);
		SigmaSet.Frame = new WORD*[SigmaSet.HWframe];
		for (i = 0; i < SigmaSet.HWframe; i++)
		SigmaSet.Frame[i] = new WORD[SigmaSet.HWframe];

		if (Nobj != 1) {    //���-�� ������������� ����� ������ 1 ��� = 0
			Form1->Memo1->Lines->Add(TimeToStr(Time()) +"  ������ �� �������!");
		}
		else {              //���-�� ������������� ����� = 1

			if (Form2->CheckBox1->Checked)
				BinFrameSigma();  //�����������
			else
			{
				//��������� ������
				for (i = 0; i < SigmaSet.HWframe; i++)
				for (j = 0; j < SigmaSet.HWframe; j++)
				if ( (int)(i+SigmaSet.y-SigmaSet.HWframe/2) >= 0 && (int)(i+SigmaSet.y-SigmaSet.HWframe/2) < SigmaSet.Height &&
					 (int)(j+SigmaSet.x-SigmaSet.HWframe/2) >= 0 && (int)(j+SigmaSet.x-SigmaSet.HWframe/2) < SigmaSet.Width )
				{
					SigmaSet.Frame[i][j] = SigmaSet.Kadr[(int)(i+SigmaSet.y-SigmaSet.HWframe/2)][(int)(j+SigmaSet.x-SigmaSet.HWframe/2)];
				}
				else SigmaSet.Frame[i][j] = 0;
			}

			F4Lines = 0;
			Filter4Lines(StrToInt(Form1->Edit28->Text), SigmaSet.Frame, SigmaSet.HWframe, SigmaSet.HWframe); //���������� �� 4� �������
			SelectMaxObject(SigmaSet.Frame, SigmaSet.HWframe, SigmaSet.HWframe, SigmaSet.HWframe/2, SigmaSet.HWframe/2, true);

			//�����
			int *pix_X	= new int [SigmaSet.HWframe*SigmaSet.HWframe];
			int *pix_Y	= new int [SigmaSet.HWframe*SigmaSet.HWframe];
			int *pix_val= new int [SigmaSet.HWframe*SigmaSet.HWframe];

			int numMeas = 0;

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
					if (SigmaSet.Frame[i][j] > 0) {
						pix_val[numMeas] = SigmaSet.Frame[i][j]+1;
						pix_X[numMeas] = j;
						pix_Y[numMeas++] = i;
					}
				}


			stream = fopen(AnsiString(Form2->Edit1->Text + "\\TestLocalGauss.txt").c_str(), "a");

//			int mass1[19];
//			mass1[0] = 1;
//			mass1[1] = 2;
//			mass1[2] = 3;
//			mass1[3] = 4;
//			mass1[4] = 5;
//			mass1[5] = 6;
//			mass1[6] = 7;
//			mass1[7] = 8;
//			mass1[8] = 9;
//			mass1[9] = 10;
//			mass1[10] = 15;
//			mass1[11] = 20;
//			mass1[12] = 25;
//			mass1[13] = 30;
//			mass1[14] = 35;
//			mass1[15] = 40;
//			mass1[16] = 45;
//			mass1[17] = 50;
//			mass1[18] = 100;
//			ProgressBar1->Max = 19*19;
//			int it, tr;
//			for (it = 0; it < 19; it++)
//			for (tr = 0; tr < 19; tr++) {
//				double Mug=0, SigX=0, SigY=0, Sx0=0, Sy0=0;
//				start_time = clock();
//				LocalGauss(pix_X, pix_Y, pix_val, numMeas, mass1[it], mass1[tr], &Sx0, &Sy0, &SigX, &SigY, &Mug);  //numMeas or numMeas-1
//				end_time = clock();
//				fprintf(stream, "%d	%d	%f	%f	%f	%f	%f	%f\n", mass1[it], mass1[tr], Mug, Sx0, Sy0, SigX, SigY, (end_time - start_time)/1000.0);
//				ProgressBar1->Position++;
//				Application->ProcessMessages();
//			}

			int it, tr;
			for (it = StrToInt(Form2->Edit2->Text); it < StrToInt(Form2->Edit3->Text)+1; it++)
			for (tr = StrToInt(Form2->Edit4->Text); tr < StrToInt(Form2->Edit5->Text)+1; tr++) {
				double Mug=0, SigX=0, SigY=0, Sx0=0, Sy0=0;
				start_time = clock();
//				LocalGauss(pix_X, pix_Y, pix_val, numMeas, it, tr, &Sx0, &Sy0, &SigX, &SigY, &Mug);  //numMeas or numMeas-1
				end_time = clock();
				fprintf(stream, "%d	%d	%f	%f	%f	%f	%f	%f\n", it, tr, Mug, Sx0, Sy0, SigX, SigY, (end_time - start_time)/1000.0);
				ProgressBar1->Position++;
			}
			delete [] pix_X;
			delete [] pix_Y;
			delete [] pix_val;
			fclose(stream);
		}

		delete [] SigmaSet.FiltKadr;
		delete [] SigmaSet.Frame;
		delete [] SigmaSet.Kadr;

	}
}
//---------------------------------------------------------------------------
void __fastcall TForm2::CheckBox1Click(TObject *Sender)
{
	if (CheckBox1 -> Checked)
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

