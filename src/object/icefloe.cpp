//  SuperTux
//  Copyright (C) 2024 Daniel Ward <weluvgoatz@gmail.com>
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

static const float WATER_PUSH_ACCELERATION = 1000.f;
static const float FLOAT_TIME = 2.f;

#include "object/icefloe.hpp"

#include "math/util.hpp"
#include "object/player.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "util/reader_mapping.hpp"

Icefloe::Icefloe(const ReaderMapping& reader) :
  MovingSprite(reader, "images/objects/platforms/icefloe_small.sprite", LAYER_OBJECTS, COLGROUP_MOVING_STATIC),
  m_physic(),
  m_in_water(),
  m_roof_water(),
  m_start_pos(),
  m_has_start_pos(),
  m_icefloe_state(IcefloeState::SEEKING),
  m_float_timer(),
  m_load_weight()
{
  m_start_pos = get_pos();
}

void
Icefloe::update(float dt_sec)
{
  Rectf in_water_box = get_bbox();
  in_water_box.set_bottom(get_bbox().get_top() + 8.f);

  Rectf roof_water_box = get_bbox();
  roof_water_box.set_bottom(get_bbox().get_top() + 4.f);
  roof_water_box.set_top(get_bbox().get_top() - 4.f);

  Rectf weight_box = get_bbox();
  weight_box.set_top(get_bbox().get_top() - 4.f);

  m_in_water = !Sector::get().is_free_of_tiles(in_water_box, true, Tile::WATER);
  m_roof_water = (m_in_water && Sector::get().is_free_of_tiles(roof_water_box, true, Tile::WATER));

  float yspeed = 5.f;
  float goal_pos = 0.f;

  m_physic.enable_gravity(!m_in_water || m_roof_water);

  for (auto& player : Sector::get().get_objects_by_type<Player>())
  {
    if (weight_box.overlaps(player.get_bbox()))
    {
      m_icefloe_state = IcefloeState::WEIGHTED;
    }
  }

  switch (m_icefloe_state) {
  case SEEKING:
    if (m_in_water && !m_has_start_pos) {
      if (m_roof_water && m_physic.get_velocity_y() <= 0.f) {
        m_start_pos = get_pos() + Vector(0.f, 20.f);
        m_has_start_pos = true;
      }
      else {
        m_physic.set_acceleration_y(-(m_physic.get_velocity_y()) - WATER_PUSH_ACCELERATION);
      }
    }

    if (m_roof_water) {
      m_physic.set_acceleration_y(1500.f);
    }

    if (m_has_start_pos) {
      m_physic.set_acceleration_y(-m_physic.get_velocity_y()*(m_start_pos.y - get_pos().y));
    }
    if (m_has_start_pos && std::abs(m_physic.get_velocity_y()) < 0.5f) {
      m_icefloe_state = IcefloeState::EQUILIBRIUM;
    }
    break;
  case EQUILIBRIUM:
    if (!m_float_timer.started())
      m_float_timer.start(FLOAT_TIME);
    if (m_float_timer.get_timeleft() <= 1.f)
    {
      yspeed = yspeed * -1.f;
    }
    m_physic.set_velocity_y(yspeed);
    break;
  case WEIGHTED:
    for (auto& player : Sector::get().get_objects_by_type<Player>())
    {
      if (weight_box.overlaps(player.get_bbox()) && player.get_physic().get_velocity_y() >= 0.f)
      {
        m_load_weight += 1;
      }
    }

    goal_pos = m_load_weight * 32.f;

    m_col.set_movement(Vector(0.f, 0.01f*(goal_pos - get_pos().y)*glm::length(m_start_pos.y - goal_pos)));
    m_icefloe_state = IcefloeState::SEEKING;
    break;
  default:
    break;
  }

  m_load_weight = 0.f;
  goal_pos = 0.f;

  if (m_icefloe_state != WEIGHTED) {
    m_col.set_movement(m_physic.get_movement(dt_sec));
  }
}

void
Icefloe::collision_solid(const CollisionHit& hit)
{
  if ((hit.top || hit.bottom) && m_in_water) {
    m_physic.set_velocity_y((-m_physic.get_velocity_y())/3.f);
  }
}

HitResponse
Icefloe::collision(GameObject& other, const CollisionHit& hit)
{

  if (hit.top) {
    m_icefloe_state = IcefloeState::WEIGHTED;
  }
  return FORCE_MOVE;
}

/* EOF */
