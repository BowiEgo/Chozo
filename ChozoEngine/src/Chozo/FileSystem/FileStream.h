#pragma once

#include "StreamWriter.h"
#include "StreamReader.h"
#include "Chozo/Core/Buffer.h"

#include <filesystem>
#include <fstream>

namespace Chozo
{
	//==============================================================================
	// FileStreamWriter
	class FileStreamWriter : public StreamWriter // NOLINT
	{
	public:
		explicit FileStreamWriter(const fs::path& path);
		FileStreamWriter(const FileStreamWriter&) = delete;
		~FileStreamWriter() override;

		bool IsStreamGood() const final { return m_Stream.good(); }
		uint64_t GetStreamPosition() final { return m_Stream.tellp(); }
		void SetStreamPosition(const uint64_t position) final { m_Stream.seekp(position); }
		bool WriteData(const char* data, size_t size) final;

	private:
		fs::path m_Path;
		std::ofstream m_Stream;
	};

	//==============================================================================
	// FileStreamReader
	class FileStreamReader : public StreamReader // NOLINT
	{
	public:
		explicit FileStreamReader(const fs::path& path);
		FileStreamReader(const FileStreamReader&) = delete;
		~FileStreamReader() override;

		bool IsStreamGood() const final { return m_Stream.good(); }
		uint64_t GetStreamPosition() override { return m_Stream.tellg(); }
		void SetStreamPosition(const uint64_t position) override { m_Stream.seekg(position); }
		uint64_t GetFileSize() override;
		bool ReadData(char* destination, size_t size) override;
		bool ReadBinary(std::vector<u_int32_t>& destination) override;
	private:
		fs::path m_Path;
		std::ifstream m_Stream;
	};

} // namespace Hazel
