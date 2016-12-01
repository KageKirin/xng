#ifndef XNG_PNG_H_INC
#define XNG_PNG_H_INC

#include "xng/xng.h"
#include "xng/common/xng_common.h"

#include <cctype>
#include <string>
#include <vector>

namespace xng
{
	namespace png
	{
		//-------------------------------------------------------------------------
		//! imports
		using common::_Optional;

		//-------------------------------------------------------------------------
		//! enums used in intermediate structures

		enum class ColorType : uint8_t
		{
			GREY	   = 0,	// greyscale: 1,2,4,8,16 bit
			RGB		   = 2,	// RGB: 8,16 bit
			PALETTE	= 3,	// palette: 1,2,4,8 bit
			GREY_ALPHA = 4,	// greyscale with alpha: 8,16 bit
			RGBA	   = 6	 // RGB with alpha: 8,16 bit
		};

		enum class CompressionMethod : uint8_t
		{

		};

		enum class FilterMethod : uint8_t
		{

		};

		enum class InterlaceMethod : uint8_t
		{

		};

		enum class RenderingIntent : uint8_t
		{
			Perceptual = 0,
			RelativeColorimetric,
			Saturation,
			AbsoluteColorimetric,
		};

		enum class AnimationFrameDisposeOperation : uint8_t
		{
			None = 0,
			Background,
			Previous,
		};

		enum class AnimationFrameBlendOperation : uint8_t
		{
			Source = 0,
			Overwrite,
		};

		//-------------------------------------------------------------------------
		//! intermediate structures to decode PNG chunk data

		struct Palette
		{
			// RGB0: alpha is set to 0xFF when first reading PLTE chunk
			// for RGBA: requires OR-ing with Transparency.alphas
			std::vector<uint32_t> colors;
		};

		struct Transparency : _Optional
		{
			std::vector<uint16_t> alphas;
		};

		struct Gamma : _Optional
		{
			uint32_t value;
		};

		struct Chroma : _Optional
		{
			uint32_t whitePoint_x;
			uint32_t whitePoint_y;
			uint32_t red_x;
			uint32_t red_y;
			uint32_t green_x;
			uint32_t green_y;
			uint32_t blue_x;
			uint32_t blue_y;
		};

		struct SRGB : _Optional
		{
			RenderingIntent intent;
		};

		struct ICCProfile : _Optional
		{
			char				 name[80];	// 79 + '0'
			CompressionMethod	compressionMethod;
			std::vector<uint8_t> profile;	// compressed
		};

		struct TextualData : _Optional
		{
			char		keyword[80];	// 79 + '0'
			std::string text;
		};

		struct CompressedTextualData : _Optional
		{
			char				 keyword[80];	// 79 + '0'
			CompressionMethod	compressionMethod;
			std::vector<uint8_t> compressedText;
		};

		struct InternationalTextualData : _Optional
		{
			char			  keyword[80];	// 79 + '0'
			uint8_t			  isCompressed;
			CompressionMethod compressionMethod;
			std::string		  language;

			std::string translatedKeyword;

			// text set depending on isCompressed
			std::vector<uint8_t> compressedText;
			std::string			 text;
		};

		struct BackgroundColor : _Optional
		{
			std::vector<uint16_t> values;
		};

		struct PhysicalDimensions : _Optional
		{
			uint32_t ppu_x;
			uint32_t ppu_y;
			uint8_t  isMetric;
		};

		struct SignificantBits : _Optional
		{
			std::vector<uint8_t> depths;
		};

		struct SuggestedPaletteEntry
		{
			uint16_t r;
			uint16_t g;
			uint16_t b;
			uint16_t a;
			uint16_t frequency;
		};

		struct SuggestedPalette : _Optional
		{
			char							   name[80];
			uint8_t							   sampleDepth;
			std::vector<SuggestedPaletteEntry> entries;
		};

		struct HistogramEntry
		{
			uint8_t colorIndex;
			uint8_t frequency;
		};

		struct PaletteHistogram : _Optional
		{
			std::vector<HistogramEntry> entries;
		};

		struct ModificationTime
		{
			uint16_t year;
			uint8_t  month;		//(1-12)
			uint8_t  day;		//(1-31)
			uint8_t  hour;		//(0-23)
			uint8_t  minute;	//(0-59)
			uint8_t  second;	//(0-60)
		};

		// APNG
		struct AnimationControl : _Optional
		{
			uint32_t num_frames;
			unit32_t num_loops;
		};

		struct FrameControl	// mandatory for fdAT
		{
			uint32_t			  sequence_number;	// Sequence number of the animation chunk, starting from 0
			uint32_t			  width;			  // Width of the following frame
			uint32_t			  height;			  // Height of the following frame
			uint32_t			  x_offset;			  // X position at which to render the following frame
			uint32_t			  y_offset;			  // Y position at which to render the following frame
			uint16_t			  delay_num;		  // Frame delay fraction numerator
			uint16_t			  delay_den;		  // Frame delay fraction denominator
			AnimationFrameDisposeOperation dispose_op;	// Type of frame area disposal to be done after rendering this frame
			AnimationFrameBlendOperation   blend_op;		 // Type of frame area rendering for this frame
		};

		struct ImageFrameData
		{
			uint32_t			 sequence_number;	// not used for single images
			std::vector<uint8_t> imagedata;			 // uncompressed imagedata
		};

		//-------------------------------------------------------------------------
		//! intermediate container

		struct DecoderInfo
		{
			// IHDR
			uint32_t		  width;
			uint32_t		  height;
			uint8_t			  bitdepth;
			ColorType		  colorType;
			CompressionMethod compressionMethod;
			FilterMethod	  filterMethod;
			InterlaceMethod   interlaceMethod;

			// PLTE
			Palette palette;

			// tRNS
			Transparency transparency;

			// gAMA
			Gamma gamma;

			// cHRM
			Chroma chroma;

			// sRGB
			SRGB srgb;

			// iCCP
			ICCProfile iccProfile;

			// tEXt
			std::vector<TextualData> texts;

			// zTXt
			std::vector<CompressedTextualData> compressedTexts;

			// iTXt
			std::vector<InternationalTextualData> internationalTexts;

			// bKGD
			BackgroundColor backgroundColor;

			// pHYS
			PhysicalDimensions physicalDimensions;

			// sPLT
			std::vector<SuggestedPalette> suggestedPalettes;

			// hIST
			Histogram histogram;

			// tIME
			ModificationTime lastModificationTime;

			// acTL (APNG)
			AnimationControl animationControl;

			// IDAT and fdAT
			std::vector<ImageFrameData> frames;
		};

		//-------------------------------------------------------------------------
		//! final data structures
		
		struct Frame
		{
			float				 duration;	 // seconds
			std::vector<uint8_t> imagedata;	// rgba image data, i.e. interpreted
		};

		struct Document
		{
			uint32_t width;
			uint32_t height;

			// all frames have the same size at this point
			std::vector<Frame> frames;
		};

	}	// namespace png

	using PNGFrame	= png::Frame;
	using PNGDocument = png::Document;
}	// namespace xng


#endif	// XNG_PNG_H_INC
