﻿#include <iostream>
#include <string>

#include <vector>
#include <unordered_map>

#include <functional>





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
namespace v1
{





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
template<typename...Tevent_args>
using event_handler = std::function<void(Tevent_args...)>;





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
class event_base_listener
{
public:
	virtual ~event_base_listener() = default;
};

//===========================================================================
template<typename...Tevent_args>
class event_listener : public event_base_listener
{
public:
	event_handler<Tevent_args...> _handler;

public:
	event_listener(event_handler<Tevent_args...> handler) : 
		_handler(handler) 
	{
	}

public:
	void invoke(Tevent_args... args)
	{
		_handler(args...);
	}
};





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
template<typename Tevent_target>
class base_event
{
public:
	virtual ~base_event() = default;
};

//===========================================================================
template<typename Tevent_target>
class event : public base_event<Tevent_target>
{
public:
	std::unordered_map<Tevent_target, std::shared_ptr<event_base_listener>> _listeners;

public:
	template<typename...Tevent_args>
	void register_handler(event_handler<Tevent_args...> handler, Tevent_target target = 0)
	{
		register_listener(
			std::make_shared<event_listener<Tevent_args...>>(std::move(handler)),
			target
		);
	}

	void register_listener(std::shared_ptr<event_base_listener> listener, Tevent_target target)
	{
		_listeners[target] = std::move(listener);
	}

	void unregister_target(Tevent_target target)
	{
		_listeners.erase(target);
	}

public:
	template<typename...Tevent_args>
	void broadcast(Tevent_args... args)
	{
		for (auto& listener : _listeners)
		{
			auto l = std::dynamic_pointer_cast<event_listener<Tevent_args...>>(listener.second);
			if (l)
			{
				l->invoke(args...);
			}
		}
	}

	template<typename...Tevent_args>
	void unicast(Tevent_target target, Tevent_args... args)
	{
		auto found = _listeners.find(target);
		if (found != _listeners.end())
		{
			auto l = std::dynamic_pointer_cast<event_listener<Tevent_args...>>((*found).second);
			if (l)
			{
				l->invoke(args...);
			}
		}
	}
};





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
template<typename Tevent_name_type, typename Tevent_target = void*>
class event_dispatcher
{
public:
	std::unordered_map<
		Tevent_name_type, 
		std::shared_ptr<base_event<Tevent_target>>
	> _events;

public:
	template<typename...Tevent_args>
	void register_event(Tevent_name_type name, Tevent_target target, event_handler<Tevent_args...> handler)
	{
		auto found = _events.find(name);
		if (found != _events.end())
		{
			auto e = std::dynamic_pointer_cast<event<Tevent_target>>((*found).second);
			e->register_handler<Tevent_args...>(handler, target);
		}
		else
		{
			auto e = std::make_shared<event<Tevent_target>>();
			_events[name] = e;
			e->register_handler<Tevent_args...>(handler, target);
		}
	}

public:
	template<typename...Tevent_args>
	void dispatch(Tevent_name_type name, Tevent_args... args)
	{
		auto found = _events.find(name);
		if (found != _events.end())
		{
			auto e = std::dynamic_pointer_cast<event<Tevent_target>>((*found).second);
			if (e)
			{
				e->broadcast<Tevent_args...>(args...);
			}
		}
	}

	template<typename...Tevent_args>
	void dispatch(Tevent_name_type name, Tevent_target target, Tevent_args... args)
	{
		auto found = _events.find(name);
		if (found != _events.end())
		{
			auto e = std::dynamic_pointer_cast<event<Tevent_target>>((*found).second);
			if (e)
			{
				e->unicast<Tevent_args...>(target, args...);
			}
		}
	}
};





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
class my_event_observer;
template class event_dispatcher<std::string, my_event_observer*>;
using my_event_dispatcher = event_dispatcher<std::string, my_event_observer*>;





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
class my_event_type_data1
{
public:
	int _a;

public:
	explicit my_event_type_data1(int a) : _a{ a }
	{
	}
};

class my_event_type_data2
{
public:
	std::string _a;

public:
	explicit my_event_type_data2(std::string a) : _a{ a }
	{
	}
};





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
class my_event_observer
{
public:
	std::string _name;

public:
	my_event_observer() = default;

public:
	explicit my_event_observer(std::string name) : 
		_name{ name }
	{
	}

public:
	void register_event_handler(my_event_dispatcher* dispatcher)
	{
		dispatcher->register_event<const my_event_type_data1&>(
			std::string{"t1"},
			this,
			[this](const my_event_type_data1& data)
			{
				this->on_event_t1_ignore(
					data
				);
			}
		);

		dispatcher->register_event<const my_event_type_data1&>(
			"t1",
			this,
			[this](auto data)
			{
				this->on_event_t1(
					data
				);
			}
		);

		dispatcher->register_event<const my_event_type_data2&>(
			"t2",
			this,
			[this](auto data)
			{
				this->on_event_t2(
					data
				);
			}
		);
	}

public:
	void on_event_t1_ignore(const my_event_type_data1& data)
	{
		std::cout << _name << "-on_event_t1_ignore: " << data._a << std::endl;
	}

	void on_event_t1(const my_event_type_data1& data)
	{
		std::cout << _name << "-on_event_t1: " << data._a << std::endl;
	}

	void on_event_t2(const my_event_type_data2& data)
	{
		std::cout << _name << "-on_event_t2: " << data._a << std::endl;
	}
};





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
}




/////////////////////////////////////////////////////////////////////////////
//===========================================================================
void test_event_v1()
{
	v1::my_event_dispatcher dispatcher;
	v1::my_event_observer observer1("observer1");
	v1::my_event_observer observer2("observer2");

	observer1.register_event_handler(&dispatcher);
	observer2.register_event_handler(&dispatcher);

	v1::my_event_type_data1 a{ 1 };
	v1::my_event_type_data2 b{ "hello" };

	dispatcher.dispatch<const v1::my_event_type_data1&>("t1", a);
	dispatcher.dispatch<const v1::my_event_type_data2&>("t2", b);
	dispatcher.dispatch<const v1::my_event_type_data1&>("t1", &observer2, a);
}
