#include "xng.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <vector>

// cheap endianess swapping
#if defined(__APPLE__) || defined(_WIN32) || defined(__EMSCRIPTEN__)
#define XNG_LITTLE_ENDIAN 1
#else
#include <endian.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define XNG_LITTLE_ENDIAN 1
#elif __BYTE_ORDER == __BIG_ENDIAN
#define XNG_LITTLE_ENDIAN 0
#endif __BYTE_ORDER
#endif	// defined(__APPLE__) || defined(_WIN32)

namespace xng
{
	///////////////////////////////////////////////////////////////////////////
	//! cheap endian swap

	inline int16_t endian_swap(const int16_t v)
	{
		const uint8_t* vp = reinterpret_cast<const uint8_t*>(&v);
		return int16_t(vp[0] << 8 | vp[1]);
	}

	inline uint16_t endian_swap(const uint16_t v)
	{
		const uint8_t* vp = reinterpret_cast<const uint8_t*>(&v);
		return uint16_t(vp[0] << 8 | vp[1]);
	}

	inline int32_t endian_swap(const int32_t v)
	{
		const uint8_t* vp = reinterpret_cast<const uint8_t*>(&v);
		return int32_t((vp[0] << 24) | (vp[1] << 16) | (vp[2] << 8) | vp[3]);
	}

	inline uint32_t endian_swap(const uint32_t v)
	{
		const uint8_t* vp = reinterpret_cast<const uint8_t*>(&v);
		return uint32_t((vp[0] << 24) | (vp[1] << 16) | (vp[2] << 8) | vp[3]);
	}


	///////////////////////////////////////////////////////////////////////////
	//! functions to read from file data (internally handling endianess)

	int8_t read_int8_t(const uint8_t* filedata, const uint8_t** next_filedata)
	{
		int8_t val = *reinterpret_cast<const int8_t*>(filedata);
		if (next_filedata)
		{
			*next_filedata = filedata + sizeof(val);
		}

		return val;
	}

	uint8_t read_uint8_t(const uint8_t* filedata, const uint8_t** next_filedata)
	{
		uint8_t val = *reinterpret_cast<const uint8_t*>(filedata);
		if (next_filedata)
		{
			*next_filedata = filedata + sizeof(val);
		}

		return val;
	}

	int16_t read_int16_t(const uint8_t* filedata, const uint8_t** next_filedata)
	{
		int16_t val = *reinterpret_cast<const int16_t*>(filedata);
		if (next_filedata)
		{
			*next_filedata = filedata + sizeof(val);
		}

#if XNG_LITTLE_ENDIAN
		return endian_swap(val);
#else
		return val;
#endif	// XNG_LITTLE_ENDIAN
	}

	uint16_t read_uint16_t(const uint8_t* filedata, const uint8_t** next_filedata)
	{
		uint16_t val = *reinterpret_cast<const uint16_t*>(filedata);
		if (next_filedata)
		{
			*next_filedata = filedata + sizeof(val);
		}

#if XNG_LITTLE_ENDIAN
		return endian_swap(val);
#else
		return val;
#endif	// XNG_LITTLE_ENDIAN
	}

	int32_t read_int32_t(const uint8_t* filedata, const uint8_t** next_filedata)
	{
		int32_t val = *reinterpret_cast<const int32_t*>(filedata);
		if (next_filedata)
		{
			*next_filedata = filedata + sizeof(val);
		}

#if XNG_LITTLE_ENDIAN
		return endian_swap(val);
#else
		return val;
#endif	// XNG_LITTLE_ENDIAN
	}

	uint32_t read_uint32_t(const uint8_t* filedata, const uint8_t** next_filedata)
	{
		uint32_t val = *reinterpret_cast<const uint32_t*>(filedata);
		if (next_filedata)
		{
			*next_filedata = filedata + sizeof(val);
		}

#if XNG_LITTLE_ENDIAN
		return endian_swap(val);
#else
		return val;
#endif	// XNG_LITTLE_ENDIAN
	}

	///////////////////////////////////////////////////////////////////////////
	//! read a chunkid from filedata

	chunkid_t read_chunkid_t(const uint8_t* filedata, const uint8_t** next_filedata)
	{
		chunkid_t val;
		val._raw = *reinterpret_cast<const uint32_t*>(filedata);
		if (next_filedata)
		{
			*next_filedata = filedata + sizeof(val);
		}

		return val;
	}

	///////////////////////////////////////////////////////////////////////////
	//! read a chunk from filedata

	chunk_t read_chunk(const uint8_t* filedata, const uint8_t** next_filedata)
	{
		const uint8_t* filedata_iter = filedata;
		chunk_t		   chunk;

		chunk.length = read_uint32_t(filedata_iter, &filedata_iter);
		chunk.id	 = read_chunkid_t(filedata_iter, &filedata_iter);
		chunk.data   = std::vector<uint8_t>(filedata_iter, filedata_iter + chunk.length);
		filedata_iter += chunk.length;

		chunk.crc = read_uint32_t(filedata_iter, &filedata_iter);

		if (next_filedata)
		{
			*next_filedata = filedata_iter;
		}

		return chunk;
	}

	///////////////////////////////////////////////////////////////////////////
	//! read all chunks from filedata

	std::vector<chunk_t> read_chunks(const uint8_t* filedata, size_t filedata_size)
	{
		const uint8_t*		 filedata_iter = filedata;
		size_t				 sum_read	  = 0;
		std::vector<chunk_t> chunks;
		chunks.reserve(filedata_size / (chunkheader_size + sizeof(uint32_t)));

		// files can have padding (e.g. after IEND). TODO: handle falsely created chunks
		while (sum_read < filedata_size)
		{
			auto chunk = read_chunk(filedata_iter, &filedata_iter);

			if (chunk.id.type[0] == 0 && chunk.id.type[1] == 0 && chunk.id.type[2] == 0 && chunk.id.type[3] == 0)
			{
				break;
			}

			sum_read += chunk.length;
			chunks.push_back(std::move(chunk));
		}

		return chunks;
	}

	///////////////////////////////////////////////////////////////////////////
	//! check_chunk

	bool check_chunk(const chunk_t& chunk, crc32computationfunc_t crc32func)
	{
		assert(crc32func);
		if (!crc32func)
		{
			return false;
		}

		std::vector<uint8_t> chunkdata;
		chunkdata.reserve(chunk.data.size() + sizeof(uint32_t));

		chunkdata.push_back(chunk.id.type[0]);
		chunkdata.push_back(chunk.id.type[1]);
		chunkdata.push_back(chunk.id.type[2]);
		chunkdata.push_back(chunk.id.type[3]);
		chunkdata.insert(chunkdata.end(), chunk.data.begin(), chunk.data.end());

		uint32_t computedCrc = crc32func(chunkdata.data(), chunkdata.size());
		return chunk.crc == computedCrc;
	}

	bool check_chunks(const std::vector<chunk_t>& chunks, crc32computationfunc_t crc32func)
	{
		assert(crc32func);
		if (!crc32func)
		{
			return false;
		}

		return std::all_of(chunks.begin(), chunks.end(), [&crc32func](auto& chunk) {
			return check_chunk(chunk, crc32func);
		});
	}


	///////////////////////////////////////////////////////////////////////////
	//! chunk handling

	inline const chunkhandler_t& find_chunkhandler(const chunkid_t& id, const chunkhandlerstate_t& state)
	{
		auto itHandler = std::find_if(state.handlers.begin(), state.handlers.end(), [&id](auto& handler) {
			return handler.id._raw == id._raw;
		});

		if (itHandler != state.handlers.end())
		{
			return *itHandler;
		}

		static chunkhandler_t default_error{
		  {.type = "ERR"},
		  [](const chunk_t* chunk, void* target) {
			  assert(chunk);
			  printf("WARNING: unhandled chunk '%c%c%c%c'\n",
					 chunk->id.type[0],
					 chunk->id.type[1],
					 chunk->id.type[2],
					 chunk->id.type[3]);
			  return 0;
		  },
		};
		return default_error;
	}

	int handle_chunk(const chunk_t& chunk, const chunkhandlerstate_t& state, void* target)
	{
		auto& handler = find_chunkhandler(chunk.id, state);
		return handler.func(&chunk, target);
	}

	int handle_chunks(const std::vector<chunk_t>& chunks, const chunkhandlerstate_t& state, void* target)
	{
		int err = 0;
		for (auto& chunk : chunks)
		{
			err = handle_chunk(chunk, state, target);
			assert(err == 0);
			if (err != 0)
			{
				return err;
			}
		}

		return err;
	}


	///////////////////////////////////////////////////////////////////////////
	//! CRC32 computation and check
	//-- scavenged from lodepng

	// returns the CRC of the bytes buf[0..len-1]
	uint32_t compute_crc32(const uint8_t* data, size_t length)
	{
		// CRC polynomial: 0xedb88320
		static uint32_t crc32_table[256]
		  = {0u,		  1996959894u, 3993919788u, 2567524794u, 124634137u,  1886057615u, 3915621685u, 2657392035u,
			 249268274u,  2044508324u, 3772115230u, 2547177864u, 162941995u,  2125561021u, 3887607047u, 2428444049u,
			 498536548u,  1789927666u, 4089016648u, 2227061214u, 450548861u,  1843258603u, 4107580753u, 2211677639u,
			 325883990u,  1684777152u, 4251122042u, 2321926636u, 335633487u,  1661365465u, 4195302755u, 2366115317u,
			 997073096u,  1281953886u, 3579855332u, 2724688242u, 1006888145u, 1258607687u, 3524101629u, 2768942443u,
			 901097722u,  1119000684u, 3686517206u, 2898065728u, 853044451u,  1172266101u, 3705015759u, 2882616665u,
			 651767980u,  1373503546u, 3369554304u, 3218104598u, 565507253u,  1454621731u, 3485111705u, 3099436303u,
			 671266974u,  1594198024u, 3322730930u, 2970347812u, 795835527u,  1483230225u, 3244367275u, 3060149565u,
			 1994146192u, 31158534u,   2563907772u, 4023717930u, 1907459465u, 112637215u,  2680153253u, 3904427059u,
			 2013776290u, 251722036u,  2517215374u, 3775830040u, 2137656763u, 141376813u,  2439277719u, 3865271297u,
			 1802195444u, 476864866u,  2238001368u, 4066508878u, 1812370925u, 453092731u,  2181625025u, 4111451223u,
			 1706088902u, 314042704u,  2344532202u, 4240017532u, 1658658271u, 366619977u,  2362670323u, 4224994405u,
			 1303535960u, 984961486u,  2747007092u, 3569037538u, 1256170817u, 1037604311u, 2765210733u, 3554079995u,
			 1131014506u, 879679996u,  2909243462u, 3663771856u, 1141124467u, 855842277u,  2852801631u, 3708648649u,
			 1342533948u, 654459306u,  3188396048u, 3373015174u, 1466479909u, 544179635u,  3110523913u, 3462522015u,
			 1591671054u, 702138776u,  2966460450u, 3352799412u, 1504918807u, 783551873u,  3082640443u, 3233442989u,
			 3988292384u, 2596254646u, 62317068u,   1957810842u, 3939845945u, 2647816111u, 81470997u,   1943803523u,
			 3814918930u, 2489596804u, 225274430u,  2053790376u, 3826175755u, 2466906013u, 167816743u,  2097651377u,
			 4027552580u, 2265490386u, 503444072u,  1762050814u, 4150417245u, 2154129355u, 426522225u,  1852507879u,
			 4275313526u, 2312317920u, 282753626u,  1742555852u, 4189708143u, 2394877945u, 397917763u,  1622183637u,
			 3604390888u, 2714866558u, 953729732u,  1340076626u, 3518719985u, 2797360999u, 1068828381u, 1219638859u,
			 3624741850u, 2936675148u, 906185462u,  1090812512u, 3747672003u, 2825379669u, 829329135u,  1181335161u,
			 3412177804u, 3160834842u, 628085408u,  1382605366u, 3423369109u, 3138078467u, 570562233u,  1426400815u,
			 3317316542u, 2998733608u, 733239954u,  1555261956u, 3268935591u, 3050360625u, 752459403u,  1541320221u,
			 2607071920u, 3965973030u, 1969922972u, 40735498u,   2617837225u, 3943577151u, 1913087877u, 83908371u,
			 2512341634u, 3803740692u, 2075208622u, 213261112u,  2463272603u, 3855990285u, 2094854071u, 198958881u,
			 2262029012u, 4057260610u, 1759359992u, 534414190u,  2176718541u, 4139329115u, 1873836001u, 414664567u,
			 2282248934u, 4279200368u, 1711684554u, 285281116u,  2405801727u, 4167216745u, 1634467795u, 376229701u,
			 2685067896u, 3608007406u, 1308918612u, 956543938u,  2808555105u, 3495958263u, 1231636301u, 1047427035u,
			 2932959818u, 3654703836u, 1088359270u, 936918000u,  2847714899u, 3736837829u, 1202900863u, 817233897u,
			 3183342108u, 3401237130u, 1404277552u, 615818150u,  3134207493u, 3453421203u, 1423857449u, 601450431u,
			 3009837614u, 3294710456u, 1567103746u, 711928724u,  3020668471u, 3272380065u, 1510334235u, 755167117u};

		unsigned r = 0xffffffffu;
		size_t   i;
		for (i = 0; i < length; ++i)
		{
			r = crc32_table[(r ^ data[i]) & 0xff] ^ (r >> 8);
		}
		return r ^ 0xffffffffu;
	}

	///////////////////////////////////////////////////////////////////////////

}	// namespace xng

///////////////////////////////////////////////////////////////////////////////
// C API

xng_chunk_t xng_get_next_chunk(const uint8_t* data, const uint8_t** next_data)
{
	const uint8_t* data_iter = data;
	xng_chunk_t	chunk;
	chunk.length = xng::read_uint32_t(data_iter, &data_iter);
	chunk.id._raw = xng::read_chunkid_t(data_iter, &data_iter)._raw;
	chunk.data	= data_iter;
	data_iter += chunk.length;
	chunk.crc = xng::read_uint32_t(data_iter, &data_iter);

	if(next_data)
	{
		*next_data = data_iter;
	}

	return chunk;
}

size_t xng_iterate_chunks(const uint8_t* data, size_t length, xng_chunk_iteration_func_t xng_chunk_iterator, void* context)
{
	size_t iter_length = 0;
	size_t	chunk_count = 0;
	const uint8_t* data_iter   = data;

	assert(length >= xng_chunkheader_min_size);

	while(iter_length < length)
	{
		xng_chunk_t chunk = xng_get_next_chunk(data_iter, &data_iter);

		if (chunk.id.type[0] == 0 && chunk.id.type[1] == 0 && chunk.id.type[2] == 0 && chunk.id.type[3] == 0)
		{
			break;
		}

		iter_length += xng_chunkheader_min_size + chunk.length;
		++chunk_count;

		if(xng_chunk_iterator)
		{
			xng_chunk_iterator(&chunk, context);
		}
	}

	return chunk_count;
}

bool xng_check_chunk_crc(const xng_chunk_t* chunk, xng_crc32_computation_func_t crc32)
{
	assert(chunk);
	assert(chunk->data);
	return chunk->crc == crc32(chunk->data - sizeof(uint32_t), chunk->length + sizeof(uint32_t));
}


///////////////////////////////////////////////////////////////////////////////