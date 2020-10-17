#pragma once

class CefBaseRefCounted {
public:
	///
	// Called to increment the reference count for the object. Should be called
	// for every new copy of a pointer to a given object.
	///
	virtual void AddRef() const = 0;

	///
	// Called to decrement the reference count for the object. Returns true if
	// the reference count is 0, in which case the object should self-delete.
	///
	virtual bool Release() const = 0;

	///
	// Returns true if the reference count is 1.
	///
	virtual bool HasOneRef() const = 0;

	///
	// Returns true if the reference count is at least 1.
	///
	//virtual bool HasAtLeastOneRef() const = 0;

protected:
	virtual ~CefBaseRefCounted() {}
};
