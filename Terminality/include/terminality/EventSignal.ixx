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

		EventSignal<Args...>* owner_ = nullptr;
		std::size_t id_ = 0;

		EventSignalConnection(EventSignal<Args...>* owner, std::size_t id);

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

		std::unordered_map<std::size_t, Handler<Args...>> handlers_;
		std::size_t nextId_ = 1;

	public:
		void operator+=(Handler<Args...> handler);
		[[nodiscard]] EventSignalConnection<Args...> Connect(Handler<Args...> handler);
		void Emit(Args... args);
		
	private:
		void Disconnect(std::size_t id);
	};
}

template<typename ...Args>
terminality::EventSignalConnection<Args...>::EventSignalConnection(EventSignal<Args...>* owner, std::size_t id)
	: owner_(owner), id_(id) { }

template<typename ...Args>
terminality::EventSignalConnection<Args...>::EventSignalConnection(EventSignalConnection<Args...>&& other) noexcept
	: owner_(other.owner_), id_(other.id_)
{
	other.owner_ = nullptr;
	other.id_ = 0;
}

template<typename ...Args>
terminality::EventSignalConnection<Args...>& terminality::EventSignalConnection<Args...>::operator=(EventSignalConnection<Args...>&& other) noexcept
{
	if (this != &other)
	{
		Disconnect();

		owner_ = other.owner_;
		id_ = other.id_;

		other.owner_ = nullptr;
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
	if (owner_ != nullptr)
	{
		owner_->Disconnect(id_);
		owner_ = nullptr;
	}
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
	return EventSignalConnection<Args...>(this, id);
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
