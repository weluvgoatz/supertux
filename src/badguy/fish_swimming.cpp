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

#include "badguy/walking_badguy.hpp"
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
  dead(),
  radius()
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
  reader.get("radius", radius, 0);
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
  result.add_float(_("Radius"), &radius, "radius");

  result.reorder({"size", "vertical", "speed", "radius", "direction", "x", "y" });

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
FishSwimming::transition_speed(float dt_sec, float targetspeed)
{
  float current_speed = vertical ? m_physic.get_velocity_y() : m_physic.get_velocity_x();

  if (m_frozen)
  {
    m_physic.set_velocity_x(0.0);
    m_physic.set_acceleration_x(0.0);
  }
  /* We're very close to our target speed. Just set it to avoid oscillation */
  else if ((current_speed > (targetspeed - 5.0f)) &&
    (current_speed < (targetspeed + 5.0f)))
  {
    if (vertical)
    {
      m_physic.set_velocity_y(targetspeed);
      m_physic.set_acceleration_y(0.0);
    }
    else
    {
      m_physic.set_velocity_x(targetspeed);
      m_physic.set_acceleration_x(0.0);
    }
  }
  /* Check if we're going too slow or even in the wrong direction */
  else if (((targetspeed <= 0.0f) && (current_speed > targetspeed)) ||
    ((targetspeed > 0.0f) && (current_speed < targetspeed)))
  {
    /* acceleration == walk-speed => it will take one second to get from zero
     * to full speed. */
    if (vertical)
      m_physic.set_acceleration_y(targetspeed * 2.f);
    else
      m_physic.set_acceleration_x(targetspeed * 2.f);
  }
  /* Check if we're going too fast */
  else if (((targetspeed <= 0.0f) && (current_speed < targetspeed)) ||
    ((targetspeed > 0.0f) && (current_speed > targetspeed)))
  {
    /* acceleration == walk-speed => it will take one second to get twice the
     * speed to normal speed. */
    if (vertical)
      m_physic.set_acceleration_y((-1.f) * targetspeed);
    else
      m_physic.set_acceleration_x((-1.f) * targetspeed);
  }
  else
  {
    /* The above should have covered all cases. */
    assert(false);
  }
  if ((m_dir == Direction::LEFT) &&
    ((vertical && m_physic.get_velocity_y() > 0.0f) ||
    (!vertical && m_physic.get_velocity_x() > 0.0f))) {
    m_dir = Direction::RIGHT;
  }
  else if ((m_dir == Direction::RIGHT) &&
    ((vertical && m_physic.get_velocity_y() < 0.0f) ||
    (!vertical && m_physic.get_velocity_x() < 0.0f))) {
    m_dir = Direction::LEFT;
  }
}

void
FishSwimming::active_update(float dt_sec) {
  BadGuy::active_update(dt_sec);
  bool can_swim_here = !Sector::get().is_free_of_tiles(get_bbox().grown(-8), true, Tile::WATER);
  Rectf seeabovewater = get_bbox();
  seeabovewater.set_bottom(m_col.m_bbox.get_top());
  seeabovewater.set_top(m_col.m_bbox.get_top() - 5.f);
  bool verticalleftturn = Sector::get().is_free_of_tiles(seeabovewater, true, Tile::WATER);
  switch (waterstate)
  {
    case FishWaterState::WATER:
      if (!m_frozen)
      {
        if (vertical)
        {
          m_physic.set_velocity_x(0.f);
          m_physic.set_acceleration_x(0.f);
          if (verticalleftturn && m_dir == Direction::LEFT && state != FishMovementState::TURNING)
          {
            m_dir = Direction::RIGHT;
            m_sprite->set_action(m_dir == Direction::LEFT ? "r-turn-l" : "l-turn-r");
            m_physic.set_velocity_y(0);
            state = FishMovementState::TURNING;
            turn_timer = .25f;
          }
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

  if (state == FishMovementState::SWIMMING && waterstate == FishWaterState::WATER && !m_frozen)
  {
    m_sprite->set_action(m_dir == Direction::LEFT ? "left" : "right");
    float targetspeed = m_dir == Direction::LEFT ? -speed : speed;
    if (radius > 0.f)
    {
        if (vertical && (m_dir != Direction::LEFT && get_pos().y > (m_start_position.y + radius) ||
          (m_dir != Direction::RIGHT && get_pos().y < (m_start_position.y - radius))) ||
          !vertical && (m_dir != Direction::LEFT && get_pos().x > (m_start_position.x + radius) ||
            m_dir != Direction::RIGHT && get_pos().x < (m_start_position.x - radius)))
        {
          targetspeed = -targetspeed;
        }
        transition_speed(dt_sec, targetspeed);
    }
    else
    {
      if (vertical)
        m_physic.set_velocity_y(m_dir == Direction::LEFT ? -speed : speed);
      else
        m_physic.set_velocity_x(m_dir == Direction::LEFT ? -speed : speed);
    }
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
  m_physic.set_velocity(0.f, 0.f);
  m_physic.set_acceleration(0.f, 0.f);
  kill_squished(object);
  dead = true;
  return true;
}

/* EOF */
