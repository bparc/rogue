WHAT TO DO

-dynamic Asset Loading 
    - asstes->planets[]->enemies[]->(static->front/back)/(anim->move_front/move_back/damage_front/damage_right/attack_front/attack_back)
    - assets->tiles->planets[]->left_corner/up_corner/no_corner_middle/bottom_border/side_border (assets can be mirrored for left right up down)
    - etc
- different approach to maps: if we have different trap types, enemy types, obstacles, interactables, simple chars wont work for long

PROOF-OF-CONCEPT/VERTICAL SLICE ROADMAP
- DONE Health bar display for hostile entities.
-    Smooth animation for health bars?
-    Isometric health bars (see the concept arts)?
- Player attack should consume "action points".
- Some nice and intuitive control scheme with button prompts.
- DONE Limiting the cursor range.
- DONE Enemies that can span multiple tiles.
- Environmental hazards.
-    How will they work exactly?
- DONE Status effects.
-    HUD that shows status effects for the player and enemies?
-    In what order they should be evaluated? The damage from poison is applied at the end or at the start of a turn?
- A simple behaviour for slime
enemies: They will walk around the map in random directions and attack the player from a distance.
- A simple map/level designed in a map editor.
- A game over state.
- An attack move that can 'push back' an enemy by some amount of tiles.
-    Apply additional damage when an enemy collides with a wall?
- better layering of entities during gameplay - they retain their z indexes from init, leading to unlogical overlaps
