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

#ifndef HEADER_SUPERTUX_OBJECT_ICEFLOE_HPP
#define HEADER_SUPERTUX_OBJECT_ICEFLOE_HPP

#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"
#include "supertux/timer.hpp"

class Icefloe : public MovingSprite
{
public:
  Icefloe(const ReaderMapping& reader);

  virtual void update(float dt_sec) override;
  virtual void collision_solid(const CollisionHit& hit) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

  static std::string class_name() { return "icefloe"; }
  virtual std::string get_class_name() const override { return class_name(); }
  static std::string display_name() { return _("Icefloe"); }
  virtual std::string get_display_name() const override { return display_name(); }

protected:
  Physic m_physic;
  bool m_in_water;
  bool m_roof_water;
  bool m_has_start_pos;
  Vector m_start_pos;
  Timer m_float_timer;
  float m_load_weight;

private:
  enum IcefloeState {
    SEEKING, // for when the icefloe first generates and needs to find a "start position"
    EQUILIBRIUM, // for when the icefloe is sitting by itself in its "start position"
    WEIGHTED // for if the icefloe is out of its "start" position due to the weight of others
  };

  IcefloeState m_icefloe_state;

private:
  Icefloe(const Icefloe&) = delete;
  Icefloe& operator=(const Icefloe&) = delete;
};

#endif

/* EOF */
