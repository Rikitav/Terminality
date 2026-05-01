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
		size_t id_ = 0;

		EventSignalConnection(EventSignal<Args...>* owner, size_t id)
			: owner_(owner), id_(id) { }

	public:
		EventSignalConnection(const EventSignalConnection<Args...>&)
			= delete;

		EventSignalConnection(EventSignalConnection&& other)
		{
			other.owner_ = nullptr;
		}

		~EventSignalConnection()
		{
			Disconnect();
		}

		void Disconnect()
		{
			if (owner_)
			{
				owner_->Disconnect(id_);
				owner_ = nullptr;
			}
		}
	};

	template<typename... Args>
	class EventSignal
	{
		friend class EventSignalConnection<Args...>;
		std::unordered_map<size_t, Handler<Args...>> handlers_;
		size_t nextId_ = 1;

	public:
		EventSignalConnection<Args...> Connect(Handler<Args...> handler)
		{
			const size_t id = nextId_++;
			handlers_.emplace(id, std::move(handler));
			return EventSignalConnection<Args...>(this, id);
		}

		void Emit(Args... args)
		{
			const std::unordered_map<size_t, Handler<Args...>> snapshot = handlers_;
			for (const auto& entry : snapshot)
			{
				entry.second(args...);
			}
		}

	private:
		void Disconnect(size_t id)
		{
			handlers_.erase(id);
		}
	};
}
