#ifndef XNG_JNG_H_INC
#define XNG_JNG_H_INC

#include "xng/xng.h"

#include <cctype>
#include <vector>

namespace xng
{
	namespace jng
	{

		struct Frame
		{
		};

		struct Document
		{

			std::vector<Frame> frames;
		};

	}	// namespace jng

	using JNGFrame	= jng::Frame;
	using JNGDocument = jng::Document;
}	// namespace xng


#endif	// XNG_JNG_H_INC
