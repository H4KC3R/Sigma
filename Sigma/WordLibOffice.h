void OpenWord(bool Visi);
void CloseWord(void);
void AddDoc(void);
void SaveDoc(AnsiString Name);
void CloseDoc(void);
void AddParagraph(AnsiString Text);
void AddPicture(AnsiString Name);
void AddTable(int Nrow, int Ncol);
void SetActiveTable(int num);
void SetCell(int Nrow, int Ncol, AnsiString str);
void SetTextFormat(float TextSize, long IsBold, long IsItalic,
						long IsUnderline, long TypeAlign, float LineUnit);
void SetBreak();
void AddParagraphNo(AnsiString Text);
void SetLineUnitAfter(float LineUnit);
void OpenW(bool Visible, AnsiString Filename);
void UnionCell(int Nrow_1, int Ncol_1, int Nrow_2, int Ncol_2);
void SetCellFormat(int Nrow, int Ncol, float TextSize, long IsBold, long IsItalic, long IsUnderline);
void SetTextColor(TColor a);

