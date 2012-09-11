#include "cg_local.h"

void BOTS_Nurse_ShowPoisonSprite(centity_t *cent)
{
	refEntity_t		ent;
	entityState_t	*es;
	gitem_t			*item;
	gitem_t			*poisonItem;

	es = &cent->currentState;
	item = &bg_itemlist[ es->modelindex ];

	if (item->giType == IT_HEALTH)
	{
		if (es->powerups & (1 << PW_POISON))
		{
			if (es->otherEntityNum == cg.clientNum)
			{
				poisonItem = BG_FindItemForPowerup(PW_POISON);

				memset( &ent, 0, sizeof( ent ) );
				ent.reType = RT_SPRITE;
				VectorCopy( cent->lerpOrigin, ent.origin );
				ent.origin[2] += 32;
				ent.radius = 7;
				ent.customShader = cg_items[ITEM_INDEX(poisonItem)].icon;
				ent.shaderRGBA[0] = 255;
				ent.shaderRGBA[1] = 255;
				ent.shaderRGBA[2] = 255;
				ent.shaderRGBA[3] = 255;
				trap_R_AddRefEntityToScene(&ent);
			}
		}
	}

}