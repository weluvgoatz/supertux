//  SuperTux
//  Copyright (C) 2021 Daniel Ward <weluvgoatz@gmail.com>
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

#include "object/letter.hpp"

#include "control/input_manager.hpp"
#include "gui/mousecursor.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/resources.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

Letter::Letter(const ReaderMapping& mapping) :
  MovingSprite(mapping, "images/objects/letter/letter-small.sprite", LAYER_TILES, COLGROUP_TOUCHABLE),
  controller(&InputManager::current()->get_controller()),
  m_can_read_letter(),
  m_is_reading_letter(),
  large_letter(Surface::from_file("images/objects/letter/letter-big.png"))
{
}

void
Letter::update(float dt_sec)
{
}

void
Letter::draw(DrawingContext& context)
{
  if (m_is_reading_letter)
  {
    context.color().draw_surface(large_letter,
      Vector(context.get_width() / 2.f - large_letter->get_width() / 2.f,
             context.get_height() / 2.f - large_letter->get_height() / 2.f),
      LAYER_HUD);
  }
  m_sprite->draw(context.color(), get_pos(), LAYER_OBJECTS + 1);
}

HitResponse
Letter::collision(GameObject& other, const CollisionHit&)
{
  auto player = dynamic_cast<Player*> (&other);
  if (player) {
    m_can_read_letter = true;
    if (controller)
    {
      if (!m_is_reading_letter && (controller->pressed(Control::UP)))
        m_is_reading_letter = true;
      else if (m_is_reading_letter && ((controller->pressed(Control::DOWN)) ||
        (controller->pressed(Control::ESCAPE)) || (controller->pressed(Control::JUMP)) ||
        (controller->pressed(Control::ACTION)) || (controller->pressed(Control::UP)) ||
        (controller->pressed(Control::LEFT)) || (controller->pressed(Control::RIGHT))))
        m_is_reading_letter = false;
    }
  }
  return ABORT_MOVE;
}

/* EOF */
