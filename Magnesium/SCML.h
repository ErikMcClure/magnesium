// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Magnesium.h

#ifndef __SCML_H__MG__
#define __SCML_H__MG__

#include "bss-util/XML.h"
#include "bss-util/Animation.h"
#include "bss-util/literals.h"
#include "mgTimeline.h"

namespace magnesium {
  namespace scml {
    typedef bss::Animation<PositionAniData, RotationAniData, PivotAniData, ScaleAniData> spriterAnimation;
    template<class T, void (T::*POSFN)(const bss::Vector<float, 3>&), void (T::*ROTFN)(float), void (T::*PIVOTFN)(const bss::Vector<float, 2>&), void (T::*SCALEFN)(const bss::Vector<float, 2>&)>
    using spriterAniState = bss::AniState<T, spriterAnimation,
      bss::AniStateSmooth<T, PositionAniData, const bss::Vector<float, 3>&, POSFN>,
      bss::AniStateSmooth<T, RotationAniData, float, ROTFN>,
      bss::AniStateSmooth<T, PivotAniData, const bss::Vector<float, 2>&, PIVOTFN>,
      bss::AniStateSmooth<T, ScaleAniData, const bss::Vector<float, 2>&, SCALEFN>>;

    struct File
    {
      File() : id(~0_sz) { pivot[0] = 0; pivot[1] = 1; }

      size_t id;
      bss::Str name;
      float width;
      float height;
      float pivot[2];

      template<typename Engine>
      void Serialize(bss::Serializer<Engine>& e, const char*)
      {
        e.template EvaluateType<File>(
          bss::GenPair("id", id),
          bss::GenPair("name", name),
          bss::GenPair("width", width),
          bss::GenPair("height", height),
          bss::GenPair("pivot_x", pivot[0]),
          bss::GenPair("pivot_y", pivot[1])
          );
      }
    };

    struct Folder
    {
      Folder() : id(~0_sz) {}

      size_t id;
      bss::Str name;
      bss::DynArray<File> files;

      template<typename Engine>
      void Serialize(bss::Serializer<Engine>& e, const char*)
      {
        e.template EvaluateType<Folder>(
          bss::GenPair("id", id),
          bss::GenPair("name", name),
          bss::GenPair("file", files)
          );
      }
    };

    struct MapInstruction
    {
      MapInstruction() : tarFolder(-1), tarFile(-1) {}

      int folder;
      int file;
      int tarFolder;
      int tarFile;

      template<typename Engine>
      void Serialize(bss::Serializer<Engine>& e, const char*)
      {
        e.template EvaluateType<MapInstruction>(
          bss::GenPair("folder", folder),
          bss::GenPair("file", file),
          bss::GenPair("tarFolder", tarFolder),
          bss::GenPair("tarFile", tarFile)
          );
      }
    };

    struct CharacterMap
    {
      size_t id;
      bss::Str name;
      bss::DynArray<MapInstruction> maps;

      template<typename Engine>
      void Serialize(bss::Serializer<Engine>& e, const char*)
      {
        e.template EvaluateType<CharacterMap>(
          bss::GenPair("id", id),
          bss::GenPair("name", name),
          bss::GenPair("map", maps)
          );
      }
    };

    struct Ref
    {
      Ref() : id(~0_sz), parent(-1) {}

      size_t id;
      int parent;
      int timeline;
      int key;
      int z_index;

      template<typename Engine>
      void Serialize(bss::Serializer<Engine>& e, const char*)
      {
        e.template EvaluateType<Ref>(
          bss::GenPair("id", id),
          bss::GenPair("parent", parent),
          bss::GenPair("timeline", timeline),
          bss::GenPair("key", key),
          bss::GenPair("z_index", z_index)
          );
      }
    };

    struct Mainline
    {
      struct Key {
        Key() : id(~0_sz), time(0) {}

        size_t id;
        int time;
        bss::DynArray<Ref> boneRefs;
        bss::DynArray<Ref> objectRefs;

        template<typename Engine>
        void Serialize(bss::Serializer<Engine>& e, const char*)
        {
          e.template EvaluateType<Key>(
            bss::GenPair("id", id),
            bss::GenPair("time", time),
            bss::GenPair("bone_ref", boneRefs),
            bss::GenPair("object_ref", objectRefs)
            );
        }
      };

      bss::DynArray<Key> keys;

      template<typename Engine>
      void Serialize(bss::Serializer<Engine>& e, const char*)
      {
        e.template EvaluateType<Key>(bss::GenPair("key", keys))
      }
    };

    struct TimelineKey
    {
      TimelineKey() : id(~0_sz), time(0), curveType(0), c1(NAN), c2(NAN), spin(1) {}

      struct SpatialKey {
        SpatialKey() : x(0), y(0), angle(0), a(1) { scale[0] = 1; scale[1] = 1; }

        float x;
        float y;
        float angle;
        float scale[2];
        float a;
      };

      size_t id;
      int time;
      int curveType;
      float c1;
      float c2;
      int spin;
      
      union
      {
        struct BoneTimelineKey : SpatialKey
        {
          BoneTimelineKey() : length(200), height(10) {}

          int length;
          int height;

          template<typename Engine>
          void Serialize(bss::Serializer<Engine>& e, const char*)
          {
            e.template EvaluateType<MainlineKey>(
              bss::GenPair("x", x),
              bss::GenPair("y", y),
              bss::GenPair("angle", angle),
              bss::GenPair("scaleX", curveType),
              bss::GenPair("scaleY", c1),
              bss::GenPair("a", a),
              bss::GenPair("length", length),
              bss::GenPair("height", height)
              );
          }
        } bone;

        struct SpriteTimelineKey : SpatialKey
        {
          SpriteTimelineKey() { pivot[0] = NAN; pivot[1] = NAN; }
          int folder; // index of the folder within the ScmlObject
          int file;
          float pivot[2]; // Will be NaN if the default should be used

          template<typename Engine>
          void Serialize(bss::Serializer<Engine>& e, const char*)
          {
            e.template EvaluateType<MainlineKey>(
              bss::GenPair("x", x),
              bss::GenPair("y", y),
              bss::GenPair("angle", angle),
              bss::GenPair("scaleX", curveType),
              bss::GenPair("scaleY", c1),
              bss::GenPair("a", a),
              bss::GenPair("folder", folder),
              bss::GenPair("file", file),
              bss::GenPair("pivot_x", pivot[0]),
              bss::GenPair("pivot_y", pivot[1])
              );
          }
        } sprite;
      };

      template<typename Engine>
      void Serialize(bss::Serializer<Engine>& e, const char*)
      {
        e.template EvaluateType<MainlineKey>(
          bss::GenPair("id", id),
          bss::GenPair("time", time),
          bss::GenPair("spin", spin),
          bss::GenPair("curve_type", curveType),
          bss::GenPair("c1", c1),
          bss::GenPair("c2", c2),
          bss::GenPair("bone", bone),
          bss::GenPair("object", sprite)
          );
      }
    };

    struct Timeline
    {
      Timeline() : id(~0_sz), objectType("sprite") {}

      size_t id;
      bss::Str name;
      bss::Str objectType;
      bss::DynArray<TimelineKey> keys;

      template<typename Engine>
      void Serialize(bss::Serializer<Engine>& e, const char*)
      {
        e.template EvaluateType<Timeline>(
          bss::GenPair("id", id),
          bss::GenPair("name", name),
          bss::GenPair("type", objectType),
          bss::GenPair("key", keys)
          );
      }
    };

    struct Animation
    {
      Animation() : id(~0_sz), looping(true) {}

      size_t id;
      bss::Str name;
      int length;
      bool looping;
      Mainline mainline;
      bss::DynArray<Timeline> timelines;

      template<typename Engine>
      void Serialize(bss::Serializer<Engine>& e, const char*)
      {
        e.template EvaluateType<Animation>(
          bss::GenPair("id", id),
          bss::GenPair("name", name),
          bss::GenPair("length", length),
          bss::GenPair("looping", looping),
          bss::GenPair("mainline", mainlineKeys),
          bss::GenPair("timeline", timelines)
          );
      }
    };

    struct Entity
    {
      Entity() : id(~0_sz) {}

      size_t id;
      bss::Str name;
      bss::DynArray<CharacterMap> characterMaps;
      bss::DynArray<Animation> animations;

      template<typename Engine>
      void Serialize(bss::Serializer<Engine>& e, const char*)
      {
        e.template EvaluateType<Entity>(
          bss::GenPair("id", id),
          bss::GenPair("name", name),
          bss::GenPair("character_map", characterMaps),
          bss::GenPair("animation", animations)
          );
      }
    };

    struct Object
    {
      bss::DynArray<Folder> folders;
      bss::DynArray<Entity> entities;

      template<typename Engine>
      void Serialize(bss::Serializer<Engine>& e, const char*)
      {
        e.template EvaluateType<Object>(
          bss::GenPair("folder", folders),
          bss::GenPair("entity", entities)
          );
      }
    };
  }

  template<template... Args>
  void LoadSCML(scml::Animation& in, mgTimeline<Args...>& out, scml::Object& ref)
  {
    out.SetLength(in.length);
    if(in.looping)
      out.SetLoop(0.0);
    for(auto& t : in.timelines)
    {
      if(!stricmp(t.objectType.c_str(), "bone"))
        assert(false);
      else // Default to "sprite"
      {
        for(auto& k : t.keys)
        {
          k.
        }
      }
    }
  }
}

#endif