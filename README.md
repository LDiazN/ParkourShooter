# ParkourShooter
Simple FPS game made in **Unreal Engine 4** where I aimed to implement and experiment with advanced mobility mechanics in an FPS. The core movement features include sliding, wall running, a grappling hook, double jump, and vaulting. 

## Wallrunning

## Sliding

## Vaulting

## Hook
The hook mechanic is pretty simple, it will travel for a certain distance and if it hits something, it will pull the player to the contact surface. The hook itself is defined by a hook head (just a sphere) and a cable (the Unreal's cable actor). There's a spawn location defined in the player character blueprint where the hook head will start, when you trigger the hook with the right click, the hook head spawns at this location, and the cable is attached to it and the starting location. 

Now we compute the travel direction, it's the direction from the start location near the player to a target location. The target location is the result of a Line Trace starting from the player's location to its camera forward direction up to a big distance. If something was hit, the result target location is the impact point, otherwise it's just the line trace endpoint. Now the hook head will continuously travel with that direction until it hits something. If it doesn't, the shoot it's cancelled, and if it does it stops its movement and we start pulling the player.

For the pulling process I first tried with a force-based approach where a force was applied each frame to the player towards the hook head location, but this led to a somewhat unpredictable behavior where it was hard to find the ideal force to apply to tune the expected velocity, and the movement was kind of awkward as well (This might not be a problem depending on your game but personally, I didn't like it). For this reason I changed my approach to set the character's velocity instead, and this worked pretty well since it's way easier to tune and it follows the hook line more accurately. I also added the possibility to add some velocity to the sides or up and down, to have a bit more control. I made it by storing a speed for the character´s right vector and another for the up vector. When you add input in some axis, it's increased in that axis and when you don't, it's decresed. 

Note that before pulling the character you need to set some movement properties in its `UCharacterMovementComponent` to avoid weird behaviors, I set the gravity, ground friction, and air control to 0. You have to restore them as well once the grappling is finished.

Finally, the "hardest" part of this is checking if you arrived or passed the hook since it's possible (specially with the force-based approach). You would usually finish the pulling process once the character is near enough the hook, but sometimes if you pass the hook head while being still far this might create unpredictable movement where you are constantly being pulled to the hook and spinning around. For this reason I also check if we pass the hook head. I do this by storing the original hook direction and checking the dot product between this original direction and the current direction from the player to the hook head. If it's negative, we passed the hook, otherwise we are still in a valid direction.
