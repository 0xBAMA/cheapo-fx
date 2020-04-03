# HAUNTED

this is basically forked from SpAce






THINK ABOUT SORTING PISTONS (and the other elements of the engine) BY DISTANCE TO AVOID ALPHA ISSUES

//define enum for each piston, etc, so that when its that piston's turn, all the appropriate offse

  //pistons, valves, cams, pipes, conrod, crankshaft - used to direct behavior to the appropriate case in a switch statement to advance the animation of that particular element




ts can be calcuated in its draw function

typedef struct element_t
{
  //glm::vec3 representative_point_for_this_object; //used to compute distance to viewer - that function has to mirror the perspective
  //projection calculations that are done on the GPU, in order to get a correct result for depth within the scope of this problem.


  //mabe keep a second point - do some averaging - this would allow it to keep up with the dynamicmovement of the con rod







  float distance < ---- calculate this in a function, same calculation , then use this to sort



  int geometry_index_start; //in order to draw the object;
  int num_points;

  //texcoord holds information in the vertex array that will become color

  //this also has to hold the information to locate the object


} element;


this pattern is in place of what was becoming a growing list of integers being held in the private member variables - by making this a vector, we can simplify the process of sorting them, I'll have to look into if there's built- in sort stuff in the vector class. If there is, I'll go through, calculate the distance for all points, keep that







Theta is the rotation of the main crankshaft -

  this is multiplied by two to get the rotation of the camshafts - they will rotate twice per rotation of the crankshaft

  the pistons will be animated with an offset that is related to theta via a sinusoidal function - they have a linear path that they travel, and they will not ever be moving anywhere other than up and down in their travel. In the morning I will have to look into what I have written in my old code about calculating these points - this time I can get away with just a single piston's geometry - that is, other than the crank and cans, I have a unit set of geometry that will each have their offsets set with uniforms, that consists of the piston, the con rod, any bits you want to have at the connection of the con rod and the crank, like a bearing or something, a set of valves, and that's pretty much it - I think this should make things work
