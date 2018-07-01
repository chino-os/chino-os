#include <kernel/kernel_iface.h>
#include <string>
#include <string_view>

namespace Chino
{
	namespace Device
	{
		// Read only iterator
		class PathIterator
		{
		public:
			using value_type = char;
			using string_type = std::string;
			using size_type = typename string_type::size_type;

			static constexpr const value_type Separator = '/';

			PathIterator() = default;
			~PathIterator() = default;

			PathIterator(const PathIterator& pi);
			PathIterator(const string_type* path, size_type i);

			PathIterator& operator=(const PathIterator& pi);

			std::string_view operator*() const;

			PathIterator& operator++();
			PathIterator operator++(int);
			PathIterator& operator--();
			PathIterator operator--(int);

			friend bool operator==(const PathIterator& lhs, const PathIterator& rhs);
			friend bool operator!=(const PathIterator& lhs, const PathIterator& rhs);

		private:
			const string_type* path_;
			size_type i_;
		};

		class Path
		{
		public:
			using value_type = char;
			using string_type = std::string;
			using view_type = std::string_view;
			using iterator = PathIterator;

			static constexpr const value_type Separator = '/';

			enum PathType
			{
				Directory,
				Undetermined
			};

		private:
			string_type path_;

		public:
			// constructor
			Path() = default;
			~Path() = default;

			// copy constructor and move constructor
			Path(const Path&) = default;
			Path(Path&& path) noexcept;

			Path(const value_type* s);
			Path(const string_type& s);
			Path(string_type&& s) noexcept;

			// assignment
			Path& operator=(const Path&) = default;
			Path& operator=(Path&& path) noexcept;

			Path& operator=(const value_type* s);
			Path& operator=(const string_type& s);
			Path& operator=(string_type&& s) noexcept;

			// iterator
			iterator begin() const noexcept;
			iterator end()   const noexcept;

			// query
			bool IsRoot() const noexcept;

			PathType Type() const noexcept;

			view_type String() const noexcept;

			view_type Extension() const noexcept;

			// modify
			Path& Append(const value_type* file);
			Path& Append(const string_type& file);

			Path& RemoveLast();
		};

	}
}