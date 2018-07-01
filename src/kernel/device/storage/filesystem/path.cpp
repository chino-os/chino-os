#include "path.hpp"
#include <cassert>

namespace Chino
{
	namespace Device
	{
		// PathIterator

		PathIterator::PathIterator(const PathIterator& pi)
			: path_(pi.path_), i_(pi.i_)
		{
		}
		PathIterator::PathIterator(const string_type* path, size_type i)
			: path_(path), i_(i)
		{
		}

		PathIterator& PathIterator::operator=(const PathIterator& pi)
		{
			if (&pi != this)
			{
				path_ = pi.path_;
				i_ = pi.i_;
			}
			return *this;
		}

		std::string_view PathIterator::operator*() const
		{
			if (i_ >= path_->size())
			{
				return {};
			}

			auto next = path_->find_first_of(Separator, i_);
			if (next == string_type::npos)
			{
				return std::string_view(path_->c_str() + i_, path_->size() - i_);
			}
			return std::string_view(path_->c_str() + i_, next - i_);
		}

		PathIterator& PathIterator::operator++()
		{
			assert(i_ != string_type::npos);
			i_ = path_->find_first_of(Separator, i_);
			if (i_ == path_->size() - 1)
			{
				i_ = string_type::npos;
			}
			else if (i_ != string_type::npos)
			{
				i_++;
			}
			return *this;
		}
		PathIterator PathIterator::operator++(int)
		{
			PathIterator tmp(*this);
			++*this;
			return tmp;
		}
		PathIterator& PathIterator::operator--()
		{
			// Guaranteed to have at least two directories
			assert(i_ > 1);
			if (i_ == string_type::npos)
			{
				i_ = path_->rfind(Separator, path_->size() - 2) + 1;
			}
			else
			{
				i_ = path_->rfind(Separator, i_ - 2) + 1;
			}
			return *this;
		}
		PathIterator PathIterator::operator--(int)
		{
			PathIterator tmp(*this);
			--*this;
			return tmp;
		}

		bool operator==(const PathIterator& lhs, const PathIterator& rhs)
		{
			return lhs.path_ == rhs.path_ && lhs.i_ == rhs.i_;
		}
		bool operator!=(const PathIterator& lhs, const PathIterator& rhs)
		{
			return !(lhs == rhs);
		}

		// Path

		Path::Path(Path&& path) noexcept
		{
			path_.swap(path.path_);
		}
		Path::Path(const value_type* s)
			: path_(s)
		{
		}
		Path::Path(const string_type& s)
			: path_(s)
		{
		}
		Path::Path(string_type&& s) noexcept
		{
			path_.swap(s);
		}

		Path& Path::operator=(Path&& path) noexcept
		{
			path_.swap(path.path_);
			return *this;
		}
		Path& Path::operator=(const value_type* s)
		{
			path_ = s;
			return *this;
		}
		Path& Path::operator=(const string_type& s)
		{
			path_ = s;
			return *this;
		}
		Path& Path::operator=(string_type&& s) noexcept
		{
			path_.swap(s);
			return *this;
		}

		Path::iterator Path::begin() const noexcept
		{
			return iterator(&path_, 1);
		}
		Path::iterator Path::end() const noexcept
		{
			return iterator(&path_, string_type::npos);
		}

		bool Path::IsRoot() const noexcept
		{
			return path_ == "/";
		}
		Path::PathType Path::Type() const noexcept
		{
			if (path_.back() == Separator)
			{
				return Directory;
			}
			return Undetermined;
		}
		Path::view_type Path::String() const noexcept
		{
			return path_;
		}

		Path::view_type Path::Extension() const noexcept
		{
			auto ext_pos = path_.rfind('.');
			if (ext_pos != string_type::npos)
			{
				return view_type(path_.c_str() + ext_pos);
			}
			return {};
		}

		Path& Path::Append(const value_type* file)
		{
			if (path_.back() != Separator)
			{
				path_.append(1, Separator);
			}
			if (*file == Separator)
			{
				path_.append(file + 1);
			}
			else
			{
				path_.append(file);
			}
			return *this;
		}
		Path& Path::Append(const string_type& file)
		{
			if (path_.back() != Separator)
			{
				path_.append(1, Separator);
			}
			if (file.front() == Separator)
			{
				path_.append(file, 1, string_type::npos);
			}
			else
			{
				path_.append(file);
			}
			return *this;
		}
		Path& Path::RemoveLast()
		{
			// Guaranteed to have at least two directories
			auto prev = path_.rfind(Separator, path_.size() - 2);
			if (prev == 0)
			{
				path_.erase(path_.begin() + 1, path_.end());
			}
			else
			{
				path_.erase(path_.begin() + prev + 1, path_.end());
			}
			return *this;
		}
	}
}