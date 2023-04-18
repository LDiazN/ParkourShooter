# ParkourShooter
Simple FPS game made in **Unreal Engine 4** where I aimed to implement and experiment with advanced mobility mechanics in an FPS. The core movement features include sliding, wall running, a grappling hook, double jump, and vaulting. The code for this project was mainly written in C++, while also using blueprints when more appropriate for the task.

## Wallrunning
Wallrunning allows the player to run along vertical surfaces to traverse spaces without floor, avoiding damage, and getting better shot positions. For this implementation I wanted the player to run indefinitely over walls and I tried to avoid solutions that required to mark surfaces as runnable. Altough it's not super hard to implement, it's the hardest mechanic to implement in this project.




## Sliding

Here's the corrected paragraph:

Sliding allows the character to reduce the size of its capsule collider, making it harder to hit and enabling it to slide between low ceilings. It also provides a speed boost in a specific direction for a short period of time, which can be useful for taking cover during a frenzied battle. Crouching is also implemented to handle situations where the slide finishes under a low ceiling surface. 

To enable sliding, I have set a parameter for the maximum sliding speed. When the player triggers a slide, the velocity of the player is set to its forward direction at this maximum speed. Additionally, I reset certain movement properties such as ground friction and breaking deceleration for walking to 0, so that the slide lasts for a longer time. At this point, I also call the `BeginSlideBP` method, which is a blueprint implementable method that starts the updating process in blueprints and manages animations. I will elaborate on this later.

Once the player enters the sliding mode, a continuous force is added each frame. The force of the slide is determined by the steepness of the terrain, which means that the player will slide for a longer period of time if they are on a hill, as compared to being on flat ground. To achieve this, I take the floor normal and use it to compute a vector that points downwards the hill. I use a vector trick to achieve this, where I take the cross product between the surface normal and the up vector. Since the cross product returns a vector perpendicular to both its inputs, this vector will be in the XY plane and the surface plane simultaneously. Then, I take the cross product between this new vector and the surface normal again, which yields a vector that is perpendicular to the previous one and the normal, and thus pointing downwards. I use this vector as the direction to add the continuous force, and it is scaled down by the steepness of the floor. The steepness factor is computed as 1 - the dot product between the surface down direction we just computed and the up vector (0,0,1). In each frame, I use `AddForce` with this non-normal vector as the direction scaled with a `MaxForce` value. Note that since the force might have magnitude 0 when the floor is horizontal, no force is added in such cases, and only the initial pull keeps the player moving.

The update logic is now handled in the blueprint layer, where I implement the BeginSlideBP and EndSlideBP events that were mentioned earlier. These nodes start and end a timeline node that calls the `UpdateSlide` function in multiple frames and stops it when necessary. Additionally, I have added another timeline node to add camera tilting during a slide. This effect gives the impression that the player is sliding on their side.

In the same `UpdateSlide` function we check if we should stop sliding, and if we do, we change states properly:

* If we can't stand, we switch to crouched, a state where our capsule size is reduced and we walk slower
* If we can, we switch to the regular running state. 

## Vaulting
The vaulting system helps the player to jump over objects that are too tall to reach by a single jump but not so tall that they are unreachable. When the player is near an edge, they can grab the edge to jump over it.

To ensure that the player can climb over objects of a reasonable height, I cast a ray starting slightly in front of the player at the height of their "forehead" and ending at their "feet". The height is calculated as the player's location height + half the height of the player capsule, and the "feet" position is calculated as the starting location - 2 * the capsule half height. If the ray doesn't hit anything, then the player cannot vault. Otherwise, we check if the hit position is "vaultable" and if we can or want to actually vault there.

To be able to vault over a surface, we need to check that:

* The surface is not too steep, like a tilted wall.
* The height of the surface **relative to the character's location** is not too small or too high. We can hit something if the floor is slightly tilted, so we ignore this cases.
* The character fits: I check this by using `SweepSingleByChannel` with the character's capsule parameters to sweep using a capsule. The start and end locations are the same, so the sweep is just in a single point. This location is the hit location + half the height of the capsule + some offset to account for slopes (I use the capsule radius as an offset). If we don't hit anything, the player can vault.

If we pass all the checks, we store the location used in the last check to start vaulting. For this, I set a progress variable to 0, store a start location (the player's location in that frame), and the target location. In each frame, I add `DeltaSeconds / TimeToVault` to the progress variable. This makes the progress variable go from 0 to 1 in `TimeToVault` seconds. Using the progress variable, I linearly interpolate (`lerp`) the start location to the target location and set the result as the new player location. Once the progress is already 1 or the distance to the target point is low enough, we end vaulting.

For the animation, I created a function called `IsVaulting` marked with `UFUNCTION` in the character's .h file, which checks if the character is currently vaulting. Then, I use this function in the animation blueprint of the skeletal mesh to set a local variable for updating the animation. This local variable is used in the animation graph to determine which pose to use. As I'm not an expert animator, the arm animation simply rotates the arms downwards. The duration of this animation is set to match the duration of the vault action.

## Hook
The hook mechanic is relatively simple: it travels for a certain distance, and if it hits something, it pulls the player to the contact surface. The hook consists of a hook head (just a sphere) and a cable (the Unreal Cable Actor). In the Player Character Blueprint, there is a spawn location defined for the hook head to start from. When the player triggers the hook with the right-click, the hook head spawns at this location, and the cable attaches to it and the starting location.

Next, we compute the travel direction, which is the direction from the start location near the player to a target location. We determine the target location by performing a Line Trace from the player's location to the camera's forward direction, up to a considerable distance. If the Line Trace hits something, the target location is the impact point; otherwise, it's the Line Trace endpoint. The hook head travels continuously in that direction until it hits something. If it doesn't hit anything, the shot is canceled. If it does, the hook head stops moving, and we start pulling the player.

Initially, I tried a force-based approach, where a force was applied each frame to the player towards the hook head location. However, this led to somewhat unpredictable behavior, making it challenging to find the ideal force for the desired velocity, and the movement was awkward. Thus, I changed my approach to set the character's velocity instead. This approach was much easier to tune and followed the hook line more accurately. I also added the possibility to add some velocity to the sides or up and down, to have a bit more control. I accomplished this by storing a speed for the character's right vector and another for the up vector. When input is added in some axis, it increases in that axis, and when there is no input, it decreases.

Note that before pulling the character, movement properties in its `UCharacterMovementComponent` must be set to avoid strange behavior. I set the gravity, ground friction, and air control to 0. After the grappling is finished, these properties must be restored.

The "hardest" part of this process is checking if you arrived or passed the hook, as it's possible to do so (especially with the force-based approach). The pulling process usually ends when the character is close enough to the hook, but sometimes, passing the hook head while still far away can create unpredictable movement where the player is constantly being pulled to the hook and spinning around. For this reason, I also check if we pass the hook head. I do this by storing the original hook direction and checking the dot product between this original direction and the current direction from the player to the hook head. If it's negative, we passed the hook; otherwise, we are still in a valid direction. For the dot product I only use the X and Y components for both directions.
