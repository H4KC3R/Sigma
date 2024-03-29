#define GLOBVAL
#include "HeadSigma.h"
#include <math.h>

#include <iostream>
#include <fstream>

using namespace std;
//---------------------------------------------------------------------------
void FindDirectories(AnsiString FindDir, AnsiString MaskF)
{
  TSearchRec srd, srf;
	  if (FindFirst(FindDir + "\\" + MaskF, faAnyFile, srf) == 0)
	  {
		if (DirCount < MAX_DIRECT)
		{
		  DirNames[DirCount]=FindDir;
		  DirCount++;
		}
		FindClose(srf);
	  }

	  if (FindFirst(FindDir+"\\*", faDirectory, srd) == 0)
	  {
		do
		{
		  if ((srd.Attr & faDirectory)&&(srd.Name!=".")&&(srd.Name!=".."))
		  {
			FindDirectories(FindDir+"\\"+srd.Name, MaskF);
		  }
		} while (FindNext(srd) == 0);
	  }
}

void FindFiles(AnsiString MaskF)
{
	TSearchRec sr;
	int i;
	for (i = 0; i < DirCount; i++) {
		if (FindFirst(DirNames[i] + "\\" + MaskF , faAnyFile, sr) == 0)
		  {
			do
			{
			  if ((sr.Attr & faAnyFile) == sr.Attr)
			  {
				FileNames[FilesCount]= DirNames[i] + "\\" + sr.Name;
				FilesCount++;
			  }
			} while ((FindNext(sr) == 0)&&(FilesCount<=MAX_FILE));
			FindClose(sr);
		  }
	  }
}

//---------------------------------------------------------------------------
void IdentifFromName(char *name, char *Ind1, char *Ind2, float *Rez)
{
	int i = 0, j, h1, h2, n, g, lenInd;
	char *filename = new char[300];

	//��� �����
	int lenName = strlen(name);
	while (name[--lenName] != '\\')
		filename[i++] = name[lenName];

	filename[i] = '\0';
	filename = strrev(filename);

	//����� �������������� 1
	h1 = 0;
	lenInd = strlen(Ind1);
	lenName = strlen(filename);
	char *IndNom = new char[lenInd+1];

	for (i = 0; i < lenName; i++) {
		if (filename[i] == Ind1[0]) {
			for (g = 0; g < lenInd; g++)  IndNom[g] = '\0';
			n = i; j = 0;
			while (filename[n++] == Ind1[j++]) {
				IndNom[j-1] = filename[n-1];
			}
			IndNom[j-1] = '\0';
			if (strcmp(IndNom, Ind1) == 0) {
			h1 = i;
			}
		}
	}
	delete [] IndNom;

	//����� �������������� 2
	h2 = 0;
	lenInd = strlen(Ind2);
	lenName = strlen(filename);
	IndNom = new char[lenInd+1];

	for (i = 0; i < lenName; i++) {
		if (filename[i] == Ind2[0]) {
			for (g = 0; g < lenInd; g++)  IndNom[g] = '\0';
			n = i; j = 0;
			while (filename[n++] == Ind2[j++]) {
				IndNom[j-1] = filename[n-1];
			}
			IndNom[j-1] = '\0';
			if (strcmp(IndNom, Ind2) == 0) {
			h2 = i;
			}
		}
	}
	delete [] IndNom;

	i = 0;
	int nomRezz = 100;
	char *rezz = new char[nomRezz];

	if (h2 > 0) {
		for (g = 0; g < nomRezz; g++)  rezz[g] = '\0';
		while (--h2>=(h1+strlen(Ind1)) && (h2 >= 0)) {
			rezz[i++] = filename[h2];
		}
		for (g = i; g < nomRezz; g++)  rezz[g] = '\0';
		rezz = strrev(rezz);

//		FormatSettings.DecimalSeparator = '.';
//		�������� �� ��, ��� � ������ ���� ����� ���� '.'
		i = 0;
		while ( (rezz[i] != ',') && (i < strlen(rezz)) )
		i++;
		if (i != (strlen(rezz)))
		rezz[i] = '.';

		i = 0;
		while ((isdigit(rezz[i]) || rezz[i] == '.') && i<strlen(rezz))  i++;
		if (i == strlen(rezz))
			*Rez = StrToFloat(rezz);
		else *Rez = 0;

	}
	else *Rez = 0;

	delete [] rezz;
	delete [] filename;
}
//---------------------------------------------------------------------------
void LocalNI (unsigned short *DATA_ALLOC, int Width, int Height, int BRmin, int BRmax, int NPmin, int NPmax)
{
  unsigned short Th;
  float sxp[8210];
  float syp[8210];
  float sbp[8210];
  int   snp[8210];    //8176

  int i,j,ii,jj,Nel,Nelp,Si,W, Br, Nstarg;
  float Bcc,XX,YY;

	for (i = 0; i < MaxObj; i++) {
		BRloc[i] = 0;
		Xloc[i] = 0;
		Yloc[i] = 0;
	}
	Nobj = 0;

  Th = 0;
//  #ifdef AVU_DEV
//  DATA_ALLOC = (unsigned short*)TREATFRAME_ALLOC;
//  #endif

	WORD **cadr;
	cadr = new WORD*[Height];
	for (i = 0; i < Height; i++)
		cadr[i] = new WORD[Width];

	int n = 0;
	for (i = 0; i < Height; i++)
	for (j = 0; j < Width; j++)
	cadr[i][j] = DATA_ALLOC[n++];

//  unsigned short (*cadr)[];
//  cadr=(unsigned short(*)[])DATA_ALLOC;

	unsigned short Ncl = Width, Nrw = Height;

  for (i=0; i<Ncl; i++)
  {
	sxp[i]=0.0; syp[i]=0.0; sbp[i]=0.0; snp[i]=0;
  }
  Nstarg=0;
  i=0;

  while ((i<Nrw) && (Nstarg<MaxObj))
  {
	W=0; Si=0;
    j=0;
	while ((j<Ncl) && (Nstarg<MaxObj))
	{
	Br=cadr[i][j];
	if (Br>Th)
	  {
	  XX=(float)(Br-Th);
	  (snp[j])+=1;
	  (sbp[j])+=XX;
	  (sxp[j])+=XX*((float)i);
      (syp[j])+=XX*((float)j);
      W=1;
	  }
    if (snp[j]>0)
	  {
      if (Si==0)
	{
	Si=1; ii=j;
	}
	  }
    else
	  {
	  if ((Si!=0) && (W==0))
	{
	Bcc=0.0; XX=0.0; YY=0.0; Nel=0;
	for (jj=(j-1); jj>=ii; jj--)
	  {
	  Bcc+=sbp[jj]; Nel+=snp[jj];
	  XX+=sxp[jj]; YY+=syp[jj];
	  sbp[jj]=0.0;
	  sxp[jj]=0.0; syp[jj]=0.0;
	  snp[jj]=0;

//	  FILE *streG = fopen(AnsiString("C:\\Cybertron\\Programms\\List.txt").c_str(), "a");
//	  fprintf(streG, "%d	%d	%d	%f	%f	%f	%d\n", &cadr[i][j], i, j, XX, YY, Bcc, Nel);
//	  fclose(streG);
	  }
	if ((Bcc<=BRmax) && (Bcc>=BRmin) && (Nel>=NPmin) && (Nel<=NPmax))
	  {
		  XX=XX/Bcc; YY=YY/Bcc;
//	  Xloc[Nstarg]=Ncl-(YY+0.5);
	  Xloc[Nstarg]=(YY+0.5);
	  Yloc[Nstarg]=XX+0.5;
//	  NNNg[Nstarg]=Nstarg;
		  BRloc[Nstarg]=Bcc;
		  NelLoc[Nstarg]=Nel;
	  Nstarg++;


//	  FILE *streG = fopen(AnsiString("C:\\Cybertron\\Programms\\List.txt").c_str(), "a");
//	  fprintf(streG, "STAR!	%d	%d	%d	%f	%f	%d	%d\n", cadr[i][j], i, j,Xloc[Nstarg-1],Yloc[Nstarg-1],BRloc[Nstarg-1],NelLoc[Nstarg-1]);
//	  fclose(streG);
		  }
	}
	  W=0; Si=0;
	  }
	j++;
	}
  sbp[Ncl-1]=0;
  i++;

  }
  Nobj=Nstarg;


//if (Form1->CheckBox4->Checked == true) {
////  fprot = fopen(AnsiString(AdrFolder + "LOC_.txt").c_str(), "w");
//  if (fprot != NULL) {
////	  fprintf(fprot,"\n����� �������� �� �����: %4d\n", NobjF);
////	  fprintf(fprot,"\n������������ ��������:   %4d\n", Nobj);
//
//	  //��������� � ������� ������ �����
//	  for (i = 0; i < Nobj; i++)
//	  if ( (Xloc[i] < OgrV ) && (Xloc[i] > OgrN) && (Yloc[i] < OgrV) && (Yloc[i] > OgrN) )
//	  {
//		  fprintf(fprot,"%10.5f %10.5f %10.5f %10.5f %6d %6d %6d %10.5f %10.5f",Xloc[i], Yloc[i], Xloc[i]-xConstBox[ix*RazmB+iy], Yloc[i]-yConstBox[ix*RazmB+iy], BRloc[i],NelLoc[i], (int)(BufLOC1 + BufLOCdSKO*StrToInt(Form1->Edit28->Text) + StrToInt(Form1->Edit14->Text) + 0.5), BufLOC1, BufLOCdSKO);
//
//	  }
//
//
////	bool ch4 = false;
////	  for (i=0; i<Nobj;i++)
////	  if (NelLoc[i]>=3 && ch4==false) {
////		  fprintf(fprot,"%10.5f %10.5f %10.5f %10.5f %6d %6d",Xloc[i], Yloc[i], Xloc[i]-xConstBox[razm], Yloc[i]-yConstBox[razm], BRloc[i],NelLoc[i]);
////		  ch4=true;
////	  }
////	  else {
////		 if (NelLoc[i]<3 && Nobj<2 && ch4==false)  fprintf(fprot,"%10.5f %10.5f %10.5f %10.5f %6d %6d Less than 3 pixels",Xloc[i], Yloc[i], Xloc[i]-xConstBox[razm], Yloc[i]-yConstBox[razm], BRloc[i],NelLoc[i]);
////		 else{
////			if (NelLoc[i]<3 && Nobj>=2 && ch4==false) fprintf(fprot,"Less than 3 pixels and more 1 Star localization \n");
////			else  if (ch4==false) fprintf(fprot,"More 1 Star localization \n");
////		  }
////	  }
////	  if (Nobj>1) {
////	   fprintf(fprot,"!!!ERROR!!!\n");
////	  }
////	  else {  // ������ ������� ��������
//////	  fprintf(fprot,"%6d %10.4f %10.4f %6d %6d %6d\n", i+1, Xloc[i], Yloc[i], BRloc[i],NelLoc[i], PixMax[i]);
////		fprintf(fprot,"%10.5f %10.5f %10.5f %10.5f %6d %6d",Xloc[i], Yloc[i], Xloc[i]-xConstBox[razm], Yloc[i]-yConstBox[razm], BRloc[i],NelLoc[i]);
////	  }
//  } }
////  fclose(fprot);
	for (i = 0; i < Height; i++) delete [] cadr[i];
	delete [] cadr;
}
//---------------------------------------------------------------------------
void SelectMaxObject(WORD **Kadr, int Wsize, int Hsize, float Xcor, float Ycor, bool SaveFrame)
{
	//��������� �������� ����� �� �������
	//SaveFrame = false ���� �� �����, ������ ����� ����� ������� ������� �� ������� ��������� ����������
	//SaveFrame = true �������� ���� � ��������� �� ��� 1 ����������� ������
	int flafD = 1, i, j;

	int **Mask;
	Mask = new int*[Hsize];
	for (i = 0; i < Hsize; i++) Mask[i] = new int[Wsize];

	//�� ������� �������� ���� � ��������� �������, ��� �� �������� �������
	//������ ��������� ������� �����������
	for (i = 0; i < Hsize; i++)
	for (j = 0; j < Wsize; j++) {
		if (i == 0 || j == 0) {
			Mask[i][j] = 0;
		}
		else {
			if (Kadr[i][j] > 0) { Mask[i][j] = flafD++; }
			else Mask[i][j] = 0;
		}
	}

//	FILE *stre3 = fopen(AnsiString("C:\\Cybertron\\Programms\\WindKBM-1.dat").c_str(), "wb");
//	for (j = 0; j < Hsize; j++) {
//	fwrite(Mask[j], sizeof(int), Wsize, stre3);}
//	fclose(stre3);

	int* Group = new int [flafD];
	for (j = 0; j < flafD; j++) Group[j] = j;

	//������� �������� �������
	for (i = 1; i < Hsize-1; i++)
	for (j = 1; j < Wsize-1; j++) {

		if (Mask[i][j] > 0) {

			if (Mask[i-1][j] > 0)   {
				if (Mask[i-1][j] <= Mask[i][j]) { Group[Mask[i][j]] = Group[Mask[i-1][j]];  Mask[i][j] = Mask[i-1][j];  }
				else 						    { Group[Mask[i-1][j]] = Group[Mask[i][j]];  Mask[i-1][j] = Mask[i][j];  }
			}

			if (Mask[i-1][j+1] > 0)   {
				if (Mask[i-1][j+1] <= Mask[i][j]) { Group[Mask[i][j]] = Group[Mask[i-1][j+1]];  Mask[i][j] = Mask[i-1][j+1];  }
				else 						    { Group[Mask[i-1][j+1]] = Group[Mask[i][j]];  Mask[i-1][j+1] = Mask[i][j];  }
			}
		   //���������

			if (Mask[i-1][j-1] > 0) {                                                                                   //���������
				if (Mask[i-1][j-1] <= Mask[i][j]) { Group[Mask[i][j]] = Group[Mask[i-1][j-1]]; Mask[i][j] = Mask[i-1][j-1]; }   //���������
				else 							 { Group[Mask[i-1][j-1]] = Group[Mask[i][j]];  Mask[i-1][j-1] = Mask[i][j];  }                             //���������
			}
			if (Mask[i][j-1] > 0) {
				if (Mask[i][j-1] <= Mask[i][j]) { Group[Mask[i][j]] = Group[Mask[i][j-1]]; Mask[i][j] = Mask[i][j-1]; }
				else 						   { Group[Mask[i][j-1]] = Group[Mask[i][j]]; Mask[i][j-1] = Mask[i][j];  }
			}

		}

	}

//	FILE *stre4 = fopen(AnsiString("C:\\Cybertron\\Programms\\WindKBM-G.txt").c_str(), "a");
//	for (j = 0; j < flafD; j++)
//	fprintf(stre4, "%d	%d\n", j, Group[j]);
//
//	stre3 = fopen(AnsiString("C:\\Cybertron\\Programms\\WindKBM-2.dat").c_str(), "wb");
//	for (j = 0; j < Hsize; j++) {
//	fwrite(Mask[j], sizeof(int), Wsize, stre3);}
//	fclose(stre3);

	for (i = 0; i < Hsize; i++)
	for (j = 0; j < Wsize; j++) {
		if (Mask[i][j] > 0) {
			while (Mask[i][j] > Group[Mask[i][j]]) {
				Mask[i][j] = Group[Mask[i][j]];
			}
		}
	}

//	stre3 = fopen(AnsiString("C:\\Cybertron\\Programms\\WindKBM-3.dat").c_str(), "wb");
//	for (j = 0; j < Hsize; j++) {
//	fwrite(Mask[j], sizeof(int), Wsize, stre3);}
//	fclose(stre3);


	//�������� �����
	if (SaveFrame) {
		//��� ������� ����� ���� �����
		//���� ��������� �� ����������� ������� �������, �� ������� 0
		int MaxNpx = 0, MaxSch = 0, MaxNom = 0, jj;

		if (Mask[(int)Xcor][(int)Ycor] > 0)
			MaxNom = Mask[(int)Xcor][(int)Ycor];

		//�������� ���� �������� - ������� �� ����������� � �������
		for (i = 1; i < SigmaSet.HWframe; i++)
		for (j = 1; j < SigmaSet.HWframe; j++) {
			if (Mask[i][j] != MaxNom)
			{
				Mask[i][j] = 0;
				SigmaSet.Frame[i][j] = 0;
			}
			else SigmaSet.Frame[i][j] = Kadr[i][j];
		}

		//������� ����� ������� � ���-�� ��������
		SigmaSet.Is2 = 0;
		SigmaSet.N2 = 0;
		for (i = 1; i < SigmaSet.HWframe; i++)
		for (j = 1; j < SigmaSet.HWframe; j++) {
			if (SigmaSet.Frame[i][j]>0)
			{
				SigmaSet.Is2 += SigmaSet.Frame[i][j];
				SigmaSet.N2++;
			}
        }
//		stre3 = fopen(AnsiString("C:\\Cybertron\\Programms\\WindKBM-4.dat").c_str(), "wb");
//		for (j = 0; j < Hsize; j++) {
//		fwrite(Mask[j], sizeof(int), Wsize, stre3);}
//		fclose(stre3);


	}
	else {   //��������� ������ ������ ������� � ������
		int MaxNom = 0;
		SigmaSet.Ispic = 0;
		MaxNom = Mask[(int)(Ycor)][(int)(Xcor)];

		if (MaxNom > 0) {
			for (i = 0; i < Hsize; i++)
			for (j = 0; j < Wsize; j++) {
				if (Mask[i][j] == MaxNom)
				if (SigmaSet.Kadr[i][j] > SigmaSet.Ispic)
				SigmaSet.Ispic = SigmaSet.Kadr[i][j];
			}
		}
		else SigmaSet.Ispic = 0;

	}
	delete [] Group;
	for (i = 0; i < Hsize; i++)  delete [] Mask[i];
	delete [] Mask;
}
//---------------------------------------------------------------------------
void Filter4Lines(int koff, WORD **Kadr, int Wsize, int Hsize)
{
	//���������� �� 4� �������
	int jLOC = 0, i, j;
//	float BufLOC = 0;
	BufLOCdSKO = 0;
	BufLOC1 = 0;

	for (j = 0; j < 4; j++){
	for (i = 0; i < Wsize; i++)
		BufLOC1 += Kadr[jLOC][i];
		if (j == 0) jLOC++;
		else jLOC = Wsize-j;
	}

	BufLOC1 = BufLOC1/(Wsize*4);

	jLOC = 0;
	for (j = 0; j < 4; j++){
	for (i = 0; i < Wsize; i++)
		BufLOCdSKO += (Kadr[jLOC][i] - BufLOC1) * (Kadr[jLOC][i] - BufLOC1);
		if (j == 0) jLOC++;
		else jLOC = Wsize-j;
	}

	BufLOCdSKO = sqrt(BufLOCdSKO/(Wsize*4));

	F4Lines = (int)(BufLOC1 + BufLOCdSKO*koff);

	Is_fot = 0;
	for (i = 0; i < Hsize; i++)
	{
		for (j = 0; j < Wsize; j++)
		{
			if (BKflag)
			{
				Is_fot += Kadr[i][j] - (BlackKadrWindow[i][j] + 0.5);
			}
			else
			{
				Is_fot += Kadr[i][j] - BufLOC1;
			}
			if ((int)(Kadr[i][j] - F4Lines)<=0)
			{
				Kadr[i][j] = 0;
			}
			else
			{
				Kadr[i][j] = (int)(Kadr[i][j] - F4Lines);
			}
		}
	}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

