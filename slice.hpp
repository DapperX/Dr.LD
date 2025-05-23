#ifndef _SLICE_HPP_
#define _SLICE_HPP_
#include <cstdint>

template<typename T>
class slice{
public:
	class iterator{
	public:
		iterator(T base) : position(base){}
		iterator(const iterator &rhs) : position(rhs.position){}
		bool operator==(const iterator &rhs) const{
			return position==rhs.position;
		}
		bool operator!=(const iterator &rhs) const{
			return position!=rhs.position;
		}
		auto& operator*(){
			return *position;
		}
		T& operator->(){
			return position;
		}
		iterator& operator+=(int offset){
			return position+=offset, *this;
		}
		iterator& operator++(){ // ++a
			return ++position, *this;
		}
		iterator operator++(int){ // a++
			return iterator(position++);
		}
		iterator operator+(int offset) const{
			return iterator(position+offset);
		}
		iterator& operator-=(int offset){
			return position-=offset, *this;
		}
		iterator& operator--(){
			return ++position, *this;
		}
		iterator operator--(int){
			return iterator(position++);
		}
		iterator operator-(int offset) const{
			return iterator(position-offset);
		}
		size_t operator-(const iterator &rhs) const {
			return position - rhs.position;
		}

	protected:
		T position;
	};

	class const_iterator : public iterator{
	public:
		const_iterator(T base) : iterator(base){}
		const_iterator(const iterator &rhs) : iterator(rhs){}
		bool operator==(const const_iterator &rhs) const{
			return iterator::position==rhs.position;
		}
		bool operator!=(const const_iterator &rhs) const{
			return iterator::position!=rhs.position;
		}
		const auto& operator*(){
			return *iterator::position;
		}
		const T& operator->(){
			return iterator::position;
		}
		const_iterator& operator+=(int offset){
			return iterator::position+=offset, *this;
		}
		const_iterator& operator++(){ // ++a
			return ++iterator::position, *this;
		}
		const_iterator operator++(int){ // a++
			return const_iterator(iterator::position++);
		}
		const_iterator operator+(int offset) const{
			return const_iterator(iterator::position+offset);
		}
		const_iterator& operator-=(int offset){
			return iterator::position-=offset, *this;
		}
		const_iterator& operator--(){
			return ++iterator::position, *this;
		}
		const_iterator operator--(int){
			return const_iterator(iterator::position++);
		}
		const_iterator operator-(int offset) const{
			return const_iterator(iterator::position-offset);
		}
		size_t operator-(const const_iterator &rhs) const {
			return iterator::position - rhs.position;
		}
	};

	typedef decltype(*std::declval<T>()) value_type;
	typedef const T const_pointer;
	typedef T pointer;

	slice(const T& begin, size_t n, size_t entbyte=sizeof(value_type))
		: base(begin), length(n), entbyte(entbyte) {}
	iterator begin(){
		return iterator(base);
	}
	const_iterator begin() const{
		return const_iterator(base);
	}
	iterator end(){
		return iterator(base+length);
	}
	const_iterator end() const{
		return const_iterator(base+length);
	}
	pointer raw(){
		return base;
	}
	const_pointer raw() const{
		return base;
	}
	size_t size() const{
		return length;
	}
	size_t byte_each() const{
		return entbyte;
	}
	auto& operator[](const int offset){
		return *(base+offset);
	}
	const auto& operator[](const int offset) const{
		return *(base+offset);
	}

private:
	const T base;
	const size_t entbyte, length;
};

#endif // _SLICE_HPP_