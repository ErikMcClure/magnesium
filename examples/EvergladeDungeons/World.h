// Copyright ©2016 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in Everglade.h

#ifndef __WORLD_H__
#define __WORLD_H__

#include "Everglade.h"
#include "mgBox2D.h"

// Each sector is a tilemap and a physics body made of a list of b2EdgeShapes. This allows box2D to use it's broad phase detection on the map chunks.
struct Sector : magnesium::b2PhysicsComponent {

};
// A map is simply a collection of sectors and interactable map entities.
struct Map  {
  bss_util::cHash<std::pair<int64_t, int64_t>, Sector> Sectors;
  // Entities
  // Enemies
  // NPCs
};
struct Floor {
  Map m;
  // Map exit
};
struct Dungeon {

};
struct Overworld {
  Map m;
  // Outposts (can be colonized or not colonized, in which case it's just a dungeon entrance)
  
};
#endif
