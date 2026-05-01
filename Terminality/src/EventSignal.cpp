module terminality;

import std;
import std.compat;

using namespace terminality;

/*
template<typename... Args>
inline EventSignalConnection<Args...>::EventSignalConnection(EventSignal<Args...>* owner, size_t id)
	: owner_(owner), id_(id) { }

template<typename... Args>
inline EventSignalConnection<Args...>::EventSignalConnection(EventSignalConnection<Args...>&& other)
{
	other.owner_ = nullptr;
}

template<typename... Args>
inline EventSignalConnection<Args...>::~EventSignalConnection()
{
	Disconnect();
}

template<typename ...Args>
inline void EventSignalConnection<Args...>::Disconnect()
{
	if (owner_)
	{
		owner_->Disconnect(id_);
		owner_ = nullptr;
	}
}

template<typename... Args>
inline EventSignalConnection<Args...> EventSignal<Args...>::Connect(Handler<Args...> handler)
{
	const size_t id = nextId_++;
	handlers_.emplace(id, std::move(handler));
	return EventSignalConnection<Args...>(this, id);
}

template<typename... Args>
inline void EventSignal<Args...>::Emit(Args... args)
{
	const std::unordered_map<size_t, Handler<Args...>> snapshot = handlers_;
	for (const auto& entry : snapshot)
	{
		entry.second(args...);
	}
}

template<typename... Args>
inline void EventSignal<Args...>::Disconnect(size_t id)
{
	handlers_.erase(id);
}
*/
