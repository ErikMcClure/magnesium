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
  typedef bss::AniDataSmooth<bss::Vector<float, 3>, bss::AniLinearData<bss::Vector<float, 3>>, bss::ARRAY_CONSTRUCT> PositionAniData;
  typedef bss::AniDataSmooth<float, bss::AniLinearData<float>, bss::ARRAY_CONSTRUCT> RotationAniData;
  typedef bss::AniDataSmooth<bss::Vector<float, 2>, bss::AniLinearData<bss::Vector<float, 2>>, bss::ARRAY_CONSTRUCT, 1> PivotAniData;
  typedef bss::AniDataSmooth<bss::Vector<float, 2>, bss::AniLinearData<bss::Vector<float, 2>>, bss::ARRAY_CONSTRUCT> ScaleAniData;

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
    struct MG_DLLEXPORT State
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

      template<class T> // T here is the AniState specialization to use
      T* Bind(typename T::Ty* src, size_t id)
      {
        const AniDef* def = _timeline->GetDef(id);
        if(!def)
          return 0;
        return _bind<T, Args...>(src, def);
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

      inline mgTimeline<Args...>* GetTimeline() const { return _timeline; }
      inline void SetTimeline(mgTimeline<Args...>* timeline) { _timeline = timeline; }

    protected:
      void _wipe()
      {
        for(auto& s : _states)
        {
          s.first->~AniStateBase();
          _timeline->_anistatealloc.deallocate<void>(s.first);
        }
        _states.Clear();
      }
      template<class T>
      T* _bindFrom(typename T::Ty* src, const AniDef* def)
      {
        assert(def != 0);
        return _bind<T, Args...>(src, def);
      }
      template<class T, class U, class... Ux> // T here is an AniState type
      T* _bind(typename T::Ty* src, const AniDef* def)
      {
        if(def->Animation.is<U>())
        {
          T* p = _timeline->_anistatealloc.allocate<T>(1);
          new (p) T(src, &def->Animation.get<U>());
          _states.Add({ p, def });
          return p;
        }
        else if constexpr(sizeof...(Ux) > 0)
          return _bind<T, Ux...>(src, def);
        else
        {
          assert(false);
          return 0;
        }
      }

      size_t _cur;
      double _time;
      mgTimeline<Args...>* _timeline;
      bss::DynArray<std::pair<bss::AniStateBase*, const AniDef*>> _states;
    };

    mgTimeline() : _length(-1.0), _loop(-1.0) {}
    mgTimeline(mgTimeline&& mov) : _idmap(std::move(mov._idmap)), _timeline(std::move(mov._timeline)),
      _anistatealloc(std::move(mov._anistatealloc)), _length(mov._length), _loop(mov._loop)
    {
      mov._length = -1.0;
      mov._loop = -1.0;
    }
    ~mgTimeline() {}
    inline void SetLength(double length = -1.0) { _length = length; }
    inline double GetLength() const { return _length < 0.0 ? _calc() : _length; }
    inline void SetLoop(double loop = 0.0) { _loop = loop; }
    inline double GetLoop() const { return _loop; }
    template<class T>
    inline void AddAnimation(double time, size_t id, T& animation)
    {
      AniDef* def = new AniDef(time, animation);
      _idmap.Insert(id, std::unique_ptr<AniDef>(def));
      _timeline.Insert(def);
    }
    inline bool RemoveAnimation(size_t id)
    {
      if(AniDef* def = _idmap[id])
      {
        for(int i = 0; i < _timeline.Length(); ++i)
          if(_timeline[i] == def)
          {
            _timeline.Remove(i);
            break;
          }
        _idmap.Remove(id);
        return true;
      }
      return false;
    }
    const AniDef* GetDef(size_t id) { return _idmap[id]; }

    static char CompAniDef(AniDef* const& l, AniDef* const& r) { return SGNCOMPARE(l->Time, r->Time); }

    inline mgTimeline& operator=(mgTimeline&& mov)
    {
      _idmap = std::move(mov._idmap);
      _timeline = std::move(mov._timeline);
      _anistatealloc = std::move(mov._anistatealloc);
      _length = mov._length;
      _loop = mov._loop;
      mov._length = -1.0;
      mov._loop = -1.0;
      return *this;
    }

  protected:
    inline double _calc() const { return !_timeline.Length() ? 0.0 : (_timeline.Back()->Time + _timeline.Back()->Animation.convertP<bss::AnimationBase>()->GetLength()); }

    bss::Hash<size_t, std::unique_ptr<AniDef>, bss::ARRAY_MOVE> _idmap;
    bss::ArraySort<AniDef*, &CompAniDef> _timeline;
    double _length;
    double _loop;
    bss::BlockPolicySize<bss::max_args(Args::STATESIZE...), bss::max_args(Args::STATEALIGN...)> _anistatealloc;
  };

  // Given a list of AniState objects, mgEffect instantiates an mgTimeline by constructing the corresponding concrete types and binding them to the timeline
  template<typename T, typename... Args>
  class MG_DLLEXPORT mgEffect : public T::State
  {
    typedef typename T::State BASE;

    template<typename U, typename... Ux>
    BSS_FORCEINLINE void _bindvar(bss::Variant<typename Args::Ty...>& v, typename const T::AniDef* def)
    {
      if(v.is<U>())
        _bindFrom<U>(&v.get<typename U::Ty>(), def);
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
        _bindvar<Args...>(_instances[i], copy._states[i].second);
    }
    explicit mgEffect(T* timeline) : BASE(timeline) {}
    template<typename U, typename... X>
    BSS_FORCEINLINE U* Add(size_t id, X... args)
    {
      _instances.AddConstruct<typename U::Ty>(typename U::Ty(args...));
      return Bind<U>(&_instances.Back().get<typename U::Ty>(), id);
    }
    BSS_FORCEINLINE bss::Variant<typename Args::Ty...>& operator[](size_t index) { return _instances[index]; }

    mgEffect& operator=(const mgEffect& copy)
    {
      _wipe();
      _instances = copy._instances;
      _cur = copy._cur;
      _time = copy._time;
      _timeline = copy._timeline;
      _states.SetCapacity(copy._states.Length());
      for(size_t i = 0; i < _instances.Length(); ++i)
        _bindvar<Args...>(_instances[i], copy._states[i].second);
      return *this;
    }

    mgEffect& operator=(mgEffect&& mov)
    {
      BASE::operator=(std::move(mov));
      _instances = std::move(mov._instances);
      return *this;
    }

  protected:
    bss::DynArray<bss::Variant<typename Args::Ty...>> _instances;
  };
}

#endif