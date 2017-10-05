// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __TIMELINE_H__MG__
#define __TIMELINE_H__MG__

#include "mg_dec.h"
#include "bss-util/Animation.h"
#include "bss-util/vector.h"
#include "bss-util/BlockAlloc.h"
#include "bss-util/Variant.h"

namespace magnesium {
  typedef bss::AniDataSmooth<bss::Vector<float, 3>> PositionAniData;
  typedef bss::AniDataSmooth<float> RotationAniData;
  typedef bss::AniDataSmooth<bss::Vector<float, 2>> PivotAniData;
  typedef bss::AniDataSmooth<bss::Vector<float, 2>> ScaleAniData;

  // Stores all animations 
  template<typename... Args>
  class MG_DLLEXPORT mgTimeline
  {
  public:
    struct AniDef
    {
      template<class T>
      AniDef(double t, T& ani) : Time(t), Animation(ani) {}

      double Time;
      bss::Variant<Args...> Animation;
    };

    // Stores the timeline execution state. This state cannot be copied because it is bound to specific instances - a copy constructor must be created in a derived class.
    struct State
    {
      State(State&& mov) : _cur(mov._cur), _time(mov._time), _timeline(mov._timeline), _states(std::move(mov._states)) { mov._timeline = 0; }
      explicit State(mgTimeline<Args...>* timeline) : _cur(0), _time(0.0), _timeline(timeline) {}
      ~State() { _wipe(); }

      void Reset()
      {
        for(auto& s : _states)
          s.first->Reset();
        _cur = 0;
        _time = 0.0;
      }

      bool Interpolate(double delta)
      {
        _time += delta;
        double length = _timeline->GetLength();
        double loop = _timeline->GetLoop();

        for(auto& s : _states)
        {
          double d = (_time - s.second->Time) - s.first->GetTime();
          if(d > 0.0)
            s.first->Interpolate(d);
        }

        if(_time >= length && loop >= 0.0) // We do the loop check down here because we need to finish calling all the discrete values on the end of the animation before looping
        {
          assert(length > 0.0); // If length is zero everything explodes
          double t = _time;
          Reset();
          _time = fmod(t - length, length - loop);// + loop; // instead of adding loop here we just pass loop into Interpolate, which adds it.
          return Interpolate(loop); // because we used fmod, it is impossible for this to result in another loop.
        }

        return _time < length;
      }

      template<class T>
      bool Bind(T* src, size_t id)
      {
        AniDef* def = _timeline->GetDef(id);
        if(!def)
          return false;
        _bind<T, Args...>(src, def);
        return true;
      }

      inline State& operator=(State&& mov)
      {
        _wipe();
        _states = std::move(mov._states);
        _cur = mov._cur;
        _time = mov._time;
        _timeline = mov._timeline;
        mov._timeline = 0;
        return *this;
      }

      inline State& operator=(const State& copy)
      {
        _states = copy._states;
        _cur = copy._cur;
        _time = copy._time;
        _timeline = copy._timeline;
      }

      inline mgTimeline<Args...>* GetTimeline() const { return _timeline; }

    protected:
      void _wipe()
      {
        for(auto& s : _states)
        {
          s.first->~AniStateBase();
          _timeline->_anistatealloc.Dealloc(s.first);
        }
      }
      template<class T>
      void _bindFrom(T* src, AniDef* def)
      {
        assert(def != 0);
        _bind<T, Args...>(src, def);
      }
      template<class T, class U, class... Ux>
      void _bind(T* src, AniDef* def)
      {
        typedef U::template State<T> STATE;
        if(def->Animation.is<U>())
        {
          STATE* p = _timeline->_anistatealloc.AllocT<STATE>(1);
          new (p) STATE(src, def->Animation.get<U>());
          _states.Add({ p, def });
        }
        else if constexpr(sizeof...(Ux) > 0)
          _bind<T, Ux...>(src, def);
        else
          assert(false);
      }

      size_t _cur;
      double _time;
      mgTimeline<Args...>* _timeline;
      bss::DynArray<std::pair<bss::AniStateBase*, AniDef*>> _states;
    };

    mgTimeline() : _length(-1.0) {}
    ~mgTimeline()
    {
      for(auto ani : _timeline)
      {
        ani->~AniDef();
        _defalloc.Dealloc(ani);
      }
    }
    inline void SetLength(double length = -1.0) { _length = length; }
    inline double GetLength() const { return _length < 0.0 ? _calc() : _length; }
    inline void SetLoop(double loop = 0.0) { _loop = loop; }
    inline double GetLoop() const { return _loop; }
    template<class T>
    inline void AddAnimation(double time, size_t id, T& animation)
    {
      AniDef* def = _defalloc.Alloc();
      new (def) AniDef(time, animation);
      _idmap.Insert(id, def);
      _timeline.Insert(def);
    }
    inline bool RemoveAnimation(size_t id)
    {
      if(AniDef* def = _idmap[id])
      {
        _idmap.Remove(id);
        for(int i = 0; i < _timeline.Length(); ++i)
          if(_timeline[i] == def)
          {
            _timeline.Remove(i);
            break;
          }
        def->~AniDef();
        _defalloc.Dealloc(def);
        return true;
      }
      return false;
    }
    const AniDef* GetDef(size_t id) { return _idmap[id]; }

    static char CompAniDef(AniDef* const& l, AniDef* const& r) { return SGNCOMPARE(l->time, r->time); }

  protected:
    inline double _calc() { !_timeline.Length() ? 0.0 : (_timeline.Back()->time + _timeline.Back()->animation.convertP<bss::AnimationBase>()->GetLength()); }

    bss::Hash<size_t, AniDef*> _idmap;
    bss::ArraySort<AniDef*, &CompAniDef> _timeline;
    double _length;
    bss::BlockAlloc<AniDef> _defalloc;
    bss::BlockAllocSize<bss::max_sizeof<typename Args::template State<bss::AniStateBase>...>::value, bss::max_alignof<typename Args::template State<bss::AniStateBase>...>::value> _anistatealloc;
  };
  
  // Given a list of concrete types, mgEffect instantiates an mgTimeline by constructing a set of images and binding them to the timeline
  template<typename T, typename... Args>
  class MG_DLLEXPORT mgEffect : public T::State
  {
    typedef typename T::State BASE;

    template<typename U, typename... Ux>
    BSS_FORCEINLINE void _bindvar(bss::Variant<Args...>& v, typename T::AniDef* def)
    {
      if(v.is<U>())
        _bindFrom<U>(&v.get<U>(), def);
      else if constexpr(sizeof...(Ux) > 0)
        _bindvar<Ux...>(v, def);
      else
        assert(false);
    }

  public:
    mgEffect(mgEffect&& mov) : BASE(std::move(mov)), _instances(std::move(mov._instances)) {}
    mgEffect(const mgEffect& copy) : BASE(copy._timeline), _instances(copy._instances)
    { // We can't clone the states - we have to rebind everything to the NEW concrete instances we just made
      _cur = copy._cur;
      _time = copy._time;
      _states.SetCapacity(copy._states.Length());
      for(size_t i = 0; i < _instances.Length(); ++i)
        _bindvar<Args...>(i, copy._states[i].second);
    }
    explicit mgEffect(T* timeline) : BASE(timeline) {}
    template<typename U, typename... X>
    BSS_FORCEINLINE void Add(size_t id, X... args)
    {
      _instances.AddConstruct<U>(U(args...));
      Bind<U>(&_instances.Back().Get<U>(), id);
    }

    mgEffect& operator=(const mgEffect& copy)
    {
      _wipe();
      _instances = copy._instances;
      _cur = copy._cur;
      _time = copy._time;
      _timeline = copy._timeline;
      _states.SetCapacity(copy._states.Length());
      for(size_t i = 0; i < _instances.Length(); ++i)
        _bindvar<Args...>(i, copy._states[i].second);
      return *this;
    }

    mgEffect& operator=(mgEffect&& mov)
    {
      BASE::operator=(std::move(mov));
      _instances = std::move(mov._instances);
      return *this;
    }

  protected:
    bss::DynArray<bss::Variant<Args...>> _instances;
  };
}

#endif