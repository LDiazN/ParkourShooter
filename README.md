# ParkourShooter
Simple FPS game made in **Unreal Engine 4** where I aimed to implement and experiment with advanced mobility mechanics in an FPS. The core movement features include sliding, wall running, a grappling hook, double jump, and vaulting. 

## Wallrunning

## Sliding

## Vaulting
The vaulting system helps the player to jump over objects that are too tall to reach them by a single jump, but not too much to cosider them unreachable. When the player is near an edge, he can grab the edge to jump over.

Since we want the player to climb over object with a reasonable height with respect to the player, I cast a ray starting a bit in front of him and at the height of his "forehead" and ending at his "feet". The height is just the player location's height + the half height of the player capsule, the "feet" position is just the starting location - 2 * the capsule half height. If we didn't hit anything, then we don't vault, otherwise we have to check if the hit position is "vaultable", if we can or want to actually vault there. 

To be able to vault over a surface we need to check that:

* The surface is not too steep, like trying to climb over a tilted wall.
* The height of this surface **relative to the character's location** is not too small or too high. We can hit something if the floor is slightly tilted, so we ignore this cases.
* The character fits: I check this by using `SweepSingleByChannel` using the character's capsule parameters to sweep using a capsule, and the start and end location are the same so that the sweep is just in a single point. This location is the hit location + the half height of the capsule + some offset to account for slopes (I use the capsule radius as offset). If we don't hit anything, we can vault.

If we check all the marks we store the location used in the last check to start vaulting. For this I set a progress variable to 0, store a start location (the player's location in that frame) and the target location. Finally in each frame I add to the progress variable `DeltaSeconds / TimeToVault`, this will make the progress variable go from 0 to 1 in `TimeToVault` seconds. With the progress variable I lineally interpolate (`lerp`) the start location to the target location and set the result as the new character's location. 

## Hook
The hook mechanic is relatively simple: it travels for a certain distance, and if it hits something, it pulls the player to the contact surface. The hook consists of a hook head (just a sphere) and a cable (the Unreal Cable Actor). In the Player Character Blueprint, there is a spawn location defined for the hook head to start from. When the player triggers the hook with the right-click, the hook head spawns at this location, and the cable attaches to it and the starting location.

Next, we compute the travel direction, which is the direction from the start location near the player to a target location. We determine the target location by performing a Line Trace from the player's location to the camera's forward direction, up to a considerable distance. If the Line Trace hits something, the target location is the impact point; otherwise, it's the Line Trace endpoint. The hook head travels continuously in that direction until it hits something. If it doesn't hit anything, the shot is canceled. If it does, the hook head stops moving, and we start pulling the player.

Initially, I tried a force-based approach, where a force was applied each frame to the player towards the hook head location. However, this led to somewhat unpredictable behavior, making it challenging to find the ideal force for the desired velocity, and the movement was awkward. Thus, I changed my approach to set the character's velocity instead. This approach was much easier to tune and followed the hook line more accurately. I also added the possibility to add some velocity to the sides or up and down, to have a bit more control. I accomplished this by storing a speed for the character's right vector and another for the up vector. When input is added in some axis, it increases in that axis, and when there is no input, it decreases.

Note that before pulling the character, movement properties in its `UCharacterMovementComponent` must be set to avoid strange behavior. I set the gravity, ground friction, and air control to 0. After the grappling is finished, these properties must be restored.

The "hardest" part of this process is checking if you arrived or passed the hook, as it's possible to do so (especially with the force-based approach). The pulling process usually ends when the character is close enough to the hook, but sometimes, passing the hook head while still far away can create unpredictable movement where the player is constantly being pulled to the hook and spinning around. For this reason, I also check if we pass the hook head. I do this by storing the original hook direction and checking the dot product between this original direction and the current direction from the player to the hook head. If it's negative, we passed the hook; otherwise, we are still in a valid direction. For the dot product I only use the X and Y components for both directions.
