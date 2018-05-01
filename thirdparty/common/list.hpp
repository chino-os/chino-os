//
//
//
#include <utility>
#include <cassert>

namespace Chino
{
	template<class TElem>
	class list
	{
	public:
		struct node
		{
			node* prev_ = nullptr;
			node* next_ = nullptr;
			TElem value_;

			template<class ...TArgs>
			node(TArgs&& ...value)
				:value_(std::forward<TArgs>(value)...) {}
		};

		struct iterator
		{
			node* node_;

			iterator(node* node = nullptr)
				:node_(node) {}

			iterator operator++() noexcept
			{
				assert(node_);
				node_ = node_->next_;
				return *this;
			}

			iterator operator--() noexcept
			{
				assert(node_);
				node_ = node_->prev_;
				return *this;
			}

			TElem* operator*() noexcept
			{
				assert(node_);
				return &node_->value_;
			}

			TElem* operator->() noexcept
			{
				assert(node_);
				return &node_->value_;
			}
		};

		list()
			:end_node_(reinterpret_cast<node*>(&end_node_st))
		{

		}

		~list()
		{
			auto cur = head_;
			while (cur && cur != end_node_)
			{
				auto next = cur->next_;
				delete cur;
				cur = next;
			}
		}

		iterator begin() noexcept
		{
			return iterator(head_);
		}

		iterator end() noexcept
		{
			return iterator(end_node_);
		}

		template<class ...TArgs>
		iterator emplace_back(TArgs&& ...value)
		{
			auto new_node = new node(std::forward<TArgs>(value)...);
			new_node->prev_ = tail_;
			new_node->next_ = end_node_;
			if (tail_)
			{
				tail_->next_ = new_node;
				tail_ = new_node;
			}
			else
			{
				head_ = tail_ = new_node;
			}

			return iterator(new_node);
		}

		void erase(node* node) noexcept
		{
			if (node->prev_)
				node->prev_->next_ = node->next_;
			if (node->next_)
				node->next_->prev_ = node->prev_;
			if (head_ == node)
				head_ = node->next_;
			if (tail_ == node)
				tail_ = node->prev_;
			delete node;
		}
	private:
		node* head_ = nullptr;
		node* tail_ = nullptr;
		node* end_node_;
		char end_node_st;
	};
}