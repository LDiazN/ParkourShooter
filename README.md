# ParkourShooter
Simple FPS game made in **Unreal Engine 4** where I aimed to implement and experiment with advanced mobility mechanics in an FPS. The core movement features include sliding, wall running, a grappling hook, double jump, and vaulting. 

## Wallrunning

## Sliding

## Vaulting

## Hook
The hook mechanic is relatively simple: it travels for a certain distance, and if it hits something, it pulls the player to the contact surface. The hook consists of a hook head (just a sphere) and a cable (the Unreal Cable Actor). In the Player Character Blueprint, there is a spawn location defined for the hook head to start from. When the player triggers the hook with the right-click, the hook head spawns at this location, and the cable attaches to it and the starting location.

Next, we compute the travel direction, which is the direction from the start location near the player to a target location. We determine the target location by performing a Line Trace from the player's location to the camera's forward direction, up to a considerable distance. If the Line Trace hits something, the target location is the impact point; otherwise, it's the Line Trace endpoint. The hook head travels continuously in that direction until it hits something. If it doesn't hit anything, the shot is canceled. If it does, the hook head stops moving, and we start pulling the player.

Initially, I tried a force-based approach, where a force was applied each frame to the player towards the hook head location. However, this led to somewhat unpredictable behavior, making it challenging to find the ideal force for the desired velocity, and the movement was awkward. Thus, I changed my approach to set the character's velocity instead. This approach was much easier to tune and followed the hook line more accurately. I also added the possibility to add some velocity to the sides or up and down, to have a bit more control. I accomplished this by storing a speed for the character's right vector and another for the up vector. When input is added in some axis, it increases in that axis, and when there is no input, it decreases.

Note that before pulling the character, movement properties in its `UCharacterMovementComponent` must be set to avoid strange behavior. I set the gravity, ground friction, and air control to 0. After the grappling is finished, these properties must be restored.

The "hardest" part of this process is checking if you arrived or passed the hook, as it's possible to do so (especially with the force-based approach). The pulling process usually ends when the character is close enough to the hook, but sometimes, passing the hook head while still far away can create unpredictable movement where the player is constantly being pulled to the hook and spinning around. For this reason, I also check if we pass the hook head. I do this by storing the original hook direction and checking the dot product between this original direction and the current direction from the player to the hook head. If it's negative, we passed the hook; otherwise, we are still in a valid direction.
