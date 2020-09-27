/* VS_XL5.C 25/01/95 13.49.16 */
VOID myioinit (HPROC hProc);
SHORT xlgetc (DWORD hFile, HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD MyXRead (DWORD hFile, LPSTR lpData, WORD size,
	 HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD MySeek (DWORD hFile, LONG count, WORD thing, HPROC
	 hProc);
VW_LOCALSC LONG VW_LOCALMOD MyTell (DWORD hFile, HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD GetPoint (SOPOINT VWPTR *pPoint, HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD GetInt (DWORD hFile, HPROC hProc);
VW_ENTRYSC LONG VW_ENTRYMOD GetLong (DWORD hFile, HPROC hProc);
VW_ENTRYSC SOCOLORREF VW_ENTRYMOD GetColor (HPROC hProc);
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc (SOFILE hFile, HPROC hProc);
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc (SOFILE hFile, HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD IDFileFromData (DWORD fp, HPROC hProc);
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamOpenFunc (SOFILE hFile, SHORT wFileId,
	 BYTE VWPTR *pFileName, SOFILTERINFO VWPTR *pFilterInfo, HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD AddChtObj (HPROC hProc);
VW_LOCALSC SHORT VW_LOCALMOD OpenChart (HPROC hProc);
VW_LOCALSC SHORT VW_LOCALMOD OpenSheet (HPROC hProc);
VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (SOFILE hFile, HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD MyCloseFile (HPROC hProc);
VW_VARSC WORD VW_VARMOD GotoNextXL5Sheet (HPROC hProc);
VW_VARSC VOID VW_VARMOD InitChartSectData (HPROC hProc);
VW_LOCALSC SHORT VW_LOCALMOD InitChartData (HPROC hProc);
VW_VARSC VOID VW_VARMOD DefaultPalette (HPROC hProc);
VW_VARSC VOID VW_VARMOD GiveHeading (SHORT Cell, BYTE FAR *Buffer, HPROC hProc
	);
VW_LOCALSC VOID VW_LOCALMOD PutOutColumnInfo (HPROC hProc);
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSectionFunc (SOFILE hFile, HPROC hProc);
VW_VARSC VOID VW_VARMOD DrawMark (SORECT VWPTR *pRct, WORD SeriesNum, HPROC
	 hProc);
VW_VARSC WORD VW_VARMOD PutOutLegMark (SOPOINT VWPTR *pPnt, WORD SeriesNum,
	 HPROC hProc);
VW_LOCALSC LONG VW_LOCALMOD GetRKVal (HPROC hProc);
VW_LOCALSC LONG VW_LOCALMOD GetDouble (BYTE VWPTR *pBuf, HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD MyTextAtPoint (BYTE VWPTR *pText, HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD PutOutLegend (HPROC hProc);
VW_VARSC VOID VW_VARMOD myltoa (LONG val, BYTE VWPTR *buf);
VW_VARSC SHORT VW_VARMOD ConvertToDate (LONG Date, BYTE VWPTR *pBuf, HPROC
	 hProc);
VW_VARSC WORD VW_VARMOD ConvertToString (LONG Val, BYTE Axis, BYTE VWPTR *pBuf
	, HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD GetCatName (SHORT Cat, BYTE VWPTR *pBuf, HPROC
	 hProc);
VW_LOCALSC VOID VW_LOCALMOD PutOutGrid (SHORT GridFlags, HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD GetChartValue (WORD VWPTR *Row, WORD VWPTR *Col,
	 HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD DrawBarLineChart (HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD DrawBarChart (HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD DrawStackBarChart (HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD DrawPieChart (HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD DrawLineChart (BOOL DrawFrame, HPROC hProc);
VW_VARSC WORD VW_VARMOD DrawAreaChart (HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD GetFont (SHORT Index, HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD DoScanStuff (HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD SkipData (HPROC hProc);
VW_LOCALSC WORD VW_LOCALMOD ScanData (HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD GetBrushStyle (HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD GetLineStyle (HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD GetText (HPROC hProc);
VW_LOCALSC SHORT VW_LOCALMOD GetRKNumber (WORD *pType, HPROC hProc);
VW_LOCALSC VOID VW_ENTRYMOD GetSOFormat (WORD XLFormat, PSODATACELL CellData,
	 HPROC hProc);
VW_LOCALSC SHORT HandleEOF (WORD wFirstCol, WORD wLastCol, PSODATACELL
	 pCellData, HPROC hProc);
VW_LOCALSC SHORT HandleEOSection (WORD wFirstCol, WORD wLastCol, PSODATACELL
	 pCellData, HPROC hProc);
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc (SOFILE hFile, HPROC hProc);
VW_LOCALSC SHORT VW_LOCALMOD StreamReadSheet (HPROC hProc);
VW_LOCALSC SHORT VW_LOCALMOD StreamReadChart (HPROC hProc);
VW_ENTRYSC SHORT VW_ENTRYMOD SOChartDone (HPROC hProc);
