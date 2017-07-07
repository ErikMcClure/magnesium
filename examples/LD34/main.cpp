/* LD34
*
* Copyright ©2016 Black Sphere Studios
*/

#include "Magnesium.h"
#include "mgPlaneshader.h"
#include "mgTinyOAL.h"
#include "mgJoinPSB2.h"
#include "mgLogic.h"
#include "mgLua.h"
#include "mgConsole.h"
#include "Box2D/Box2D.h"
#include "planeshader/psPass.h"
#include "bss-util/RefCounter.h"
#include "feathergui/fgTextbox.h"

using namespace bss;
using namespace magnesium;
using namespace planeshader;
using namespace tinyoal;

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
  psVec sdim = psEngine::Instance()->GetDriver()->GetBackBuffer()->GetDim()*0.25f;
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

//class BackgroundGen : public mgEntityT<RawRenderableComponent>
//{
//public:
//  BackgroundGen(uint64_t seed)
//  {
//    xorshift_engine<uint64_t> engine;
//    std::uniform_real_distribution<float> dx(10, 100);
//    std::uniform_real_distribution<float> dz(1, 20);
//
//    Get<RawRenderableComponent>()->render = [](mgEntity* e) {
//      BackgroundGen* bg = static_cast<BackgroundGen*>(e);
//
//    };
//  }
//
//  DynArray<psVec3D> _buildings;
//};

#include <sstream>

std::function<size_t(fgRoot&, const FG_Msg&)> guifunction;

int main(int argc, char** argv)
{
  SetWorkDirToCur();
  mgEngine engine;

  mgConsole console;
  std::ostream ssLog(&console);
  engine.GetLog().AddTarget(ssLog);

  PSINIT init;
  //init.width = 1024;
  //init.height = 768;
  init.mediapath = "../media/shaders";
  init.vsync = false;
  Box2DSystem::B2INIT b2init;
  b2init.ppm = 16;
  b2init.gravity[1] = 27.0f;
#ifdef BSS_DEBUG
  //b2init.hertz = 0;
#endif
  LiquidFunPlaneshaderSystem ps(init, -1);

  fgTransform consoletf = { {0, 0, 0, 0, 0, 1.0f, 320, 0 }, 0, {0, 0, 0, 0 } };
  console.Load(ps.GetGUI().gui, 0, "console", FGELEMENT_BACKGROUND | FGELEMENT_IGNORE, &consoletf, 0);

  TinyOALSystem tinyoal;
  LiquidFunSystem psbox2d(b2init, 0);
  LogicSystem logic;
  LuaSystem lua;

  engine.AddSystem(&ps);
  engine.AddSystem(&tinyoal);
  engine.AddSystem(&psbox2d);
  engine.AddSystem(&logic);
  engine.AddSystem(&lua);

  fgTransform inputtf = { { 0, 0, 320, 0, 0, 1.0f, 320, 0 }, 0,{ 0, 0, 0, 0 } };
  fgElement* input = fgCreate("textbox", ps.GetGUI().gui, 0, "console_input", FGELEMENT_EXPANDY | FGTEXTBOX_SINGLELINE | FGTEXTBOX_ACTION, &inputtf, 0);
  std::function<void(struct _FG_ELEMENT* e, const FG_Msg* m)> inputaction = [&](struct _FG_ELEMENT* e, const FG_Msg* m) {
    if(!m->subtype)
    {
      std::istringstream ss(std::string(e->GetText()));
      lua.Load(ss, ssLog);
      e->SetText("");
    }
  };
  fgElement_AddLambdaListener(input, FG_ACTION, inputaction);
  psDebugDraw psdd;
  //ps[0].Insert(&psdd);
  psbox2d.SetDebugDraw(&psdd);

  maxcam.left = -ps.GetDriver()->GetBackBuffer()->GetDim().x*0.375; // 3/8
  maxcam.top = -ps.GetDriver()->GetBackBuffer()->GetDim().y*0.375;
  //globalcam.SetPosition(maxcam.left, maxcam.top, -0.25f);
  globalcam.SetExtent(psVec(0.2, 50000));
  ps[0].SetCamera(&globalcam);
  ps[0].SetClearColor(0xFF111122);
  //ps[0].SetDPI(psVeciu(psGUIManager::BASE_DPI * 4));

  mgEntityT<psTilesetComponent, b2PhysicsComponent> map;
  auto tsmap = map.Get<psTilesetComponent>();
  tsmap->SetTexture(LoadPointImg("../media/LD34/tile1.png"));
  tsmap->AutoGenDefs(psVec(16, 16));
  tsmap->SetPass(&ps[0]);

  LoadMap(psTex::Create("../media/LD34/testmap.png", USAGE_STAGING), tsmap, &map, psTex::Create("../media/LD34/key.png", USAGE_STAGING));
  
  ref_ptr<Player> player = new Player(LoadPointImg("../media/LD34/player.png"), 100);

  fgInject injector;
  guifunction = [&](fgRoot& self, const FG_Msg& evt) -> size_t {
    if(evt.type == FG_KEYDOWN || evt.type == FG_KEYUP)
    {
      bool isdown = evt.type == FG_KEYDOWN;
      dirkeys[8] = evt.IsShiftDown();
      switch(evt.keycode)
      {
      case FG_KEY_LEFT: dirkeys[0] = isdown; break;
      case FG_KEY_RIGHT: dirkeys[1] = isdown; break;
      case FG_KEY_UP: dirkeys[2] = isdown; break;
      case FG_KEY_DOWN: dirkeys[3] = isdown; break;
      case FG_KEY_X:
      case FG_KEY_OEM_PLUS: dirkeys[4] = isdown; break;
      case FG_KEY_Z:
      case FG_KEY_OEM_MINUS: dirkeys[5] = isdown; break;
      case FG_KEY_Q: dirkeys[6] = isdown; break;
      case FG_KEY_E: dirkeys[7] = isdown; break;
      case FG_KEY_A: dirkeys[10] = isdown; break;
      case FG_KEY_D: dirkeys[11] = isdown; break;
      case FG_KEY_RETURN: player->Health()->Modify(-10); break;
      case FG_KEY_W:
        if(IsOnFloor(player))
        {
          const float jump = 0.5f;
          b2Body* b = player->Physics()->GetBody();
          b->SetLinearVelocity(b2Vec2(b->GetLinearVelocity().x, 0));
          b->ApplyLinearImpulse(b2Vec2(0, -jump*psbox2d.GetGravity().y), b->GetLocalCenter(), true);
        }
        break;
      case FG_KEY_ESCAPE:
        if(isdown) ps.Quit();
        break;
      }
    }
    return injector(&self, &evt);
  };

  injector = fgSetInjectFunc([](struct _FG_ROOT* self, const FG_Msg* msg) -> size_t { return guifunction(*self, *msg); });
  //ps.GetGUI().SetInject(psRoot::PS_MESSAGE(guifunction));

  player->Get<mgLogicComponent>()->onlogic = [](mgEntity& e) {
    float secdelta = mgEngine::Instance()->GetDeltaNS() / 1000000000.0f;
    float maxspeed = 8.0f;
    b2PhysicsComponent* phys = e.Get<b2PhysicsComponent>();
    if(dirkeys[10])
      phys->GetBody()->ApplyForceToCenter(b2Vec2(mgclamp(-maxspeed - phys->GetBody()->GetLinearVelocity().x, -maxspeed, 0), 0), true);
    if(dirkeys[11])
      phys->GetBody()->ApplyForceToCenter(b2Vec2(mgclamp(maxspeed - phys->GetBody()->GetLinearVelocity().x, 0, maxspeed), 0), true);
    if(!dirkeys[10] && !dirkeys[11])
      phys->GetBody()->ApplyForceToCenter(b2Vec2(mgclamp(-phys->GetBody()->GetLinearVelocity().x, -maxspeed*0.5f, maxspeed*0.5f), 0), true);
  };

  logic.process = [&]() {
    float secdelta = mgEngine::Instance()->GetDeltaNS() / 1000000000.0f;
    float scale = dirkeys[8] ? 0.01f : 1.0f;
    psLocatable* loc = player->Get<psLocatableComponent>()->Get();
    psVec sdim = psEngine::Instance()->GetDriver()->GetBackBuffer()->GetRawDim();
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

  //BackgroundGen bg(0);

  fgLayout layout;
  fgLayout_Init(&layout, 0, 0);
  fgLayout_LoadFileXML(&layout, "../media/console.xml");
  fgSingleton()->gui->LayoutLoad(&layout);


  engine.ResetDelta();
  engine.ResetTime();
  while(!ps.GetQuit())
  {
    engine.Process();
  }

  console.Destroy();
  fgLayout_Destroy(&layout);
}

struct HINSTANCE__;

// WinMain function, simply a catcher that calls the main function
int __stdcall WinMain(HINSTANCE__* hInstance, HINSTANCE__* hPrevInstance, char* lpCmdLine, int nShowCmd)
{
  main(0, (char**)hInstance);
}