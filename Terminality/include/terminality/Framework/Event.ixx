export module terminality:Event;

import <unordered_map>;
import <functional>;
import <cstdint>;
import <memory>;

export namespace terminality
{
	template<typename... Args>
	class Event;

	template<typename... Args>
	using Handler = std::function<void(Args...)>;

	template<typename... Args>
	class EventConnection
	{
		friend class Event<Args...>;

		std::weak_ptr<Event<Args...>*> tracker_;
		std::size_t id_ = 0;

		EventConnection(std::weak_ptr<Event<Args...>*> tracker, std::size_t id);

	public:
		EventConnection() = default;

		EventConnection(EventConnection<Args...>&& other) noexcept;
		EventConnection& operator=(EventConnection<Args...>&& other) noexcept;

		EventConnection(const EventConnection<Args...>&) = delete;
		EventConnection& operator=(const EventConnection<Args...>&) = delete;

		~EventConnection();

		void Disconnect();
	};

	template<typename... Args>
	class Event
	{
		friend class EventConnection<Args...>;

		std::shared_ptr<Event<Args...>*> selfToken_;
		std::unordered_map<std::size_t, Handler<Args...>> handlers_;
		std::size_t nextId_ = 1;

	public:
		Event();
		~Event();

		Event(Event&& other) noexcept;
		Event& operator=(Event&& other) noexcept;

		Event(const Event&) = delete;
		Event& operator=(const Event&) = delete;

		void operator+=(Handler<Args...> handler);
		[[nodiscard]] EventConnection<Args...> Connect(Handler<Args...> handler);
		void Emit(Args... args);
		
	private:
		void Disconnect(std::size_t id);
	};
}

template<typename ...Args>
terminality::EventConnection<Args...>::EventConnection(std::weak_ptr<Event<Args...>*> tracker, std::size_t id)
	: tracker_(std::move(tracker)), id_(id) { }

template<typename ...Args>
terminality::EventConnection<Args...>::EventConnection(EventConnection<Args...>&& other) noexcept
	: tracker_(std::move(other.tracker_)), id_(other.id_)
{
	other.id_ = 0;
}

template<typename ...Args>
terminality::EventConnection<Args...>& terminality::EventConnection<Args...>::operator=(EventConnection<Args...>&& other) noexcept
{
	if (this != &other)
	{
		Disconnect();

		tracker_ = std::move(other.tracker_);
		id_ = other.id_;
		other.id_ = 0;
	}

	return *this;
}

template<typename... Args>
terminality::EventConnection<Args...>::~EventConnection()
{
	Disconnect();
}

template<typename... Args>
void terminality::EventConnection<Args...>::Disconnect()
{
	if (auto locked = tracker_.lock())
	{
		if (Event<Args...>* owner = *locked)
			owner->Disconnect(id_);
	}

	tracker_.reset();
}

template<typename ...Args>
terminality::Event<Args...>::Event()
	: selfToken_(std::make_shared<Event<Args...>*>(this)) { }

template<typename ...Args>
terminality::Event<Args...>::~Event()
{
	if (selfToken_)
	{
		*selfToken_ = nullptr;
	}
}

template<typename ...Args>
terminality::Event<Args...>::Event(Event&& other) noexcept
	: handlers_(std::move(other.handlers_)), nextId_(other.nextId_), selfToken_(std::move(other.selfToken_))
{
	if (selfToken_)
	{
		*selfToken_ = this;
	}
}

template<typename ...Args>
terminality::Event<Args...>& terminality::Event<Args...>::operator=(Event&& other) noexcept
{
	if (this != &other)
	{
		if (selfToken_)
			*selfToken_ = nullptr;

		handlers_ = std::move(other.handlers_);
		nextId_ = other.nextId_;
		selfToken_ = std::move(other.selfToken_);

		if (selfToken_)
			*selfToken_ = this;
	}

	return *this;
}

template<typename ...Args>
void terminality::Event<Args...>::operator+=(terminality::Handler<Args...> handler)
{
	const std::size_t id = nextId_++;
	handlers_.emplace(id, std::move(handler));
}

template<typename... Args>
terminality::EventConnection<Args...> terminality::Event<Args...>::Connect(terminality::Handler<Args...> handler)
{
	const std::size_t id = nextId_++;
	handlers_.emplace(id, std::move(handler));

	return EventConnection<Args...>(selfToken_, id);
}

template<typename... Args>
void terminality::Event<Args...>::Emit(Args... args)
{
	const std::unordered_map<std::size_t, Handler<Args...>> snapshot = handlers_;
	for (const auto& entry : snapshot)
	{
		entry.second(args...);
	}
}

template<typename... Args>
void terminality::Event<Args...>::Disconnect(std::size_t id)
{
	handlers_.erase(id);
}
