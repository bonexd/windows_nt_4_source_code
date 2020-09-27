/*
 *		tokens.c
 *
 *		The sorted table of strings and token values
 *
 *		Note: if you insert new keywords, be sure to insert the corresponding
 *		i_keyword in the i_xxx enum in tokens.h.  This enum defines the
 *		indices used by RTFWrit to output RTF control words.
 *
 *		Original RichEdit 1.0 RTF converter: Anthony Francisco
 *		Conversion to C++ and RichEdit 2.0:  Murray Sargent
 */
#include "_common.h"
#include "tokens.h"

KEYWORD rgKeyword[] =
{
	{"animtext",	tokenAnimText},
	{"ansi",		tokenCharSetAnsi},
	{"ansicpg",		tokenAnsiCodePage},
	{"b",			tokenBold},
	{"bin",			tokenBinaryData},
	{"blue",		tokenColorBlue},
	{"bullet",		BULLET},
	{"caps",		tokenCaps},
	{"cell",		tokenCell},
	{"cellx",		tokenCellX},
	{"cf",			tokenColorForeground},
	{"colortbl",	tokenColorTable},
	{"cpg",			tokenCodePage},
	{"cs",			tokenCharStyle},
	{"deff",		tokenDefaultFont},
	{"deflang",		tokenDefaultLanguage},
	{"deftab",		tokenDefaultTabWidth},
	{"deleted",		tokenDeleted},
	{"dibitmap",	tokenPictureWindowsDIB},
	{"disabled",	tokenDisabled},
	{"dn",			tokenDown},
	{"embo",		tokenEmboss},
	{"emdash",		EMDASH},
	{"emspace",		EMSPACE},
	{"endash",		ENDASH},
	{"enspace",		ENSPACE},
	{"expndtw",		tokenExpand},
	{"f",			tokenFontSelect},
	{"fbidi",		tokenFontFamilyBidi},
	{"fcharset",	tokenCharSet},
	{"fdecor",		tokenFontFamilyDecorative},
	{"fi",			tokenIndentFirst},
	{"field",		tokenField},
	{"fldinst",		tokenFieldInstruction},
	{"fldrslt",		tokenFieldResult},
	{"fmodern",		tokenFontFamilyModern},
	{"fname",		tokenRealFontName},
	{"fnil",		tokenFontFamilyDefault},
	{"fontemb",		tokenFontEmbedded},
	{"fontfile",	tokenFontFile},
	{"fonttbl",		tokenFontTable},
	{"footer",		tokenNullDestination},
	{"footerf",		tokenNullDestination},
	{"footerl",		tokenNullDestination},
	{"footerr",		tokenNullDestination},
	{"footnote",	tokenNullDestination},
	{"fprq",		tokenPitch},
	{"froman",		tokenFontFamilyRoman},
	{"fs",			tokenFontSize},
	{"fscript",		tokenFontFamilyScript},
	{"fswiss",		tokenFontFamilySwiss},
	{"ftech",		tokenFontFamilyTechnical},
	{"ftncn",		tokenNullDestination},
	{"ftnsep",		tokenNullDestination},
	{"ftnsepc",		tokenNullDestination},
	{"green",		tokenColorGreen},
	{"header",		tokenNullDestination},
	{"headerf",		tokenNullDestination},
	{"headerl",		tokenNullDestination},
	{"headerr",		tokenNullDestination},
	{"highlight",	tokenColorBackground},
	{"hyphpar",		tokenHyphPar},
	{"i",			tokenItalic},
	{"impr",		tokenImprint},
	{"info",		tokenDocumentArea},
	{"intbl",		tokenInTable},
	{"keep",		tokenKeep},
	{"keepn",		tokenKeepNext},
	{"kerning",		tokenKerning},
	{"lang",		tokenLanguage},
	{"ldblquote",	LDBLQUOTE},
	{"li",			tokenIndentLeft},
	{"line",		tokenLineBreak},
	{"lnkd",		tokenLink},
	{"lquote",		LQUOTE},
	{"ltrpar",		tokenLToRPara},
	{"mac",			tokenCharSetMacintosh},
	{"macpict",		tokenPictureQuickDraw},
	{"noline",		tokenNoLineNumber},
	{"nosupersub",	tokenNoSuperSub},
	{"nowidctlpar", tokenNoWidCtlPar},  
	{"objattph",	tokenObjectPlaceholder},
	{"objautlink",	tokenObjectAutoLink},
	{"objclass",	tokenObjectClass},
	{"objcropb",	tokenCropBottom},
	{"objcropl",	tokenCropLeft},
	{"objcropr",	tokenCropRight},
	{"objcropt",	tokenCropTop},
	{"objdata",		tokenObjectData},
	{"object",		tokenObject},
	{"objemb",		tokenObjectEmbedded},
	{"objh",		tokenHeight},
	{"objicemb",	tokenObjectMacICEmbedder},
	{"objlink",		tokenObjectLink},
	{"objname",		tokenObjectName},
	{"objpub",		tokenObjectMacPublisher},
	{"objscalex",	tokenScaleX},
	{"objscaley",	tokenScaleY},
	{"objsetsize",	tokenObjectSetSize},
	{"objsub",		tokenObjectMacSubscriber},
	{"objupdate",   tokenObjectUpdate},
	{"objw",		tokenWidth},
	{"outl",		tokenOutline},
	{"page",		FF},
	{"pagebb",		tokenPageBreakBefore},
	{"par",			tokenEndParagraph},
	{"pard",		tokenParagraphDefault},
	{"pc",			tokenCharSetPc},
	{"pca",			tokenCharSetPs2},
	{"piccropb",	tokenCropBottom},
	{"piccropl",	tokenCropLeft},
	{"piccropr",	tokenCropRight},
	{"piccropt",	tokenCropTop},
	{"pich",		tokenHeight},
	{"pichgoal",	tokenDesiredHeight},
	{"picscalex",	tokenScaleX},
	{"picscaley",	tokenScaleY},
	{"pict",		tokenPicture},
	{"picw",		tokenWidth},
	{"picwgoal",	tokenDesiredWidth},
	{"plain",		tokenCharacterDefault},
	{"pmmetafile",	tokenPictureOS2Metafile},
	{"pn",			tokenParaNum},
	{"pnindent",	tokenParaNumIndent},
	{"pnlvlblt",	tokenParaNumBullet},
	{"pntext",		tokenParaNumText},
	{"pntxta",		tokenParaNumAfter},
	{"pntxtb",		tokenParaNumBefore},
	{"protect",		tokenProtect},
	{"qc",			tokenAlignCenter},
	{"qj",			tokenAlignJustify},
	{"ql",			tokenAlignLeft},
	{"qr",			tokenAlignRight},
	{"rdblquote",	RDBLQUOTE},
	{"red",			tokenColorRed},
	{"result",		tokenObjectResult},
	{"revauth",		tokenRevAuthor},
	{"revised",		tokenRevised},
	{"ri",			tokenIndentRight},
	{"row",			tokenRow},
	{"rquote",		RQUOTE},
	{"rtf",			tokenRtf},
	{"rtlpar",		tokenRToLPara},
	{"s",			tokenStyle},
 	{"sa",			tokenSpaceAfter},
	{"sb",			tokenSpaceBefore},
	{"sbys",		tokenSideBySide},
	{"scaps",		tokenSmallCaps},
	{"sect",		tokenEndSection},
	{"sectd",		tokenSectionDefault},
	{"shad",		tokenShadow},
	{"sl",			tokenLineSpacing},
	{"slmult",		tokenLineSpacingRule},
	{"strike",		tokenStrikeOut},
	{"stylesheet",	tokenStyleSheet},
	{"sub",			tokenSubscript},
	{"super",		tokenSuperscript},
	{"tab",			9},
	{"tb",			tokenTabBar},
	{"tc",			tokenNullDestination},
	{"tldot",		tokenTabLeaderDots},
	{"tleq",		tokenTabLeaderEqual},
	{"tlhyph",		tokenTabLeaderHyphen},
	{"tlth",		tokenTabLeaderThick},
	{"tlul",		tokenTabLeaderUnderline},
	{"tqc",			tokenCenterTab},
	{"tqdec",		tokenDecimalTab},
	{"tqr",			tokenFlushRightTab},
	{"trgaph",		tokenCellHalfGap},
	{"trleft",		tokenRowLeft},
	{"trowd",		tokenRowDefault},
	{"tx",			tokenTabPosition},
	{"u",			tokenUnicode},
	{"ul",			tokenUnderline},
	{"uld",			tokenUnderlineDotted},
	{"uldb",		tokenUnderlineDouble},
	{"ulnone",		tokenStopUnderline},
	{"ulw",			tokenUnderlineWord},
	{"up",			tokenUp},
	{"v",			tokenHiddenText},
	{"wbitmap",		tokenPictureWindowsBitmap},
	{"wbmbitspixel",tokenBitmapBitsPerPixel},
	{"wbmplanes",	tokenBitmapNumPlanes},
	{"wbmwidthbytes",tokenBitmapWidthBytes},
	{"wmetafile",	tokenPictureWindowsMetafile},
	{"xe",			tokenNullDestination}
};

INT cKeywords = sizeof(rgKeyword) / sizeof(rgKeyword[0]);

BYTE  szSymbolKeywords[] = "*-:{}\\_|~\r\n";

TOKEN tokenSymbol[] =				// Keep in same order as szSymbolKeywords
{
	tokenOptionalDestination,		// *
	0xad,							// - (optional hyphen)
	tokenIndexSubentry,				// :
	'{',							// {
	'}',							// }
	'\\',							// BSLASH
	'-',							// _ (nonbreaking hyphen; should be 0x2011)
	tokenFormulaCharacter,			// |
	0xa0,							// ~ (nonbreaking space)
	CR,								// CR
	LF								// LF
};
