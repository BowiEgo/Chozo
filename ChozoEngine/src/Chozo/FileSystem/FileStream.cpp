#include "czpch.h"
#include "FileStream.h"

namespace Chozo
{
	//==============================================================================
	// FileStreamWriter
	FileStreamWriter::FileStreamWriter(const fs::path& path)
		: m_Path(path)
	{
		m_Stream = std::ofstream(path, std::ifstream::out | std::ifstream::binary);
	}

	FileStreamWriter::~FileStreamWriter()
	{
		m_Stream.close();
	}

	bool FileStreamWriter::WriteData(const char* data, const size_t size)
	{
		m_Stream.write(data, size);
		return true;
	}

	//==============================================================================
	// FileStreamReader
	FileStreamReader::FileStreamReader(const fs::path& path)
		: m_Path(path)
	{
		m_Stream = std::ifstream(path, std::ifstream::in | std::ifstream::binary);
	}

	FileStreamReader::~FileStreamReader()
	{
		m_Stream.close();
	}

    uint64_t FileStreamReader::GetFileSize()
    {
        m_Stream.seekg(0, std::ios::end);
		auto size = m_Stream.tellg();
		m_Stream.seekg(0, std::ios::beg);

		return size;
    }

    bool FileStreamReader::ReadData(char *destination, const size_t size)
    {
		m_Stream.read(destination, size);
		return true;
	}

    bool FileStreamReader::ReadBinary(std::vector<u_int32_t>& destination)
    {
		const auto size = GetFileSize();

		destination.resize(size / sizeof(uint32_t));
		m_Stream.read((char*)destination.data(), size);
        return true;
    }

} // namespace Chozo
