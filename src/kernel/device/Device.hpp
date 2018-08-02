//
// Kernel Device
//
#pragma once
#include "../kernel_iface.h"
#include "../object/Object.hpp"
#include "Driver.hpp"
#include <array>
#include <gsl/gsl>

namespace Chino
{
	namespace Device
	{
		enum class DeviceType
		{
			Storage,
			Partition,
			Other
		};

		enum class TransmitRole
		{
			Sender,
			Receiver
		};

		template<class T>
		struct BufferList;

		namespace details
		{
			template<class T>
			struct BufferListSelect
			{
			public:
				BufferListSelect()
					:bufferCount_(0)
				{

				}

				BufferListSelect(BufferList<T> bufferList) noexcept
					:bufferCount_(bufferList.Buffers.size())
				{
					for (size_t i = 0; i < bufferCount_; i++)
						buffers_[i] = bufferList.Buffers[i];
				}

				BufferList<T> AsBufferList() noexcept
				{
					return gsl::span<gsl::span<T>>(buffers_.data(), bufferCount_);
				}

				BufferListSelect Skip(size_t size) const noexcept
				{
					BufferListSelect result;
					size_t count = 0;
					for (size_t i = 0; i < bufferCount_; i++)
					{
						auto& buffer = buffers_[i];
						auto bufSize = size_t(buffer.size());
						auto offset = std::min(size, bufSize);
						if (offset != bufSize)
							result.buffers_[count++] = buffer.subspan(offset, bufSize - offset);
						size -= offset;
					}

					result.bufferCount_ = count;
					return result;
				}

				BufferListSelect Take(size_t size) const noexcept
				{
					BufferListSelect result;
					size_t count = 0;
					for (size_t i = 0; i < bufferCount_; i++)
					{
						auto& buffer = buffers_[i];
						auto bufSize = size_t(buffer.size());
						auto offset = std::min(size, bufSize);
						result.buffers_[count++] = buffer.subspan(0, offset);
						size -= offset;
						if (!size)break;
					}

					result.bufferCount_ = count;
					return result;
				}

				size_t Count() const noexcept
				{
					size_t result = 0;
					for (size_t i = 0; i < bufferCount_; i++)
					{
						auto& buffer = buffers_[i];
						result += buffer.size();
					}

					return result;
				}

				bool IsEmpty() const noexcept
				{
					return bufferCount_ == 0;
				}

				gsl::span<T> First() const noexcept
				{
					return bufferCount_ ? buffers_[0] : gsl::span<T>{};
				}

				gsl::span<T> Pop() noexcept
				{
					if (bufferCount_)
					{
						auto result = buffers_[0];
						std::copy(buffers_.begin() + 1, buffers_.end(), buffers_.begin());
						bufferCount_--;
						return result;
					}

					return {};
				}

				template<class TOut>
				size_t CopyTo(gsl::span<TOut> buffer) const noexcept
				{
					auto cnt = buffer.data();
					size_t written = 0;
					auto toWrite = std::min(Count(), size_t(buffer.size()));
					for (size_t i = 0; i < bufferCount_; i++)
					{
						auto& src = buffers_[i];
						auto cntWrite = std::min(toWrite, size_t(src.size()));
						auto sub = src.subspan(0, cntWrite);
						std::copy(sub.begin(), sub.end(), cnt);
						cnt += cntWrite;
						written += cntWrite;
						toWrite -= cntWrite;
						if (!toWrite)break;
					}

					return written;
				}

				BufferListSelect Prepend(gsl::span<T> buffer) const
				{
					if (bufferCount_ >= BufferList<T>::MaxBuffersCount)
						throw std::bad_array_new_length();

					BufferListSelect result;
					std::copy(buffers_.begin(), buffers_.begin() + bufferCount_, result.buffers_.begin() + 1);
					result.buffers_[0] = buffer;
					result.bufferCount_ = bufferCount_ + 1;
					return result;
				}

				template<class TOut>
				BufferListSelect<TOut> Cast() const noexcept
				{
					static_assert(sizeof(T) % sizeof(TOut) == 0, "Invalid cast");

					BufferListSelect<TOut> result;
					for (size_t i = 0; i < bufferCount_; i++)
						result.buffers_[i] = { reinterpret_cast<TOut*>(buffers_[i].data()), ptrdiff_t(buffers_[i].size() * sizeof(T) / sizeof(TOut)) };
					result.bufferCount_ = bufferCount_;
					return result;
				}
			private:
				template<class U>
				friend class BufferListSelect;

				std::array<gsl::span<T>, BufferList<T>::MaxBuffersCount> buffers_;
				size_t bufferCount_;
			};
		}

		template<class T>
		struct BufferList
		{
			static constexpr size_t MaxBuffersCount = 8;

			gsl::span<gsl::span<T>> Buffers;

			BufferList() = default;

			BufferList(gsl::span<gsl::span<T>> buffers)
				:Buffers(buffers)
			{
				if (buffers.size() > MaxBuffersCount)
					throw std::invalid_argument("Maximum buffers count exceeded.");
			}

			size_t GetTotalSize() const noexcept
			{
				size_t result = 0;
				for (auto& buffer : Buffers)
					result += buffer.size();
				return result;
			}

			bool IsEmpty() const noexcept
			{
				return Buffers.empty() || GetTotalSize() == 0;
			}

			details::BufferListSelect<T> Select() const noexcept
			{
				return *this;
			}
		};

		class Device : public Object, public virtual IObjectAccess
		{
		public:
			virtual ObjectPtr<Driver> TryLoadDriver();
			virtual DeviceType GetType() const noexcept;
		};
	}
}
