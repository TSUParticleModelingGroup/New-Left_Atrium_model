// nvcc SVT.cu -o svt -lglut -lm -lGLU -lGL

/*
 This file contains all the the main controller functions that setup the simulation, then run and manage the simulation.
 The functions are listed below in the order they appear.
 
 void n_body(float);
 void setupCudaInvironment();
 void readSimulationParameters();
 void setup();
 int main(int, char**);
*/

// Local include files
#include "./header.h"
#include "./setNodesAndMuscles.h"
#include "./callBackFunctions.h"
#include "./viewDrawAndTerminalFunctions.h"
#include "./cudaFunctions.h"

void n_body(float dt)
{	
	if(PauseIs == false)
	{	
		if(ContractionIsOn == true)
		{
			getForces<<<GridNodes, BlockNodes>>>(MuscleGPU, NodeGPU, dt, NumberOfNodes, CenterOfSimulation, MuscleCompresionStopFraction, RadiusOfLeftAtrium, DiastolicPressureLA, SystolicPressureLA);
			cudaErrorCheck(__FILE__, __LINE__);
			cudaDeviceSynchronize();
		}
		updateNodes<<<GridNodes, BlockNodes>>>(NodeGPU, NumberOfNodes, MUSCLES_PER_NODE, MuscleGPU, Drag, dt, RunTime, ContractionIsOn);
		cudaErrorCheck(__FILE__, __LINE__);
		cudaDeviceSynchronize();
		updateMuscles<<<GridMuscles, BlockMuscles>>>(MuscleGPU, NodeGPU, NumberOfMuscles, NumberOfNodes, dt, ReadyColor, ContractingColor, RestingColor, RelativeColor);
		cudaErrorCheck(__FILE__, __LINE__);
		cudaDeviceSynchronize();
		
		if(ContractionIsOn == true)
		{
			RecenterCount++;
			if(RecenterCount == RecenterRate) 
			{
				recenter<<<1, BLOCKCENTEROFMASS>>>(NodeGPU, NumberOfNodes, MassOfLeftAtrium, CenterOfSimulation);
				cudaErrorCheck(__FILE__, __LINE__);
				RecenterCount = 0;
			}
		}
		
		DrawTimer++;
		if(DrawTimer == DrawRate) 
		{
			copyNodesMusclesFromGPU();
			drawPicture();
			DrawTimer = 0;
		}
		
		PrintTimer += dt;
		if(PrintRate <= PrintTimer) 
		{
			terminalPrint();
			PrintTimer = 0.0;
		}
		
		RunTime += dt; 
	}
	else
	{
		drawPicture();
	}
}

void setupCudaInvironment()
{
	BlockNodes.x = BLOCKNODES;
	BlockNodes.y = 1;
	BlockNodes.z = 1;
	
	GridNodes.x = (NumberOfNodes - 1)/BlockNodes.x + 1;
	GridNodes.y = 1;
	GridNodes.z = 1;
	
	BlockMuscles.x = BLOCKMUSCLES;
	BlockMuscles.y = 1;
	BlockMuscles.z = 1;
	
	GridMuscles.x = (NumberOfMuscles - 1)/BlockMuscles.x + 1;
	GridMuscles.y = 1;
	GridMuscles.z = 1;
	
	if((BLOCKCENTEROFMASS > 0) && (BLOCKCENTEROFMASS & (BLOCKCENTEROFMASS - 1)) != 0) 
	{
        	printf("\nBLOCKCENTEROFMASS = %d. This is not a power of 2.", BLOCKCENTEROFMASS);
        	printf("\nBLOCKCENTEROFMASS must be a power of 2 for the center of mass reduction to work.");
        	printf("\nFix this number in the header.h file and try again.");
        	printf("\nGood Bye.\n");
        	exit(0);
        }
}

void readSimulationParameters()
{
	ifstream data;
	string name;
	
	data.open("./simulationSetup");
	
	if(data.is_open() == 1)
	{
		getline(data,name,'=');
		data >> NodesMusclesFileOrPreviousRunsFile;
		
		getline(data,name,'=');
		data >> NodesMusclesFileName;
		
		getline(data,name,'=');
		data >> PreviousRunFileName;
		
		getline(data,name,'=');
		data >> LineWidth;
		
		getline(data,name,'=');
		data >> NodeRadiusAdjustment;
		
		getline(data,name,'=');
		data >> MyocyteForcePerMass;
		
		getline(data,name,'=');
		data >> MyocyteForcePerMassMultiplier;
		
		getline(data,name,'=');
		data >> MyocyteForcePerMassSTD;
		
		getline(data,name,'=');
		data >> DiastolicPressureLA;
		
		getline(data,name,'=');
		data >> SystolicPressureLA;
		
		getline(data,name,'=');
		data >> PressureMultiplier;
		
		getline(data,name,'=');
		data >> MassOfLeftAtrium;
		
		getline(data,name,'=');
		data >> RadiusOfLeftAtrium;
		
		getline(data,name,'=');
		data >> Drag;
		
		getline(data,name,'=');
		data >> ContractionIsOn;
		
		getline(data,name,'=');
		data >> MuscleRelaxedStrengthFraction;
		
		getline(data,name,'=');
		data >> MuscleCompresionStopFraction;
		
		getline(data,name,'=');
		data >> MuscleCompresionStopFractionSTD;
		
		getline(data,name,'=');
		data >> BaseMuscleRefractoryPeriod;
		
		getline(data,name,'=');
		data >> MuscleRefractoryPeriodSTD;
		        
		getline(data,name,'=');
		data >> BaseAbsoluteRefractoryPeriodFraction;
		
		getline(data,name,'=');
		data >> AbsoluteRefractoryPeriodFractionSTD;
		
		getline(data,name,'=');
		data >> BaseMuscleConductionVelocity;
		
		getline(data,name,'=');
		data >> MuscleConductionVelocitySTD;
		
		getline(data,name,'=');
		data >> BeatPeriod;
		
		getline(data,name,'=');
		data >> PrintRate;
		
		getline(data,name,'=');
		data >> DrawRate;
		
		getline(data,name,'=');
		data >> Dt;
		
		getline(data,name,'=');
		data >> ReadyColor.x;
		
		getline(data,name,'=');
		data >> ReadyColor.y;
		
		getline(data,name,'=');
		data >> ReadyColor.z;
		
		getline(data,name,'=');
		data >> ContractingColor.x;
		
		getline(data,name,'=');
		data >> ContractingColor.y;
		
		getline(data,name,'=');
		data >> ContractingColor.z;
		
		getline(data,name,'=');
		data >> RestingColor.x;
		
		getline(data,name,'=');
		data >> RestingColor.y;
		
		getline(data,name,'=');
		data >> RestingColor.z;
		
		getline(data,name,'=');
		data >> RelativeColor.x;
		
		getline(data,name,'=');
		data >> RelativeColor.y;
		
		getline(data,name,'=');
		data >> RelativeColor.z;
		
		getline(data,name,'=');
		data >> DeadColor.x;
		
		getline(data,name,'=');
		data >> DeadColor.y;
		
		getline(data,name,'=');
		data >> DeadColor.z;
		
		getline(data,name,'=');
		data >> BackGroundRed;
		
		getline(data,name,'=');
		data >> BackGroundGreen;
		
		getline(data,name,'=');
		data >> BackGroundBlue;
	}
	else
	{
		printf("\nTSU Error could not open simulationSetup file\n");
		exit(0);
	}
	
	data.close();
	printf("\n Simulation Parameters have been read in.");
}

void setup()
{	
	// Seading the random number generater.
	time_t t;
	srand((unsigned) time(&t));
	
	readSimulationParameters();
	
	if(NodesMusclesFileOrPreviousRunsFile == 0)
	{
		setNodesFromBlenderFile();
		checkNodes();
		setMusclesFromBlenderFile();
		linkNodesToMuscles();
		setRemainingNodeAndMuscleAttributes();
		hardCodedAblations();
		hardCodedPeriodicEctopicEvents();
		hardCodedIndividualMuscleAttributes();
		for(int i = 0; i < NumberOfMuscles; i++)
		{	
			checkMuscle(i);
		}
	}
	else if(NodesMusclesFileOrPreviousRunsFile == 1)
	{
		getNodesandMusclesFromPreviuosRun();
	}
	else
	{
		printf("\n Bad NodesMusclesFileOrPreviousRunsFile type.");
		printf("\n Good Bye.");
		exit(0);
	}
	
	setRemainingParameters();
	
	setupCudaInvironment();
	
	copyNodesMusclesToGPU();
        
	printf("\n");
	char temp;
	printf("\033[0;31m");
	printf("\n\n The simulation has not been started.");
	printf("\n Hit any key and return to begin.\n\n");
	printf("\033[0m");
	scanf("%s", &temp); 
	
	terminalPrint();
}

int main(int argc, char** argv)
{
	setup();
	
	XWindowSize = 1000;
	YWindowSize = 1000; 

	// Clip plains
	Near = 0.2;
	Far = 80.0*RadiusOfLeftAtrium;

	//Direction here your eye is located location
	EyeX = 0.0*RadiusOfLeftAtrium;
	EyeY = 0.0*RadiusOfLeftAtrium;
	EyeZ = 2.0*RadiusOfLeftAtrium;

	//Where you are looking
	CenterX = 0.0;
	CenterY = 0.0;
	CenterZ = 0.0;

	//Up vector for viewing
	UpX = 0.0;
	UpY = 1.0;
	UpZ = 0.0;

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	glutInitWindowSize(XWindowSize,YWindowSize);
	glutInitWindowPosition(5,5);
	Window = glutCreateWindow("SVT");
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	
	gluLookAt(EyeX, EyeY, EyeZ, CenterX, CenterY, CenterZ, UpX, UpY, UpZ);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-0.2, 0.2, -0.2, 0.2, Near, Far);
	glMatrixMode(GL_MODELVIEW);
	glClearColor(BackGroundRed, BackGroundGreen, BackGroundBlue, 0.0);
	
	//GLfloat light_position[] = {EyeX, EyeY, EyeZ, 0.0};
	GLfloat light_position[] = {1.0, 1.0, 1.0, 1.0}; //where the light is: {x,y,z,w}, w=0.0 is infinite light aiming at x,y,z, w=1.0 is a point light radiating from x,y,z
	GLfloat light_ambient[]  = {1.0, 1.0, 1.0, 1.0}; //what color is the ambient light, {r,g,b,a}, a= opacity 1.0 is fully visible, 0.0 is invisible
	GLfloat light_diffuse[]  = {1.0, 1.0, 1.0, 1.0}; //does light reflect off of the object, {r,g,b,a}, a has no effect
	GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0}; //does light highlight shiny surfaces, {r,g,b,a}. i.e what light reflects to viewer
	GLfloat lmodel_ambient[] = {1.0, 1.0, 1.0, 1.0}; //global ambient light, {r,g,b,a}, applies uniformly to all objects in the scene
	GLfloat mat_specular[]   = {1.0, 1.0, 1.0, 1.0}; //reflective properties of an object, {r,g,b,a}, highlights are currently white
	GLfloat mat_shininess[]  = {128.0}; //how shiny is the surface of an object, 0.0 is dull, 128.0 is very shiny
	glShadeModel(GL_SMOOTH);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	
	//glutMouseFunc(mouseWheelCallback);
	//glutMouseWheelFunc(mouseWheelCallback);
	//glutMotionFunc(mouseMotionCallback);
    	glutPassiveMotionFunc(mousePassiveMotionCallback);
	glutDisplayFunc(Display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mymouse);
	glutKeyboardFunc(KeyPressed);
	glutIdleFunc(idle);
	glutSetCursor(GLUT_CURSOR_DESTROY);
	glEnable(GL_DEPTH_TEST);
	
	glutMainLoop();
	return 0;
}
