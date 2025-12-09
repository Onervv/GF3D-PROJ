#include "gf2d_ui.h"

void gf2d_ui_draw(UI_Element* ui){
    gf2d_sprite_draw(
        ui->sprite,
        ui->position,
        ui->scale,
        ui->center,
        ui->rotation,
        ui->flip,
        ui->colorShift,
        ui->clip,
        ui->frame
    );
}