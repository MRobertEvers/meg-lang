

#pragma once

#include <iostream>
#include <memory>

template<typename T>
class OwnPtr
{
private:
	std::unique_ptr<T> internal_ = nullptr;

public:
	OwnPtr(T* p)
		: internal_(p){
			  // std::cout << "OwnPtr(T*) " << std::hex << p << std::endl;
		  };

	OwnPtr(T& r)
		: internal_(new T(std::move(r))){
			  // std::cout << "OwnPtr(T&) " << std::hex << internal_.get() << std::endl;
		  };
	OwnPtr(T&& r)
		: internal_(new T(std::move(r))){
			  // std::cout << "OwnPtr(T&&) " << std::hex << internal_.get() << std::endl;
		  };

	template<
		typename TPolymorphic,
		typename = std::enable_if_t<std::is_base_of<T, TPolymorphic>::value>>
	OwnPtr(TPolymorphic&& other)
		: internal_(new TPolymorphic(std::move(other))){

		  };

	template<typename TPolymorphic>
	OwnPtr(OwnPtr<TPolymorphic>&& other)
		: internal_(other.get())
	{
		other.release();
	};

	// ~OwnPtr() { std::cout << "~OwnPtr(T&&) " << std::hex << internal_.get() << std::endl; }

	void release() { internal_.release(); }
	T* operator->() const { return internal_.get(); }
	T& operator*() const { return *internal_.get(); }

	T* get() const { return internal_.get(); }
	bool is_null() const { return internal_.get() == nullptr; }

	template<typename... TAny>
	static OwnPtr<T> of(TAny&&... args)
	{
		return OwnPtr(new T(std::forward<TAny>(args)...));
	}

	static OwnPtr<T> of(T* p) { return OwnPtr(p); }

	static OwnPtr<T> null() { return OwnPtr(nullptr); }
};