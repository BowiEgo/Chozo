#pragma once

#include "CoreExport.h"
#include "CoreMinimal.h"

struct FBuffer {
    virtual ~FBuffer() = default;

    void* Data;
    uint64_t Size;

    FBuffer() : Data(nullptr), Size(0) {}

    FBuffer(const void* data, uint64_t size = 0) // NOLINT
        : Data((void*)data), Size(size) {}

    static FBuffer Copy(const FBuffer& other) {
        FBuffer buffer;
        buffer.Allocate(other.Size);
        memcpy(buffer.Data, other.Data, other.Size);
        return buffer;
    }

    static FBuffer Copy(const void* data, uint64_t size) {
        FBuffer buffer;
        buffer.Allocate(size);
        memcpy(buffer.Data, data, size);
        return buffer;
    }

    void CopyTo(FBuffer& other) const {
        other.Allocate(Size);
        memcpy(other.Data, Data, Size);
    }

    virtual void Allocate(uint64_t size) {
        delete[] (FByte*)Data;
        Data = nullptr;

        if (size == 0) return;

        Data = new FByte[size]; // NOLINT
        Size = size;
    }

    virtual void Release() {
        delete[] (FByte*)Data; // NOLINT
        Data = nullptr;
        Size = 0;
    }

    void ZeroInitialize() const {
        if (Data) memset(Data, 0, Size);
    }

    template <typename T> T& Read(const uint64_t offset = 0) {
        return *(T*)((FByte*)Data + offset);
    }

    template <typename T> const T& Read(uint64_t offset = 0) const {
        return *(T*)((FByte*)Data + offset);
    }

    [[nodiscard]] FByte* ReadBytes(const uint64_t size, const uint64_t offset) const {
        CZ_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
        const auto buffer = new FByte[size];
        memcpy(buffer, (FByte*)Data + offset, size);
        return buffer;
    }

    void Write(const void* data, const uint64_t size, const uint64_t offset = 0) const {
        CZ_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
        memcpy((FByte*)Data + offset, data, size);
    }

    explicit operator bool() const { return Data; }

    FByte& operator[](const int index) // NOLINT
    {
        return ((FByte*)Data)[index];
    }

    FByte operator[](int index) const { return ((FByte*)Data)[index]; }

    template <typename T> T* As() const { return (T*)Data; }

    [[nodiscard]] uint64_t GetSize() const { return Size; }
};

struct FSafeBuffer final : public FBuffer {
    ~FSafeBuffer() override { FBuffer::Release(); }

    static FSafeBuffer Copy(const void* data, const uint64_t size) {
        FSafeBuffer buffer;
        buffer.Allocate(size);
        memcpy(buffer.Data, data, size);
        return buffer; // NOLINT
    }
};

struct FSharedBuffer final : public FBuffer {
    std::shared_ptr<FByte[]> SharedData;

    FSharedBuffer() : FBuffer() {}

    explicit FSharedBuffer(const void* data, const uint64_t size = 0) {
        FSharedBuffer::Allocate(size);
        memcpy(SharedData.get(), data, size);
    }

    void Copy(const FBuffer& other) {
        Allocate(other.Size);
        memcpy(SharedData.get(), other.Data, other.Size);
    }

    void Allocate(const uint64_t size) override {
        Release();

        if (size == 0) return;

        SharedData.reset(new FByte[size], std::default_delete<FByte[]>());
        Data = SharedData.get();
        Size = size;
    }

    void Release() override {
        SharedData.reset();
        Data = nullptr;
        Size = 0;
    }

    FSharedBuffer(const FSharedBuffer& other) : FBuffer(other) {
        SharedData = other.SharedData;
        Data = SharedData.get();
        Size = other.Size;
    }

    FSharedBuffer& operator=(const FSharedBuffer& other) {
        if (this != &other) {
            Release();
            SharedData = other.SharedData;
            Data = SharedData.get();
            Size = other.Size;
        }
        return *this;
    }
};