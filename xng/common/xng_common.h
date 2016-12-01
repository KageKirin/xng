#ifndef XNG_COMMON_H_INC
#define XNG_COMMON_H_INC

#include "xng/xng.h"

#include <cctype>
#include <vector>

namespace xng
{
	namespace common
	{
		struct _Optional
		{
			bool isDefined = false;
		};


		typedef int (*deflatefunc_t)((unsigned char**, size_t*, const unsigned char*, void* settings);
		typedef int (*inflatefunc_t)((unsigned char**, size_t*, const unsigned char*, void* settings);
		
		//! deflate
		//! compresses the input data, returns the compressed buffer
		//! param[in] data: uncompressed data
		//! param[in] deflatefunc: deflate function (e.g. wrapping zlib)
		//! param[in] settings: settings for deflate function
		//! returns compressed data
		std::vector<uint8_t> deflate(const std::vector<uint8_t>& data, deflatefunc_t deflatefunc, void* settings);


		//! inflate
		//! decompresses the input data, returns the decompressed buffer
		//! param[in] data: compressed data
		//! param[in] inflatefunc: inflate function (e.g. wrapping zlib)
		//! param[in] settings: settings for inflate function
		//! returns decompressed data
		std::vector<uint8_t> inflate(const std::vector<uint8_t>& data, inflatefunc_t inflatefunc, void* settings);

	}	// namespace common
}	// namespace xng


#endif	// XNG_COMMON_H_INC
