// programmed by Bryce Mehring
// Start Date: 8/20/2011
// Last edit: 9/10/2011

// Note: These delegates only support 1 argument 
// todo: I could add support for n amount of arguments with Variadic templates when MSVS updates to the new
// C++0x standard which just got passed. 

// good programming project
// to implement this version of delegates you need to know:

// 1. Interfaces
// 2. Templates
// 3. Template specialization
// 4. Function pointers
// 5. Member function pointers
// 6. Polymorphism
// 7. operator overloading
// 8. Custom Copy Constructor
// 9. Ref Counting
// 10. overloading functions 
// 11. overloading casting operators
// 12. Memory pools to make it faster - have not added yet. 


#ifndef _DELEGATES_
#define _DELEGATES_

#include "RefCounting.h"
#include "asVM.h"
#include <map>

// Base Interface for Delegates
template< class RETURN, class PARAM>
class IFunction : public RefCounting
{
public:

	virtual ~IFunction() {}
	virtual RETURN Call(PARAM) const = 0;

};

// GlobalFunction version of IFunction
template< class RETURN, class PARAM>
class GlobalFunction : public IFunction<RETURN,PARAM>
{
public:

	// global function signature
	typedef RETURN (*PTR)(PARAM);

	GlobalFunction(PTR function) : m_function(function) {}

	virtual RETURN Call(PARAM param) const
	{
		return (*m_function)(param);
	}

private:

	PTR m_function;

};

template< class RETURN >
class GlobalFunction<RETURN,void> : public IFunction<RETURN,void>
{
public:

	typedef RETURN (*PTR)();

	GlobalFunction(PTR function) : m_function(function) {}

	virtual RETURN Call() const
	{
		return (*m_function)();
	}

private:

	PTR m_function;

};

// MemberFunction version of IFunction
template<class CLASS, class RETURN, class PARAM>
class MemberFunction : public IFunction<RETURN,PARAM>
{
public:

	// class method signature 
	typedef RETURN (CLASS::*PTR)(PARAM);

	MemberFunction(CLASS* pThis,PTR method) : m_pThis(pThis), m_method(method) {}

	virtual RETURN Call(PARAM param) const
	{
		return (m_pThis->*m_method)(param);
	}

private:

	CLASS* m_pThis;
	PTR m_method;
	
};

template<class CLASS, class RETURN>
class MemberFunction<CLASS,RETURN,void> : public IFunction<RETURN,void>
{
public:

	typedef RETURN (CLASS::*PTR)();

	MemberFunction(CLASS* pThis,PTR method) : m_pThis(pThis), m_method(method) {}

	virtual RETURN Call() const
	{
		return (m_pThis->*m_method)();
	}

private:

	CLASS* m_pThis;
	PTR m_method;
	
};

// ConstMemberFunction version of IFunction
template<class CLASS, class RETURN, class PARAM>
class ConstMemberFunction : public IFunction<RETURN,PARAM>
{
public:

	// class method signature 
	typedef RETURN (CLASS::*PTR)(PARAM) const;

	ConstMemberFunction(CLASS* pThis,PTR method) : m_pThis(pThis), m_method(method) {}

	virtual RETURN Call(PARAM param) const
	{
		return (m_pThis->*m_method)(param);
	}

private:

	CLASS* m_pThis;
	PTR m_method;
	
};

template<class CLASS, class RETURN>
class ConstMemberFunction<CLASS,RETURN,void> : public IFunction<RETURN,void>
{
public:

	typedef RETURN (CLASS::*PTR)() const;

	ConstMemberFunction(CLASS* pThis,PTR method) : m_pThis(pThis), m_method(method) {}

	virtual RETURN Call() const
	{
		return (m_pThis->*m_method)();
	}

private:

	CLASS* m_pThis;
	PTR m_method;
	
};

class ScriptFunction : public IFunction<void,void>
{
public:

	ScriptFunction(asVM* pVM,const ScriptFunctionStruct& func) : m_pVM(pVM), m_func(func) {}

	virtual void Call() const
	{
		m_pVM->ExecuteScriptFunction(m_func);
	}

private:

	asVM* m_pVM;
	ScriptFunctionStruct m_func;

};

// The Base Delegate class
// This class has a IFunction object which manages binding

// common functionality of delegates

template< class RETURN, class PARAM>
class DelegateBase
{
public:

	typedef IFunction<RETURN,PARAM> GENERIC_FUNC;

	DelegateBase() : m_ptr(nullptr) {}
	DelegateBase(const DelegateBase& d)
	{
		m_ptr = d.m_ptr;
		if(m_ptr)
		{
			m_ptr->AddRef();
		}
	}
	~DelegateBase()
	{
		Unbind();
	}

	void Bind(typename GlobalFunction<RETURN,PARAM>::PTR pFunc)
	{
		if(m_ptr == nullptr)
		{
			m_ptr = new GlobalFunction<RETURN,PARAM>(pFunc);
		}
	}

	void Bind(asVM* pm, const ScriptFunctionStruct& func)
	{
		if(m_ptr == nullptr)
		{
			m_ptr = new ScriptFunction(pm,func);
		}
	}

	template< class CLASS >
	void Bind(CLASS* pThis,typename MemberFunction<CLASS,RETURN,PARAM>::PTR pFunc)
	{
		if(m_ptr == nullptr)
		{
			m_ptr = new MemberFunction<CLASS,RETURN,PARAM>(pThis,pFunc);
		}
	}
	
	template< class CLASS >
	void Bind(CLASS* pThis,typename ConstMemberFunction<CLASS,RETURN,PARAM>::PTR pFunc)
	{
		if(m_ptr == nullptr)
		{
			m_ptr = new ConstMemberFunction<CLASS,RETURN,PARAM>(pThis,pFunc);
		}
	}

	void Unbind()
	{
		if(m_ptr)
		{
			m_ptr->Release();
			m_ptr = nullptr;
		}
	}

	DelegateBase& operator =(const DelegateBase& d)
	{
		if(this != &d)
		{
			// Release old pointer
			if(m_ptr)
			{
				m_ptr->Release();
			}

			// copy the new pointer
			m_ptr = d.m_ptr;

			// Keep a copy, call AddRef
			if(m_ptr)
			{
				m_ptr->AddRef();
			}
		}

		return *this;
	}
	operator void*()
	{
		return (void*)m_ptr;
	}

protected:

	GENERIC_FUNC* m_ptr;

};


// Delegate with parameter
template< class RETURN, class PARAM>
class Delegate : public DelegateBase<RETURN,PARAM>
{
public:

	RETURN Call(PARAM param) const
	{
		return m_ptr->Call(param);
	}

};

// Delegate without parameters
template< class RETURN >
class Delegate<RETURN,void> : public DelegateBase<RETURN,void>
{
public:

	RETURN Call() const
	{
		return m_ptr->Call();
	}
};


// these event class ignores the return value

// common event functionality
template < class RETURN , class PARAM >
class EventBase
{
public:

	typedef Delegate<RETURN,PARAM> DELEGATE;

	// adds a Delegate to the map and returns its key.
	void Attach(int id, DELEGATE d)
	{
		DelegateMap::iterator iter = m_delegates.find(id);
		if(iter != m_delegates.end())
		{
			iter->first = id;
			iter->second = d;
		}
	}
	// Detaches a delegate by its key
    void Detach(int id)
    {
        DelegateMap::iterator iter = m_delegates.find(id);

        if(iter != m_delegates.end())
		{
			m_delegates.erase(iter);
		}
    }

protected:

	/*
	data structures: map vs vector

	map: 
	
	cons - a bit slower traversal performance in Notifying the delegates because of the random memory locations,
	 but this could be solved by using a memory pool with a custom stl allocator. Logarithmic time for Attaching

	pros - Logarithmic time for detaching

	vector:

	cons - very slow in detaching delegates.
	pros - very fast at traversing, like a C array. Constant time Attaching delegates.

	*/

	typedef std::map<int,DELEGATE> DelegateMap;
    DelegateMap m_delegates;

};

// Event with parameters
template < class RETURN , class PARAM >
class Event : public EventBase<RETURN,PARAM>
{
public:

	// Notify all delegates
    void Notify(PARAM a)
    {
        DelegateMap::iterator iter = m_delegates.begin();
        for(; iter != m_delegates.end(); iter++)
        {
			iter->second.Call(a);
        }
    }
};

// Event without parameters
template < class RETURN >
class Event<RETURN,void> : public EventBase<RETURN,void>
{
public:

    void Notify()
    {
        DelegateMap::iterator iter = m_delegates.begin();
        for(; iter != m_delegates.end(); iter++)
        {
			iter->second.Call();
        }
    }
};

#endif // _DELEGATES_