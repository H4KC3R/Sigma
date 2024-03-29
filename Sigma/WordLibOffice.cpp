//#include "Word_2K_SRVR.h"
#include "Word_XP_srvr.h"

TWordApplication *WordApplication1;
TWordDocument *WordDocument1;
TablePtr table;
TVariant SaveChanges, Direction; //= wdCollapseEnd;
int NumDoc, NumPar, NumTab;

void OpenWord(bool Visi)
{
   WordApplication1=new TWordApplication(0);
   WordDocument1=new TWordDocument(0);

   WordApplication1->Connect();   // ��������� ���������� � ��������
   WordApplication1->set_Visible(Visi);  // ������ Word �������

   NumDoc=0;
}

void CloseWord(void)
{
   WordApplication1->Quit();
   WordApplication1->Disconnect();
}

void AddDoc(void)
{
   NumDoc++;
   NumPar=0;
   NumTab=0;

   WordApplication1->Documents->Add();   // ������� ����� ��������
   WordDocument1->ConnectTo(WordApplication1->ActiveDocument); // ��������� WordDocument � �������� ����������
}

void SaveDoc(AnsiString Name)
{
   WordDocument1->SaveAs(TVariant(Name));
}

void CloseDoc(void)
{
   WordDocument1->Close();
   WordDocument1->Disconnect();
}

void AddParagraph(AnsiString Text)
{
   NumPar++;
   WordDocument1->Range(TVariant(WordDocument1->Characters->Count-1), EmptyParam())->
				InsertAfter(TVariant(Text+"\n"));
}

void AddPicture(AnsiString Name)
{
   WordDocument1->Range(TVariant(WordDocument1->Characters->Count-1), EmptyParam())->
                InlineShapes->AddPicture(StringToOleStr(Name));
   AddParagraph("");
}

void AddTable(int Nrow, int Ncol)
{
   NumTab++;
   WordDocument1->Tables->Add(WordDocument1->Range(TVariant(WordDocument1->Characters->Count-1) ,EmptyParam()),
						Nrow, Ncol, TVariant(1),TVariant(0));
   table = WordDocument1->Tables->Item(NumTab);
   table->LeftPadding  = 0;//������� � ������� �����
   table->RightPadding = 0;//� ������
}

void SetActiveTable (int num)
{
   table = WordDocument1->Tables->Item(num);
}

void SetCell(int Nrow, int Ncol, AnsiString str)
{
   table->Cell(Nrow, Ncol)->Range->InsertAfter( TVariant(str));
}

void SetCellFormat(int Nrow, int Ncol, float TextSize, long IsBold, long IsItalic, long IsUnderline)
{
   table->Cell(Nrow, Ncol)->Range->Font->set_Bold(IsBold);
   table->Cell(Nrow, Ncol)->Range->Font->set_Italic(IsItalic);
   table->Cell(Nrow, Ncol)->Range->Font->set_Underline(IsUnderline);
   table->Cell(Nrow, Ncol)->Range->Font->set_Size(TextSize);
}
void SetTextColor(TColor a)
{
   WordDocument1->Paragraphs->get_Last()->Range->Font->set_Color(a);
}
void SetTextFormat(float TextSize, long IsBold, long IsItalic, long IsUnderline, long TypeAlign, float LineUnit)
{
   WordDocument1->Paragraphs->get_Last()->Range->Font->set_Bold(IsBold);
   WordDocument1->Paragraphs->get_Last()->Range->Font->set_Italic(IsItalic);
   WordDocument1->Paragraphs->get_Last()->Range->Font->set_Underline(IsUnderline);
   WordDocument1->Paragraphs->get_Last()->Range->Font->set_Size(TextSize);
   WordDocument1->Paragraphs->get_Last()->Format->set_Alignment(WdParagraphAlignment(TypeAlign));
   WordDocument1->Paragraphs->get_Last()->set_LineUnitAfter(LineUnit);
}
void SetBreak()
{
	const int wdPageBreak = 7;
	TVariant b_var = wdPageBreak;
	WordDocument1->Paragraphs->get_Last()->Range->InsertBreak(&b_var);
}
void AddParagraphNo(AnsiString Text)
{
   NumPar++;
   WordDocument1->Range(TVariant(WordDocument1->Characters->Count-1), EmptyParam())->InsertAfter(TVariant(Text));
//   WordDocument1->r
//   TVariant b_var = 0;
//   WordDocument1->Paragraphs->get_Last()->Range->Collapse(&b_var);
}
void SetLineUnitAfter(float LineUnit)
{
   WordDocument1->Paragraphs->get_Last()->set_LineUnitAfter(LineUnit);
}
void OpenW(bool Visible, AnsiString Filename)
{
	WordApplication1->Documents->Open(TVariant(Filename));
//	WordDocument1->ConnectTo(WordApplication1->ActiveDocument); // ��������� WordDocument � �������� ����������

}
void UnionCell(int Nrow_1, int Ncol_1, int Nrow_2, int Ncol_2)
{
   table = WordDocument1->Tables->Item(NumTab);
   table->Cell(Nrow_1,Ncol_1)->Merge(table->Cell(Nrow_2, Ncol_2));
}
