//
// Kernel Device
//
#include "adxl345.hpp"
#include <kernel/device/sensor/Accelerometer.hpp>
#include <kernel/device/io/I2c.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <libbsp/bsp.hpp>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Threading;

DEFINE_FDT_DRIVER_DESC_1(ADXL345Driver, "accelerometer", "adi,adxl345");

#define ADXL345DeviceId 0xE5

enum Register
{
	DEVID = 0x00,			//!< Device ID
	THRESH_TAP = 0x1D,		//!< Tap threshold
	OFSX = 0x1E,			//!< X-axis offset
	OFSY = 0x1F,			//!< Y-axis offset
	OFSZ = 0x20,			//!< Z-axis offset
	DUR = 0x21,				//!< Tap duration
	Latent = 0x22,			//!< Tap latency
	Window = 0x23,			//!< Tap window
	THRESH_ACT = 0x24,		//!< Activity threshold
	THRESH_INACT = 0x25,	//!< Inactivity threshold
	TIME_INACT = 0x26,		//!< Inactivity time
	ACT_INACT_CTL = 0x27,	//!< Axis enable control for activity and inactivity detection
	THRESH_FF = 0x28,		//!< Free-fall threshold
	TIME_FF = 0x29,			//!< Free-fall time
	TAP_AXES = 0x2A,		//!< Axis control for single tap/double tap
	ACT_TAP_STATUS = 0x2B,	//!< Source of single tap/double tap
	BW_RATE = 0x2C,			//!< Data rate and power mode control
	POWER_CTL = 0x2D,		//!< Power-saving features control
	INT_ENABLE = 0x2E,		//!< Interrupt enable control
	INT_MAP = 0x2F,			//!< Interrupt mapping control
	INT_SOURCE = 0x30,		//!< Source of interrupts
	DATA_FORMAT = 0x31,		//!< Data format control
	DATAX0 = 0x32,			//!< X-Axis Data 0
	DATAX1 = 0x33,			//!< X-Axis Data 1
	DATAY0 = 0x34,			//!< Y-Axis Data 0
	DATAY1 = 0x35,			//!< Y-Axis Data 1
	DATAZ0 = 0x36,			//!< Z-Axis Data 0
	DATAZ1 = 0x37,			//!< Z-Axis Data 1
	FIFO_CTL = 0x38,		//!< FIFO control
	FIFO_STATUS = 0x39,		//!< FIFO status
};

class ADXL345Device : public Accelerometer, public ExclusiveObjectAccess
{
public:
	ADXL345Device(const FDTDevice& fdt)
		:fdt_(fdt)
	{
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
	}

	virtual AccelerometerReading GetCurrentReading() override
	{
		uint8_t send[1] = { DATAX0 };
		uint8_t recv[6];
		gsl::span<const uint8_t> sendBuffers[] = { send };
		gsl::span<uint8_t> recvBuffers[] = { recv };

		kassert(i2cDev_->WriteRead({ sendBuffers }, { recvBuffers }) == 6);

		return 
		{ 
			FixedToFloat(recv[0], recv[1]), 
			FixedToFloat(recv[2], recv[3]),
			FixedToFloat(recv[4], recv[5])
		};
	}

	virtual void OnFirstOpen() override
	{
		auto access = OA_Read | OA_Write;

		auto i2cName = fdt_.GetProperty("i2c");
		auto slaveAddr = fdt_.GetProperty("slave_address");
		kassert(i2cName && slaveAddr);
		auto i2c = g_ObjectMgr->GetDirectory(WKD_Device).Open(i2cName->GetString(), access).MoveAs<I2cController>();
		i2cDev_ = i2c->OpenDevice(slaveAddr->GetUInt32(0), access);

		SetupDevice();
	}

	virtual void OnLastClose() override
	{
		i2cDev_.Reset();
	}
private:
	void SetupDevice()
	{
		auto devId = ReadRegister(DEVID);
		kassert(devId == ADXL345DeviceId);
		WriteRegister(DATA_FORMAT, 0x2B);
		WriteRegister(BW_RATE, 0x0A);
		WriteRegister(POWER_CTL, 0x28);
	}

	uint8_t ReadRegister(Register reg)
	{
		uint8_t send[1] = { reg };
		uint8_t recv[1];
		gsl::span<const uint8_t> sendBuffers[] = { send };
		gsl::span<uint8_t> recvBuffers[] = { recv };

		kassert(i2cDev_->WriteRead({ sendBuffers }, { recvBuffers }) == 1);
		return recv[0];
	}

	void WriteRegister(Register reg, uint8_t value)
	{
		uint8_t send[2] = { reg, value };
		gsl::span<const uint8_t> sendBuffers[] = { send };

		i2cDev_->Write({ sendBuffers });
	}

	static double FixedToFloat(uint8_t low, uint8_t high)
	{
		auto value = static_cast<int16_t>(low | (high << 8));
		return value * (9.80991 * 4e-3); // 4mg
	}
private:
	ObjectAccessor<I2cDevice> i2cDev_;
	const FDTDevice& fdt_;
};

ADXL345Driver::ADXL345Driver(const FDTDevice& device)
	:device_(device)
{

}

void ADXL345Driver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<ADXL345Device>(device_));
}
