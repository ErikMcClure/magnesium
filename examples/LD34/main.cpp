/* LD34
*
* Copyright ©2016 Black Sphere Studios
*/

#include "Magnesium.h"
#include "mgPlaneshader.h"
#include "mgTinyOAL.h"
#include "mgJoinPSB2.h"
#include "mgLogic.h"
#include "Box2D/Box2D.h"
#include "planeshader/psPass.h"

using namespace bss_util;
using namespace magnesium;
using namespace planeshader;
using namespace tinyoal;

#ifdef BSS_COMPILER_MSC
#ifdef BSS_DEBUG
#define BSS_LIBDEBUG "_d"
#else
#define BSS_LIBDEBUG 
#endif
#ifndef BSS_CPU_x86_64
#define BSS_LIB32 "32"
#else
#define BSS_LIB32 
#endif

#ifdef MG_STATIC_LIB
#pragma comment(lib, "../../bin/Magnesium" BSS_LIB32 "_s" BSS_LIBDEBUG ".lib")
#else
#pragma comment(lib, "../../bin/Magnesium" BSS_LIB32 BSS_LIBDEBUG ".lib")
#endif
#pragma comment(lib, "../../lib/PlaneShader" BSS_LIB32 BSS_LIBDEBUG  ".lib")
#pragma comment(lib, "../../lib/bss-util" BSS_LIB32 BSS_LIBDEBUG  ".lib")
#pragma comment(lib, "../../lib/liquidfun" BSS_LIB32 BSS_LIBDEBUG  ".lib")
#pragma comment(lib, "../../lib/TinyOAL" BSS_LIB32 BSS_LIBDEBUG  ".lib")
#endif

#pragma warning(disable:4244)

psCamera globalcam;
bool dirkeys[11] = { false }; // left right up down in out counterclockwise clockwise shift
float dirspeeds[4] = { 600.0f, 600.0f, 2.0f, 1.0f }; // X Y Z R
psRect maxcam;

float mgclamp(float x, float min, float max) { return bssclamp(x, min, max); }

void LoadMap(psTex* map, psTileset* tileset, mgEntity* object, psTex* key)
{
  psVeciu dim = map->GetRawDim();
  DYNARRAY(psTile, tiles, dim.x*dim.y);
  memset(tiles, 0, sizeof(psTile)*dim.x*dim.y);

  psVeciu keydim = key->GetRawDim();
  DYNARRAY(psColor32, keys, keydim.x*keydim.y);
  auto pixels = key->LockPixels(LOCK_READ);
  for(uint32_t j = 0; j < keydim.y; ++j)
    for(uint32_t i = 0; i < keydim.x; ++i)
      keys[i + (j*keydim.x)] = pixels[j][i];
      
  pixels = map->LockPixels(LOCK_READ);
  for(uint32_t j = 0; j < dim.y; ++j)
    for(uint32_t i = 0; i < dim.x; ++i)
    {
      psTile& t = tiles[i + (j*dim.x)];
      t.color = 0;
      t.index = 0;
      for(uint32_t k = 0; k < keydim.x*keydim.y; ++k)
        if(keys[k] == pixels[j][i])
        {
          t.color = 0xFFFFFFFF;
          t.index = k;
        }
    }

  tileset->SetTiles(tiles, dim.x*dim.y, dim.x);
  psVec sdim = psEngine::Instance()->GetDriver()->screendim*0.25f;
  maxcam.right = maxcam.left + std::max<float>(0, tileset->GetDim().x - sdim.x);
  maxcam.bottom = maxcam.top + std::max<float>(0, tileset->GetDim().y - sdim.y);

  // Define the ground body.
  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(0.0f, 0.0f);
  object->Get<b2PhysicsComponent>()->Init(groundBodyDef);

  // Define the ground box shape.
  b2PolygonShape groundBox;
  b2Vec2 polypoints[4];

  for(uint32_t j = 0; j < dim.y; ++j)
    for(uint32_t i = 0; i < dim.x; ++i)
    {
      psTile& t = tiles[i + (j*dim.x)];
      if(!t.color) continue;
      switch(t.index)
      {
      default:
      case 0:
        polypoints[0] = b2Vec2(i, j);
        polypoints[1] = b2Vec2(i+1, j);
        polypoints[2] = b2Vec2(i, j+1);
        polypoints[3] = b2Vec2(i+1, j+1);
        groundBox.Set(polypoints, 4);
        break;
      case 1:
        polypoints[0] = b2Vec2(i + 1, j);
        polypoints[1] = b2Vec2(i, j + 1);
        polypoints[2] = b2Vec2(i + 1, j + 1);
        groundBox.Set(polypoints, 3);
        break;
      case 2:
        polypoints[0] = b2Vec2(i, j);
        polypoints[1] = b2Vec2(i, j + 1);
        polypoints[2] = b2Vec2(i + 1, j + 1);
        groundBox.Set(polypoints, 3);
        break;
      case 5:
        polypoints[0] = b2Vec2(i, j);
        polypoints[1] = b2Vec2(i + 1, j);
        polypoints[2] = b2Vec2(i + 1, j + 1);
        groundBox.Set(polypoints, 3);
        break;
      case 6:
        polypoints[0] = b2Vec2(i, j);
        polypoints[1] = b2Vec2(i + 1, j);
        polypoints[2] = b2Vec2(i, j + 1);
        groundBox.Set(polypoints, 3);
        break;
      }
      object->Get<b2PhysicsComponent>()->AddCompoundFixture(groundBox, 0.0f);
    }

}

void BuildImageBox(mgEntity* e, float x = 0, float y = 0)
{
  auto phys = e->Get<b2PhysicsComponent>();
  auto img = e->Get<psImageComponent>();

  if(!phys->GetBody())
  {
    b2BodyDef groundBodyDef;
    groundBodyDef.type = b2_dynamicBody;
    groundBodyDef.position.Set(x, y);
    phys->Init(groundBodyDef);
  }
  psVec dim = img->GetDim()*Box2DSystem::Instance()->INV_PPM;

  b2PolygonShape box;
  b2Vec2 polypoints[4];
  polypoints[0] = b2Vec2(0, 0);
  polypoints[1] = b2Vec2(dim.x, 0);
  polypoints[2] = b2Vec2(0, dim.y);
  polypoints[3] = b2Vec2(dim.x, dim.y);
  box.Set(polypoints, 4);
  phys->AddCompoundFixture(box, 0.0f);
}

bool IsOnFloor(mgEntity* e)
{
  auto phys = e->Get<b2PhysicsComponent>();
  b2ContactEdge* c = phys->GetBody()->GetContactList();
  b2WorldManifold m;

  while(c)
  {
    if(c->contact->IsTouching())
    {
      c->contact->GetWorldManifold(&m);
      m.normal.Normalize();
      if(m.normal.y < -0.2)
        return true;
    }
    c = c->next;
  }
  return false;
}

psTex* LoadPointImg(const char* src)
{
  return psTex::Create(src, USAGE_SHADER_RESOURCE, FILTER_TRIANGLE, 0, FILTER_NONE, false, STATEBLOCK_LIBRARY::POINTSAMPLE);
}

struct HealthComponent : mgComponent<HealthComponent>
{
  HealthComponent(mgEntity* e) : mgComponent(e), maxhealth(0), health(0), onmodify(0) {}
  void Modify(int amt) { health += bssclamp(amt, -health, maxhealth - health); if(onmodify) onmodify(entity, this, amt); }
  int maxhealth;
  int health;
  void(*onmodify)(mgEntity*, HealthComponent*, int);
};

class Player : public mgEntityT<psImageComponent, b2PhysicsComponent, mgLogicComponent, HealthComponent>
{
public:
  Player(psTex* img, int maxhealth)
  {
    auto playerimg = Get<psImageComponent>();
    playerimg->SetTexture(img);
    playerimg->SetPass();

    BuildImageBox(this, 3, 15);
    Get<b2PhysicsComponent>()->GetRootFixture()->SetFriction(0);

    auto health = Get<HealthComponent>();
    health->health = maxhealth;
    health->maxhealth = maxhealth;
    health->onmodify = &onmodifyhealth;
  }
  static void onmodifyhealth(mgEntity* e, HealthComponent* c, int amt)
  {
    if(c->health <= 0)
      e->Get<psImageComponent>()->SetPass(0);
  }
  COMPONENT_REF(HealthComponent) Health() { return Get<HealthComponent>(); }
  COMPONENT_REF(b2PhysicsComponent) Physics() { return Get<b2PhysicsComponent>(); }
  COMPONENT_REF(mgLogicComponent) Logic() { return Get<mgLogicComponent>(); }
};

class BackgroundGen : public mgEntityT<RawRenderableComponent>
{
public:
  BackgroundGen(uint64_t seed)
  {
    xorshift_engine<uint64_t> engine;
    std::uniform_real_distribution<float> dx(10, 100);
    std::uniform_real_distribution<float> dz(1, 20);

    Get<RawRenderableComponent>()->render = [](mgEntity* e) {
      BackgroundGen* bg = static_cast<BackgroundGen*>(e);

    };
  }

  cDynArray<psVec3D> _buildings;
};

int main(int argc, char** argv)
{
  SetWorkDirToCur();
  PSINIT init;
  //init.width = 1024;
  //init.height = 768;
  init.mediapath = "../media/shaders";
  Box2DSystem::B2INIT b2init;
  b2init.ppm = 16;
  b2init.gravity[1] = 27.0f;
#ifdef BSS_DEBUG
  b2init.hertz = 0;
#endif
  PlaneshaderSystem ps(init, 0);
  TinyOALSystem tinyoal;
  LiquidFunPlaneshaderSystem psbox2d(b2init, -1);
  LogicSystem logic;

  mgEngine engine;
  engine.AddSystem(&ps);
  engine.AddSystem(&tinyoal);
  engine.AddSystem(&psbox2d);
  engine.AddSystem(&logic);

  psDebugDraw psdd;
  ps[0].Insert(&psdd);
  psbox2d.SetDebugDraw(&psdd);

  maxcam.left = -ps.GetDriver()->screendim.x*0.375; // 3/8
  maxcam.top = -ps.GetDriver()->screendim.y*0.375;
  //globalcam.SetPosition(maxcam.left, maxcam.top, -0.25f);
  globalcam.SetExtent(psVec(0.2, 50000));
  ps[0].SetCamera(&globalcam);
  ps[0].SetClearColor(0xFF111122);
  ps[0].GetDriver()->SetDPI(psVeciu(psDriver::BASE_DPI * 4));

  mgEntityT<psTilesetComponent, b2PhysicsComponent> map;
  auto tsmap = map.Get<psTilesetComponent>();
  tsmap->SetTexture(LoadPointImg("../media/LD34/tile1.png"));
  tsmap->AutoGenDefs(psVec(16, 16));
  tsmap->SetPass(&ps[0]);

  LoadMap(psTex::Create("../media/LD34/testmap.png", USAGE_STAGING), tsmap, &map, psTex::Create("../media/LD34/key.png", USAGE_STAGING));
  
  Player* player = new Player(LoadPointImg("../media/LD34/player.png"), 100);

  std::function<bool(const psGUIEvent&)> guicallback = [&](const psGUIEvent& evt) -> bool
  {
    if(evt.type == GUI_KEYDOWN || evt.type == GUI_KEYUP)
    {
      bool isdown = evt.type == GUI_KEYDOWN;
      dirkeys[8] = evt.IsShiftDown();
      switch(evt.keycode)
      {
      case KEY_LEFT: dirkeys[0] = isdown; break;
      case KEY_RIGHT: dirkeys[1] = isdown; break;
      case KEY_UP: dirkeys[2] = isdown; break;
      case KEY_DOWN: dirkeys[3] = isdown; break;
      case KEY_X:
      case KEY_OEM_PLUS: dirkeys[4] = isdown; break;
      case KEY_Z:
      case KEY_OEM_MINUS: dirkeys[5] = isdown; break;
      case KEY_Q: dirkeys[6] = isdown; break;
      case KEY_E: dirkeys[7] = isdown; break;
      case KEY_A: dirkeys[10] = isdown; break;
      case KEY_D: dirkeys[11] = isdown; break;
      case KEY_RETURN: player->Health()->Modify(-10); break;
      case KEY_W:
        if(IsOnFloor(player))
        {
          const float jump = 0.5f;
          b2Body* b = player->Physics()->GetBody();
          b->SetLinearVelocity(b2Vec2(b->GetLinearVelocity().x, 0));
          b->ApplyLinearImpulse(b2Vec2(0, -jump*psbox2d.GetGravity().y), b->GetLocalCenter(), true);
        }
      break;
      case KEY_ESCAPE:
        if(isdown) ps.Quit();
        break;
      }
    }
    return false;
  };
  ps.SetInputReceiver(guicallback);

  player->Get<mgLogicComponent>()->onlogic = [](mgEntity* e) {
    const float maxspeed = 8.0f;
    b2PhysicsComponent* phys = e->Get<b2PhysicsComponent>();
    if(dirkeys[10])
      phys->GetBody()->ApplyForceToCenter(b2Vec2(mgclamp(-maxspeed - phys->GetBody()->GetLinearVelocity().x, -maxspeed, 0), 0), true);
    if(dirkeys[11])
      phys->GetBody()->ApplyForceToCenter(b2Vec2(mgclamp(maxspeed - phys->GetBody()->GetLinearVelocity().x, 0, maxspeed), 0), true);
    if(!dirkeys[10] && !dirkeys[11])
      phys->GetBody()->ApplyForceToCenter(b2Vec2(mgclamp(-phys->GetBody()->GetLinearVelocity().x, -maxspeed*0.5f, maxspeed*0.5f), 0), true);
  };

  logic.preprocess = [&]() {
    float secdelta = mgEngine::Instance()->GetDeltaNS() / 1000000000.0f;
    float scale = dirkeys[8] ? 0.01f : 1.0f;
    psLocatable* loc = ps.ResolveComponent<psLocatableComponent>(player);
    psVec sdim = psEngine::Instance()->GetDriver()->screendim;
    globalcam.SetPositionX(mgclamp(loc->GetPosition().x - sdim.x*0.5, maxcam.left, maxcam.right));
    globalcam.SetPositionY(mgclamp(loc->GetPosition().y - sdim.y*0.5, maxcam.top, maxcam.bottom));
    /*if(dirkeys[0])
      globalcam.SetPositionX(mgclamp(globalcam.GetPosition().x - dirspeeds[0] * secdelta * scale, maxcam.left, maxcam.right));
    if(dirkeys[1])
      globalcam.SetPositionX(mgclamp(globalcam.GetPosition().x + dirspeeds[0] * secdelta * scale, maxcam.left, maxcam.right));
    if(dirkeys[2])
      globalcam.SetPositionY(mgclamp(globalcam.GetPosition().y - dirspeeds[1] * secdelta * scale, maxcam.top, maxcam.bottom));
    if(dirkeys[3])
      globalcam.SetPositionY(mgclamp(globalcam.GetPosition().y + dirspeeds[1] * secdelta * scale, maxcam.top, maxcam.bottom));
    if(dirkeys[4])
      globalcam.SetPositionZ(globalcam.GetPosition().z + dirspeeds[2] * secdelta * scale);
    if(dirkeys[5])
      globalcam.SetPositionZ(globalcam.GetPosition().z - dirspeeds[2] * secdelta * scale);
    if(dirkeys[6])
      globalcam.SetRotation(globalcam.GetRotation() + dirspeeds[3] * secdelta * scale);
    if(dirkeys[7])
      globalcam.SetRotation(globalcam.GetRotation() - dirspeeds[3] * secdelta * scale);*/
  };

  BackgroundGen bg(0);

  engine.ResetDelta();
  engine.ResetTime();
  while(!ps.GetQuit())
  {
    engine.Process();
  }
}

struct HINSTANCE__;

// WinMain function, simply a catcher that calls the main function
int __stdcall WinMain(HINSTANCE__* hInstance, HINSTANCE__* hPrevInstance, char* lpCmdLine, int nShowCmd)
{
  main(0, (char**)hInstance);
}