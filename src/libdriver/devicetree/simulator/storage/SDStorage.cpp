//
// Kernel Device
//
#include "SDStorage.hpp"
#include <kernel/device/DeviceManager.hpp>
#include <kernel/device/storage/Storage.hpp>
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <vhdf.hpp>

using namespace Chino;
using namespace Chino::Device;

DEFINE_FDT_DRIVER_DESC_1(SDStorageDriver, "storage", "simulator,sd-storage");

class VHDStorage : public SDStorage, public ExclusiveObjectAccess
{
public:
    VHDStorage(const FDTDevice& fdt)
        : fdt_(fdt), disk_(0)
    {
        g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
    }

protected:
    virtual void OnFirstOpen() override
    {
        blocksCount_ = fdt_.GetProperty("size_in_mb")->GetUInt32(0) * 1024 * 1024 / vhdf::BLOCK_SIZE;
        disk_ = vhdf::openDisk(fdt_.GetProperty("filename")->GetString().data(), blocksCount_ * vhdf::BLOCK_SIZE, true);
        kassert(disk_ != -1);
    }

    virtual void OnLastClose() override
    {
        vhdf::closeDisk(disk_);
        disk_ = 0;
    }

    virtual size_t GetReadWriteBlockSize() override
    {
        return vhdf::BLOCK_SIZE;
    }

    virtual size_t GetEraseSectorSize() override
    {
        return vhdf::BLOCK_SIZE;
    }

    virtual size_t GetBlocksCount() override
    {
        return blocksCount_;
    }

    virtual uint64_t GetSize() override
    {
        return GetReadWriteBlockSize() * GetBlocksCount();
    }

    virtual void ReadBlocks(size_t startBlock, size_t blocksCount, BufferList<uint8_t> bufferList)
    {
        auto toRead = bufferList.Select();
        while (!toRead.IsEmpty())
        {
            uint8_t buffer[vhdf::BLOCK_SIZE];
            auto blockToRead = toRead.Take(vhdf::BLOCK_SIZE);
            toRead = toRead.Skip(vhdf::BLOCK_SIZE);
            kassert(vhdf::readBlock(disk_, startBlock, buffer) == 0);
            blockToRead.CopyFrom<const uint8_t>(buffer);
            startBlock++;
        }
    }

    virtual void WriteBlocks(size_t startBlock, size_t blocksCount, BufferList<const uint8_t> bufferList)
    {
        auto toWrite = bufferList.Select();
        while (!toWrite.IsEmpty())
        {
            uint8_t buffer[vhdf::BLOCK_SIZE];
            auto blockToWrite = toWrite.Take(vhdf::BLOCK_SIZE);
            toWrite = toWrite.Skip(vhdf::BLOCK_SIZE);
            blockToWrite.CopyTo<uint8_t>(buffer);
            kassert(vhdf::writeBlock(disk_, startBlock, buffer) == 0);
            startBlock++;
        }
    }

private:
private:
    const FDTDevice& fdt_;
    int disk_;
    size_t blocksCount_;
};

SDStorageDriver::SDStorageDriver(const FDTDevice& device)
    : device_(device)
{
}

void SDStorageDriver::Install()
{
    g_DeviceMgr->InstallDevice(MakeObject<VHDStorage>(device_));
}
