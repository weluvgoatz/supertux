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

#ifndef HEADER_SUPERTUX_OBJECT_LETTER_HPP
#define HEADER_SUPERTUX_OBJECT_LETTER_HPP

#include "control/controller.hpp"
#include "object/moving_sprite.hpp"
#include "sprite/sprite_ptr.hpp"
#include "video/surface_ptr.hpp"

class DrawingContext;

class Letter final : public MovingSprite
{
public:
  Letter(const ReaderMapping& mapping);

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

  virtual std::string get_class() const override { return "letter"; }
  virtual std::string get_display_name() const override { return _("Letter"); }

protected:
  const Controller* controller;

private:
  bool m_can_read_letter;
  bool m_is_reading_letter;
  SurfacePtr large_letter;
};

#endif

/* EOF */
