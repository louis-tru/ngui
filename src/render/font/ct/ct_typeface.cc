/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./ct_typeface.h"
#include "../sfnt/QkOTTable_OS_2.h"
#include "../../codec/codec.h"
#include "../../../util/fs.h"

String QkCFTypeIDDescription(CFTypeID id) {
	QkUniqueCFRef<CFStringRef> typeDescription(CFCopyTypeIDDescription(id));
	return QkStringFromCFString(typeDescription.get());
}

template<typename CF> CFTypeID QkCFGetTypeID();
#define Qk_GETCFTYPEID(cf) \
template<> CFTypeID QkCFGetTypeID<cf##Ref>() { return cf##GetTypeID(); }
Qk_GETCFTYPEID(CFBoolean);
Qk_GETCFTYPEID(CFDictionary);
Qk_GETCFTYPEID(CFNumber);

/* Checked dynamic downcast of CFTypeRef.
 *
 * @param cf the ref to downcast.
 * @param cfAsCF if cf can be cast to the type CF, receives the downcast ref.
 * @param name if non-nullptr the cast is expected to succeed and failures will be logged.
 * @return true if the cast succeeds, false otherwise.
 */
template <typename CF>
static bool SkCFDynamicCast(CFTypeRef cf, CF* cfAsCF, char const* name) {
	// Qk_DEBUG("SkCFDynamicCast '%s' of type %s to type %s\n", name ? name : "<annon>",
	//         SkCFTypeIDDescription(  CFGetTypeID(cf)  ).c_str()
	//         SkCFTypeIDDescription(SkCFGetTypeID<CF>()).c_str());
	if (!cf) {
		if (name) {
			Qk_DEBUG("%s not present\n", name);
		}
		return false;
	}
	if (CFGetTypeID(cf) != QkCFGetTypeID<CF>()) {
		if (name) {
			Qk_DEBUG("%s is a %s but expected a %s\n", name,
								QkCFTypeIDDescription(CFGetTypeID(cf)  ).c_str(),
								QkCFTypeIDDescription(QkCFGetTypeID<CF>()).c_str());
		}
		return false;
	}
	*cfAsCF = static_cast<CF>(cf);
	return true;
}

template <typename S, typename D, typename C> struct QkLinearInterpolater {
	struct Mapping {
		S src_val;
		D dst_val;
	};
	constexpr QkLinearInterpolater(Mapping const mapping[], int mappingCount)
		: fMapping(mapping), fMappingCount(mappingCount) {}

	static D map(S value, S src_min, S src_max, D dst_min, D dst_max) {
		Qk_ASSERT(src_min < src_max);
		Qk_ASSERT(dst_min <= dst_max);
		return C()(dst_min + (((value - src_min) * (dst_max - dst_min)) / (src_max - src_min)));
	}

	D map(S val) const {
		// -Inf to [0]
		if (val < fMapping[0].src_val) {
			return fMapping[0].dst_val;
		}
		// Linear from [i] to [i+1]
		for (int i = 0; i < fMappingCount - 1; ++i) {
			if (val < fMapping[i+1].src_val) {
				return map(val, fMapping[i].src_val, fMapping[i+1].src_val,
												fMapping[i].dst_val, fMapping[i+1].dst_val);
			}
		}
		// From [n] to +Inf
		// if (fcweight < Inf)
		return fMapping[fMappingCount - 1].dst_val;
	}

	Mapping const * fMapping;
	int fMappingCount;
};

struct RoundCGFloatToInt {
		int operator()(CGFloat s) { return s + 0.5; }
};
struct CGFloatIdentity {
		CGFloat operator()(CGFloat s) { return s; }
};

/** Convert the [0, 1000] CSS weight to [-1, 1] CTFontDescriptor weight (for system fonts).
 *
 *  The -1 to 1 weights reported by CTFontDescriptors have different mappings depending on if the
 *  CTFont is native or created from a CGDataProvider.
 */
CGFloat QkCTFontCTWeightForCSSWeight(TextWeight fontstyleWeight) {
	using Interpolator = QkLinearInterpolater<int, CGFloat, CGFloatIdentity>;

	// Note that Mac supports the old OS2 version A so 0 through 10 are as if multiplied by 100.
	// However, on this end we can't tell, so this is ignored.

	static Interpolator::Mapping nativeWeightMappings[11];
	
	static int onceInit = []{
		const CGFloat(&nsFontWeights)[11] = QkCTFontGetNSFontWeightMapping();
		for (int i = 0; i < 11; ++i) {
			nativeWeightMappings[i].src_val = i * 100;
			nativeWeightMappings[i].dst_val = nsFontWeights[i];
		}
		return 0;
	}();

	static constexpr Interpolator nativeInterpolator(
					nativeWeightMappings, Qk_ARRAY_COUNT(nativeWeightMappings));

	return nativeInterpolator.map(int(fontstyleWeight));
}

/** Convert the [0, 10] CSS weight to [-1, 1] CTFontDescriptor width. */
CGFloat QkCTFontCTWidthForCSSWidth(TextWidth fontstyleWidth) {
	using Interpolator = QkLinearInterpolater<int, CGFloat, CGFloatIdentity>;

	// Values determined by creating font data with every width, creating a CTFont,
	// and asking the CTFont for its width. See TypefaceStyle test for basics.
	static constexpr Interpolator::Mapping widthMappings[] = {
		{  0, -0.5 },
		{ 10,  0.5 },
	};
	static constexpr Interpolator interpolator(widthMappings, Qk_ARRAY_COUNT(widthMappings));
	return interpolator.map(int(fontstyleWidth));
}

// In macOS 10.12 and later any variation on the CGFont which has default axis value will be
// dropped when creating the CTFont. Unfortunately, in macOS 10.15 the priority of setting
// the optical size (and opsz variation) is
// 1. the value of kCTFontOpticalSizeAttribute in the CTFontDescriptor (undocumented)
// 2. the opsz axis default value if kCTFontOpticalSizeAttribute is 'none' (undocumented)
// 3. the opsz variation on the nascent CTFont from the CGFont (was dropped if default)
// 4. the opsz variation in kCTFontVariationAttribute in CTFontDescriptor (crashes 10.10)
// 5. the size requested (can fudge in SkTypeface but not SkScalerContext)
// The first one which is found will be used to set the opsz variation (after clamping).
static void add_opsz_attr(CFMutableDictionaryRef attr, double opsz) {
	QkUniqueCFRef<CFNumberRef> opszValueNumber(
			CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &opsz));
	// Avoid using kCTFontOpticalSizeAttribute directly
	CFStringRef SkCTFontOpticalSizeAttribute = CFSTR("NSCTFontOpticalSizeAttribute");
	CFDictionarySetValue(attr, SkCTFontOpticalSizeAttribute, opszValueNumber.get());
}

// This turns off application of the 'trak' table to advances, but also all other tracking.
static void add_notrak_attr(CFMutableDictionaryRef attr) {
	int zero = 0;
	QkUniqueCFRef<CFNumberRef> unscaledTrackingNumber(
			CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &zero));
	CFStringRef SkCTFontUnscaledTrackingAttribute = CFSTR("NSCTFontUnscaledTrackingAttribute");
	CFDictionarySetValue(attr, SkCTFontUnscaledTrackingAttribute, unscaledTrackingNumber.get());
}

QkUniqueCFRef<CTFontRef> QkCTFontCreateExactCopy(CTFontRef baseFont, CGFloat textSize, OpszVariation opsz)
{
	QkUniqueCFRef<CFMutableDictionaryRef> attr(
	CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
														&kCFTypeDictionaryKeyCallBacks,
														&kCFTypeDictionaryValueCallBacks));

	if (opsz.isSet) {
			add_opsz_attr(attr.get(), opsz.value);
	} else {
		// On (at least) 10.10 though 10.14 the default system font was SFNSText/SFNSDisplay.
		// The CTFont is backed by both; optical size < 20 means SFNSText else SFNSDisplay.
		// On at least 10.11 the glyph ids in these fonts became non-interchangable.
		// To keep glyph ids stable over size changes, preserve the optical size.
		// In 10.15 this was replaced with use of variable fonts with an opsz axis.
		// A CTFont backed by multiple fonts picked by opsz where the multiple backing fonts are
		// variable fonts with opsz axis and non-interchangeable glyph ids would break the
		// opsz.isSet branch above, but hopefully that never happens.
		// See https://crbug.com/524646 .
		CFStringRef SkCTFontOpticalSizeAttribute = CFSTR("NSCTFontOpticalSizeAttribute");
		QkUniqueCFRef<CFTypeRef> opsz(CTFontCopyAttribute(baseFont, SkCTFontOpticalSizeAttribute));
		double opsz_val;
		if (!opsz ||
				CFGetTypeID(opsz.get()) != CFNumberGetTypeID() ||
				!CFNumberGetValue(static_cast<CFNumberRef>(opsz.get()),kCFNumberDoubleType,&opsz_val) ||
				opsz_val <= 0)
		{
				opsz_val = CTFontGetSize(baseFont);
		}
		add_opsz_attr(attr.get(), opsz_val);
	}
	add_notrak_attr(attr.get());

	QkUniqueCFRef<CTFontDescriptorRef> desc(CTFontDescriptorCreateWithAttributes(attr.get()));

	return QkUniqueCFRef<CTFontRef>(
					CTFontCreateCopyWithAttributes(baseFont, textSize, nullptr, desc.get()));
}

/** Convert the [-1, 1] CTFontDescriptor weight to [0, 1000] CSS weight.
 *
 *  The -1 to 1 weights reported by CTFontDescriptors have different mappings depending on if the
 *  CTFont is native or created from a CGDataProvider.
 */
static int ct_weight_to_fontstyle(CGFloat cgWeight, bool fromDataProvider) {
	using Interpolator = QkLinearInterpolater<CGFloat, int, RoundCGFloatToInt>;

	// Note that Mac supports the old OS2 version A so 0 through 10 are as if multiplied by 100.
	// However, on this end we can't tell, so this is ignored.

	static Interpolator::Mapping nativeWeightMappings[11];
	static Interpolator::Mapping dataProviderWeightMappings[11];
	static int once = [] {
		const CGFloat(&nsFontWeights)[11] = QkCTFontGetNSFontWeightMapping();
		const CGFloat(&userFontWeights)[11] = QkCTFontGetDataFontWeightMapping();
		for (int i = 0; i < 11; ++i) {
			nativeWeightMappings[i].src_val = nsFontWeights[i];
			nativeWeightMappings[i].dst_val = i * 100;

			dataProviderWeightMappings[i].src_val = userFontWeights[i];
			dataProviderWeightMappings[i].dst_val = i * 100;
		}
		return 0;
	}();
	static constexpr Interpolator nativeInterpolator(
					nativeWeightMappings, Qk_ARRAY_COUNT(nativeWeightMappings));
	static constexpr Interpolator dataProviderInterpolator(
					dataProviderWeightMappings, Qk_ARRAY_COUNT(dataProviderWeightMappings));

	return fromDataProvider ? dataProviderInterpolator.map(cgWeight)
													: nativeInterpolator.map(cgWeight);
}

/** Convert the [-1, 1] CTFontDescriptor width to [0, 10] CSS weight. */
static int ct_width_to_fontstyle(CGFloat cgWidth) {
	using Interpolator = QkLinearInterpolater<CGFloat, int, RoundCGFloatToInt>;

	// Values determined by creating font data with every width, creating a CTFont,
	// and asking the CTFont for its width. See TypefaceStyle test for basics.
	static constexpr Interpolator::Mapping widthMappings[] = {
		{ -0.5,  0 },
		{  0.5, 10 },
	};
	static constexpr Interpolator interpolator(widthMappings, Qk_ARRAY_COUNT(widthMappings));
	return interpolator.map(cgWidth);
}

static bool find_dict_CGFloat(CFDictionaryRef dict, CFStringRef name, CGFloat* value) {
	CFNumberRef num;
	return CFDictionaryGetValueIfPresent(dict, name, (const void**)&num)
		&& CFNumberIsFloatType(num)
		&& CFNumberGetValue(num, kCFNumberCGFloatType, value);
}

FontStyle QkCTFontDescriptorGetSkFontStyle(CTFontDescriptorRef desc, bool fromDataProvider) {
	QkUniqueCFRef<CFTypeRef> traits(CTFontDescriptorCopyAttribute(desc, kCTFontTraitsAttribute));
	CFDictionaryRef fontTraitsDict;
	if (!SkCFDynamicCast(traits.get(), &fontTraitsDict, "Font traits")) {
		return FontStyle();
	}

	CGFloat weight, width, slant;
	if (!find_dict_CGFloat(fontTraitsDict, kCTFontWeightTrait, &weight)) {
		weight = 0;
	}
	if (!find_dict_CGFloat(fontTraitsDict, kCTFontWidthTrait, &width)) {
		width = 0;
	}
	if (!find_dict_CGFloat(fontTraitsDict, kCTFontSlantTrait, &slant)) {
		slant = 0;
	}

	return FontStyle((TextWeight)ct_weight_to_fontstyle(weight, fromDataProvider),
										(TextWidth)ct_width_to_fontstyle(width),
										slant ? TextSlant::ITALIC: TextSlant::DEFAULT);
}


// If, as is the case with web fonts, the CTFont data isn't available,
// the CGFont data may work. While the CGFont may always provide the
// right result, leave the CTFont code path to minimize disruption.
static QkUniqueCFRef<CFDataRef> copy_table_from_font(CTFontRef ctFont, FontTableTag tag) {
	QkUniqueCFRef<CFDataRef> data(CTFontCopyTable(ctFont, (CTFontTableTag) tag,
																								kCTFontTableOptionNoOptions));
	if (!data) {
		QkUniqueCFRef<CGFontRef> cgFont(CTFontCopyGraphicsFont(ctFont, nullptr));
		data.reset(CGFontCopyTableForTag(cgFont.get(), tag));
	}
	return data;
}

static inline bool QkUTF16_IsLeadingSurrogate(uint16_t c) { return ((c) & 0xFC00) == 0xD800; }

size_t QkToUTF16(Unichar uni, uint16_t utf16[2]) {
	if ((uint32_t)uni > 0x10FFFF) {
		return 0;
	}
	int extra = (uni > 0xFFFF);
	if (utf16) {
		if (extra) {
			utf16[0] = (uint16_t)((0xD800 - 64) + (uni >> 10));
			utf16[1] = (uint16_t)(0xDC00 | (uni & 0x3FF));
		} else {
			utf16[0] = (uint16_t)uni;
		}
	}
	return 1 + extra;
}

// -----------------------------------------------------------------------------------

Typeface_Mac::Typeface_Mac(QkUniqueCFRef<CTFontRef> font, OpszVariation opszVariation, bool isData)
	: Typeface(FontStyle(), true)
	, fRGBSpace(nullptr)
	, fFontRef(std::move(font))
	, fOpszVariation(opszVariation)
	, fHasColorGlyphs(CTFontGetSymbolicTraits(fFontRef.get()) & kCTFontColorGlyphsTrait)
	, fIsData(isData)
{
	Qk_ASSERT(fFontRef);
	QkUniqueCFRef<CTFontDescriptorRef> desc(CTFontCopyFontDescriptor(fFontRef.get()));
	FontStyle style = QkCTFontDescriptorGetSkFontStyle(desc.get(), fIsData);
	CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(font.get());
	bool isFixedPitch = (traits & kCTFontMonoSpaceTrait);
	setFontStyle(style);
	setIsFixedPitch(isFixedPitch);
}

int Typeface_Mac::onCountGlyphs() const {
	return int(CTFontGetGlyphCount(fFontRef.get()));
}

int Typeface_Mac::onGetUPEM() const {
	QkUniqueCFRef<CGFontRef> cgFont(CTFontCopyGraphicsFont(fFontRef.get(), nullptr));
	return CGFontGetUnitsPerEm(cgFont.get());
}

int Typeface_Mac::onGetTableTags(FontTableTag tags[]) const {
	QkUniqueCFRef<CFArrayRef> cfArray(
		CTFontCopyAvailableTables(fFontRef.get(), kCTFontTableOptionNoOptions));
	if (!cfArray) {
		return 0;
	}
	int count = int(CFArrayGetCount(cfArray.get()));
	if (tags) {
		for (int i = 0; i < count; ++i) {
			uintptr_t fontTag = reinterpret_cast<uintptr_t>(
					CFArrayGetValueAtIndex(cfArray.get(), i));
			tags[i] = static_cast<FontTableTag>(fontTag);
		}
	}
	return count;
}

bool Typeface_Mac::onGetPostScriptName(String* skPostScriptName) const {
	QkUniqueCFRef<CFStringRef> ctPostScriptName(CTFontCopyPostScriptName(fFontRef.get()));
	if (!ctPostScriptName) {
		return false;
	}
	if (skPostScriptName) {
		*skPostScriptName = QkStringFromCFString(ctPostScriptName.get());
	}
	return true;
}

String Typeface_Mac::onGetFamilyName() const {
	QkUniqueCFRef<CFStringRef> familyName(CTFontCopyFamilyName(fFontRef.get()));
	return QkStringFromCFString(familyName.get());
}

size_t Typeface_Mac::onGetTableData(FontTableTag tag, size_t offset,size_t length, void* dstData) const {
	QkUniqueCFRef<CFDataRef> srcData = copy_table_from_font(fFontRef.get(), tag);
	if (!srcData) {
		return 0;
	}
	size_t srcSize = CFDataGetLength(srcData.get());
	if (offset >= srcSize) {
		return 0;
	}
	if (length > srcSize - offset) {
		length = srcSize - offset;
	}
	if (dstData) {
		memcpy(dstData, CFDataGetBytePtr(srcData.get()) + offset, length);
	}
	return length;
}

void Typeface_Mac::onCharsToGlyphs(const Unichar uni[], int count, GlyphID glyphs[]) const {
	// Undocumented behavior of CTFontGetGlyphsForCharacters with non-bmp code points:
	// When a surrogate pair is detected, the glyph index used is the index of the high surrogate.
	// It is documented that if a mapping is unavailable, the glyph will be set to 0.

	ArrayBuffer<uint16_t> charStorage = ArrayBuffer<uint16_t>::alloc(2 * count);
	const UniChar* src = *charStorage; // UniChar is a UTF-16 16-bit code unit.
	const Unichar* utf32 = uni;
	UniChar* utf16 = *charStorage;
	for (int i = 0; i < count; ++i) {
		utf16 += QkToUTF16(utf32[i], utf16);
	}
	int srcCount = int(utf16 - src);

	// If there are any non-bmp code points, the provided 'glyphs' storage will be inadequate.
	ArrayBuffer<uint16_t> glyphStorage;
	uint16_t* macGlyphs = glyphs;
	if (srcCount > count) {
		glyphStorage.extend(srcCount);
		macGlyphs = *glyphStorage;
	}

	CTFontGetGlyphsForCharacters(fFontRef.get(), src, macGlyphs, srcCount);

	// If there were any non-bmp, then copy and compact.
	// If all are bmp, 'glyphs' already contains the compact glyphs.
	// If some are non-bmp, copy and compact into 'glyphs'.
	if (srcCount > count) {
		Qk_ASSERT(glyphs != macGlyphs);
		int extra = 0;
		for (int i = 0; i < count; ++i) {
			glyphs[i] = macGlyphs[i + extra];
			if (QkUTF16_IsLeadingSurrogate(src[i + extra])) {
				++extra;
			}
		}
	} else {
		Qk_ASSERT(glyphs == macGlyphs);
	}
}

QkUniqueCFRef<CTFontRef> Typeface_Mac::ctFont(float fontSize) const {
	if (fontSize != CTFontGetSize(fFontRef.get())) {
		return QkUniqueCFRef<CTFontRef>(CTFontCreateCopyWithAttributes(fFontRef.get(), fontSize, nullptr, nullptr));
	} else {
		return QkUniqueCFRef<CTFontRef>(fFontRef.get());
	}
}

static inline float SkScalarFromCGFloat(CGFloat cgFloat) {
	return static_cast<float>(cgFloat);
}

static inline CGFloat SkCGRectGetMaxY(const CGRect& rect) {
		return rect.origin.y + rect.size.height;
}

static inline CGFloat SkCGRectGetWidth(const CGRect& rect) {
		return rect.size.width;
}

static inline CGFloat SkCGRectGetMinY(const CGRect& rect) {
		return rect.origin.y;
}

static inline CGFloat SkCGRectGetMaxX(const CGRect& rect) {
		return rect.origin.x + rect.size.width;
}

static inline CGFloat SkCGRectGetMinX(const CGRect& rect) {
		return rect.origin.x;
}

void Typeface_Mac::onGetMetrics(FontMetrics* metrics) const {
	if (nullptr == metrics)
		return;

	constexpr float fontSize = 64.0;
	auto font = ctFont(fontSize);
	CTFontRef fontRef = font.get();

	CGRect theBounds = CTFontGetBoundingBox(fontRef);

	metrics->fTop          = SkScalarFromCGFloat(-SkCGRectGetMaxY(theBounds));
	metrics->fAscent       = SkScalarFromCGFloat(-CTFontGetAscent(fontRef));
	metrics->fDescent      = SkScalarFromCGFloat( CTFontGetDescent(fontRef));
	metrics->fBottom       = SkScalarFromCGFloat(-SkCGRectGetMinY(theBounds));
	metrics->fLeading      = SkScalarFromCGFloat( CTFontGetLeading(fontRef));
	metrics->fAvgCharWidth = SkScalarFromCGFloat( SkCGRectGetWidth(theBounds));
	metrics->fXMin         = SkScalarFromCGFloat( SkCGRectGetMinX(theBounds));
	metrics->fXMax         = SkScalarFromCGFloat( SkCGRectGetMaxX(theBounds));
	metrics->fMaxCharWidth = metrics->fXMax - metrics->fXMin;
	metrics->fXHeight      = SkScalarFromCGFloat( CTFontGetXHeight(fontRef));
	metrics->fCapHeight    = SkScalarFromCGFloat( CTFontGetCapHeight(fontRef));
	metrics->fUnderlineThickness = SkScalarFromCGFloat( CTFontGetUnderlineThickness(fontRef));
	metrics->fUnderlinePosition = -SkScalarFromCGFloat( CTFontGetUnderlinePosition(fontRef));

	metrics->fFlags = 0;
	metrics->fFlags |= FontMetrics::kUnderlineThicknessIsValid_Flag;
	metrics->fFlags |= FontMetrics::kUnderlinePositionIsValid_Flag;

	QkUniqueCFRef<CFArrayRef> ctAxes(CTFontCopyVariationAxes(fontRef));
	if (ctAxes && CFArrayGetCount(ctAxes.get()) > 0) {
		// The bounds are only valid for the default variation.
		metrics->fFlags |= FontMetrics::kBoundsInvalid_Flag;
	}

	// See https://bugs.chromium.org/p/skia/issues/detail?id=6203
	// At least on 10.12.3 with memory based fonts the x-height is always 0.6666 of the ascent and
	// the cap-height is always 0.8888 of the ascent. It appears that the values from the 'OS/2'
	// table are read, but then overwritten if the font is not a system font. As a result, if there
	// is a valid 'OS/2' table available use the values from the table if they aren't too strange.
	struct OS2HeightMetrics {
		uint16_t sxHeight;
		uint16_t sCapHeight;
	} heights;
	size_t bytesRead = onGetTableData(
					QkEndian_SwapBE32(QkOTTableOS2::TAG), offsetof(QkOTTableOS2, version.v2.sxHeight),
					sizeof(heights), &heights);
	if (bytesRead == sizeof(heights)) {
		// 'fontSize' is correct because the entire resolved size is set by the constructor.
		unsigned upem = CTFontGetUnitsPerEm(fontRef);
		unsigned maxSaneHeight = upem * 2;
		uint16_t xHeight = QkEndian_SwapBE16(heights.sxHeight);
		if (xHeight && xHeight < maxSaneHeight) {
			metrics->fXHeight = SkScalarFromCGFloat(xHeight * fontSize / upem);
		}
		uint16_t capHeight = QkEndian_SwapBE16(heights.sCapHeight);
		if (capHeight && capHeight < maxSaneHeight) {
			metrics->fCapHeight = SkScalarFromCGFloat(capHeight * fontSize / upem);
		}
	}
}

static inline bool QkCGRectIsEmpty(const CGRect& rect) {
	return rect.size.width <= 0 || rect.size.height <= 0;
}

void Typeface_Mac::onGetGlyph(GlyphID id, FontGlyphMetrics* glyph) const {
	if (nullptr == glyph)
		return;

	auto font = ctFont(64.0);
	CTFontRef fontRef = font.get();

	const CGGlyph cgGlyph = (CGGlyph) id;

	// The following block produces cgAdvance in CG units (pixels, y up).
	CGSize cgAdvance;
	CTFontGetAdvancesForGlyphs(fontRef, kCTFontOrientationHorizontal,
															&cgGlyph, &cgAdvance, 1);
	// cgAdvance = CGSizeApplyAffineTransform(cgAdvance, fTransform);
	glyph->advanceX =  static_cast<float>(cgAdvance.width);
	glyph->advanceY = -static_cast<float>(cgAdvance.height);

	// The following produces skBounds in SkGlyph units (pixels, y down),
	// or returns early if skBounds would be empty.
	quark::Rect bounds;

	// Glyphs are always drawn from the horizontal origin. The caller must manually use the result
	// of CTFontGetVerticalTranslationsForGlyphs to calculate where to draw the glyph for vertical
	// glyphs. As a result, always get the horizontal bounds of a glyph and translate it if the
	// glyph is vertical. This avoids any diagreement between the various means of retrieving
	// vertical metrics.
	{
		// CTFontGetBoundingRectsForGlyphs produces cgBounds in CG units (pixels, y up).
		CGRect cgBounds;
		CTFontGetBoundingRectsForGlyphs(fontRef, kCTFontOrientationHorizontal, &cgGlyph, &cgBounds, 1);
		// cgBounds = CGRectApplyAffineTransform(cgBounds, fTransform);

		if (QkCGRectIsEmpty(cgBounds)) {
			return;
		}

		// Convert cgBounds to SkGlyph units (pixels, y down).
		bounds = {
			Vec2(cgBounds.origin.x, -cgBounds.origin.y - cgBounds.size.height),
			Vec2(cgBounds.size.width, cgBounds.size.height),
		};
		
		Qk_DEBUG("#Typeface_Mac#onGetGlyph,f%", bounds.origin.x());
	}

	glyph->left = bounds.origin.x();
	glyph->top = bounds.origin.y();
	glyph->width = bounds.size.x();
	glyph->height = bounds.size.y();
}

void Typeface_Mac::onGetPath(GlyphID glyph, PathLine *path) const {
	if (nullptr == path)
		return;
	auto font = ctFont(64.0);
	CTFontRef fontRef = font.get();

	// TODO ...
}

float Typeface_Mac::onGetImage(const Array<GlyphID>& glyphs, float fontSize, Sp<ImageSource> *imgOut) {

	if (!fRGBSpace) {
		//It doesn't appear to matter what color space is specified.
		//Regular blends and antialiased text are always (s*a + d*(1-a))
		//and subpixel antialiased text is always g=2.0.
		fRGBSpace.reset(CGColorSpaceCreateDeviceRGB());
	}

	auto font = ctFont(fontSize);
	CTFontRef fontRef = font.get();

	const CGGlyph* cgGlyph = (const CGGlyph*) *glyphs;
	
	Array<CGSize> cgAdvance(glyphs.length());
	Array<CGRect> cgBounds(glyphs.length());
	Array<CGPoint> drawPoints(glyphs.length());

	CTFontGetAdvancesForGlyphs(
		fontRef, kCTFontOrientationHorizontal, cgGlyph, *cgAdvance, glyphs.length());
	CGRect bounds = CTFontGetBoundingRectsForGlyphs(
		fontRef, kCTFontOrientationHorizontal, cgGlyph, *cgBounds, glyphs.length());
	
	float top = bounds.size.height + bounds.origin.y;
	float width_f = 0;
	
	for (int i = 0; i < glyphs.length(); i++) {
		drawPoints[i].x = width_f;
		drawPoints[i].y = -cgBounds[i].origin.y;
		Qk_DEBUG("#Typeface_Mac#onGetImage,bounds, left:%f, top:%f, width:%f, height:%f | advanceX:%f",
			cgBounds[i].origin.x, cgBounds[i].origin.y,
			cgBounds[i].size.width, cgBounds[i].size.height, cgAdvance[i].width);
		width_f += cgAdvance[i].width;
	}

	int width = ceilf(width_f + cgBounds.back().origin.x);
	int height = ceilf(bounds.size.height);
	
	Qk_DEBUG("#Typeface_Mac#onGetImage,bounds[i].origin.y=%f,top=%f,height=%d", bounds.origin.y, top, height);

	int rowBytes = width * sizeof(uint32_t);
	Buffer image = Buffer::alloc(rowBytes * height);

	memset(*image, 0, image.length()); // reset storage

	const CGImageAlphaInfo alpha = kCGImageAlphaPremultipliedFirst;
	//const CGImageAlphaInfo alpha = kCGImageAlphaNoneSkipFirst;
	const CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Host | (CGBitmapInfo)alpha;
	
	QkUniqueCFRef<CGContextRef> fCG(
		CGBitmapContextCreate(*image, width, height, 8, rowBytes, fRGBSpace.get(), bitmapInfo));

	// Skia handles quantization and subpixel positioning,
	// so disable quantization and enable subpixel positioning in CG.
	CGContextSetAllowsFontSubpixelQuantization(fCG.get(), false);
	CGContextSetShouldSubpixelQuantizeFonts(fCG.get(), false);

	// Because CG always draws from the horizontal baseline,
	// if there is a non-integral translation from the horizontal origin to the vertical origin,
	// then CG cannot draw the glyph in the correct location without subpixel positioning.
	CGContextSetAllowsFontSubpixelPositioning(fCG.get(), true);
	CGContextSetShouldSubpixelPositionFonts(fCG.get(), true);

	CGContextSetTextDrawingMode(fCG.get(), kCGTextFill);

	// Draw black on white to create mask. (Special path exists to speed this up in CG.)
	CGContextSetGrayFillColor(fCG.get(), 0.0f, 1.0f);

	CGContextSetShouldAntialias(fCG.get(), true);
	CGContextSetShouldSmoothFonts(fCG.get(), false);

	CTFontDrawGlyphs(fontRef, cgGlyph, *drawPoints, glyphs.length(), fCG.get());
	
	Pixel pix(PixelInfo(width, height, kColor_Type_RGBA_8888, kAlphaType_Unpremul), image);
	
	//auto data = ImageCodec::Make(ImageCodec::TGA)->encode(pix);
	//auto path = fs_documents("test.tga");
	//auto write = fs_write_file_sync(path, *data, data.size());
	//Qk_DEBUG("#Typeface_Mac#onGetImage,write:%d,%s", write, path.c_str());

	*imgOut = new ImageSource(std::move(pix));
	
	return top;
}