#pragma once
#include <ScintillaTypes.h>
#include <ScintillaMessages.h>
#include <ScintillaStructures.h>

using namespace Scintilla;

template <typename T>
class CScintillaImpl : public CWindowImpl<T, CWindow, CControlWinTraits>
{
public:
	DECLARE_WND_SUPERCLASS2(L"CScintillaImpl", CScintillaImpl, CWindow::GetWndClassName())

	Accessibility GetAccessibility() { return static_cast<Accessibility>(Call(Message::GetAccessibility)); }
	Alpha GetAdditionalSelAlpha() { return static_cast<Alpha>(Call(Message::GetAdditionalSelAlpha)); }
	Alpha GetCaretLineBackAlpha() { return static_cast<Alpha>(Call(Message::GetCaretLineBackAlpha)); }
	Alpha GetSelAlpha() { return static_cast<Alpha>(Call(Message::GetSelAlpha)); }
	Alpha IndicGetAlpha(int indicator) { return static_cast<Alpha>(Call(Message::IndicGetAlpha, indicator)); }
	Alpha IndicGetOutlineAlpha(int indicator) { return static_cast<Alpha>(Call(Message::IndicGetOutlineAlpha, indicator)); }
	AnnotationVisible AnnotationGetVisible() { return static_cast<AnnotationVisible>(Call(Message::AnnotationGetVisible)); }
	AutoCompleteOption AutoCGetOptions() { return static_cast<AutoCompleteOption>(Call(Message::AutoCGetOptions)); }
	AutomaticFold GetAutomaticFold() { return static_cast<AutomaticFold>(Call(Message::GetAutomaticFold)); }
	Bidirectional GetBidirectional() { return static_cast<Bidirectional>(Call(Message::GetBidirectional)); }
	CaretSticky GetCaretSticky() { return static_cast<CaretSticky>(Call(Message::GetCaretSticky)); }
	CaretStyle GetCaretStyle() { return static_cast<CaretStyle>(Call(Message::GetCaretStyle)); }
	CaseInsensitiveBehaviour AutoCGetCaseInsensitiveBehaviour() { return static_cast<CaseInsensitiveBehaviour>(Call(Message::AutoCGetCaseInsensitiveBehaviour)); }
	CaseVisible StyleGetCase(int style) { return static_cast<CaseVisible>(Call(Message::StyleGetCase, style)); }
	ChangeHistoryOption GetChangeHistory() { return static_cast<ChangeHistoryOption>(Call(Message::GetChangeHistory)); }
	CharacterSet StyleGetCharacterSet(int style) { return static_cast<CharacterSet>(Call(Message::StyleGetCharacterSet, style)); }
	Colour GetAdditionalCaretFore() { return static_cast<Colour>(Call(Message::GetAdditionalCaretFore)); }
	Colour GetCaretFore() { return static_cast<Colour>(Call(Message::GetCaretFore)); }
	Colour GetCaretLineBack() { return static_cast<Colour>(Call(Message::GetCaretLineBack)); }
	Colour GetEdgeColour() { return static_cast<Colour>(Call(Message::GetEdgeColour)); }
	Colour GetHotspotActiveBack() { return static_cast<Colour>(Call(Message::GetHotspotActiveBack)); }
	Colour GetHotspotActiveFore() { return static_cast<Colour>(Call(Message::GetHotspotActiveFore)); }
	Colour GetMarginBackN(int margin) { return static_cast<Colour>(Call(Message::GetMarginBackN, margin)); }
	Colour IndicGetFore(int indicator) { return static_cast<Colour>(Call(Message::IndicGetFore, indicator)); }
	Colour IndicGetHoverFore(int indicator) { return static_cast<Colour>(Call(Message::IndicGetHoverFore, indicator)); }
	Colour StyleGetBack(int style) { return static_cast<Colour>(Call(Message::StyleGetBack, style)); }
	Colour StyleGetFore(int style) { return static_cast<Colour>(Call(Message::StyleGetFore, style)); }
	ColourAlpha GetElementBaseColour(Element element) { return static_cast<ColourAlpha>(Call(Message::GetElementBaseColour, static_cast<uintptr_t>(element))); }
	ColourAlpha GetElementColour(Element element) { return static_cast<ColourAlpha>(Call(Message::GetElementColour, static_cast<uintptr_t>(element))); }
	ColourAlpha GetRepresentationColour(const char* encodedCharacter) { return static_cast<ColourAlpha>(Call(Message::GetRepresentationColour, reinterpret_cast<uintptr_t>(encodedCharacter))); }
	CursorShape GetCursor() { return static_cast<CursorShape>(Call(Message::GetCursor)); }
	CursorShape GetMarginCursorN(int margin) { return static_cast<CursorShape>(Call(Message::GetMarginCursorN, margin)); }
	DocumentOption GetDocumentOptions() { return static_cast<DocumentOption>(Call(Message::GetDocumentOptions)); }
	EOLAnnotationVisible EOLAnnotationGetVisible() { return static_cast<EOLAnnotationVisible>(Call(Message::EOLAnnotationGetVisible)); }
	EdgeVisualStyle GetEdgeMode() { return static_cast<EdgeVisualStyle>(Call(Message::GetEdgeMode)); }
	EndOfLine GetEOLMode() { return static_cast<EndOfLine>(Call(Message::GetEOLMode)); }
	FindOption GetSearchFlags() { return static_cast<FindOption>(Call(Message::GetSearchFlags)); }
	FoldDisplayTextStyle FoldDisplayTextGetStyle() { return static_cast<FoldDisplayTextStyle>(Call(Message::FoldDisplayTextGetStyle)); }
	FoldLevel GetFoldLevel(Line line) { return static_cast<FoldLevel>(Call(Message::GetFoldLevel, line)); }
	FontQuality GetFontQuality() { return static_cast<FontQuality>(Call(Message::GetFontQuality)); }
	FontStretch StyleGetStretch(int style) { return static_cast<FontStretch>(Call(Message::StyleGetStretch, style)); }
	FontWeight StyleGetWeight(int style) { return static_cast<FontWeight>(Call(Message::StyleGetWeight, style)); }
	IMEInteraction GetIMEInteraction() { return static_cast<IMEInteraction>(Call(Message::GetIMEInteraction)); }
	IdleStyling GetIdleStyling() { return static_cast<IdleStyling>(Call(Message::GetIdleStyling)); }
	IndentView GetIndentationGuides() { return static_cast<IndentView>(Call(Message::GetIndentationGuides)); }
	IndicFlag IndicGetFlags(int indicator) { return static_cast<IndicFlag>(Call(Message::IndicGetFlags, indicator)); }
	IndicatorStyle IndicGetHoverStyle(int indicator) { return static_cast<IndicatorStyle>(Call(Message::IndicGetHoverStyle, indicator)); }
	IndicatorStyle IndicGetStyle(int indicator) { return static_cast<IndicatorStyle>(Call(Message::IndicGetStyle, indicator)); }
	Layer GetCaretLineLayer() { return static_cast<Layer>(Call(Message::GetCaretLineLayer)); }
	Layer GetSelectionLayer() { return static_cast<Layer>(Call(Message::GetSelectionLayer)); }
	Layer MarkerGetLayer(int markerNumber) { return static_cast<Layer>(Call(Message::MarkerGetLayer, markerNumber)); }
	Line ContractedFoldNext(Line lineStart) { return Call(Message::ContractedFoldNext, lineStart); }
	Line DocLineFromVisible(Line displayLine) { return Call(Message::DocLineFromVisible, displayLine); }
	Line GetFirstVisibleLine() { return Call(Message::GetFirstVisibleLine); }
	Line GetFoldParent(Line line) { return Call(Message::GetFoldParent, line); }
	Line GetLastChild(Line line, FoldLevel level) { return Call(Message::GetLastChild, line, static_cast<intptr_t>(level)); }
	Line GetLineCount() { return Call(Message::GetLineCount); }
	Line LineFromIndexPosition(Position pos, LineCharacterIndexType lineCharacterIndex) { return Call(Message::LineFromIndexPosition, pos, static_cast<intptr_t>(lineCharacterIndex)); }
	Line LineFromPosition(Position pos) { return Call(Message::LineFromPosition, pos); }
	Line LinesOnScreen() { return Call(Message::LinesOnScreen); }
	Line MarkerLineFromHandle(int markerHandle) { return Call(Message::MarkerLineFromHandle, markerHandle); }
	Line MarkerNext(Line lineStart, int markerMask) { return Call(Message::MarkerNext, lineStart, markerMask); }
	Line MarkerPrevious(Line lineStart, int markerMask) { return Call(Message::MarkerPrevious, lineStart, markerMask); }
	Line VisibleFromDocLine(Line docLine) { return Call(Message::VisibleFromDocLine, docLine); }
	Line WrapCount(Line docLine) { return Call(Message::WrapCount, docLine); }
	LineCache GetLayoutCache() { return static_cast<LineCache>(Call(Message::GetLayoutCache)); }
	LineCharacterIndexType GetLineCharacterIndex() { return static_cast<LineCharacterIndexType>(Call(Message::GetLineCharacterIndex)); }
	LineEndType GetLineEndTypesActive() { return static_cast<LineEndType>(Call(Message::GetLineEndTypesActive)); }
	LineEndType GetLineEndTypesAllowed() { return static_cast<LineEndType>(Call(Message::GetLineEndTypesAllowed)); }
	LineEndType GetLineEndTypesSupported() { return static_cast<LineEndType>(Call(Message::GetLineEndTypesSupported)); }
	MarginOption GetMarginOptions() { return static_cast<MarginOption>(Call(Message::GetMarginOptions)); }
	MarginType GetMarginTypeN(int margin) { return static_cast<MarginType>(Call(Message::GetMarginTypeN, margin)); }
	MarkerSymbol MarkerSymbolDefined(int markerNumber) { return static_cast<MarkerSymbol>(Call(Message::MarkerSymbolDefined, markerNumber)); }
	ModificationFlags GetModEventMask() { return static_cast<ModificationFlags>(Call(Message::GetModEventMask)); }
	MultiAutoComplete AutoCGetMulti() { return static_cast<MultiAutoComplete>(Call(Message::AutoCGetMulti)); }
	MultiPaste GetMultiPaste() { return static_cast<MultiPaste>(Call(Message::GetMultiPaste)); }
	Ordering AutoCGetOrder() { return static_cast<Ordering>(Call(Message::AutoCGetOrder)); }
	PhasesDraw GetPhasesDraw() { return static_cast<PhasesDraw>(Call(Message::GetPhasesDraw)); }
	Position AutoCPosStart() { return Call(Message::AutoCPosStart); }
	Position BraceMatch(Position pos, int maxReStyle) { return Call(Message::BraceMatch, pos, maxReStyle); }
	Position BraceMatchNext(Position pos, Position startPos) { return Call(Message::BraceMatchNext, pos, startPos); }
	Position CallTipPosStart() { return Call(Message::CallTipPosStart); }
	Position CharPositionFromPoint(int x, int y) { return Call(Message::CharPositionFromPoint, x, y); }
	Position CharPositionFromPointClose(int x, int y) { return Call(Message::CharPositionFromPointClose, x, y); }
	Position CountCharacters(Position start, Position end) { return Call(Message::CountCharacters, start, end); }
	Position CountCodeUnits(Position start, Position end) { return Call(Message::CountCodeUnits, start, end); }
	Position EncodedFromUTF8(const char* utf8, char* encoded) { return CallPointer(Message::EncodedFromUTF8, reinterpret_cast<uintptr_t>(utf8), encoded); }
	Position FindColumn(Line line, Position column) { return Call(Message::FindColumn, line, column); }
	Position FindText(FindOption searchFlags, void* ft) { return CallPointer(Message::FindText, static_cast<uintptr_t>(searchFlags), ft); }
	Position FindTextFull(FindOption searchFlags, TextToFindFull* ft) { return CallPointer(Message::FindTextFull, static_cast<uintptr_t>(searchFlags), ft); }
	Position FormatRange(bool draw, void* fr) { return CallPointer(Message::FormatRange, draw, fr); }
	Position FormatRangeFull(bool draw, RangeToFormatFull* fr) { return CallPointer(Message::FormatRangeFull, draw, fr); }
	Position GetAnchor() { return Call(Message::GetAnchor); }
	Position GetColumn(Position pos) { return Call(Message::GetColumn, pos); }
	Position GetCurLine(Position length, char* text) { return CallPointer(Message::GetCurLine, length, text); }
	Position GetCurrentPos() { return Call(Message::GetCurrentPos); }
	Position GetEdgeColumn() { return Call(Message::GetEdgeColumn); }
	Position GetEndStyled() { return Call(Message::GetEndStyled); }
	Position GetGapPosition() { return Call(Message::GetGapPosition); }
	Position GetHighlightGuide() { return Call(Message::GetHighlightGuide); }
	Position GetLength() { return Call(Message::GetLength); }
	Position GetLine(Line line, char* text) { return CallPointer(Message::GetLine, line, text); }
	Position GetLineEndPosition(Line line) { return Call(Message::GetLineEndPosition, line); }
	Position GetLineIndentPosition(Line line) { return Call(Message::GetLineIndentPosition, line); }
	Position GetLineSelEndPosition(Line line) { return Call(Message::GetLineSelEndPosition, line); }
	Position GetLineSelStartPosition(Line line) { return Call(Message::GetLineSelStartPosition, line); }
	Position GetMultiEdgeColumn(int which) { return Call(Message::GetMultiEdgeColumn, which); }
	Position GetRectangularSelectionAnchor() { return Call(Message::GetRectangularSelectionAnchor); }
	Position GetRectangularSelectionAnchorVirtualSpace() { return Call(Message::GetRectangularSelectionAnchorVirtualSpace); }
	Position GetRectangularSelectionCaret() { return Call(Message::GetRectangularSelectionCaret); }
	Position GetRectangularSelectionCaretVirtualSpace() { return Call(Message::GetRectangularSelectionCaretVirtualSpace); }
	Position GetSelText(char* text) { return CallPointer(Message::GetSelText, 0, text); }
	Position GetSelectionEnd() { return Call(Message::GetSelectionEnd); }
	Position GetSelectionNAnchor(int selection) { return Call(Message::GetSelectionNAnchor, selection); }
	Position GetSelectionNAnchorVirtualSpace(int selection) { return Call(Message::GetSelectionNAnchorVirtualSpace, selection); }
	Position GetSelectionNCaret(int selection) { return Call(Message::GetSelectionNCaret, selection); }
	Position GetSelectionNCaretVirtualSpace(int selection) { return Call(Message::GetSelectionNCaretVirtualSpace, selection); }
	Position GetSelectionNEnd(int selection) { return Call(Message::GetSelectionNEnd, selection); }
	Position GetSelectionNEndVirtualSpace(int selection) { return Call(Message::GetSelectionNEndVirtualSpace, selection); }
	Position GetSelectionNStart(int selection) { return Call(Message::GetSelectionNStart, selection); }
	Position GetSelectionNStartVirtualSpace(int selection) { return Call(Message::GetSelectionNStartVirtualSpace, selection); }
	Position GetSelectionSerialized(char* selectionString) { return CallPointer(Message::GetSelectionSerialized, 0, selectionString); }
	Position GetSelectionStart() { return Call(Message::GetSelectionStart); }
	Position GetStyledText(void* tr) { return CallPointer(Message::GetStyledText, 0, tr); }
	Position GetStyledTextFull(TextRangeFull* tr) { return CallPointer(Message::GetStyledTextFull, 0, tr); }
	Position GetTargetEnd() { return Call(Message::GetTargetEnd); }
	Position GetTargetEndVirtualSpace() { return Call(Message::GetTargetEndVirtualSpace); }
	Position GetTargetStart() { return Call(Message::GetTargetStart); }
	Position GetTargetStartVirtualSpace() { return Call(Message::GetTargetStartVirtualSpace); }
	Position GetTargetText(char* text) { return CallPointer(Message::GetTargetText, 0, text); }
	Position GetText(Position length, char* text) { return CallPointer(Message::GetText, length, text); }
	Position GetTextLength() { return Call(Message::GetTextLength); }
	Position GetTextRange(void* tr) { return CallPointer(Message::GetTextRange, 0, tr); }
	Position GetTextRangeFull(TextRangeFull* tr) { return CallPointer(Message::GetTextRangeFull, 0, tr); }
	Position GetUndoActionPosition(int action) { return Call(Message::GetUndoActionPosition, action); }
	Position IndexPositionFromLine(Line line, LineCharacterIndexType lineCharacterIndex) { return Call(Message::IndexPositionFromLine, line, static_cast<intptr_t>(lineCharacterIndex)); }
	Position IndicatorEnd(int indicator, Position pos) { return Call(Message::IndicatorEnd, indicator, pos); }
	Position IndicatorStart(int indicator, Position pos) { return Call(Message::IndicatorStart, indicator, pos); }
	Position LineLength(Line line) { return Call(Message::LineLength, line); }
	Position PositionAfter(Position pos) { return Call(Message::PositionAfter, pos); }
	Position PositionBefore(Position pos) { return Call(Message::PositionBefore, pos); }
	Position PositionFromLine(Line line) { return Call(Message::PositionFromLine, line); }
	Position PositionFromPoint(int x, int y) { return Call(Message::PositionFromPoint, x, y); }
	Position PositionFromPointClose(int x, int y) { return Call(Message::PositionFromPointClose, x, y); }
	Position PositionRelative(Position pos, Position relative) { return Call(Message::PositionRelative, pos, relative); }
	Position PositionRelativeCodeUnits(Position pos, Position relative) { return Call(Message::PositionRelativeCodeUnits, pos, relative); }
	Position ReplaceTarget(Position length, const char* text) { return CallString(Message::ReplaceTarget, length, text); }
	Position ReplaceTargetMinimal(Position length, const char* text) { return CallString(Message::ReplaceTargetMinimal, length, text); }
	Position ReplaceTargetRE(Position length, const char* text) { return CallString(Message::ReplaceTargetRE, length, text); }
	Position SearchInTarget(Position length, const char* text) { return CallString(Message::SearchInTarget, length, text); }
	Position SearchNext(FindOption searchFlags, const char* text) { return CallString(Message::SearchNext, static_cast<uintptr_t>(searchFlags), text); }
	Position SearchPrev(FindOption searchFlags, const char* text) { return CallString(Message::SearchPrev, static_cast<uintptr_t>(searchFlags), text); }
	Position TargetAsUTF8(char* s) { return CallPointer(Message::TargetAsUTF8, 0, s); }
	Position WordEndPosition(Position pos, bool onlyWordCharacters) { return Call(Message::WordEndPosition, pos, onlyWordCharacters); }
	Position WordStartPosition(Position pos, bool onlyWordCharacters) { return Call(Message::WordStartPosition, pos, onlyWordCharacters); }
	PrintOption GetPrintColourMode() { return static_cast<PrintOption>(Call(Message::GetPrintColourMode)); }
	RepresentationAppearance GetRepresentationAppearance(const char* encodedCharacter) { return static_cast<RepresentationAppearance>(Call(Message::GetRepresentationAppearance, reinterpret_cast<uintptr_t>(encodedCharacter))); }
	SelectionMode GetSelectionMode() { return static_cast<SelectionMode>(Call(Message::GetSelectionMode)); }
	Status GetStatus() { return static_cast<Status>(Call(Message::GetStatus)); }
	TabDrawMode GetTabDrawMode() { return static_cast<TabDrawMode>(Call(Message::GetTabDrawMode)); }
	Technology GetTechnology() { return static_cast<Technology>(Call(Message::GetTechnology)); }
	TypeProperty PropertyType(const char* name) { return static_cast<TypeProperty>(Call(Message::PropertyType, reinterpret_cast<uintptr_t>(name))); }
	UndoSelectionHistoryOption GetUndoSelectionHistory() { return static_cast<UndoSelectionHistoryOption>(Call(Message::GetUndoSelectionHistory)); }
	VirtualSpace GetVirtualSpaceOptions() { return static_cast<VirtualSpace>(Call(Message::GetVirtualSpaceOptions)); }
	WhiteSpace GetViewWS() { return static_cast<WhiteSpace>(Call(Message::GetViewWS)); }
	Wrap GetPrintWrapMode() { return static_cast<Wrap>(Call(Message::GetPrintWrapMode)); }
	Wrap GetWrapMode() { return static_cast<Wrap>(Call(Message::GetWrapMode)); }
	WrapIndentMode GetWrapIndentMode() { return static_cast<WrapIndentMode>(Call(Message::GetWrapIndentMode)); }
	WrapVisualFlag GetWrapVisualFlags() { return static_cast<WrapVisualFlag>(Call(Message::GetWrapVisualFlags)); }
	WrapVisualLocation GetWrapVisualFlagsLocation() { return static_cast<WrapVisualLocation>(Call(Message::GetWrapVisualFlagsLocation)); }
	bool AutoCActive() { return Call(Message::AutoCActive); }
	bool AutoCGetAutoHide() { return Call(Message::AutoCGetAutoHide); }
	bool AutoCGetCancelAtStart() { return Call(Message::AutoCGetCancelAtStart); }
	bool AutoCGetChooseSingle() { return Call(Message::AutoCGetChooseSingle); }
	bool AutoCGetDropRestOfWord() { return Call(Message::AutoCGetDropRestOfWord); }
	bool AutoCGetIgnoreCase() { return Call(Message::AutoCGetIgnoreCase); }
	bool CallTipActive() { return Call(Message::CallTipActive); }
	bool CanPaste() { return Call(Message::CanPaste); }
	bool CanRedo() { return Call(Message::CanRedo); }
	bool CanUndo() { return Call(Message::CanUndo); }
	bool GetAdditionalCaretsBlink() { return Call(Message::GetAdditionalCaretsBlink); }
	bool GetAdditionalCaretsVisible() { return Call(Message::GetAdditionalCaretsVisible); }
	bool GetAdditionalSelectionTyping() { return Call(Message::GetAdditionalSelectionTyping); }
	bool GetAllLinesVisible() { return Call(Message::GetAllLinesVisible); }
	bool GetBackSpaceUnIndents() { return Call(Message::GetBackSpaceUnIndents); }
	bool GetBufferedDraw() { return Call(Message::GetBufferedDraw); }
	bool GetCaretLineHighlightSubLine() { return Call(Message::GetCaretLineHighlightSubLine); }
	bool GetCaretLineVisible() { return Call(Message::GetCaretLineVisible); }
	bool GetCaretLineVisibleAlways() { return Call(Message::GetCaretLineVisibleAlways); }
	bool GetCommandEvents() { return Call(Message::GetCommandEvents); }
	bool GetElementAllowsTranslucent(Element element) { return Call(Message::GetElementAllowsTranslucent, static_cast<uintptr_t>(element)); }
	bool GetElementIsSet(Element element) { return Call(Message::GetElementIsSet, static_cast<uintptr_t>(element)); }
	bool GetEndAtLastLine() { return Call(Message::GetEndAtLastLine); }
	bool GetFocus() { return Call(Message::GetFocus); }
	bool GetFoldExpanded(Line line) { return Call(Message::GetFoldExpanded, line); }
	bool GetHScrollBar() { return Call(Message::GetHScrollBar); }
	bool GetHotspotActiveUnderline() { return Call(Message::GetHotspotActiveUnderline); }
	bool GetHotspotSingleLine() { return Call(Message::GetHotspotSingleLine); }
	bool GetLineVisible(Line line) { return Call(Message::GetLineVisible, line); }
	bool GetMarginSensitiveN(int margin) { return Call(Message::GetMarginSensitiveN, margin); }
	bool GetModify() { return Call(Message::GetModify); }
	bool GetMouseDownCaptures() { return Call(Message::GetMouseDownCaptures); }
	bool GetMouseSelectionRectangularSwitch() { return Call(Message::GetMouseSelectionRectangularSwitch); }
	bool GetMouseWheelCaptures() { return Call(Message::GetMouseWheelCaptures); }
	bool GetMoveExtendsSelection() { return Call(Message::GetMoveExtendsSelection); }
	bool GetMultipleSelection() { return Call(Message::GetMultipleSelection); }
	bool GetOvertype() { return Call(Message::GetOvertype); }
	bool GetPasteConvertEndings() { return Call(Message::GetPasteConvertEndings); }
	bool GetReadOnly() { return Call(Message::GetReadOnly); }
	bool GetScrollWidthTracking() { return Call(Message::GetScrollWidthTracking); }
	bool GetSelEOLFilled() { return Call(Message::GetSelEOLFilled); }
	bool GetSelectionEmpty() { return Call(Message::GetSelectionEmpty); }
	bool GetSelectionHidden() { return Call(Message::GetSelectionHidden); }
	bool GetTabIndents() { return Call(Message::GetTabIndents); }
	bool GetUndoCollection() { return Call(Message::GetUndoCollection); }
	bool GetUseTabs() { return Call(Message::GetUseTabs); }
	bool GetVScrollBar() { return Call(Message::GetVScrollBar); }
	bool GetViewEOL() { return Call(Message::GetViewEOL); }
	bool IndicGetUnder(int indicator) { return Call(Message::IndicGetUnder, indicator); }
	bool IsRangeWord(Position start, Position end) { return Call(Message::IsRangeWord, start, end); }
	bool SelectionIsRectangle() { return Call(Message::SelectionIsRectangle); }
	bool StyleGetBold(int style) { return Call(Message::StyleGetBold, style); }
	bool StyleGetChangeable(int style) { return Call(Message::StyleGetChangeable, style); }
	bool StyleGetCheckMonospaced(int style) { return Call(Message::StyleGetCheckMonospaced, style); }
	bool StyleGetEOLFilled(int style) { return Call(Message::StyleGetEOLFilled, style); }
	bool StyleGetHotSpot(int style) { return Call(Message::StyleGetHotSpot, style); }
	bool StyleGetItalic(int style) { return Call(Message::StyleGetItalic, style); }
	bool StyleGetUnderline(int style) { return Call(Message::StyleGetUnderline, style); }
	bool StyleGetVisible(int style) { return Call(Message::StyleGetVisible, style); }
	bool SupportsFeature(Supports feature) { return Call(Message::SupportsFeature, static_cast<uintptr_t>(feature)); }
	int AllocateExtendedStyles(int numberStyles) { return static_cast<int>(Call(Message::AllocateExtendedStyles, numberStyles)); }
	int AllocateSubStyles(int styleBase, int numberStyles) { return static_cast<int>(Call(Message::AllocateSubStyles, styleBase, numberStyles)); }
	int AnnotationGetLines(Line line) { return static_cast<int>(Call(Message::AnnotationGetLines, line)); }
	int AnnotationGetStyle(Line line) { return static_cast<int>(Call(Message::AnnotationGetStyle, line)); }
	int AnnotationGetStyleOffset() { return static_cast<int>(Call(Message::AnnotationGetStyleOffset)); }
	int AnnotationGetStyles(Line line, char* styles) { return static_cast<int>(CallPointer(Message::AnnotationGetStyles, line, styles)); }
	int AnnotationGetText(Line line, char* text) { return static_cast<int>(CallPointer(Message::AnnotationGetText, line, text)); }
	int AutoCGetCurrent() { return static_cast<int>(Call(Message::AutoCGetCurrent)); }
	int AutoCGetCurrentText(char* text) { return static_cast<int>(CallPointer(Message::AutoCGetCurrentText, 0, text)); }
	int AutoCGetImageScale() { return static_cast<int>(Call(Message::AutoCGetImageScale)); }
	int AutoCGetMaxHeight() { return static_cast<int>(Call(Message::AutoCGetMaxHeight)); }
	int AutoCGetMaxWidth() { return static_cast<int>(Call(Message::AutoCGetMaxWidth)); }
	int AutoCGetSeparator() { return static_cast<int>(Call(Message::AutoCGetSeparator)); }
	int AutoCGetStyle() { return static_cast<int>(Call(Message::AutoCGetStyle)); }
	int AutoCGetTypeSeparator() { return static_cast<int>(Call(Message::AutoCGetTypeSeparator)); }
	int ChangeLexerState(Position start, Position end) { return static_cast<int>(Call(Message::ChangeLexerState, start, end)); }
	int DescribeKeyWordSets(char* descriptions) { return static_cast<int>(CallPointer(Message::DescribeKeyWordSets, 0, descriptions)); }
	int DescribeProperty(const char* name, char* description) { return static_cast<int>(CallPointer(Message::DescribeProperty, reinterpret_cast<uintptr_t>(name), description)); }
	int DescriptionOfStyle(int style, char* description) { return static_cast<int>(CallPointer(Message::DescriptionOfStyle, style, description)); }
	int DistanceToSecondaryStyles() { return static_cast<int>(Call(Message::DistanceToSecondaryStyles)); }
	int EOLAnnotationGetStyle(Line line) { return static_cast<int>(Call(Message::EOLAnnotationGetStyle, line)); }
	int EOLAnnotationGetStyleOffset() { return static_cast<int>(Call(Message::EOLAnnotationGetStyleOffset)); }
	int EOLAnnotationGetText(Line line, char* text) { return static_cast<int>(CallPointer(Message::EOLAnnotationGetText, line, text)); }
	int GetCaretLineFrame() { return static_cast<int>(Call(Message::GetCaretLineFrame)); }
	int GetCaretPeriod() { return static_cast<int>(Call(Message::GetCaretPeriod)); }
	int GetCaretWidth() { return static_cast<int>(Call(Message::GetCaretWidth)); }
	int GetCharAt(Position pos) { return static_cast<int>(Call(Message::GetCharAt, pos)); }
	int GetCharacterCategoryOptimization() { return static_cast<int>(Call(Message::GetCharacterCategoryOptimization)); }
	int GetCodePage() { return static_cast<int>(Call(Message::GetCodePage)); }
	int GetControlCharSymbol() { return static_cast<int>(Call(Message::GetControlCharSymbol)); }
	int GetCopySeparator(char* separator) { return static_cast<int>(CallPointer(Message::GetCopySeparator, 0, separator)); }
	int GetDefaultFoldDisplayText(char* text) { return static_cast<int>(CallPointer(Message::GetDefaultFoldDisplayText, 0, text)); }
	int GetExtraAscent() { return static_cast<int>(Call(Message::GetExtraAscent)); }
	int GetExtraDescent() { return static_cast<int>(Call(Message::GetExtraDescent)); }
	int GetFontLocale(char* localeName) { return static_cast<int>(CallPointer(Message::GetFontLocale, 0, localeName)); }
	int GetIdentifier() { return static_cast<int>(Call(Message::GetIdentifier)); }
	int GetIndent() { return static_cast<int>(Call(Message::GetIndent)); }
	int GetIndicatorCurrent() { return static_cast<int>(Call(Message::GetIndicatorCurrent)); }
	int GetIndicatorValue() { return static_cast<int>(Call(Message::GetIndicatorValue)); }
	int GetLayoutThreads() { return static_cast<int>(Call(Message::GetLayoutThreads)); }
	int GetLexer() { return static_cast<int>(Call(Message::GetLexer)); }
	int GetLexerLanguage(char* language) { return static_cast<int>(CallPointer(Message::GetLexerLanguage, 0, language)); }
	int GetLineIndentation(Line line) { return static_cast<int>(Call(Message::GetLineIndentation, line)); }
	int GetLineState(Line line) { return static_cast<int>(Call(Message::GetLineState, line)); }
	int GetMainSelection() { return static_cast<int>(Call(Message::GetMainSelection)); }
	int GetMarginLeft() { return static_cast<int>(Call(Message::GetMarginLeft)); }
	int GetMarginMaskN(int margin) { return static_cast<int>(Call(Message::GetMarginMaskN, margin)); }
	int GetMarginRight() { return static_cast<int>(Call(Message::GetMarginRight)); }
	int GetMarginWidthN(int margin) { return static_cast<int>(Call(Message::GetMarginWidthN, margin)); }
	int GetMargins() { return static_cast<int>(Call(Message::GetMargins)); }
	int GetMaxLineState() { return static_cast<int>(Call(Message::GetMaxLineState)); }
	int GetMouseDwellTime() { return static_cast<int>(Call(Message::GetMouseDwellTime)); }
	int GetNamedStyles() { return static_cast<int>(Call(Message::GetNamedStyles)); }
	int GetNextTabStop(Line line, int x) { return static_cast<int>(Call(Message::GetNextTabStop, line, x)); }
	int GetPositionCache() { return static_cast<int>(Call(Message::GetPositionCache)); }
	int GetPrimaryStyleFromStyle(int style) { return static_cast<int>(Call(Message::GetPrimaryStyleFromStyle, style)); }
	int GetPrintMagnification() { return static_cast<int>(Call(Message::GetPrintMagnification)); }
	int GetProperty(const char* key, char* value) { return static_cast<int>(CallPointer(Message::GetProperty, reinterpret_cast<uintptr_t>(key), value)); }
	int GetPropertyExpanded(const char* key, char* value) { return static_cast<int>(CallPointer(Message::GetPropertyExpanded, reinterpret_cast<uintptr_t>(key), value)); }
	int GetPropertyInt(const char* key, int defaultValue) { return static_cast<int>(Call(Message::GetPropertyInt, reinterpret_cast<uintptr_t>(key), defaultValue)); }
	int GetPunctuationChars(char* characters) { return static_cast<int>(CallPointer(Message::GetPunctuationChars, 0, characters)); }
	int GetRectangularSelectionModifier() { return static_cast<int>(Call(Message::GetRectangularSelectionModifier)); }
	int GetRepresentation(const char* encodedCharacter, char* representation) { return static_cast<int>(CallPointer(Message::GetRepresentation, reinterpret_cast<uintptr_t>(encodedCharacter), representation)); }
	int GetScrollWidth() { return static_cast<int>(Call(Message::GetScrollWidth)); }
	int GetSelections() { return static_cast<int>(Call(Message::GetSelections)); }
	int GetStyleAt(Position pos) { return static_cast<int>(Call(Message::GetStyleAt, pos)); }
	int GetStyleFromSubStyle(int subStyle) { return static_cast<int>(Call(Message::GetStyleFromSubStyle, subStyle)); }
	int GetStyleIndexAt(Position pos) { return static_cast<int>(Call(Message::GetStyleIndexAt, pos)); }
	int GetSubStyleBases(char* styles) { return static_cast<int>(CallPointer(Message::GetSubStyleBases, 0, styles)); }
	int GetSubStylesLength(int styleBase) { return static_cast<int>(Call(Message::GetSubStylesLength, styleBase)); }
	int GetSubStylesStart(int styleBase) { return static_cast<int>(Call(Message::GetSubStylesStart, styleBase)); }
	int GetTabMinimumWidth() { return static_cast<int>(Call(Message::GetTabMinimumWidth)); }
	int GetTabWidth() { return static_cast<int>(Call(Message::GetTabWidth)); }
	int GetTag(int tagNumber, char* tagValue) { return static_cast<int>(CallPointer(Message::GetTag, tagNumber, tagValue)); }
	int GetUndoActionText(int action, char* text) { return static_cast<int>(CallPointer(Message::GetUndoActionText, action, text)); }
	int GetUndoActionType(int action) { return static_cast<int>(Call(Message::GetUndoActionType, action)); }
	int GetUndoActions() { return static_cast<int>(Call(Message::GetUndoActions)); }
	int GetUndoCurrent() { return static_cast<int>(Call(Message::GetUndoCurrent)); }
	int GetUndoDetach() { return static_cast<int>(Call(Message::GetUndoDetach)); }
	int GetUndoSavePoint() { return static_cast<int>(Call(Message::GetUndoSavePoint)); }
	int GetUndoSequence() { return static_cast<int>(Call(Message::GetUndoSequence)); }
	int GetUndoTentative() { return static_cast<int>(Call(Message::GetUndoTentative)); }
	int GetWhitespaceChars(char* characters) { return static_cast<int>(CallPointer(Message::GetWhitespaceChars, 0, characters)); }
	int GetWhitespaceSize() { return static_cast<int>(Call(Message::GetWhitespaceSize)); }
	int GetWordChars(char* characters) { return static_cast<int>(CallPointer(Message::GetWordChars, 0, characters)); }
	int GetWrapStartIndent() { return static_cast<int>(Call(Message::GetWrapStartIndent)); }
	int GetXOffset() { return static_cast<int>(Call(Message::GetXOffset)); }
	int GetZoom() { return static_cast<int>(Call(Message::GetZoom)); }
	int IndicGetStrokeWidth(int indicator) { return static_cast<int>(Call(Message::IndicGetStrokeWidth, indicator)); }
	int IndicatorAllOnFor(Position pos) { return static_cast<int>(Call(Message::IndicatorAllOnFor, pos)); }
	int IndicatorValueAt(int indicator, Position pos) { return static_cast<int>(Call(Message::IndicatorValueAt, indicator, pos)); }
	int MarginGetStyle(Line line) { return static_cast<int>(Call(Message::MarginGetStyle, line)); }
	int MarginGetStyleOffset() { return static_cast<int>(Call(Message::MarginGetStyleOffset)); }
	int MarginGetStyles(Line line, char* styles) { return static_cast<int>(CallPointer(Message::MarginGetStyles, line, styles)); }
	int MarginGetText(Line line, char* text) { return static_cast<int>(CallPointer(Message::MarginGetText, line, text)); }
	int MarkerAdd(Line line, int markerNumber) { return static_cast<int>(Call(Message::MarkerAdd, line, markerNumber)); }
	int MarkerGet(Line line) { return static_cast<int>(Call(Message::MarkerGet, line)); }
	int MarkerHandleFromLine(Line line, int which) { return static_cast<int>(Call(Message::MarkerHandleFromLine, line, which)); }
	int MarkerNumberFromLine(Line line, int which) { return static_cast<int>(Call(Message::MarkerNumberFromLine, line, which)); }
	int NameOfStyle(int style, char* name) { return static_cast<int>(CallPointer(Message::NameOfStyle, style, name)); }
	int PointXFromPosition(Position pos) { return static_cast<int>(Call(Message::PointXFromPosition, 0, pos)); }
	int PointYFromPosition(Position pos) { return static_cast<int>(Call(Message::PointYFromPosition, 0, pos)); }
	int PropertyNames(char* names) { return static_cast<int>(CallPointer(Message::PropertyNames, 0, names)); }
	int SelectionFromPoint(int x, int y) { return static_cast<int>(Call(Message::SelectionFromPoint, x, y)); }
	int StyleGetFont(int style, char* fontName) { return static_cast<int>(CallPointer(Message::StyleGetFont, style, fontName)); }
	int StyleGetInvisibleRepresentation(int style, char* representation) { return static_cast<int>(CallPointer(Message::StyleGetInvisibleRepresentation, style, representation)); }
	int StyleGetSize(int style) { return static_cast<int>(Call(Message::StyleGetSize, style)); }
	int StyleGetSizeFractional(int style) { return static_cast<int>(Call(Message::StyleGetSizeFractional, style)); }
	int TagsOfStyle(int style, char* tags) { return static_cast<int>(CallPointer(Message::TagsOfStyle, style, tags)); }
	int TextHeight(Line line) { return static_cast<int>(Call(Message::TextHeight, line)); }
	int TextWidth(int style, const char* text) { return static_cast<int>(CallString(Message::TextWidth, style, text)); }
	void AddRefDocument(void* doc) { CallPointer(Message::AddRefDocument, 0, doc); }
	void AddSelection(Position caret, Position anchor) { Call(Message::AddSelection, caret, anchor); }
	void AddStyledText(Position length, const char* c) { CallString(Message::AddStyledText, length, c); }
	void AddTabStop(Line line, int x) { Call(Message::AddTabStop, line, x); }
	void AddText(Position length, const char* text) { CallString(Message::AddText, length, text); }
	void AddUndoAction(int token, UndoFlags flags) { Call(Message::AddUndoAction, token, static_cast<intptr_t>(flags)); }
	void Allocate(Position bytes) { Call(Message::Allocate, bytes); }
	void AllocateLineCharacterIndex(LineCharacterIndexType lineCharacterIndex) { Call(Message::AllocateLineCharacterIndex, static_cast<uintptr_t>(lineCharacterIndex)); }
	void AllocateLines(Line lines) { Call(Message::AllocateLines, lines); }
	void AnnotationClearAll() { Call(Message::AnnotationClearAll); }
	void AnnotationSetStyle(Line line, int style) { Call(Message::AnnotationSetStyle, line, style); }
	void AnnotationSetStyleOffset(int style) { Call(Message::AnnotationSetStyleOffset, style); }
	void AnnotationSetStyles(Line line, const char* styles) { CallString(Message::AnnotationSetStyles, line, styles); }
	void AnnotationSetText(Line line, const char* text) { CallString(Message::AnnotationSetText, line, text); }
	void AnnotationSetVisible(AnnotationVisible visible) { Call(Message::AnnotationSetVisible, static_cast<uintptr_t>(visible)); }
	void AppendText(Position length, const char* text) { CallString(Message::AppendText, length, text); }
	void AssignCmdKey(int keyDefinition, int sciCommand) { Call(Message::AssignCmdKey, keyDefinition, sciCommand); }
	void AutoCCancel() { Call(Message::AutoCCancel); }
	void AutoCComplete() { Call(Message::AutoCComplete); }
	void AutoCSelect(const char* select) { CallString(Message::AutoCSelect, 0, select); }
	void AutoCSetAutoHide(bool autoHide) { Call(Message::AutoCSetAutoHide, autoHide); }
	void AutoCSetCancelAtStart(bool cancel) { Call(Message::AutoCSetCancelAtStart, cancel); }
	void AutoCSetCaseInsensitiveBehaviour(CaseInsensitiveBehaviour behaviour) { Call(Message::AutoCSetCaseInsensitiveBehaviour, static_cast<uintptr_t>(behaviour)); }
	void AutoCSetChooseSingle(bool chooseSingle) { Call(Message::AutoCSetChooseSingle, chooseSingle); }
	void AutoCSetDropRestOfWord(bool dropRestOfWord) { Call(Message::AutoCSetDropRestOfWord, dropRestOfWord); }
	void AutoCSetFillUps(const char* characterSet) { CallString(Message::AutoCSetFillUps, 0, characterSet); }
	void AutoCSetIgnoreCase(bool ignoreCase) { Call(Message::AutoCSetIgnoreCase, ignoreCase); }
	void AutoCSetImageScale(int scalePercent) { Call(Message::AutoCSetImageScale, scalePercent); }
	void AutoCSetMaxHeight(int rowCount) { Call(Message::AutoCSetMaxHeight, rowCount); }
	void AutoCSetMaxWidth(int characterCount) { Call(Message::AutoCSetMaxWidth, characterCount); }
	void AutoCSetMulti(MultiAutoComplete multi) { Call(Message::AutoCSetMulti, static_cast<uintptr_t>(multi)); }
	void AutoCSetOptions(AutoCompleteOption options) { Call(Message::AutoCSetOptions, static_cast<uintptr_t>(options)); }
	void AutoCSetOrder(Ordering order) { Call(Message::AutoCSetOrder, static_cast<uintptr_t>(order)); }
	void AutoCSetSeparator(int separatorCharacter) { Call(Message::AutoCSetSeparator, separatorCharacter); }
	void AutoCSetStyle(int style) { Call(Message::AutoCSetStyle, style); }
	void AutoCSetTypeSeparator(int separatorCharacter) { Call(Message::AutoCSetTypeSeparator, separatorCharacter); }
	void AutoCShow(Position lengthEntered, const char* itemList) { CallString(Message::AutoCShow, lengthEntered, itemList); }
	void AutoCStops(const char* characterSet) { CallString(Message::AutoCStops, 0, characterSet); }
	void BackTab() { Call(Message::BackTab); }
	void BeginUndoAction() { Call(Message::BeginUndoAction); }
	void BraceBadLight(Position pos) { Call(Message::BraceBadLight, pos); }
	void BraceBadLightIndicator(bool useSetting, int indicator) { Call(Message::BraceBadLightIndicator, useSetting, indicator); }
	void BraceHighlight(Position posA, Position posB) { Call(Message::BraceHighlight, posA, posB); }
	void BraceHighlightIndicator(bool useSetting, int indicator) { Call(Message::BraceHighlightIndicator, useSetting, indicator); }
	void CallTipCancel() { Call(Message::CallTipCancel); }
	void CallTipSetBack(Colour back) { Call(Message::CallTipSetBack, back); }
	void CallTipSetFore(Colour fore) { Call(Message::CallTipSetFore, fore); }
	void CallTipSetForeHlt(Colour fore) { Call(Message::CallTipSetForeHlt, fore); }
	void CallTipSetHlt(Position highlightStart, Position highlightEnd) { Call(Message::CallTipSetHlt, highlightStart, highlightEnd); }
	void CallTipSetPosStart(Position posStart) { Call(Message::CallTipSetPosStart, posStart); }
	void CallTipSetPosition(bool above) { Call(Message::CallTipSetPosition, above); }
	void CallTipShow(Position pos, const char* definition) { CallString(Message::CallTipShow, pos, definition); }
	void CallTipUseStyle(int tabSize) { Call(Message::CallTipUseStyle, tabSize); }
	void Cancel() { Call(Message::Cancel); }
	void ChangeInsertion(Position length, const char* text) { CallString(Message::ChangeInsertion, length, text); }
	void ChangeLastUndoActionText(Position length, const char* text) { CallString(Message::ChangeLastUndoActionText, length, text); }
	void ChangeSelectionMode(SelectionMode selectionMode) { Call(Message::ChangeSelectionMode, static_cast<uintptr_t>(selectionMode)); }
	void CharLeft() { Call(Message::CharLeft); }
	void CharLeftExtend() { Call(Message::CharLeftExtend); }
	void CharLeftRectExtend() { Call(Message::CharLeftRectExtend); }
	void CharRight() { Call(Message::CharRight); }
	void CharRightExtend() { Call(Message::CharRightExtend); }
	void CharRightRectExtend() { Call(Message::CharRightRectExtend); }
	void ChooseCaretX() { Call(Message::ChooseCaretX); }
	void Clear() { Call(Message::Clear); }
	void ClearAll() { Call(Message::ClearAll); }
	void ClearAllCmdKeys() { Call(Message::ClearAllCmdKeys); }
	void ClearAllRepresentations() { Call(Message::ClearAllRepresentations); }
	void ClearCmdKey(int keyDefinition) { Call(Message::ClearCmdKey, keyDefinition); }
	void ClearDocumentStyle() { Call(Message::ClearDocumentStyle); }
	void ClearRegisteredImages() { Call(Message::ClearRegisteredImages); }
	void ClearRepresentation(const char* encodedCharacter) { Call(Message::ClearRepresentation, reinterpret_cast<uintptr_t>(encodedCharacter)); }
	void ClearSelections() { Call(Message::ClearSelections); }
	void ClearTabStops(Line line) { Call(Message::ClearTabStops, line); }
	void Colourise(Position start, Position end) { Call(Message::Colourise, start, end); }
	void ConvertEOLs(EndOfLine eolMode) { Call(Message::ConvertEOLs, static_cast<uintptr_t>(eolMode)); }
	void Copy() { Call(Message::Copy); }
	void CopyAllowLine() { Call(Message::CopyAllowLine); }
	void CopyRange(Position start, Position end) { Call(Message::CopyRange, start, end); }
	void CopyText(Position length, const char* text) { CallString(Message::CopyText, length, text); }
	void Cut() { Call(Message::Cut); }
	void CutAllowLine() { Call(Message::CutAllowLine); }
	void DelLineLeft() { Call(Message::DelLineLeft); }
	void DelLineRight() { Call(Message::DelLineRight); }
	void DelWordLeft() { Call(Message::DelWordLeft); }
	void DelWordRight() { Call(Message::DelWordRight); }
	void DelWordRightEnd() { Call(Message::DelWordRightEnd); }
	void DeleteBack() { Call(Message::DeleteBack); }
	void DeleteBackNotLine() { Call(Message::DeleteBackNotLine); }
	void DeleteRange(Position start, Position lengthDelete) { Call(Message::DeleteRange, start, lengthDelete); }
	void DocumentEnd() { Call(Message::DocumentEnd); }
	void DocumentEndExtend() { Call(Message::DocumentEndExtend); }
	void DocumentStart() { Call(Message::DocumentStart); }
	void DocumentStartExtend() { Call(Message::DocumentStartExtend); }
	void DropSelectionN(int selection) { Call(Message::DropSelectionN, selection); }
	void EOLAnnotationClearAll() { Call(Message::EOLAnnotationClearAll); }
	void EOLAnnotationSetStyle(Line line, int style) { Call(Message::EOLAnnotationSetStyle, line, style); }
	void EOLAnnotationSetStyleOffset(int style) { Call(Message::EOLAnnotationSetStyleOffset, style); }
	void EOLAnnotationSetText(Line line, const char* text) { CallString(Message::EOLAnnotationSetText, line, text); }
	void EOLAnnotationSetVisible(EOLAnnotationVisible visible) { Call(Message::EOLAnnotationSetVisible, static_cast<uintptr_t>(visible)); }
	void EditToggleOvertype() { Call(Message::EditToggleOvertype); }
	void EmptyUndoBuffer() { Call(Message::EmptyUndoBuffer); }
	void EndUndoAction() { Call(Message::EndUndoAction); }
	void EnsureVisible(Line line) { Call(Message::EnsureVisible, line); }
	void EnsureVisibleEnforcePolicy(Line line) { Call(Message::EnsureVisibleEnforcePolicy, line); }
	void ExpandChildren(Line line, FoldLevel level) { Call(Message::ExpandChildren, line, static_cast<intptr_t>(level)); }
	void FindIndicatorFlash(Position start, Position end) { Call(Message::FindIndicatorFlash, start, end); }
	void FindIndicatorHide() { Call(Message::FindIndicatorHide); }
	void FindIndicatorShow(Position start, Position end) { Call(Message::FindIndicatorShow, start, end); }
	void FoldAll(FoldAction action) { Call(Message::FoldAll, static_cast<uintptr_t>(action)); }
	void FoldChildren(Line line, FoldAction action) { Call(Message::FoldChildren, line, static_cast<intptr_t>(action)); }
	void FoldDisplayTextSetStyle(FoldDisplayTextStyle style) { Call(Message::FoldDisplayTextSetStyle, static_cast<uintptr_t>(style)); }
	void FoldLine(Line line, FoldAction action) { Call(Message::FoldLine, line, static_cast<intptr_t>(action)); }
	void FormFeed() { Call(Message::FormFeed); }
	void FreeSubStyles() { Call(Message::FreeSubStyles); }
	void GotoLine(Line line) { Call(Message::GotoLine, line); }
	void GotoPos(Position caret) { Call(Message::GotoPos, caret); }
	void GrabFocus() { Call(Message::GrabFocus); }
	void HideLines(Line lineStart, Line lineEnd) { Call(Message::HideLines, lineStart, lineEnd); }
	void HideSelection(bool hide) { Call(Message::HideSelection, hide); }
	void Home() { Call(Message::Home); }
	void HomeDisplay() { Call(Message::HomeDisplay); }
	void HomeDisplayExtend() { Call(Message::HomeDisplayExtend); }
	void HomeExtend() { Call(Message::HomeExtend); }
	void HomeRectExtend() { Call(Message::HomeRectExtend); }
	void HomeWrap() { Call(Message::HomeWrap); }
	void HomeWrapExtend() { Call(Message::HomeWrapExtend); }
	void IndicSetAlpha(int indicator, Alpha alpha) { Call(Message::IndicSetAlpha, indicator, static_cast<intptr_t>(alpha)); }
	void IndicSetFlags(int indicator, IndicFlag flags) { Call(Message::IndicSetFlags, indicator, static_cast<intptr_t>(flags)); }
	void IndicSetFore(int indicator, Colour fore) { Call(Message::IndicSetFore, indicator, fore); }
	void IndicSetHoverFore(int indicator, Colour fore) { Call(Message::IndicSetHoverFore, indicator, fore); }
	void IndicSetHoverStyle(int indicator, IndicatorStyle indicatorStyle) { Call(Message::IndicSetHoverStyle, indicator, static_cast<intptr_t>(indicatorStyle)); }
	void IndicSetOutlineAlpha(int indicator, Alpha alpha) { Call(Message::IndicSetOutlineAlpha, indicator, static_cast<intptr_t>(alpha)); }
	void IndicSetStrokeWidth(int indicator, int hundredths) { Call(Message::IndicSetStrokeWidth, indicator, hundredths); }
	void IndicSetStyle(int indicator, IndicatorStyle indicatorStyle) { Call(Message::IndicSetStyle, indicator, static_cast<intptr_t>(indicatorStyle)); }
	void IndicSetUnder(int indicator, bool under) { Call(Message::IndicSetUnder, indicator, under); }
	void IndicatorClearRange(Position start, Position lengthClear) { Call(Message::IndicatorClearRange, start, lengthClear); }
	void IndicatorFillRange(Position start, Position lengthFill) { Call(Message::IndicatorFillRange, start, lengthFill); }
	void InsertText(Position pos, const char* text) { CallString(Message::InsertText, pos, text); }
	void LineCopy() { Call(Message::LineCopy); }
	void LineCut() { Call(Message::LineCut); }
	void LineDedent() { Call(Message::LineDedent); }
	void LineDelete() { Call(Message::LineDelete); }
	void LineDown() { Call(Message::LineDown); }
	void LineDownExtend() { Call(Message::LineDownExtend); }
	void LineDownRectExtend() { Call(Message::LineDownRectExtend); }
	void LineDuplicate() { Call(Message::LineDuplicate); }
	void LineEnd() { Call(Message::LineEnd); }
	void LineEndDisplay() { Call(Message::LineEndDisplay); }
	void LineEndDisplayExtend() { Call(Message::LineEndDisplayExtend); }
	void LineEndExtend() { Call(Message::LineEndExtend); }
	void LineEndRectExtend() { Call(Message::LineEndRectExtend); }
	void LineEndWrap() { Call(Message::LineEndWrap); }
	void LineEndWrapExtend() { Call(Message::LineEndWrapExtend); }
	void LineIndent() { Call(Message::LineIndent); }
	void LineReverse() { Call(Message::LineReverse); }
	void LineScroll(Position columns, Line lines) { Call(Message::LineScroll, columns, lines); }
	void LineScrollDown() { Call(Message::LineScrollDown); }
	void LineScrollUp() { Call(Message::LineScrollUp); }
	void LineTranspose() { Call(Message::LineTranspose); }
	void LineUp() { Call(Message::LineUp); }
	void LineUpExtend() { Call(Message::LineUpExtend); }
	void LineUpRectExtend() { Call(Message::LineUpRectExtend); }
	void LinesJoin() { Call(Message::LinesJoin); }
	void LinesSplit(int pixelWidth) { Call(Message::LinesSplit, pixelWidth); }
	void LowerCase() { Call(Message::LowerCase); }
	void MarginSetStyle(Line line, int style) { Call(Message::MarginSetStyle, line, style); }
	void MarginSetStyleOffset(int style) { Call(Message::MarginSetStyleOffset, style); }
	void MarginSetStyles(Line line, const char* styles) { CallString(Message::MarginSetStyles, line, styles); }
	void MarginSetText(Line line, const char* text) { CallString(Message::MarginSetText, line, text); }
	void MarginTextClearAll() { Call(Message::MarginTextClearAll); }
	void MarkerAddSet(Line line, int markerSet) { Call(Message::MarkerAddSet, line, markerSet); }
	void MarkerDefine(int markerNumber, MarkerSymbol markerSymbol) { Call(Message::MarkerDefine, markerNumber, static_cast<intptr_t>(markerSymbol)); }
	void MarkerDefinePixmap(int markerNumber, const char* pixmap) { CallString(Message::MarkerDefinePixmap, markerNumber, pixmap); }
	void MarkerDefineRGBAImage(int markerNumber, const char* pixels) { CallString(Message::MarkerDefineRGBAImage, markerNumber, pixels); }
	void MarkerDelete(Line line, int markerNumber) { Call(Message::MarkerDelete, line, markerNumber); }
	void MarkerDeleteAll(int markerNumber) { Call(Message::MarkerDeleteAll, markerNumber); }
	void MarkerDeleteHandle(int markerHandle) { Call(Message::MarkerDeleteHandle, markerHandle); }
	void MarkerEnableHighlight(bool enabled) { Call(Message::MarkerEnableHighlight, enabled); }
	void MarkerSetAlpha(int markerNumber, Alpha alpha) { Call(Message::MarkerSetAlpha, markerNumber, static_cast<intptr_t>(alpha)); }
	void MarkerSetBack(int markerNumber, Colour back) { Call(Message::MarkerSetBack, markerNumber, back); }
	void MarkerSetBackSelected(int markerNumber, Colour back) { Call(Message::MarkerSetBackSelected, markerNumber, back); }
	void MarkerSetBackSelectedTranslucent(int markerNumber, ColourAlpha back) { Call(Message::MarkerSetBackSelectedTranslucent, markerNumber, back); }
	void MarkerSetBackTranslucent(int markerNumber, ColourAlpha back) { Call(Message::MarkerSetBackTranslucent, markerNumber, back); }
	void MarkerSetFore(int markerNumber, Colour fore) { Call(Message::MarkerSetFore, markerNumber, fore); }
	void MarkerSetForeTranslucent(int markerNumber, ColourAlpha fore) { Call(Message::MarkerSetForeTranslucent, markerNumber, fore); }
	void MarkerSetLayer(int markerNumber, Layer layer) { Call(Message::MarkerSetLayer, markerNumber, static_cast<intptr_t>(layer)); }
	void MarkerSetStrokeWidth(int markerNumber, int hundredths) { Call(Message::MarkerSetStrokeWidth, markerNumber, hundredths); }
	void MoveCaretInsideView() { Call(Message::MoveCaretInsideView); }
	void MoveSelectedLinesDown() { Call(Message::MoveSelectedLinesDown); }
	void MoveSelectedLinesUp() { Call(Message::MoveSelectedLinesUp); }
	void MultiEdgeAddLine(Position column, Colour edgeColour) { Call(Message::MultiEdgeAddLine, column, edgeColour); }
	void MultiEdgeClearAll() { Call(Message::MultiEdgeClearAll); }
	void MultipleSelectAddEach() { Call(Message::MultipleSelectAddEach); }
	void MultipleSelectAddNext() { Call(Message::MultipleSelectAddNext); }
	void NewLine() { Call(Message::NewLine); }
	void Null() { Call(Message::Null); }
	void PageDown() { Call(Message::PageDown); }
	void PageDownExtend() { Call(Message::PageDownExtend); }
	void PageDownRectExtend() { Call(Message::PageDownRectExtend); }
	void PageUp() { Call(Message::PageUp); }
	void PageUpExtend() { Call(Message::PageUpExtend); }
	void PageUpRectExtend() { Call(Message::PageUpRectExtend); }
	void ParaDown() { Call(Message::ParaDown); }
	void ParaDownExtend() { Call(Message::ParaDownExtend); }
	void ParaUp() { Call(Message::ParaUp); }
	void ParaUpExtend() { Call(Message::ParaUpExtend); }
	void Paste() { Call(Message::Paste); }
	void PushUndoActionType(int type, Position pos) { Call(Message::PushUndoActionType, type, pos); }
	void RGBAImageSetHeight(int height) { Call(Message::RGBAImageSetHeight, height); }
	void RGBAImageSetScale(int scalePercent) { Call(Message::RGBAImageSetScale, scalePercent); }
	void RGBAImageSetWidth(int width) { Call(Message::RGBAImageSetWidth, width); }
	void Redo() { Call(Message::Redo); }
	void RegisterImage(int type, const char* xpmData) { CallString(Message::RegisterImage, type, xpmData); }
	void RegisterRGBAImage(int type, const char* pixels) { CallString(Message::RegisterRGBAImage, type, pixels); }
	void ReleaseAllExtendedStyles() { Call(Message::ReleaseAllExtendedStyles); }
	void ReleaseDocument(void* doc) { CallPointer(Message::ReleaseDocument, 0, doc); }
	void ReleaseLineCharacterIndex(LineCharacterIndexType lineCharacterIndex) { Call(Message::ReleaseLineCharacterIndex, static_cast<uintptr_t>(lineCharacterIndex)); }
	void ReplaceRectangular(Position length, const char* text) { CallString(Message::ReplaceRectangular, length, text); }
	void ReplaceSel(const char* text) { CallString(Message::ReplaceSel, 0, text); }
	void ResetElementColour(Element element) { Call(Message::ResetElementColour, static_cast<uintptr_t>(element)); }
	void RotateSelection() { Call(Message::RotateSelection); }
	void ScrollCaret() { Call(Message::ScrollCaret); }
	void ScrollRange(Position secondary, Position primary) { Call(Message::ScrollRange, secondary, primary); }
	void ScrollToEnd() { Call(Message::ScrollToEnd); }
	void ScrollToStart() { Call(Message::ScrollToStart); }
	void ScrollVertical(Line docLine, Line subLine) { Call(Message::ScrollVertical, docLine, subLine); }
	void SearchAnchor() { Call(Message::SearchAnchor); }
	void SelectAll() { Call(Message::SelectAll); }
	void SelectionDuplicate() { Call(Message::SelectionDuplicate); }
	void SetAccessibility(Accessibility accessibility) { Call(Message::SetAccessibility, static_cast<uintptr_t>(accessibility)); }
	void SetAdditionalCaretFore(Colour fore) { Call(Message::SetAdditionalCaretFore, fore); }
	void SetAdditionalCaretsBlink(bool additionalCaretsBlink) { Call(Message::SetAdditionalCaretsBlink, additionalCaretsBlink); }
	void SetAdditionalCaretsVisible(bool additionalCaretsVisible) { Call(Message::SetAdditionalCaretsVisible, additionalCaretsVisible); }
	void SetAdditionalSelAlpha(Alpha alpha) { Call(Message::SetAdditionalSelAlpha, static_cast<uintptr_t>(alpha)); }
	void SetAdditionalSelBack(Colour back) { Call(Message::SetAdditionalSelBack, back); }
	void SetAdditionalSelFore(Colour fore) { Call(Message::SetAdditionalSelFore, fore); }
	void SetAdditionalSelectionTyping(bool additionalSelectionTyping) { Call(Message::SetAdditionalSelectionTyping, additionalSelectionTyping); }
	void SetAnchor(Position anchor) { Call(Message::SetAnchor, anchor); }
	void SetAutomaticFold(AutomaticFold automaticFold) { Call(Message::SetAutomaticFold, static_cast<uintptr_t>(automaticFold)); }
	void SetBackSpaceUnIndents(bool bsUnIndents) { Call(Message::SetBackSpaceUnIndents, bsUnIndents); }
	void SetBidirectional(Bidirectional bidirectional) { Call(Message::SetBidirectional, static_cast<uintptr_t>(bidirectional)); }
	void SetBufferedDraw(bool buffered) { Call(Message::SetBufferedDraw, buffered); }
	void SetCaretFore(Colour fore) { Call(Message::SetCaretFore, fore); }
	void SetCaretLineBack(Colour back) { Call(Message::SetCaretLineBack, back); }
	void SetCaretLineBackAlpha(Alpha alpha) { Call(Message::SetCaretLineBackAlpha, static_cast<uintptr_t>(alpha)); }
	void SetCaretLineFrame(int width) { Call(Message::SetCaretLineFrame, width); }
	void SetCaretLineHighlightSubLine(bool subLine) { Call(Message::SetCaretLineHighlightSubLine, subLine); }
	void SetCaretLineLayer(Layer layer) { Call(Message::SetCaretLineLayer, static_cast<uintptr_t>(layer)); }
	void SetCaretLineVisible(bool show) { Call(Message::SetCaretLineVisible, show); }
	void SetCaretLineVisibleAlways(bool alwaysVisible) { Call(Message::SetCaretLineVisibleAlways, alwaysVisible); }
	void SetCaretPeriod(int periodMilliseconds) { Call(Message::SetCaretPeriod, periodMilliseconds); }
	void SetCaretSticky(CaretSticky useCaretStickyBehaviour) { Call(Message::SetCaretSticky, static_cast<uintptr_t>(useCaretStickyBehaviour)); }
	void SetCaretStyle(CaretStyle caretStyle) { Call(Message::SetCaretStyle, static_cast<uintptr_t>(caretStyle)); }
	void SetCaretWidth(int pixelWidth) { Call(Message::SetCaretWidth, pixelWidth); }
	void SetChangeHistory(ChangeHistoryOption changeHistory) { Call(Message::SetChangeHistory, static_cast<uintptr_t>(changeHistory)); }
	void SetCharacterCategoryOptimization(int countCharacters) { Call(Message::SetCharacterCategoryOptimization, countCharacters); }
	void SetCharsDefault() { Call(Message::SetCharsDefault); }
	void SetCodePage(int codePage) { Call(Message::SetCodePage, codePage); }
	void SetCommandEvents(bool commandEvents) { Call(Message::SetCommandEvents, commandEvents); }
	void SetControlCharSymbol(int symbol) { Call(Message::SetControlCharSymbol, symbol); }
	void SetCopySeparator(const char* separator) { CallString(Message::SetCopySeparator, 0, separator); }
	void SetCurrentPos(Position caret) { Call(Message::SetCurrentPos, caret); }
	void SetCursor(CursorShape cursorType) { Call(Message::SetCursor, static_cast<uintptr_t>(cursorType)); }
	void SetDefaultFoldDisplayText(const char* text) { CallString(Message::SetDefaultFoldDisplayText, 0, text); }
	void SetDocPointer(void* doc) { CallPointer(Message::SetDocPointer, 0, doc); }
	void SetEOLMode(EndOfLine eolMode) { Call(Message::SetEOLMode, static_cast<uintptr_t>(eolMode)); }
	void SetEdgeColour(Colour edgeColour) { Call(Message::SetEdgeColour, edgeColour); }
	void SetEdgeColumn(Position column) { Call(Message::SetEdgeColumn, column); }
	void SetEdgeMode(EdgeVisualStyle edgeMode) { Call(Message::SetEdgeMode, static_cast<uintptr_t>(edgeMode)); }
	void SetElementColour(Element element, ColourAlpha colourElement) { Call(Message::SetElementColour, static_cast<uintptr_t>(element), colourElement); }
	void SetEmptySelection(Position caret) { Call(Message::SetEmptySelection, caret); }
	void SetEndAtLastLine(bool endAtLastLine) { Call(Message::SetEndAtLastLine, endAtLastLine); }
	void SetExtraAscent(int extraAscent) { Call(Message::SetExtraAscent, extraAscent); }
	void SetExtraDescent(int extraDescent) { Call(Message::SetExtraDescent, extraDescent); }
	void SetFirstVisibleLine(Line displayLine) { Call(Message::SetFirstVisibleLine, displayLine); }
	void SetFocus(bool focus) { Call(Message::SetFocus, focus); }
	void SetFoldExpanded(Line line, bool expanded) { Call(Message::SetFoldExpanded, line, expanded); }
	void SetFoldFlags(FoldFlag flags) { Call(Message::SetFoldFlags, static_cast<uintptr_t>(flags)); }
	void SetFoldLevel(Line line, FoldLevel level) { Call(Message::SetFoldLevel, line, static_cast<intptr_t>(level)); }
	void SetFoldMarginColour(bool useSetting, Colour back) { Call(Message::SetFoldMarginColour, useSetting, back); }
	void SetFoldMarginHiColour(bool useSetting, Colour fore) { Call(Message::SetFoldMarginHiColour, useSetting, fore); }
	void SetFontLocale(const char* localeName) { CallString(Message::SetFontLocale, 0, localeName); }
	void SetFontQuality(FontQuality fontQuality) { Call(Message::SetFontQuality, static_cast<uintptr_t>(fontQuality)); }
	void SetHScrollBar(bool visible) { Call(Message::SetHScrollBar, visible); }
	void SetHighlightGuide(Position column) { Call(Message::SetHighlightGuide, column); }
	void SetHotspotActiveBack(bool useSetting, Colour back) { Call(Message::SetHotspotActiveBack, useSetting, back); }
	void SetHotspotActiveFore(bool useSetting, Colour fore) { Call(Message::SetHotspotActiveFore, useSetting, fore); }
	void SetHotspotActiveUnderline(bool underline) { Call(Message::SetHotspotActiveUnderline, underline); }
	void SetHotspotSingleLine(bool singleLine) { Call(Message::SetHotspotSingleLine, singleLine); }
	void SetILexer(void* ilexer) { CallPointer(Message::SetILexer, 0, ilexer); }
	void SetIMEInteraction(IMEInteraction imeInteraction) { Call(Message::SetIMEInteraction, static_cast<uintptr_t>(imeInteraction)); }
	void SetIdentifier(int identifier) { Call(Message::SetIdentifier, identifier); }
	void SetIdentifiers(int style, const char* identifiers) { CallString(Message::SetIdentifiers, style, identifiers); }
	void SetIdleStyling(IdleStyling idleStyling) { Call(Message::SetIdleStyling, static_cast<uintptr_t>(idleStyling)); }
	void SetIndent(int indentSize) { Call(Message::SetIndent, indentSize); }
	void SetIndentationGuides(IndentView indentView) { Call(Message::SetIndentationGuides, static_cast<uintptr_t>(indentView)); }
	void SetIndicatorCurrent(int indicator) { Call(Message::SetIndicatorCurrent, indicator); }
	void SetIndicatorValue(int value) { Call(Message::SetIndicatorValue, value); }
	void SetKeyWords(int keyWordSet, const char* keyWords) { CallString(Message::SetKeyWords, keyWordSet, keyWords); }
	void SetLayoutCache(LineCache cacheMode) { Call(Message::SetLayoutCache, static_cast<uintptr_t>(cacheMode)); }
	void SetLayoutThreads(int threads) { Call(Message::SetLayoutThreads, threads); }
	void SetLengthForEncode(Position bytes) { Call(Message::SetLengthForEncode, bytes); }
	void SetLineEndTypesAllowed(LineEndType lineEndBitSet) { Call(Message::SetLineEndTypesAllowed, static_cast<uintptr_t>(lineEndBitSet)); }
	void SetLineIndentation(Line line, int indentation) { Call(Message::SetLineIndentation, line, indentation); }
	void SetLineState(Line line, int state) { Call(Message::SetLineState, line, state); }
	void SetMainSelection(int selection) { Call(Message::SetMainSelection, selection); }
	void SetMarginBackN(int margin, Colour back) { Call(Message::SetMarginBackN, margin, back); }
	void SetMarginCursorN(int margin, CursorShape cursor) { Call(Message::SetMarginCursorN, margin, static_cast<intptr_t>(cursor)); }
	void SetMarginLeft(int pixelWidth) { Call(Message::SetMarginLeft, 0, pixelWidth); }
	void SetMarginMaskN(int margin, int mask) { Call(Message::SetMarginMaskN, margin, mask); }
	void SetMarginOptions(MarginOption marginOptions) { Call(Message::SetMarginOptions, static_cast<uintptr_t>(marginOptions)); }
	void SetMarginRight(int pixelWidth) { Call(Message::SetMarginRight, 0, pixelWidth); }
	void SetMarginSensitiveN(int margin, bool sensitive) { Call(Message::SetMarginSensitiveN, margin, sensitive); }
	void SetMarginTypeN(int margin, MarginType marginType) { Call(Message::SetMarginTypeN, margin, static_cast<intptr_t>(marginType)); }
	void SetMarginWidthN(int margin, int pixelWidth) { Call(Message::SetMarginWidthN, margin, pixelWidth); }
	void SetMargins(int margins) { Call(Message::SetMargins, margins); }
	void SetModEventMask(ModificationFlags eventMask) { Call(Message::SetModEventMask, static_cast<uintptr_t>(eventMask)); }
	void SetMouseDownCaptures(bool captures) { Call(Message::SetMouseDownCaptures, captures); }
	void SetMouseDwellTime(int periodMilliseconds) { Call(Message::SetMouseDwellTime, periodMilliseconds); }
	void SetMouseSelectionRectangularSwitch(bool mouseSelectionRectangularSwitch) { Call(Message::SetMouseSelectionRectangularSwitch, mouseSelectionRectangularSwitch); }
	void SetMouseWheelCaptures(bool captures) { Call(Message::SetMouseWheelCaptures, captures); }
	void SetMoveExtendsSelection(bool moveExtendsSelection) { Call(Message::SetMoveExtendsSelection, moveExtendsSelection); }
	void SetMultiPaste(MultiPaste multiPaste) { Call(Message::SetMultiPaste, static_cast<uintptr_t>(multiPaste)); }
	void SetMultipleSelection(bool multipleSelection) { Call(Message::SetMultipleSelection, multipleSelection); }
	void SetOvertype(bool overType) { Call(Message::SetOvertype, overType); }
	void SetPasteConvertEndings(bool convert) { Call(Message::SetPasteConvertEndings, convert); }
	void SetPhasesDraw(PhasesDraw phases) { Call(Message::SetPhasesDraw, static_cast<uintptr_t>(phases)); }
	void SetPositionCache(int size) { Call(Message::SetPositionCache, size); }
	void SetPrintColourMode(PrintOption mode) { Call(Message::SetPrintColourMode, static_cast<uintptr_t>(mode)); }
	void SetPrintMagnification(int magnification) { Call(Message::SetPrintMagnification, magnification); }
	void SetPrintWrapMode(Wrap wrapMode) { Call(Message::SetPrintWrapMode, static_cast<uintptr_t>(wrapMode)); }
	void SetProperty(const char* key, const char* value) { CallString(Message::SetProperty, reinterpret_cast<uintptr_t>(key), value); }
	void SetPunctuationChars(const char* characters) { CallString(Message::SetPunctuationChars, 0, characters); }
	void SetReadOnly(bool readOnly) { Call(Message::SetReadOnly, readOnly); }
	void SetRectangularSelectionAnchor(Position anchor) { Call(Message::SetRectangularSelectionAnchor, anchor); }
	void SetRectangularSelectionAnchorVirtualSpace(Position space) { Call(Message::SetRectangularSelectionAnchorVirtualSpace, space); }
	void SetRectangularSelectionCaret(Position caret) { Call(Message::SetRectangularSelectionCaret, caret); }
	void SetRectangularSelectionCaretVirtualSpace(Position space) { Call(Message::SetRectangularSelectionCaretVirtualSpace, space); }
	void SetRectangularSelectionModifier(int modifier) { Call(Message::SetRectangularSelectionModifier, modifier); }
	void SetRepresentation(const char* encodedCharacter, const char* representation) { CallString(Message::SetRepresentation, reinterpret_cast<uintptr_t>(encodedCharacter), representation); }
	void SetRepresentationAppearance(const char* encodedCharacter, RepresentationAppearance appearance) { Call(Message::SetRepresentationAppearance, reinterpret_cast<uintptr_t>(encodedCharacter), static_cast<intptr_t>(appearance)); }
	void SetRepresentationColour(const char* encodedCharacter, ColourAlpha colour) { Call(Message::SetRepresentationColour, reinterpret_cast<uintptr_t>(encodedCharacter), colour); }
	void SetSavePoint() { Call(Message::SetSavePoint); }
	void SetScrollWidth(int pixelWidth) { Call(Message::SetScrollWidth, pixelWidth); }
	void SetScrollWidthTracking(bool tracking) { Call(Message::SetScrollWidthTracking, tracking); }
	void SetSearchFlags(FindOption searchFlags) { Call(Message::SetSearchFlags, static_cast<uintptr_t>(searchFlags)); }
	void SetSel(Position anchor, Position caret) { Call(Message::SetSel, anchor, caret); }
	void SetSelAlpha(Alpha alpha) { Call(Message::SetSelAlpha, static_cast<uintptr_t>(alpha)); }
	void SetSelBack(bool useSetting, Colour back) { Call(Message::SetSelBack, useSetting, back); }
	void SetSelEOLFilled(bool filled) { Call(Message::SetSelEOLFilled, filled); }
	void SetSelFore(bool useSetting, Colour fore) { Call(Message::SetSelFore, useSetting, fore); }
	void SetSelection(Position caret, Position anchor) { Call(Message::SetSelection, caret, anchor); }
	void SetSelectionEnd(Position caret) { Call(Message::SetSelectionEnd, caret); }
	void SetSelectionLayer(Layer layer) { Call(Message::SetSelectionLayer, static_cast<uintptr_t>(layer)); }
	void SetSelectionMode(SelectionMode selectionMode) { Call(Message::SetSelectionMode, static_cast<uintptr_t>(selectionMode)); }
	void SetSelectionNAnchor(int selection, Position anchor) { Call(Message::SetSelectionNAnchor, selection, anchor); }
	void SetSelectionNAnchorVirtualSpace(int selection, Position space) { Call(Message::SetSelectionNAnchorVirtualSpace, selection, space); }
	void SetSelectionNCaret(int selection, Position caret) { Call(Message::SetSelectionNCaret, selection, caret); }
	void SetSelectionNCaretVirtualSpace(int selection, Position space) { Call(Message::SetSelectionNCaretVirtualSpace, selection, space); }
	void SetSelectionNEnd(int selection, Position caret) { Call(Message::SetSelectionNEnd, selection, caret); }
	void SetSelectionNStart(int selection, Position anchor) { Call(Message::SetSelectionNStart, selection, anchor); }
	void SetSelectionSerialized(const char* selectionString) { CallString(Message::SetSelectionSerialized, 0, selectionString); }
	void SetSelectionStart(Position anchor) { Call(Message::SetSelectionStart, anchor); }
	void SetStatus(Status status) { Call(Message::SetStatus, static_cast<uintptr_t>(status)); }
	void SetStyling(Position length, int style) { Call(Message::SetStyling, length, style); }
	void SetStylingEx(Position length, const char* styles) { CallString(Message::SetStylingEx, length, styles); }
	void SetTabDrawMode(TabDrawMode tabDrawMode) { Call(Message::SetTabDrawMode, static_cast<uintptr_t>(tabDrawMode)); }
	void SetTabIndents(bool tabIndents) { Call(Message::SetTabIndents, tabIndents); }
	void SetTabMinimumWidth(int pixels) { Call(Message::SetTabMinimumWidth, pixels); }
	void SetTabWidth(int tabWidth) { Call(Message::SetTabWidth, tabWidth); }
	void SetTargetEnd(Position end) { Call(Message::SetTargetEnd, end); }
	void SetTargetEndVirtualSpace(Position space) { Call(Message::SetTargetEndVirtualSpace, space); }
	void SetTargetRange(Position start, Position end) { Call(Message::SetTargetRange, start, end); }
	void SetTargetStart(Position start) { Call(Message::SetTargetStart, start); }
	void SetTargetStartVirtualSpace(Position space) { Call(Message::SetTargetStartVirtualSpace, space); }
	void SetTechnology(Technology technology) { Call(Message::SetTechnology, static_cast<uintptr_t>(technology)); }
	void SetText(const char* text) { CallString(Message::SetText, 0, text); }
	void SetUndoCollection(bool collectUndo) { Call(Message::SetUndoCollection, collectUndo); }
	void SetUndoCurrent(int action) { Call(Message::SetUndoCurrent, action); }
	void SetUndoDetach(int action) { Call(Message::SetUndoDetach, action); }
	void SetUndoSavePoint(int action) { Call(Message::SetUndoSavePoint, action); }
	void SetUndoSelectionHistory(UndoSelectionHistoryOption undoSelectionHistory) { Call(Message::SetUndoSelectionHistory, static_cast<uintptr_t>(undoSelectionHistory)); }
	void SetUndoTentative(int action) { Call(Message::SetUndoTentative, action); }
	void SetUseTabs(bool useTabs) { Call(Message::SetUseTabs, useTabs); }
	void SetVScrollBar(bool visible) { Call(Message::SetVScrollBar, visible); }
	void SetViewEOL(bool visible) { Call(Message::SetViewEOL, visible); }
	void SetViewWS(WhiteSpace viewWS) { Call(Message::SetViewWS, static_cast<uintptr_t>(viewWS)); }
	void SetVirtualSpaceOptions(VirtualSpace virtualSpaceOptions) { Call(Message::SetVirtualSpaceOptions, static_cast<uintptr_t>(virtualSpaceOptions)); }
	void SetVisiblePolicy(VisiblePolicy visiblePolicy, int visibleSlop) { Call(Message::SetVisiblePolicy, static_cast<uintptr_t>(visiblePolicy), visibleSlop); }
	void SetWhitespaceBack(bool useSetting, Colour back) { Call(Message::SetWhitespaceBack, useSetting, back); }
	void SetWhitespaceChars(const char* characters) { CallString(Message::SetWhitespaceChars, 0, characters); }
	void SetWhitespaceFore(bool useSetting, Colour fore) { Call(Message::SetWhitespaceFore, useSetting, fore); }
	void SetWhitespaceSize(int size) { Call(Message::SetWhitespaceSize, size); }
	void SetWordChars(const char* characters) { CallString(Message::SetWordChars, 0, characters); }
	void SetWrapIndentMode(WrapIndentMode wrapIndentMode) { Call(Message::SetWrapIndentMode, static_cast<uintptr_t>(wrapIndentMode)); }
	void SetWrapMode(Wrap wrapMode) { Call(Message::SetWrapMode, static_cast<uintptr_t>(wrapMode)); }
	void SetWrapStartIndent(int indent) { Call(Message::SetWrapStartIndent, indent); }
	void SetWrapVisualFlags(WrapVisualFlag wrapVisualFlags) { Call(Message::SetWrapVisualFlags, static_cast<uintptr_t>(wrapVisualFlags)); }
	void SetWrapVisualFlagsLocation(WrapVisualLocation wrapVisualFlagsLocation) { Call(Message::SetWrapVisualFlagsLocation, static_cast<uintptr_t>(wrapVisualFlagsLocation)); }
	void SetXCaretPolicy(CaretPolicy caretPolicy, int caretSlop) { Call(Message::SetXCaretPolicy, static_cast<uintptr_t>(caretPolicy), caretSlop); }
	void SetXOffset(int xOffset) { Call(Message::SetXOffset, xOffset); }
	void SetYCaretPolicy(CaretPolicy caretPolicy, int caretSlop) { Call(Message::SetYCaretPolicy, static_cast<uintptr_t>(caretPolicy), caretSlop); }
	void SetZoom(int zoomInPoints) { Call(Message::SetZoom, zoomInPoints); }
	void ShowLines(Line lineStart, Line lineEnd) { Call(Message::ShowLines, lineStart, lineEnd); }
	void StartRecord() { Call(Message::StartRecord); }
	void StartStyling(Position start, int unused) { Call(Message::StartStyling, start, unused); }
	void StopRecord() { Call(Message::StopRecord); }
	void StutteredPageDown() { Call(Message::StutteredPageDown); }
	void StutteredPageDownExtend() { Call(Message::StutteredPageDownExtend); }
	void StutteredPageUp() { Call(Message::StutteredPageUp); }
	void StutteredPageUpExtend() { Call(Message::StutteredPageUpExtend); }
	void StyleClearAll() { Call(Message::StyleClearAll); }
	void StyleResetDefault() { Call(Message::StyleResetDefault); }
	void StyleSetBack(int style, Colour back) { Call(Message::StyleSetBack, style, back); }
	void StyleSetBold(int style, bool bold) { Call(Message::StyleSetBold, style, bold); }
	void StyleSetCase(int style, CaseVisible caseVisible) { Call(Message::StyleSetCase, style, static_cast<intptr_t>(caseVisible)); }
	void StyleSetChangeable(int style, bool changeable) { Call(Message::StyleSetChangeable, style, changeable); }
	void StyleSetCharacterSet(int style, CharacterSet characterSet) { Call(Message::StyleSetCharacterSet, style, static_cast<intptr_t>(characterSet)); }
	void StyleSetCheckMonospaced(int style, bool checkMonospaced) { Call(Message::StyleSetCheckMonospaced, style, checkMonospaced); }
	void StyleSetEOLFilled(int style, bool eolFilled) { Call(Message::StyleSetEOLFilled, style, eolFilled); }
	void StyleSetFont(int style, const char* fontName) { CallString(Message::StyleSetFont, style, fontName); }
	void StyleSetFore(int style, Colour fore) { Call(Message::StyleSetFore, style, fore); }
	void StyleSetHotSpot(int style, bool hotspot) { Call(Message::StyleSetHotSpot, style, hotspot); }
	void StyleSetInvisibleRepresentation(int style, const char* representation) { CallString(Message::StyleSetInvisibleRepresentation, style, representation); }
	void StyleSetItalic(int style, bool italic) { Call(Message::StyleSetItalic, style, italic); }
	void StyleSetSize(int style, int sizePoints) { Call(Message::StyleSetSize, style, sizePoints); }
	void StyleSetSizeFractional(int style, int sizeHundredthPoints) { Call(Message::StyleSetSizeFractional, style, sizeHundredthPoints); }
	void StyleSetStretch(int style, FontStretch stretch) { Call(Message::StyleSetStretch, style, static_cast<intptr_t>(stretch)); }
	void StyleSetUnderline(int style, bool underline) { Call(Message::StyleSetUnderline, style, underline); }
	void StyleSetVisible(int style, bool visible) { Call(Message::StyleSetVisible, style, visible); }
	void StyleSetWeight(int style, FontWeight weight) { Call(Message::StyleSetWeight, style, static_cast<intptr_t>(weight)); }
	void SwapMainAnchorCaret() { Call(Message::SwapMainAnchorCaret); }
	void Tab() { Call(Message::Tab); }
	void TargetFromSelection() { Call(Message::TargetFromSelection); }
	void TargetWholeDocument() { Call(Message::TargetWholeDocument); }
	void ToggleCaretSticky() { Call(Message::ToggleCaretSticky); }
	void ToggleFold(Line line) { Call(Message::ToggleFold, line); }
	void ToggleFoldShowText(Line line, const char* text) { CallString(Message::ToggleFoldShowText, line, text); }
	void Undo() { Call(Message::Undo); }
	void UpperCase() { Call(Message::UpperCase); }
	void UsePopUp(PopUp popUpMode) { Call(Message::UsePopUp, static_cast<uintptr_t>(popUpMode)); }
	void UserListShow(int listType, const char* itemList) { CallString(Message::UserListShow, listType, itemList); }
	void VCHome() { Call(Message::VCHome); }
	void VCHomeDisplay() { Call(Message::VCHomeDisplay); }
	void VCHomeDisplayExtend() { Call(Message::VCHomeDisplayExtend); }
	void VCHomeExtend() { Call(Message::VCHomeExtend); }
	void VCHomeRectExtend() { Call(Message::VCHomeRectExtend); }
	void VCHomeWrap() { Call(Message::VCHomeWrap); }
	void VCHomeWrapExtend() { Call(Message::VCHomeWrapExtend); }
	void VerticalCentreCaret() { Call(Message::VerticalCentreCaret); }
	void WordLeft() { Call(Message::WordLeft); }
	void WordLeftEnd() { Call(Message::WordLeftEnd); }
	void WordLeftEndExtend() { Call(Message::WordLeftEndExtend); }
	void WordLeftExtend() { Call(Message::WordLeftExtend); }
	void WordPartLeft() { Call(Message::WordPartLeft); }
	void WordPartLeftExtend() { Call(Message::WordPartLeftExtend); }
	void WordPartRight() { Call(Message::WordPartRight); }
	void WordPartRightExtend() { Call(Message::WordPartRightExtend); }
	void WordRight() { Call(Message::WordRight); }
	void WordRightEnd() { Call(Message::WordRightEnd); }
	void WordRightEndExtend() { Call(Message::WordRightEndExtend); }
	void WordRightExtend() { Call(Message::WordRightExtend); }
	void ZoomIn() { Call(Message::ZoomIn); }
	void ZoomOut() { Call(Message::ZoomOut); }
	void* CreateDocument(Position bytes, DocumentOption documentOptions) { return reinterpret_cast<void*>(Call(Message::CreateDocument, bytes, static_cast<intptr_t>(documentOptions))); }
	void* CreateLoader(Position bytes, DocumentOption documentOptions) { return reinterpret_cast<void*>(Call(Message::CreateLoader, bytes, static_cast<intptr_t>(documentOptions))); }
	void* GetCharacterPointer() { return reinterpret_cast<void*>(Call(Message::GetCharacterPointer)); }
	void* GetDirectFunction() { return reinterpret_cast<void*>(Call(Message::GetDirectFunction)); }
	void* GetDirectPointer() { return reinterpret_cast<void*>(Call(Message::GetDirectPointer)); }
	void* GetDirectStatusFunction() { return reinterpret_cast<void*>(Call(Message::GetDirectStatusFunction)); }
	void* GetDocPointer() { return reinterpret_cast<void*>(Call(Message::GetDocPointer)); }
	void* GetRangePointer(Position start, Position lengthRange) { return reinterpret_cast<void*>(Call(Message::GetRangePointer, start, lengthRange)); }
	void* PrivateLexerCall(int operation, void* pointer) { return reinterpret_cast<void*>(CallPointer(Message::PrivateLexerCall, operation, pointer)); }

	void SetFnPtr()
	{
		ATLASSERT(::IsWindow(this->m_hWnd));
		fn = reinterpret_cast<FunctionDirect>(::SendMessageW(this->m_hWnd, static_cast<uint32_t>(Message::GetDirectFunction), 0, 0));
		ptr = ::SendMessageW(this->m_hWnd, static_cast<uint32_t>(Message::GetDirectPointer), 0, 0);
	}

private:
	using FunctionDirect = intptr_t(*)(intptr_t ptr, uint32_t msg, uintptr_t wParam, intptr_t lParam);

	intptr_t Call(Message msg, uintptr_t wParam = 0, intptr_t lParam = 0)
	{
		return fn(ptr, static_cast<uint32_t>(msg), wParam, lParam);
	}

	intptr_t CallPointer(Message msg, uintptr_t wParam, void* s)
	{
		return Call(msg, wParam, reinterpret_cast<intptr_t>(s));
	}

	intptr_t CallString(Message msg, uintptr_t wParam, const char* s)
	{
		return Call(msg, wParam, reinterpret_cast<intptr_t>(s));
	}

	FunctionDirect fn{};
	intptr_t ptr{};
};
