#include "czpch.h"
#include "FileStream.h"

namespace Chozo
{
	//==============================================================================
	/// FileStreamWriter
	FileStreamWriter::FileStreamWriter(const fs::path& path)
		: m_Path(path)
	{
		m_Stream = std::ofstream(path, std::ifstream::out | std::ifstream::binary);
	}

	FileStreamWriter::~FileStreamWriter()
	{
		m_Stream.close();
	}

	bool FileStreamWriter::WriteData(const char* data, size_t size)
	{
		m_Stream.write(data, size);
		return true;
	}

	//==============================================================================
	/// FileStreamReader
	FileStreamReader::FileStreamReader(const fs::path& path)
		: m_Path(path)
	{
		m_Stream = std::ifstream(path, std::ifstream::in | std::ifstream::binary);
	}

	FileStreamReader::~FileStreamReader()
	{
		m_Stream.close();
	}

	bool FileStreamReader::ReadData(char* destination, size_t size)
	{
		m_Stream.read(destination, size);
		return true;
	}

} // namespace Chozo
