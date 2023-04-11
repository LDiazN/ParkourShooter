# ParkourShooter
Simple FPS game made in **Unreal Engine 4** where I aimed to implement and experiment with advanced mobility mechanics in an FPS. The core movement features include sliding, wall running, a grappling hook, double jump, and vaulting. 

## Wallrunning

## Sliding

## Vaulting

## Hook
The hook mechanic is pretty simple, it will travel for a certain distance and if it hits something, it will pull the player to the contact surface. The hook itself is defined by a hook head (just a sphere) and a cable (the Unreal's cable actor). There's a spawn location defined in the player character blueprint where the hook head will start, when you trigger the hook with the right click, the hook head spawns at this location, and the cable is attached to it and the starting location. 

Now we compute the travel direction, it's the direction from the start location near the player to a target location. The target location is the result of a Line Trace starting from the player's location to its camera forward direction up to a big distance. If something was hit, the result target location is the impact point, otherwise it's just the line trace endpoint. Now the hook head will continuously travel with that direction until it hits something. If it doesn't, the shoot it's cancelled, and if it does it stops its movement and we start pulling the player.

For the pulling process I first tried with a force-based approach where a force was applied each frame to the player towards the hook head location, but this led to a somewhat unpredictable behavior where it was hard to find the ideal force to apply to tune the expected velocity, 
