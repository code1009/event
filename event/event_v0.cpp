#include <iostream>

#include <string>
#include <map>

#include <functional>





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
namespace v0
{





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
class event_data
{
public:
	virtual ~event_data() = default;
};





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
template <typename Tevent_type>
class event
{
public:
	Tevent_type _type;
	std::shared_ptr<event_data> _data;

public:
	explicit event(Tevent_type type, std::shared_ptr<event_data> data):
		_type{ type },
		_data{ data }
	{
	}

public:
	virtual ~event() = default;
};





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
using event_handler = std::function<void(const event_data* )>;





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
class event_handler_data
{
public:
	event_handler _handler;
};





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
template <typename Tevent_type>
class event_dispatcher 
{
public:
	std::unordered_map<
		Tevent_type, 
		std::vector< std::shared_ptr<event_handler_data> >
	> _map;

public:
	event_dispatcher()
	{
	}

public:
	virtual ~event_dispatcher()
	{

	}

public:
	void register_handler(Tevent_type t, event_handler handler)
	{
		std::shared_ptr<event_handler_data> data = std::make_shared<event_handler_data>();

		data->_handler = handler;
		_map[t].push_back(data);
	}

public:
	template<typename Tevent_data>
	void push_event(Tevent_type type, Tevent_data data)
	{
		auto e = event< Tevent_type >
		{ 
			type, 
			std::make_shared< Tevent_data >
			(
				data
			) 
		};

		dispatch_event(e);
	}

public:
	void dispatch_event(const event<Tevent_type>& e)
	{
		if (_map.find(e._type) != _map.end())
		{
			for (auto& handler : _map[e._type])
			{
				handler->_handler(e._data.get());
			}
		}
	}
};





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
enum class my_event_type
{
	t1,
	t2
};

/////////////////////////////////////////////////////////////////////////////
//===========================================================================
class my_event_type_data1 : public event_data
{
public:
	int _a;

public:
	explicit my_event_type_data1(int a) : _a{ a }
	{
	}
};

class my_event_type_data2 : public event_data
{
public:
	std::string _a;

public:
	explicit my_event_type_data2(std::string a) : _a{ a }
	{
	}
};

//===========================================================================
class my_event_dispatcher : public event_dispatcher<my_event_type>
{
public:
	my_event_dispatcher() = default;
};

//===========================================================================
class my_event_observer
{
public:
	my_event_observer() = default;

public:
	void register_event_handler(my_event_dispatcher* dispatcher)
	{
		dispatcher->register_handler(
			my_event_type::t1,
				[this](const event_data* data)
				{
					this->on_event_t1(
						reinterpret_cast<const my_event_type_data1 &>(*data)
					);
				}
		);

		dispatcher->register_handler(
			my_event_type::t2,
				[this](const event_data* data)
				{
					this->on_event_t2(
						reinterpret_cast<const my_event_type_data2&>(*data)
					);
				}
		);
	}

public:
	void on_event_t1(const my_event_type_data1& data)
	{
		std::cout << "on_event_t1: " << data._a << std::endl;
	}

	void on_event_t2(const my_event_type_data2& data)
	{
		std::cout << "on_event_t2: " << data._a << std::endl;
	}
};





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
}





/////////////////////////////////////////////////////////////////////////////
//===========================================================================
void test_event_v0()
{
	v0::my_event_dispatcher dispatcher;
	v0::my_event_observer observer;

	observer.register_event_handler(&dispatcher);


	//dispatcher.push_event<my_event_type_data1>(v0::my_event_type::t1, v0::my_event_type_data1{ 1 });
	//dispatcher.push_event<my_event_type_data2>(v0::my_event_type::t2, v0::my_event_type_data2{ "hello" });
	dispatcher.push_event(v0::my_event_type::t1, v0::my_event_type_data1{ 1 });
	dispatcher.push_event(v0::my_event_type::t2, v0::my_event_type_data2{ "hello" });
}
