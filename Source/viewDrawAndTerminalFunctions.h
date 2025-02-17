/*
 This file contains all the draw and draw related function and the function that does the terminal print. 
 In short the is file holds the functions that display to the user.
 The functions are listed below in the order they appear.
 
 float4 findCenterOfMass();
 void centerObject();
 void rotateXAxis(float);
 void rotateYAxis(float);
 void rotateZAxis(float);
 void ReferenceView();
 void PAView();
 void APView();
 void setView(int);
 void drawPicture();
 void terminalPrint();
*/

float4 findCenterOfMass()
{
	float4 centerOfMass;
	
	centerOfMass.x = 0.0;
	centerOfMass.y = 0.0;
	centerOfMass.z = 0.0;
	centerOfMass.w = 0.0;
	for(int i = 0; i < NumberOfNodes; i++)
	{
		 centerOfMass.x += Node[i].position.x*Node[i].mass;
		 centerOfMass.y += Node[i].position.y*Node[i].mass;
		 centerOfMass.z += Node[i].position.z*Node[i].mass;
		 centerOfMass.w += Node[i].mass;
	}
	if(centerOfMass.w < 0.00001)
	{
		printf("\n Mass is too small\n");
		printf("\nw Good Bye\n");
		exit(0);
	}
	else
	{
		centerOfMass.x /= centerOfMass.w;
		centerOfMass.y /= centerOfMass.w;
		centerOfMass.z /= centerOfMass.w;
	}
	return(centerOfMass);
}

void centerObject()
{
	float4 centerOfMass = findCenterOfMass();
	for(int i = 0; i < NumberOfNodes; i++)
	{
		Node[i].position.x -= centerOfMass.x;
		Node[i].position.y -= centerOfMass.y;
		Node[i].position.z -= centerOfMass.z;
		
		Node[i].velocity.x = 0.0;
		Node[i].velocity.y = 0.0;
		Node[i].velocity.z = 0.0;
	}
	CenterOfSimulation.x = 0.0;
	CenterOfSimulation.y = 0.0;
	CenterOfSimulation.z = 0.0;
}

void rotateXAxis(float angle)
{
	float temp;
	for(int i = 0; i < NumberOfNodes; i++)
	{
		temp = cos(angle)*Node[i].position.y - sin(angle)*Node[i].position.z;
		Node[i].position.z  = sin(angle)*Node[i].position.y + cos(angle)*Node[i].position.z;
		Node[i].position.y  = temp;
	}
	AngleOfSimulation.x += angle;
}

void rotateYAxis(float angle)
{
	float temp;
	for(int i = 0; i < NumberOfNodes; i++)
	{
		temp =  cos(-angle)*Node[i].position.x + sin(-angle)*Node[i].position.z;
		Node[i].position.z  = -sin(-angle)*Node[i].position.x + cos(-angle)*Node[i].position.z;
		Node[i].position.x  = temp;
	}
	AngleOfSimulation.y += angle;
}

void rotateZAxis(float angle)
{
	float temp;
	for(int i = 0; i < NumberOfNodes; i++)
	{
		temp = cos(angle)*Node[i].position.x - sin(angle)*Node[i].position.y;
		Node[i].position.y  = sin(angle)*Node[i].position.x + cos(angle)*Node[i].position.y;
		Node[i].position.x  = temp;
	}
	AngleOfSimulation.z += angle;
}

void ReferenceView()
{
	// We set the reference view looking straight at the four
	// pulminary viens with a vien in each of the four quadrants 
	// of the x-y plane as semetric as you can make it with the 
	// valve down.
	
	float angle, temp;
	
	centerObject();
		
	// Rotating until the up Node is on x-y plane above or below the positive x-axis.
	angle = atan(Node[UpNode].position.z/Node[UpNode].position.x);
	if(Node[UpNode].position.x < 0.0) angle -= PI;
	for(int i = 0; i < NumberOfNodes; i++)
	{
		temp = cos(angle)*Node[i].position.x + sin(angle)*Node[i].position.z;
		Node[i].position.z  = -sin(angle)*Node[i].position.x + cos(angle)*Node[i].position.z;
		Node[i].position.x  = temp;
	}
	AngleOfSimulation.y += angle;
	
	// Rotating until up Node is on the positive y axis.
	angle = PI/2.0 - atan(Node[UpNode].position.y/Node[UpNode].position.x);
	for(int i = 0; i < NumberOfNodes; i++)
	{
		temp = cos(angle)*Node[i].position.x - sin(angle)*Node[i].position.y;
		Node[i].position.y  = sin(angle)*Node[i].position.x + cos(angle)*Node[i].position.y;
		Node[i].position.x  = temp;
	}
	AngleOfSimulation.z += angle;
	
	// Rotating until front Node is on the positive z axis.
	angle = atan(Node[FrontNode].position.z/Node[FrontNode].position.x) - PI/2.0;
	if(Node[FrontNode].position.x < 0.0) angle -= PI;
	for(int i = 0; i < NumberOfNodes; i++)
	{
		temp = cos(angle)*Node[i].position.x + sin(angle)*Node[i].position.z;
		Node[i].position.z  = -sin(angle)*Node[i].position.x + cos(angle)*Node[i].position.z;
		Node[i].position.x  = temp;
	}
	AngleOfSimulation.y += angle;
}

void PAView()
{  
	// The heart does not set in the chest at a straight on angle.
	// Hense we need to adjust our view to what they use in the EP lab.
	// We first set the AP view because it is the closest to ours.
	float angle;
	
	angle = PI/6.0; // Rotate 30 degrees counterclockwise on the y-axis 
	rotateYAxis(angle);

	angle = PI/6.0; // Rotate 30 degrees counterclockwise on the z-axis
	rotateZAxis(angle);
}

void APView()
{ 
	// To get the AP view we just rotate the PA view 180 degrees on the y-axis
	float angle;
	
	PAView();
	angle = PI; // Rotate 180 degrees counterclockwise on the y-axis 
	rotateYAxis(angle);
}

void setView(int view)
{
	// Putting object into reference view because everything is based off of this.
	ReferenceView();
	
	if(view == 6)
	{
		// It is already in reference view from the function call above.
		strcpy(ViewName, "Ref");
	}
	if(view == 4)
	{
		PAView();
		strcpy(ViewName, "PA");
	}
	if(view == 2)
	{
		APView();
		strcpy(ViewName, "AP");
	}
	if(view == 3)
	{
		APView();
		rotateYAxis(-PI/6.0);
		strcpy(ViewName, "RAO");
	}
	if(view == 1)
	{
		APView();
		rotateYAxis(PI/3.0);
		strcpy(ViewName, "LAO");
	}
	if(view == 7)
	{
		APView();
		rotateYAxis(PI/2.0);
		strcpy(ViewName, "LL");
	}
	if(view == 9)
	{
		APView();
		rotateYAxis(-PI/2.0);
		strcpy(ViewName, "RL");
	}
	if(view == 8)
	{
		APView();
		rotateXAxis(PI/2.0);
		strcpy(ViewName, "SUP");
	}
	if(view == 5)
	{
		APView();
		rotateXAxis(-PI/2.0);
		strcpy(ViewName, "INF");
	}
	else
	{
		printf("\n Undefined view reverting back to Ref view.");
	}
}

void drawPicture()
{
	//int nodeNumber;
	int muscleNumber;
	int k;
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	// Draw Nodes
	// Pulse node
	if(PauseIs == false) glColor3d(0.0,1.0,0.0);
	else glColor3d(1.0,0.0,0.0);
	glPushMatrix();
	glTranslatef(Node[PulsePointNode].position.x, Node[PulsePointNode].position.y, Node[PulsePointNode].position.z);
	glutSolidSphere(0.03*RadiusOfLeftAtrium,20,20);
	glPopMatrix();
	
	// Drawing center node
	
	glColor3d(1.0,1.0,1.0);
	glPushMatrix();
	glTranslatef(CenterOfSimulation.x, CenterOfSimulation.y, CenterOfSimulation.z);
	glutSolidSphere(0.02*RadiusOfLeftAtrium,20,20);
	glPopMatrix();
	

	// Drawing other nodes
	if(DrawNodesFlag == 1 || DrawNodesFlag == 2)
	{
		for(int i = 1; i < NumberOfNodes; i++)
		{
			if(DrawFrontHalfFlag == 1 || DrawNodesFlag == 1)
			{
				if(CenterOfSimulation.z - 0.001 < Node[i].position.z)  // Only drawing the nodes in the front.
				{
					glColor3d(Node[i].color.x, Node[i].color.y, Node[i].color.z);
					glPushMatrix();
					glTranslatef(Node[i].position.x, Node[i].position.y, Node[i].position.z);
					glutSolidSphere(NodeRadiusAdjustment*RadiusOfLeftAtrium,20,20);
					glPopMatrix();
				}
			}
			else
			{
				glColor3d(Node[i].color.x, Node[i].color.y, Node[i].color.z);
				glPushMatrix();
				glTranslatef(Node[i].position.x, Node[i].position.y, Node[i].position.z);
				glutSolidSphere(NodeRadiusAdjustment*RadiusOfLeftAtrium,20,20);
				glPopMatrix();
			}	
		}
	}
	else
	{
		glPointSize(5.0);
		glBegin(GL_POINTS);
		 	for(int i = 1; i < NumberOfNodes; i++)
			{
				if(DrawFrontHalfFlag == 1)
				{
					if(CenterOfSimulation.z - 0.001 < Node[i].position.z)  // Only drawing the nodes in the front.
					{
						glColor3d(Node[i].color.x, Node[i].color.y, Node[i].color.z);
						if(Node[i].drawNodeIs == true)
						{
							glVertex3f(Node[i].position.x, Node[i].position.y, Node[i].position.z);
						}
					}
				}
				else
				{
					glColor3d(Node[i].color.x, Node[i].color.y, Node[i].color.z);
					if(Node[i].drawNodeIs == true)
					{
						glVertex3f(Node[i].position.x, Node[i].position.y, Node[i].position.z);
					}
				}
			}
		glEnd();
	}
	
	// Drawing muscles
	glLineWidth(LineWidth);
	for(int i = 0; i < NumberOfNodes; i++)
	{
		for(int j = 0; j < MUSCLES_PER_NODE; j++)
		{
			muscleNumber = Node[i].muscle[j];
			if(muscleNumber != -1)
			{
				k = Muscle[muscleNumber].nodeA;
				if(k == i) 
				{
					k = Muscle[muscleNumber].nodeB;
				}
				
				if(DrawFrontHalfFlag == 1)
				{
					if(CenterOfSimulation.z - 0.001 < Node[i].position.z && CenterOfSimulation.z - 0.001 < Node[k].position.z)  // Only drawing the nodes in the front.
					{
						glColor3d(Muscle[muscleNumber].color.x, Muscle[muscleNumber].color.y, Muscle[muscleNumber].color.z);
						glBegin(GL_LINES);
							glVertex3f(Node[i].position.x, Node[i].position.y, Node[i].position.z);
							glVertex3f(Node[k].position.x, Node[k].position.y, Node[k].position.z);
						glEnd();
					}
				}
				else
				{
					glColor3d(Muscle[muscleNumber].color.x, Muscle[muscleNumber].color.y, Muscle[muscleNumber].color.z);
					glBegin(GL_LINES);
						glVertex3f(Node[i].position.x, Node[i].position.y, Node[i].position.z);
						glVertex3f(Node[k].position.x, Node[k].position.y, Node[k].position.z);
					glEnd();
				}
			}
		}	
	}
	
	if(MouseFunctionModeIs == true)
	{
		//glColor3d(1.0, 1.0, 1.0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    		glEnable(GL_DEPTH_TEST);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		glPushMatrix();
		glTranslatef(MouseX, MouseY, MouseZ);
		
		glutSolidSphere(HitMultiplier*RadiusOfLeftAtrium,20,20);
		//glutSolidSphere(5.0*NodeRadiusAdjustment*RadiusOfAtria,20,20);
		glPopMatrix();
		glDisable(GL_BLEND);
	}

	glutSwapBuffers();
	
	if(MovieIsOn == true)
	{
		glReadPixels(5, 5, XWindowSize, YWindowSize, GL_RGBA, GL_UNSIGNED_BYTE, Buffer);
		fwrite(Buffer, sizeof(int)*XWindowSize*YWindowSize, 1, MovieFile);
	}
}

void terminalPrint()
{
	system("clear");
	//printf("\033[0;34m"); // blue.
	//printf("\033[0;36m"); // cyan
	//printf("\033[0;33m"); // yellow
	//printf("\033[0;31m"); // red
	//printf("\033[0;32m"); // green
	printf("\033[0m"); // back to white.
	
	printf("\n");
	printf("\033[0;33m");
	printf("\n **************************** Simulation Stats ****************************");
	printf("\033[0m");
	
	printf("\n Total run time = %7.2f milliseconds", RunTime);
	
	//printf("\n Driving beat node is %d.", EctopicEvents[0].node);
	printf("\n The beat rate is %f milliseconds.", Node[PulsePointNode].beatPeriod);
	printf("\n");
	
	if(AdjustMuscleAreaModeIs == true || AdjustMuscleLineModeIs == true) 
	{
		printf("\n Muscle refractory period multiplier =");
		printf("\033[0;36m");
		printf(" %f", RefractoryPeriodAdjustmentMultiplier);
		printf("\033[0m");
		printf("\033[0m");
		printf("\n Muscle electrical conduction speed multiplier =");
		printf("\033[0;36m");
		printf(" %f", MuscleConductionVelocityAdjustmentMultiplier);
		printf("\033[0m");
	}
	
	for(int i = 0; i < NumberOfNodes; i++)
	{
		if(Node[i].isBeatNode == true && i != PulsePointNode)
		{
			printf("\n Ectopic Beat Node = %d Rate = %f milliseconds.", i, Node[i].beatPeriod);
		}
	}
	
	printf("\033[0;33m");
	printf("\n **************************** Terminal Comands ****************************");
	printf("\033[0m");
	printf("\n h: Help");
	printf("\n c: Recenter View");
	printf("\n S: Screenshot");
	printf("\n k: Save Current Run");
	printf("\n B: Lengthen Beat");
	printf("\n b: Shorten Beat");
	printf("\n ?: Find Front and Top Nodes");
	printf("\n");
	
	printf("\n Toggles");
	printf("\n r: Run/Pause            - ");
	if (PauseIs == false) 
	{
		printf("\033[0;32m");
		printf(BOLD_ON "Simulation Running" BOLD_OFF);
	} 
	else 
	{
		printf("\033[0;31m");
		printf(BOLD_ON "Simulation Paused" BOLD_OFF);
	}
	printf("\n g: Front/Full           - ");
	if (DrawFrontHalfFlag == 0) printf(BOLD_ON "Full" BOLD_OFF); else printf(BOLD_ON "Front" BOLD_OFF);
	printf("\n n: Nodes Off/Half/Full  - ");
	if (DrawNodesFlag == 0) printf(BOLD_ON "Off" BOLD_OFF); else if (DrawNodesFlag == 1) printf(BOLD_ON "Half" BOLD_OFF); else printf(BOLD_ON "Full" BOLD_OFF);
	printf("\n v: Orthogonal/Frustum   - ");
	if (ViewFlag == 0) printf(BOLD_ON "Orthogonal" BOLD_OFF); else printf(BOLD_ON "Frustrum" BOLD_OFF);
	printf("\n m: Video On/Off         - ");
	if (MovieIsOn == false) 
	{
		printf("\033[0;31m");
		printf(BOLD_ON "Video Recording Off" BOLD_OFF); 
	}
	else 
	{
		printf("\033[0;32m");
		printf(BOLD_ON "Video Recording On" BOLD_OFF);
	}
	printf("\n");
	printf("\n Views" );
	
	printf("\n 7 8 9 | LL  SUP RL" );
	printf("\n 4 5 6 | PA  INF Ref" );
	printf("\n 1 2 3 | LOA AP  ROA" );
	
	printf("   You are in");
	printf("\033[0;36m");
	printf(BOLD_ON " %s", ViewName);
	printf("\033[0m" BOLD_OFF);
	printf(" view");
	
	printf("\n");
	printf("\n Adjust views");
	printf("\n w/s: CCW/CW x-axis");
	printf("\n d/a: CCW/CW y-axis");
	printf("\n z/Z: CCW/CW z-axis");
	printf("\n e/E: In/Out Zoom");
	printf("\n");
	printf("\n Set Mouse actions");
	
	printf("\n !: Ablate ---------------------- ");
	if (AblateModeIs == true) 
	{
		printf("\033[0;36m");
		printf(BOLD_ON "On" BOLD_OFF); 
	}
	else printf(BOLD_ON "Off" BOLD_OFF);
	
	printf("\n @: Ectoic Beat ----------------- ");
	if (EctopicBeatModeIs == true) 
	{
		printf("\033[0;36m");
		printf(BOLD_ON "On" BOLD_OFF); 
	}
	else printf(BOLD_ON "Off" BOLD_OFF);
	
	printf("\n #: Ectopic Trigger ------------- ");
	if (EctopicEventModeIs == true) 
	{
		printf("\033[0;36m");
		printf(BOLD_ON "On" BOLD_OFF); 
	}
	else printf(BOLD_ON "Off" BOLD_OFF);
	
	printf("\n $: Muscle Adjustment Area Mode - ");
	if (AdjustMuscleAreaModeIs == true) 
	{
		printf("\033[0;36m");
		printf(BOLD_ON "On" BOLD_OFF); 
	}
	else printf(BOLD_ON "Off" BOLD_OFF);
	
	printf("\n %%: Muscle Adjustment Line Mode - ");
	if (AdjustMuscleLineModeIs == true) 
	{
		printf("\033[0;36m");
		printf(BOLD_ON "On" BOLD_OFF); 
	}
	else printf(BOLD_ON "Off" BOLD_OFF);
	
	printf("\n ^: Identify Node --------------- ");
	if (FindNodeModeIs == true) 
	{
		printf("\033[0;36m");
		printf(BOLD_ON "On" BOLD_OFF); 
	}
	else printf(BOLD_ON "Off" BOLD_OFF);
	
	printf("\n ): Turns all Mouse functions off.");
	printf("\n");
	printf("\n [/]: (left/right bracket) Increase/Decrease mouse selection area.");
	printf("\n      The current selection area is");
	printf("\033[0;36m");
	printf(" %f", HitMultiplier);
	printf("\033[0m");
	printf(" times the radius of atrium.");
	printf("\033[0;33m");
	printf("\n ********************************************************************");
	printf("\033[0m");
	printf("\n");
}

