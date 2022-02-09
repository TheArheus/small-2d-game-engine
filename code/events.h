#if !defined EVENTS_H_

class event
{
public:
    event() = default;
};

class event_callback_
{
private:
    virtual void Call(event& Event) = 0;
public:
    virtual ~event_callback_() = default;
    void Execute(event& Event)
    {
        Call(Event);
    }
};

template<typename owner_type, typename event_type>
class event_callback : public event_callback_
{
private:
    typedef void (owner_type::*callback_function)(event_type&);

    owner_type* OwnerInstance;
    callback_function Callback;

    void Call(event& Event) override
    {
        std::invoke(Callback, OwnerInstance, static_cast<event_type&>(Event));
    }

public:
    event_callback(owner_type* OwnerInstance_, callback_function Callback_)
    {
        OwnerInstance = OwnerInstance_;
        Callback = Callback_;
    }

    void Execute(event& Event)
    {
        Call(Event);
    }
};

typedef std::list<std::unique_ptr<event_callback_>> handler_list;

class event_bus
{
private:
    std::map<std::type_index, std::unique_ptr<handler_list>> Subscribers;
public:
    event_bus()  = default;
    ~event_bus() = default;

    void Reset()
    {
        Subscribers.clear();
    }

    template<typename event_type, typename owner_type> 
    void SubscribeToEvent(owner_type* OwnerInstance, void (owner_type::*CallbackFunction)(event_type&))
    {
        if(!Subscribers[typeid(event_type)].get())
        {
            Subscribers[typeid(event_type)] = std::make_unique<handler_list>();
        }

        auto Subscriber = std::make_unique<event_callback<owner_type, event_type>>(OwnerInstance, CallbackFunction);
        Subscribers[typeid(event_type)]->push_back(std::move(Subscriber));
    }

    template<typename event_type, typename ...targs> 
    void EmitEvent(targs&& ...Args)
    {
        auto Handlers = Subscribers[typeid(event_type)].get();
        if(Handlers)
        {
            for(auto Iterator = Handlers->begin(); Iterator != Handlers->end(); ++Iterator)
            {
                event_callback_* Handler = Iterator->get();
                event_type Event(std::forward<targs>(Args)...);
                Handler->Execute(Event);
            }
        }
    }
};

class collision_event : public event
{
public:
    entity A;
    entity B;
    collision_event(entity A_, entity B_): A(A_), B(B_) {}
};

class key_pressed_event : public event
{
public:
    SDL_Keycode Symbol;

    key_pressed_event(SDL_Keycode Symbol_): Symbol(Symbol_) {};
};

class key_up_event : public event
{
public:
    SDL_Keycode Symbol;

    key_up_event(SDL_Keycode Symbol_): Symbol(Symbol_) {};
};

#define EVENTS_H_
#endif
