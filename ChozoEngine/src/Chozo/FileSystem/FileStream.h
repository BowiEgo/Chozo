#pragma once

#include "StreamWriter.h"
#include "StreamReader.h"
#include "Chozo/Core/Buffer.h"

#include <filesystem>
#include <fstream>

namespace Chozo
{
	//==============================================================================
	/// FileStreamWriter
	class FileStreamWriter : public StreamWriter
	{
	public:
		FileStreamWriter(const fs::path& path);
		FileStreamWriter(const FileStreamWriter&) = delete;
		virtual ~FileStreamWriter();

		bool IsStreamGood() const final { return m_Stream.good(); }
		uint64_t GetStreamPosition() final { return m_Stream.tellp(); }
		void SetStreamPosition(uint64_t position) final { m_Stream.seekp(position); }
		bool WriteData(const char* data, size_t size) final;

	private:
		fs::path m_Path;
		std::ofstream m_Stream;
	};

	//==============================================================================
	/// FileStreamReader
	class FileStreamReader : public StreamReader
	{
	public:
		FileStreamReader(const fs::path& path);
		FileStreamReader(const FileStreamReader&) = delete;
		~FileStreamReader();

		bool IsStreamGood() const final { return m_Stream.good(); }
		uint64_t GetStreamPosition() override { return m_Stream.tellg(); }
		void SetStreamPosition(uint64_t position) override { m_Stream.seekg(position); }
		bool ReadData(char* destination, size_t size) override;

	private:
		fs::path m_Path;
		std::ifstream m_Stream;
	};

} // namespace Hazel
