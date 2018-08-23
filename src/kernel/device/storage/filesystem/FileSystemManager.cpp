//
// Kernel Device
//
#include "FileSystemManager.hpp"
#include <kernel/kdebug.hpp>
#include <libbsp/bsp.hpp>
#include <kernel/threading/ThreadSynchronizer.hpp>
#include <ff.h>
#include <diskio.h>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Threading;

static void CheckFatFsError(FRESULT result)
{
	static const char* err_str[] =
	{
		"(0) Succeeded",
		"(1) A hard error occurred in the low level disk I/O layer",
		"(2) Assertion failed",
		"(3) The physical drive cannot work",
		"(4) Could not find the file",
		"(5) Could not find the path",
		"(6) The path name format is invalid",
		"(7) Access denied due to prohibited access or directory full",
		"(8) Access denied due to prohibited access",
		"(9) The file/directory object is invalid",
		"(10) The physical drive is write protected",
		"(11) The logical drive number is invalid",
		"(12) The volume has no work area",
		"(13) There is no valid FAT volume",
		"(14) The f_mkfs() aborted due to any problem",
		"(15) Could not get a grant to access the volume within defined period",
		"(16) The operation is rejected according to the file sharing policy",
		"(17) LFN working buffer could not be allocated",
		"(18) Number of open files > FF_FS_LOCK",
		"(19) Given parameter is invalid"
	};

	if (result != FR_OK)
		throw std::runtime_error(err_str[result]);
}

namespace
{
	class FatFileSystem final : public FileSystem
	{
	public:
		FATFS FatFS;

		FatFileSystem(ObjectAccessor<SDStorage>&& storage)
			:storage_(std::move(storage))
		{

		}

		ObjectAccessor<SDStorage>& GetStorage() noexcept { return storage_; }
	private:
		ObjectAccessor<SDStorage> storage_;
	};

	class FatFile final : public File
	{
	public:
		FatFile(std::string_view fileName, FileAccess fileAccess, FileMode fileMode)
			:file_{}
		{
			BYTE mode = 0;
			if ((fileAccess & FileAccess::Read) == FileAccess::Read)
				mode |= FA_READ;
			if ((fileAccess & FileAccess::Write) == FileAccess::Write)
				mode |= FA_WRITE;
			if (fileMode == FileMode::OpenExisting)
				mode |= FA_OPEN_EXISTING;
			else if (fileMode == FileMode::OpenAlways)
				mode |= FA_OPEN_ALWAYS;
			else if (fileMode == FileMode::CreateNew)
				mode |= FA_CREATE_NEW;
			else if (fileMode == FileMode::CreateAlways)
				mode |= FA_CREATE_ALWAYS;
			else if (fileMode == FileMode::Append)
				mode |= FA_OPEN_APPEND;
			CheckFatFsError(f_open(&file_, fileName.data(), mode));
		}

		~FatFile()
		{
			f_close(&file_);
			file_ = {};
		}

		virtual size_t Read(BufferList<uint8_t> bufferList) override
		{
			size_t totalRead = 0;

			for (auto& buffer : bufferList.Buffers)
			{
				UINT read = buffer.size();
				CheckFatFsError(f_read(&file_, buffer.data(), read, &read));
				if (!read)break;
				totalRead += read;
			}

			return totalRead;
		}

		virtual void Write(BufferList<const uint8_t> bufferList) override
		{
			for (auto& buffer : bufferList.Buffers)
			{
				UINT written = buffer.size();
				CheckFatFsError(f_write(&file_, buffer.data(), written, &written));
				if (written != buffer.size())
					throw std::runtime_error("Disk full.");
			}
		}

		virtual fpos_t GetPosition() override
		{
			return f_tell(&file_);
		}

		virtual void SetPosition(fpos_t position) override
		{
			f_lseek(&file_, position);
		}

		virtual uint64_t GetSize() override
		{
			return f_size(&file_);
		}

		virtual void Flush() override
		{
			CheckFatFsError(f_sync(&file_));
		}
	private:
		FIL file_;
	};
}

extern "C"
{
	DSTATUS disk_initialize(BYTE pdrv)
	{
		g_FileSystemMgr->GetFileSystem(pdrv);
		return RES_OK;
	}

	DSTATUS disk_status(
		BYTE pdrv		/* Physical drive nmuber to identify the drive */
	)
	{
		g_FileSystemMgr->GetFileSystem(pdrv);
		return RES_OK;
	}

	DRESULT disk_read(
		BYTE pdrv,		/* Physical drive nmuber to identify the drive */
		BYTE *buff,		/* Data buffer to store read data */
		DWORD sector,	/* Start sector in LBA */
		UINT count		/* Number of sectors to read */
	)
	{
		auto fs = g_FileSystemMgr->GetFileSystem(pdrv).As<FatFileSystem>();
		auto& st = fs->GetStorage();

		gsl::span<uint8_t> buffers[] = { {buff, ptrdiff_t(st->GetReadWriteBlockSize() * count)} };
		st->ReadBlocks(sector, count, { buffers });
		return RES_OK;
	}

	DRESULT disk_write(
		BYTE pdrv,			/* Physical drive nmuber to identify the drive */
		const BYTE *buff,	/* Data to be written */
		DWORD sector,		/* Start sector in LBA */
		UINT count			/* Number of sectors to write */
	)
	{
		auto fs = g_FileSystemMgr->GetFileSystem(pdrv).As<FatFileSystem>();
		auto& st = fs->GetStorage();

		gsl::span<const uint8_t> buffers[] = { {buff, ptrdiff_t(st->GetReadWriteBlockSize() * count)} };
		st->WriteBlocks(sector, count, { buffers });
		return RES_OK;
	}

	DRESULT disk_ioctl(
		BYTE pdrv,		/* Physical drive nmuber (0..) */
		BYTE cmd,		/* Control code */
		void *buff		/* Buffer to send/receive control data */
	)
	{
		auto fs = g_FileSystemMgr->GetFileSystem(pdrv).As<FatFileSystem>();
		auto& st = fs->GetStorage();

		switch (cmd)
		{
		case CTRL_SYNC:
			break;
		case GET_SECTOR_COUNT:
			*(DWORD *)buff = st->GetBlocksCount();
			break;
		case GET_SECTOR_SIZE:
			*(DWORD *)buff = st->GetReadWriteBlockSize();
			break;
		case GET_BLOCK_SIZE:
			*(DWORD *)buff = st->GetEraseSectorSize();
			break;
		default:
			return RES_PARERR;
		}

		return RES_OK;
	}

	DWORD get_fattime(void)
	{
		return 0;
	}
}

FileSystemManager::FileSystemManager()
{

}

void FileSystemManager::Mount(std::string_view name, ObjectAccessor<SDStorage>&& storage)
{
	auto fs = MakeObject<FatFileSystem>(std::move(storage));
	fileSystems_.emplace_back(fs);
	CheckFatFsError(f_mount(&fs->FatFS, name.data(), 1));
}

ObjectPtr<File> FileSystemManager::OpenFile(std::string_view fileName, FileAccess fileAccess, FileMode fileMode)
{
	return MakeObject<FatFile>(fileName, fileAccess, fileMode);
}
