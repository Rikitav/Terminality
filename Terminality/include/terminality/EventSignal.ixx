export module terminality:EventSignal;

import std;
import std.compat;

export namespace terminality
{
	template<typename... Args>
	class EventSignal;

	template<typename... Args>
	using Handler = std::function<void(Args...)>;

	template<typename... Args>
	class EventSignalConnection
	{
		friend class EventSignal<Args...>;

		std::weak_ptr<EventSignal<Args...>*> tracker_;
		std::size_t id_ = 0;

		EventSignalConnection(std::weak_ptr<EventSignal<Args...>*> tracker, std::size_t id);

	public:
		EventSignalConnection() = default;

		EventSignalConnection(EventSignalConnection<Args...>&& other) noexcept;
		EventSignalConnection& operator=(EventSignalConnection<Args...>&& other) noexcept;

		EventSignalConnection(const EventSignalConnection<Args...>&) = delete;
		EventSignalConnection& operator=(const EventSignalConnection<Args...>&) = delete;

		~EventSignalConnection();

		void Disconnect();
	};

	template<typename... Args>
	class EventSignal
	{
		friend class EventSignalConnection<Args...>;

		std::shared_ptr<EventSignal<Args...>*> selfToken_;
		std::unordered_map<std::size_t, Handler<Args...>> handlers_;
		std::size_t nextId_ = 1;

	public:
		EventSignal();
		~EventSignal();

		EventSignal(EventSignal&& other) noexcept;
		EventSignal& operator=(EventSignal&& other) noexcept;

		EventSignal(const EventSignal&) = delete;
		EventSignal& operator=(const EventSignal&) = delete;

		void operator+=(Handler<Args...> handler);
		[[nodiscard]] EventSignalConnection<Args...> Connect(Handler<Args...> handler);
		void Emit(Args... args);
		
	private:
		void Disconnect(std::size_t id);
	};
}

template<typename ...Args>
terminality::EventSignalConnection<Args...>::EventSignalConnection(std::weak_ptr<EventSignal<Args...>*> tracker, std::size_t id)
	: tracker_(std::move(tracker)), id_(id) { }

template<typename ...Args>
terminality::EventSignalConnection<Args...>::EventSignalConnection(EventSignalConnection<Args...>&& other) noexcept
	: tracker_(std::move(other.tracker_)), id_(other.id_)
{
	other.id_ = 0;
}

template<typename ...Args>
terminality::EventSignalConnection<Args...>& terminality::EventSignalConnection<Args...>::operator=(EventSignalConnection<Args...>&& other) noexcept
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
terminality::EventSignalConnection<Args...>::~EventSignalConnection()
{
	Disconnect();
}

template<typename... Args>
void terminality::EventSignalConnection<Args...>::Disconnect()
{
	if (auto locked = tracker_.lock())
	{
		if (EventSignal<Args...>* owner = *locked)
			owner->Disconnect(id_);
	}

	tracker_.reset();
}

template<typename ...Args>
terminality::EventSignal<Args...>::EventSignal()
	: selfToken_(std::make_shared<EventSignal<Args...>*>(this)) { }

template<typename ...Args>
terminality::EventSignal<Args...>::~EventSignal()
{
	if (selfToken_)
	{
		*selfToken_ = nullptr;
	}
}

template<typename ...Args>
terminality::EventSignal<Args...>::EventSignal(EventSignal&& other) noexcept
	: handlers_(std::move(other.handlers_)), nextId_(other.nextId_), selfToken_(std::move(other.selfToken_))
{
	if (selfToken_)
	{
		*selfToken_ = this;
	}
}

template<typename ...Args>
terminality::EventSignal<Args...>& terminality::EventSignal<Args...>::operator=(EventSignal&& other) noexcept
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
void terminality::EventSignal<Args...>::operator+=(terminality::Handler<Args...> handler)
{
	const std::size_t id = nextId_++;
	handlers_.emplace(id, std::move(handler));
}

template<typename... Args>
terminality::EventSignalConnection<Args...> terminality::EventSignal<Args...>::Connect(terminality::Handler<Args...> handler)
{
	const std::size_t id = nextId_++;
	handlers_.emplace(id, std::move(handler));

	return EventSignalConnection<Args...>(selfToken_, id);
}

template<typename... Args>
void terminality::EventSignal<Args...>::Emit(Args... args)
{
	const std::unordered_map<std::size_t, Handler<Args...>> snapshot = handlers_;
	for (const auto& entry : snapshot)
	{
		entry.second(args...);
	}
}

template<typename... Args>
void terminality::EventSignal<Args...>::Disconnect(std::size_t id)
{
	handlers_.erase(id);
}
