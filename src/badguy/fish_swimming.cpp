//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "badguy/fish_swimming.hpp"

#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

FishSwimming::FishSwimming(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/fish_swimming/fish_swimming.sprite"),
  state(FishMovementState::SWIMMING),
  waterstate(FishWaterState::AIR),
  size(FishSize::MEDIUM),
  turn_timer(0.f),
  speed(),
  beached_timer(),
  vertical(),
  dead()
{
  std::string size_str;
  if (reader.get("size", size_str))
  {
    if (size_str == "small")
      size = FishSize::SMALL;
    else if (size_str == "large")
      size = FishSize::LARGE;
    else
      size = FishSize::MEDIUM;
  }
  reader.get("speed", speed, 100);
  reader.get("vertical", vertical, false);
  m_physic.enable_gravity(false);
}

void
FishSwimming::initialize()
{
  if (vertical)
  {
    m_physic.set_velocity_y(m_dir == Direction::LEFT ? -speed : speed);
    m_sprite->set_action(m_dir == Direction::LEFT ? "left" : "right");
  }
  else
  {
    m_physic.set_velocity_x(m_dir == Direction::LEFT ? -speed : speed);
    m_sprite->set_action(m_dir == Direction::LEFT ? "left" : "right");
  }
  state = FishMovementState::SWIMMING;
  waterstate = FishWaterState::AIR;
}

ObjectSettings
FishSwimming::get_settings()
{
  ObjectSettings result = BadGuy::get_settings();

  result.add_enum(_("Size"), reinterpret_cast<int*>(&size),
    {_("small"), _("medium"), _("large")},
    {"small", "medium", "large"},
    static_cast<int>(FishSize::MEDIUM), "size");
  result.add_bool(_("Vertical Orientation"), &vertical, "vertical", false);
  result.add_float(_("Speed"), &speed, "speed");

  result.reorder({"size", "vertical", "speed", "direction", "x", "y" });

  return result;
}

void
FishSwimming::collision_solid(const CollisionHit& hit)
{
  if (m_frozen)
  {
    if (hit.bottom)
      m_physic.set_velocity_y(0);
    m_physic.set_velocity_x(0);
    return;
  }
  if (hit.bottom)
  {
    if (waterstate == FishWaterState::AIR)
    {
      if (!beached_timer.started())
        beached_timer.start(5.f);
      waterstate = FishWaterState::FLOP;
    }
    if (!dead)
    m_physic.set_velocity_y(waterstate == FishWaterState::FLOP ? -250.f : vertical ? -speed : 0.f);
    else m_physic.set_velocity_y(0);
  }
  if (hit.top && !vertical)
  {
    m_physic.set_velocity_y(0);
  }
  if (vertical && waterstate == FishWaterState::WATER)
  {
    if ((hit.top || hit.bottom) && !dead)
    {
      m_dir = (m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT);
      m_sprite->set_action(m_dir == Direction::LEFT ? "r-turn-l" : "l-turn-r");
      m_physic.set_velocity_y(0);
      state = FishMovementState::TURNING;
      turn_timer = .25f;
    }
  }
  else
  {
    if (hit.left || hit.right)
    {
      m_dir = (m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT);
      m_sprite->set_action(m_dir == Direction::LEFT ? "r-turn-l" : "l-turn-r");
      m_physic.set_velocity_x(0);
      state = FishMovementState::TURNING;
      turn_timer = .25f;
    }
  }
}

HitResponse
FishSwimming::collision_badguy(BadGuy&, const CollisionHit& hit)
{
  if (vertical)
  {
    if ((hit.top && (m_dir == Direction::LEFT)) || (hit.bottom && (m_dir == Direction::RIGHT)))
    {
      m_dir = (m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT);
    }
  }
  else
  {
    if ((hit.left && (m_dir == Direction::LEFT)) || (hit.right && (m_dir == Direction::RIGHT)))
    {
      m_dir = (m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT);
    }
  }
  if (waterstate != FishWaterState::WATER && hit.bottom)
  {
    if (!beached_timer.started())
      beached_timer.start(5.f);
    waterstate = FishWaterState::FLOP;
    if (waterstate == FishWaterState::FLOP)
      m_physic.set_velocity_y(-250.f);
  }
  return CONTINUE;
}

HitResponse
FishSwimming::collision_player(Player& player, const CollisionHit& hit)
{
  if (size != FishSize::SMALL)
    BadGuy::collision_player(player, hit);
  else
  {
    if (waterstate == FishWaterState::WATER)
    {
      //bounce players off
      if (hit.left)
        player.get_physic().set_velocity_x(-300.f);
      else if (hit.right)
        player.get_physic().set_velocity_x(300.f);
      else if (hit.bottom)
        player.get_physic().set_velocity_y(300.f);
    }
  }
  return FORCE_MOVE;
}

void
FishSwimming::active_update(float dt_sec) {
  BadGuy::active_update(dt_sec);
  if (vertical)
    m_sprite->set_angle(90.f);
  Rectf swimherebox = get_bbox();
  swimherebox.grown(-8);
  bool can_swim_here = !Sector::get().is_free_of_tiles(swimherebox, true, Tile::WATER);
  switch (waterstate)
  {
    case FishWaterState::WATER:
      if (!m_frozen)
      {
        if (vertical)
        {
          m_physic.set_velocity_x(0.f);
          m_physic.set_acceleration_x(0.f);
        }
        else
        {
          m_physic.set_velocity_y(0.f);
          m_physic.set_acceleration_y(0.f);
        }
      }
      m_physic.enable_gravity(m_frozen);
      if (!can_swim_here)
      {
        if (vertical)
        {
          m_physic.set_velocity(0.f, 0.f);
          m_physic.set_acceleration(0.f, 0.f);
        }
        waterstate = FishWaterState::AIR;
      }
      break;
    case FishWaterState::AIR:
      if (vertical)
        m_dir = Direction::RIGHT;
      m_physic.enable_gravity(true);
      if (can_swim_here)
        waterstate = FishWaterState::WATER;
      break;
    case FishWaterState::FLOP:
      m_physic.enable_gravity(true);
      m_physic.set_velocity_x(0.f);
      m_physic.set_acceleration_x(0.f);
      if (can_swim_here)
        waterstate = FishWaterState::WATER;
      if (beached_timer.check())
        kill_fall();
      break;
  }

  if (turn_timer > 0.f)
  {
    turn_timer -= dt_sec;
    if (turn_timer <= 0.f)
    {
      turn_timer = 0.f;
      state = FishMovementState::SWIMMING;
    }
  }

  if (state == FishMovementState::SWIMMING && waterstate == FishWaterState::WATER && !m_frozen) {
    m_sprite->set_action(m_dir == Direction::LEFT ? "left" : "right");
    if (vertical)
      m_physic.set_velocity_y(m_dir == Direction::LEFT ? -speed : speed);
    else
      m_physic.set_velocity_x(m_dir == Direction::LEFT ? -speed : speed);
  }
}

void
FishSwimming::freeze()
{
  BadGuy::freeze();
  m_physic.enable_gravity(true);
}

void
FishSwimming::unfreeze()
{
  BadGuy::unfreeze();
  m_physic.enable_gravity(false);
  initialize();
}

bool
FishSwimming::is_freezable() const
{
  return true;
}

bool
FishSwimming::collision_squished(GameObject& object)
{
  m_sprite->set_action(m_dir == Direction::LEFT ? "squished-left" : "squished-right");
  m_physic.enable_gravity(true);
  m_physic.set_acceleration_y(0);
  m_physic.set_velocity_y(0);
  kill_squished(object);
  dead = true;
  return true;
}

/* EOF */
