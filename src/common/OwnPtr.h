

#pragma once

#include <memory>

template<typename T>
class OwnPtr
{
private:
	std::unique_ptr<T> internal_ = nullptr;

public:
	OwnPtr(T* p)
		: internal_(p){};
	OwnPtr(T r)
		: internal_(new T(r)){};

	T* operator->() const { return internal_.get(); }
	T& operator*() const { return *internal_.get(); }

	T* get() { return internal_.get(); }
	bool is_null() const { return internal_.get() == nullptr; }

	template<typename... TAny>
	static OwnPtr<T> of(TAny&&... args)
	{
		return OwnPtr(new T(std::forward<TAny>(args)...));
	}

	static OwnPtr<T> of(T* p) { return OwnPtr(p); }

	static OwnPtr<T> null() { return OwnPtr(nullptr); }
};