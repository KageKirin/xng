#include "xng/xng.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <memory>

int main(int argc, char** argv)
{
	assert(argc >= 2);
	std::shared_ptr<FILE> file(fopen(argv[1], "rb"), fclose);
	assert(file);

	if (fseek(file.get(), 0, SEEK_END) != 0)
	{
		return -1;
	}

	size_t size = ftell(file.get());
	rewind(file.get());

	assert(size > 8);
	uint8_t signature[8];
	fread(signature, 1, 8, file.get());

	std::vector<uint8_t> filedata(size - 8);
	fread(&filedata[0], size - 8, 1, file.get());

	auto chunks = xng::read_chunks(filedata.data(), filedata.size());

	printf("read %i chunks\n", chunks.size());
	std::for_each(chunks.begin(), chunks.end(), [](auto& chunk) {
		printf("\t'%c%c%c%c': length: %i, crc: 0x%x\n",
			   chunk.id.type[0],
			   chunk.id.type[1],
			   chunk.id.type[2],
			   chunk.id.type[3],
			   chunk.length,
			   chunk.crc);
	});

	bool crcCorrect = check_chunks(chunks);
	printf("chunks are CRC %s\n", crcCorrect ? "correct" : "incorrect");


	xng::chunkhandlerstate_t state = {{
	  xng::chunkhandler_t{{.type = {'I', 'H', 'D', 'R'}},
						  [](const xng::chunk_t* chunk, void* target) {
							  printf("IHDR\n");
							  return 0;
						  }},
	  xng::chunkhandler_t{{.type = {'I', 'E', 'N', 'D'}},
						  [](const xng::chunk_t* chunk, void* target) {
							  printf("IEND\n");
							  return 0;
						  }},
	  xng::chunkhandler_t{{.type = {'I', 'D', 'A', 'T'}},
						  [](const xng::chunk_t* chunk, void* target) {
							  printf("IDAT\n");
							  return 0;
						  }},
	  xng::chunkhandler_t{{.type = {'a', 'c', 'T', 'L'}},
						  [](const xng::chunk_t* chunk, void* target) {
							  printf("acTL\n");
							  return 0;
						  }},
	  xng::chunkhandler_t{{.type = {'f', 'c', 'T', 'L'}},
						  [](const xng::chunk_t* chunk, void* target) {
							  printf("fcTL\n");
							  return 0;
						  }},
	  xng::chunkhandler_t{{.type = {'f', 'd', 'A', 'T'}},
						  [](const xng::chunk_t* chunk, void* target) {
							  printf("fdAT\n");
							  return 0;
						  }},
	}};

	int err = xng::handle_chunks(chunks, state, nullptr);


	//C-API test
	printf("//C - API test\n");
	size_t chunk_count = xng_iterate_chunks(filedata.data(),
											filedata.size(),	
											[](const xng_chunk_t* chunk, void* context) {
												printf("\t'%c%c%c%c': length: %i, crc: 0x%x %s\n",
													   chunk->id.type[0],
													   chunk->id.type[1],
													   chunk->id.type[2],
													   chunk->id.type[3],
													   chunk->length,
													   chunk->crc,
													   xng_check_chunk_crc(chunk, xng::compute_crc32) ? "valid" : "invalid");
												return 0;
											},
											(void*)nullptr);
	printf("iterated over %i chunks\n", chunk_count);

	return 0;
}
