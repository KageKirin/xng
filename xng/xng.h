#ifndef XNG_H_INC
#define XNG_H_INC

#include <cctype>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// C low-level interface

typedef struct xng_chunkid_t {
	union {
			uint32_t _raw;	// used for fast comparison
			char	 type[4];
		};
} xng_chunkid_t;

static const size_t xng_chunkheader_size = sizeof(xng_chunkid_t) + sizeof(uint32_t);
static const size_t xng_chunkheader_min_size = sizeof(xng_chunkid_t) + sizeof(uint32_t) + sizeof(uint32_t);
typedef struct xng_chunk_t
{
	// png/mng/jng: length, type, data, crc
	xng_chunkid_t		id;
	uint32_t			length;
	uint32_t			crc;
	const uint8_t* 		data;
} xng_chunk_t;

typedef int (*xng_chunk_iteration_func_t)(const xng_chunk_t* chunk, void* context);
size_t xng_iterate_chunks(const uint8_t* data, size_t length, xng_chunk_iteration_func_t xng_chunk_iterator, void* context);
xng_chunk_t xng_get_next_chunk(const uint8_t* data, const uint8_t** next_data);

typedef uint32_t (*xng_crc32_computation_func_t)(const uint8_t* data, size_t length);
bool xng_check_chunk_crc(const xng_chunk_t* chunk, xng_crc32_computation_func_t crc32);


#ifdef __cplusplus
}
#endif //__cplusplus


namespace xng
{
	//-------------------------------------------------------------------------
	//! structures to read/write xng chunks
	struct chunkid_t
	{
		union {
			uint32_t _raw;	// used for fast comparison
			char	 type[4];
		};
	};

	static const size_t chunkheader_size = sizeof(chunkid_t) + sizeof(uint32_t);
	struct chunk_t
	{
		// png/mng/jng: length, type, data, crc
		chunkid_t			 id;
		uint32_t			 length;
		uint32_t			 crc;
		std::vector<uint8_t> data;	// for C: use uint8_t*
	};

	//-------------------------------------------------------------------------
	//! structures to handle cng chunks
	typedef int (*chunkhandlerfunc_t)(const chunk_t* chunk, void* target);

	struct chunkhandler_t
	{
		chunkid_t		   id;
		chunkhandlerfunc_t func;
	};

	struct chunkhandlerstate_t
	{
		std::vector<chunkhandler_t> handlers;
	};

	//-------------------------------------------------------------------------
	//! functions to read from file data (internally handling endianess)
	//! if next_filedata != NULL, it will be set to the next chunk's address,
	//! or whatever comes after the current one
	//!  *next_filedata = filedata + sizeof(intX_t)
	int8_t read_int8_t(const uint8_t* filedata, const uint8_t** next_filedata);
	uint8_t read_uint8_t(const uint8_t* filedata, const uint8_t** next_filedata);
	int16_t read_int16_t(const uint8_t* filedata, const uint8_t** next_filedata);
	uint16_t read_uint16_t(const uint8_t* filedata, const uint8_t** next_filedata);
	int32_t read_int32_t(const uint8_t* filedata, const uint8_t** next_filedata);
	uint32_t read_uint32_t(const uint8_t* filedata, const uint8_t** next_filedata);

	//! read a chunkid from filedata
	//!  *next_filedata = filedata + sizeof(chunkid_t) //the latter being 4 bytes
	chunkid_t read_chunkid_t(const uint8_t* filedata, const uint8_t** next_filedata);

	//! read a chunk from filedata
	//!  *next_filedata = filedata + chunkheader_size + chunk.length + sizeof(chunk.crc)
	chunk_t read_chunk(const uint8_t* filedata, const uint8_t** next_filedata);

	//! read all chunks from filedata
	std::vector<chunk_t> read_chunks(const uint8_t* filedata, size_t filedata_size);

	//-------------------------------------------------------------------------
	//! functions to write to file data (internally handling endianess)
	//! if next_filedata != NULL, it will be set to the next chunk's address,
	//! or whatever comes after the current one
	//!  *next_filedata = filedata + sizeof(intX_t)
	size_t write_int8_t(int8_t val, uint8_t* filedata, uint8_t** next_filedata);
	size_t write_uint8_t(uint8_t val, uint8_t* filedata, uint8_t** next_filedata);
	size_t write_int16_t(int16_t val, uint8_t* filedata, uint8_t** next_filedata);
	size_t write_uint16_t(uint16_t val, uint8_t* filedata, uint8_t** next_filedata);
	size_t write_int32_t(int32_t val, uint8_t* filedata, uint8_t** next_filedata);
	size_t write_uint32_t(uint32_t val, uint8_t* filedata, uint8_t** next_filedata);

	//! read a chunkid from filedata
	//!  *next_filedata = filedata + sizeof(chunkid_t) //the latter being 4 bytes
	size_t write_chunkid_t(const chunkid_t& val, uint8_t* filedata, uint8_t** next_filedata);

	//! read a chunk from filedata
	//!  *next_filedata = filedata + chunkheader_size + chunk.length + sizeof(chunk.crc)
	size_t write_chunk(const chunk_t& val, uint8_t* filedata, uint8_t** next_filedata);

	//! read all chunks from filedata
	size_t read_chunks(const std::vector<chunk_t>& chunks, const uint8_t* filedata, size_t filedata_size);


	//-------------------------------------------------------------------------
	//! functions to check chunks for CRC correctness

	//! check chunks
	typedef uint32_t (*crc32computationfunc_t)(const uint8_t* data, size_t length);

	//! "our" (ok, lodepng's) implementation of crc32 computation
	uint32_t compute_crc32(const uint8_t* data, size_t length);

	//! check_chunk
	// crc32func is the computation function. see signature above
	//! returns true if crc is correct
	bool check_chunk(const chunk_t& chunk, crc32computationfunc_t crc32func = compute_crc32);
	bool check_chunks(const std::vector<chunk_t>& chunks, crc32computationfunc_t crc32func = compute_crc32);

	//-------------------------------------------------------------------------
	//! functions to handle (aka interprete) chunks

	int handle_chunk(const chunk_t& chunk, const chunkhandlerstate_t& state, void* target);
	int handle_chunks(const std::vector<chunk_t>& chunks, const chunkhandlerstate_t& state, void* target);

	//-------------------------------------------------------------------------
}	// namespace xng

#endif	// XNG_H_INC