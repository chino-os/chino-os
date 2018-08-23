//
// Kernel Device
//
#pragma once
#include "../Storage.hpp"
#include "../../../utils.hpp"

namespace Chino
{
	namespace Device
	{
		enum class FileAccess
		{
			Read = 0b01,
			Write = 0b10,
			ReadWrite = 0b11
		};

		MAKE_ENUM_CLASS_BITMASK_TYPE(FileAccess);

		enum class FileMode
		{
			OpenExisting,
			CreateNew,
			CreateAlways,
			OpenAlways,
			Append
		};

		struct File : public Object
		{
			virtual size_t Read(BufferList<uint8_t> bufferList) = 0;
			virtual void Write(BufferList<const uint8_t> bufferList) = 0;
			virtual fpos_t GetPosition() = 0;
			virtual void SetPosition(fpos_t position) = 0;
			virtual uint64_t GetSize() = 0;
			virtual void Flush() = 0;
		};

		struct FileSystem : public Object
		{

		};

		class FileSystemManager
		{
		public:
			FileSystemManager();

			ObjectPtr<FileSystem> GetFileSystem(size_t index) noexcept { return fileSystems_.at(index); }
			void Mount(std::string_view name, ObjectAccessor<SDStorage>&& storage);

			ObjectPtr<File> OpenFile(std::string_view fileName, FileAccess fileAccess, FileMode fileMode = FileMode::OpenExisting);
		private:
			std::vector<ObjectPtr<FileSystem>> fileSystems_;
		};
	}
}

extern StaticHolder<Chino::Device::FileSystemManager> g_FileSystemMgr;
