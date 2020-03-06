#include <iostream>
#include <boost/msm/back/state_machine.hpp>

#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/static_assert.hpp>

namespace msm = boost::msm;
namespace msmf = boost::msm::front;
namespace mpl = boost::mpl;

// ----- Events
struct ev1 {
};
struct ev2 {
};
struct ev3 {
};
struct stop {
};
struct recover {
};

// ----- State machine
struct YourSystem_ : msmf::state_machine_def<YourSystem_> {
  struct Normal_ : msmf::state_machine_def<Normal_> {

    std::function<void()> func;

    void setCallback(std::function<void()> f)
    {
      func = f;
    }

    template <class Event, class Fsm>
    void on_entry(Event const&, Fsm&) const
    {
      std::cout << "Normal::on_entry()" << std::endl;
    }
    template <class Event, class Fsm>
    void on_exit(Event const&, Fsm&) const
    {
      std::cout << "Normal::on_exit()" << std::endl;
    }

    struct Idle : msmf::state<> {
      template <class Event, class Fsm>
      void on_entry(Event const&, Fsm&) const
      {
        std::cout << "Idle::on_entry()" << std::endl;
      }
      template <class Event, class Fsm>
      void on_exit(Event const&, Fsm&) const
      {
        std::cout << "Idle::on_exit()" << std::endl;
      }
    };

    struct Work : msmf::state<> {
      template <class Event, class Fsm>
      void on_entry(Event const&, Fsm&) const
      {
        std::cout << "Work::on_entry()" << std::endl;
      }
      template <class Event, class Fsm>
      void on_exit(Event const&, Fsm&) const
      {
        std::cout << "Work::on_exit()" << std::endl;
      }
    };

    struct InternalWork : msmf::state<> {
      template <class Event, class Fsm>
      void on_entry(Event const&, Fsm&) const
      {
        std::cout << "InternalWork::on_entry()" << std::endl;
      }
      template <class Event, class Fsm>
      void on_exit(Event const&, Fsm&) const
      {
        std::cout << "InternalWork::on_exit()" << std::endl;
      }
    };

    struct AllOk : msmf::state<> {
      template <class Event, class Fsm>
      void on_entry(Event const&, Fsm&) const
      {
        std::cout << "AllOk::on_entry()" << std::endl;
      }
      template <class Event, class Fsm>
      void on_exit(Event const&, Fsm&) const
      {
        std::cout << "AllOk::on_exit()" << std::endl;
      }
    };

    struct Crash {
      template <class EVT, class FSM, class SourceState, class TargetState>
      void operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
      {
        fsm.func();
      }
    };

    struct guard1 {
      template <class EVT, class FSM, class SourceState, class TargetState>
      bool operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
      {
        std::cout << "guard1 : Idle->Work ?\n";
        return true;
      }
    };
    struct guard2 {
      template <class EVT, class FSM, class SourceState, class TargetState>
      bool operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
      {
        std::cout << "guard2 : Work->InternalWork ?\n";
        return true;
      }
    };
    struct guard3 {
      template <class EVT, class FSM, class SourceState, class TargetState>
      bool operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
      {
        std::cout << "guard3 : InternalWork->AllOk ?\n";
        return true;
      }
    };
    struct guard4 {
      template <class EVT, class FSM, class SourceState, class TargetState>
      bool operator()(const EVT& evt, FSM& fsm, SourceState& src, TargetState& tgt)
      {
        std::cout << "guard4 : AllOK->Idle ?\n";
        return true;
      }
    };

    // Set initial state
    typedef mpl::vector<Idle> initial_state;
    // Transition table
    struct transition_table : mpl::vector<
                                //          Start      Event   Next       Action      Guard
                                msmf::Row<Idle, ev1, Work, msmf::none, guard1>,
                                msmf::Row<Work, msmf::none, InternalWork, msmf::none, guard2>,
                                msmf::Row<InternalWork, ev2, AllOk, Crash, guard3>,
                                msmf::Row<AllOk, ev3, Idle, msmf::none, guard4>> {
    };
  };
  struct EmergencyStopped : msmf::state<> {
    template <class Event, class Fsm>
    void on_entry(Event const&, Fsm&) const
    {
      std::cout << "EmergencyStopped::on_entry()" << std::endl;
    }
    template <class Event, class Fsm>
    void on_exit(Event const&, Fsm&) const
    {
      std::cout << "EmergencyStopped::on_exit()" << std::endl;
    }
  };

  typedef msm::back::state_machine<Normal_> Normal;

  // Set initial state
  typedef Normal initial_state;
  // Transition table
  struct transition_table : mpl::vector<
                              // clang-format off
                              //        Start             Event     Next               Action      Guard
                              msmf::Row<Normal          , stop    , EmergencyStopped  , msmf::none, msmf::none>,
                              msmf::Row<EmergencyStopped, recover , Normal, msmf::none, msmf::none>> {
    // clang-format on
  };
};

// Pick a back-end
typedef msm::back::state_machine<YourSystem_> Ys;

int main()
{
  Ys ys;
  ys.start();

  ys.get_state<YourSystem_::Normal&>().setCallback([&ys]() { std::cout << "callback called !\n"; ys.process_event(stop()); });

  std::cout << "> Send ev1()" << std::endl;
  ys.process_event(ev1());
  std::cout << "> Send ev2()" << std::endl;
  ys.process_event(ev2());

  // std::cout << "> Send stop()" << std::endl;
  // ys.process_event(stop());
  // std::cout << "> Send recover()" << std::endl;
  // ys.process_event(recover());
  std::cout << "> Send ev1()" << std::endl;
  ys.process_event(ev1());
  std::cout << "> Send ev3()" << std::endl;
  ys.process_event(ev3());
  std::cout << "> Send ev2()" << std::endl;
  ys.process_event(ev2());

  std::cout << "now ! > Send recover()" << std::endl;
  ys.process_event(recover());

  std::cout << "> Send ev1()" << std::endl;
  ys.process_event(ev1());
  std::cout << "> Send ev3()" << std::endl;
  ys.process_event(ev3());
  std::cout << "> Send ev2()" << std::endl;
  ys.process_event(ev2());
}
