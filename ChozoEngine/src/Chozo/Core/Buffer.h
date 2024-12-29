#pragma once

#include "Chozo/Core/Base.h"

namespace Chozo {

	struct Buffer
	{
		virtual ~Buffer() = default;

		void* Data;
		uint64_t Size;

		Buffer()
			: Data(nullptr), Size(0)
		{
		}

		Buffer(const void* data, uint64_t size = 0) // NOLINT
			: Data((void*)data), Size(size)
		{
		}

		static Buffer Copy(const Buffer& other)
		{
			Buffer buffer;
			buffer.Allocate(other.Size);
			memcpy(buffer.Data, other.Data, other.Size);
			return buffer;
		}

		static Buffer Copy(const void* data, uint64_t size)
		{
			Buffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.Data, data, size);
			return buffer;
		}

		void CopyTo(Buffer& other) const
		{
			other.Allocate(Size);
			memcpy(other.Data, Data, Size);
		}

		virtual void Allocate(uint64_t size)
		{
			delete[] (byte*)Data;
			Data = nullptr;

			if (size == 0)
				return;

			Data = new byte[size]; // NOLINT
			Size = size;
		}

		virtual void Release()
		{
			delete[] (byte*)Data; // NOLINT
			Data = nullptr;
			Size = 0;
		}

		void ZeroInitialize() const
		{
			if (Data)
				memset(Data, 0, Size);
		}

		template<typename T>
		T& Read(const uint64_t offset = 0)
		{
			return *(T*)((byte*)Data + offset);
		}

		template<typename T>
		const T& Read(uint64_t offset = 0) const
		{
			return *(T*)((byte*)Data + offset);
		}

		[[nodiscard]] byte* ReadBytes(const uint64_t size, const uint64_t offset) const
		{
			CZ_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
			const auto buffer = new byte[size];
			memcpy(buffer, (byte*)Data + offset, size);
			return buffer;
		}
				
		void Write(const void* data, const uint64_t size, const uint64_t offset = 0) const
		{
			CZ_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
			memcpy((byte*)Data + offset, data, size);
		}

		explicit operator bool() const
		{
			return Data;
		}

		byte& operator[](const int index) // NOLINT
		{
			return ((byte*)Data)[index];
		}

		byte operator[](int index) const
		{
			return ((byte*)Data)[index];
		}

		template<typename T>
		T* As() const
		{
			return (T*)Data;
		}

		[[nodiscard]] uint64_t GetSize() const { return Size; }
	};

	struct SafeBuffer final : public Buffer
	{
		~SafeBuffer() override
		{
			Buffer::Release();
		}

		static SafeBuffer Copy(const void* data, const uint64_t size)
		{
			SafeBuffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.Data, data, size);
			return buffer; // NOLINT
		}
	};

	struct SharedBuffer final : public Buffer
	{
		std::shared_ptr<byte[]> SharedData;

		SharedBuffer() : Buffer() {}

		explicit SharedBuffer(const void* data, const uint64_t size = 0)
		{
			SharedBuffer::Allocate(size);
			memcpy(SharedData.get(), data, size);
		}

		void Copy(const Buffer& other)
		{
			Allocate(other.Size);
			memcpy(SharedData.get(), other.Data, other.Size);
		}

		void Allocate(const uint64_t size) override
		{
			Release();

			if (size == 0)
				return;

			SharedData.reset(new byte[size], std::default_delete<byte[]>());
			Data = SharedData.get();
			Size = size;
		}

		void Release() override
		{
			SharedData.reset();
			Data = nullptr;
			Size = 0;
		}

		SharedBuffer(const SharedBuffer& other)
			: Buffer(other)
		{
			SharedData = other.SharedData;
			Data = SharedData.get();
			Size = other.Size;
		}

		SharedBuffer& operator=(const SharedBuffer& other)
		{
			if (this != &other)
			{
				Release();
				SharedData = other.SharedData;
				Data = SharedData.get();
				Size = other.Size;
			}
			return *this;
		}
	};
}
