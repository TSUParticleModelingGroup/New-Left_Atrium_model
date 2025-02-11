/*
 This file contains all the functions that read in the nodes and muscle, links them together, 
 sets up the node and muscle atributes, and asigns them there values in our units.
 The functions are listed below in the order they appear.
 
 void setNodesFromBlenderFile();
 void checkNodes();
 void setMusclesFromBlenderFile();
 void linkNodesToMuscles();
 double getLogNormal();
 void setMuscleAttributesAndNodeMasses();
*/

void setNodesFromBlenderFile()
{	
	FILE *inFile;
	float x, y, z;
	int id;
	char fileName[256];
	
	// Generating the name of the file that holds the nodes.
	char directory[] = "./NodesMuscles/";
	strcpy(fileName, "");
	strcat(fileName, directory);
	strcat(fileName, NodesMusclesFileName);
	strcat(fileName, "/Nodes");
	
	// Opening the node file.
	inFile = fopen(fileName,"rb");
	if(inFile == NULL)
	{
		printf("\n Can't open Nodes file.\n");
		exit(0);
	}
	
	// Reading the header information.
	fscanf(inFile, "%d", &NumberOfNodes);
	printf("\n NumberOfNodes = %d", NumberOfNodes);
	fscanf(inFile, "%d", &PulsePointNode);
	printf("\n PulsePointNode = %d", PulsePointNode);
	fscanf(inFile, "%d", &UpNode);
	printf("\n UpNode = %d", UpNode);
	fscanf(inFile, "%d", &FrontNode);
	printf("\n FrontNode = %d", FrontNode);
	
	// Allocating memory for the CPU and GPU nodes. 
	Node = (nodeAtributesStructure*)malloc(NumberOfNodes*sizeof(nodeAtributesStructure));
	cudaMalloc((void**)&NodeGPU, NumberOfNodes*sizeof(nodeAtributesStructure));
	cudaErrorCheck(__FILE__, __LINE__);
	
	// Setting all nodes to zero or their default settings; 
	for(int i = 0; i < NumberOfNodes; i++)
	{
		Node[i].position.x = 0.0;
		Node[i].position.y = 0.0;
		Node[i].position.z = 0.0;
		Node[i].position.w = 0.0;
		
		Node[i].velocity.x = 0.0;
		Node[i].velocity.y = 0.0;
		Node[i].velocity.z = 0.0;
		Node[i].velocity.w = 0.0;
		
		Node[i].force.x = 0.0;
		Node[i].force.y = 0.0;
		Node[i].force.z = 0.0;
		Node[i].force.w = 0.0;
		
		Node[i].mass = 0.0;
		Node[i].area = 0.0;
		
		Node[i].beatNode = false; // Setting all nodes to start out as not be a beat node.
		Node[i].beatPeriod = -1.0; // Setting bogus number so it will throw a flag later if something happens latter on.
		Node[i].beatTimer = -1.0; // Setting bogus number so it will throw a flag later if something happens latter on.
		Node[i].fire = false; // Setting the node fire button to false so it will not fire as soon as it is turned on.
		Node[i].ablated = false; // Setting all nodes to not ablated.
		Node[i].drawNode = false; // This flag will allow you to draw certain nodes even when the draw nodes flag is set to off. Set it to off to start with.
		
		// Setting all node colors to not ablated (green)
		Node[i].color.x = 0.0;
		Node[i].color.y = 1.0;
		Node[i].color.z = 0.0;
		Node[i].color.w = 0.0;
		
		for(int j = 0; j < MUSCLES_PER_NODE; j++)
		{
			Node[i].muscle[j] = -1; // -1 sets the muscle to not used.
		}
	}
	
	// Reading in the nodes positions.
	for(int i = 0; i < NumberOfNodes; i++)
	{
		fscanf(inFile, "%d %f %f %f", &id, &x, &y, &z);
		
		Node[id].position.x = x;
		Node[id].position.y = y;
		Node[id].position.z = z;
	}
	
	fclose(inFile);
	printf("\n Blender generated nodes have been created.");
}

/* This functions checks to see if two nodes are too close relative to all the other nodes 
   in the simulations. 
   1: This for loop finds all the nearest nieghbor distances. Then it takes the average of this value. 
      This get a sense of how close nodes are in general. If you have more nodes they are going to be 
      closer together, this number just gets you a scale to compair to.
   2: This for loop checks to see if two nodes are closer than an cutoffDivider times smaller than the 
      average minimal distance. If it is, the nodes are printed out with thier seperation and a flag is set.
      Adjust the cutoffDivider for tighter and looser tollerances.
   3: If the flag was set the simulation is terminated so the user can correct the node file that contains the faulty nodes.
*/
void checkNodes()
{
	float dx, dy, dz, d;
	float averageMinSeperation, minSeperation;
	int flag;
	float cutoffDivider = 100.0;
	float cutoff;
	
	// 1: Finding average nearest nieghbor distance.
	averageMinSeperation = 0;
	for(int i = 0; i < NumberOfNodes; i++)
	{
		minSeperation = 10000000.0; // Setting min as a huge value just to get it started.
		for(int j = 0; j < NumberOfNodes; j++)
		{
			if(i != j)
			{
				dx = Node[i].position.x - Node[j].position.x;
				dy = Node[i].position.y - Node[j].position.y;
				dz = Node[i].position.z - Node[j].position.z;
				d = sqrt(dx*dx + dy*dy + dz*dz);
				if(d < minSeperation) 
				{
					minSeperation = d;
				}
			}
		}
		averageMinSeperation += minSeperation;
	}
	averageMinSeperation = averageMinSeperation/NumberOfNodes;
	
	// 2: Checking to see if nodes are too close together.
	cutoff = averageMinSeperation/cutoffDivider;
	flag =0;
	for(int i = 0; i < NumberOfNodes; i++)
	{
		for(int j = 0; j < NumberOfNodes; j++)
		{
			if(i != j)
			{
				dx = Node[i].position.x - Node[j].position.x;
				dy = Node[i].position.y - Node[j].position.y;
				dz = Node[i].position.z - Node[j].position.z;
				d = sqrt(dx*dx + dy*dy + dz*dz);
				if(d < cutoff)
				{
					printf("\n Nodes %d and %d are too close. Their separation is %f", i, j, d);
					flag = 1;
				}
			}
		}
	}
	
	// 3: Terminating the simulation if nodes were flagged.
	if(flag == 1)
	{
		printf("\n The average nearest seperation for all the nodes is %f.", averageMinSeperation);
		printf("\n The cutoff seperation was %f.\n\n", averageMinSeperation/10.0);
		exit(0);
	}
	printf("\n Nodes have been checked for minimal separation.");
}

void setMusclesFromBlenderFile()
{	
	FILE *inFile;
	int id, idNode1, idNode2;
	char fileName[256];
    
	// Generating the name of the file that holds the muscles.
	char directory[] = "./NodesMuscles/";
	strcpy(fileName, "");
	strcat(fileName, directory);
	strcat(fileName, NodesMusclesFileName);
	strcat(fileName, "/Muscles");
	
	// Opening the muscle file.
	inFile = fopen(fileName,"rb");
	if (inFile == NULL)
	{
		printf("\n Can't open Muscles file.\n");
		exit(0);
	}
	
	fscanf(inFile, "%d", &NumberOfMuscles);
	printf("\n NumberOfMuscles = %d", NumberOfMuscles);
	
	// Allocating memory for the CPU and GPU muscles. 
	Muscle = (muscleAtributesStructure*)malloc(NumberOfMuscles*sizeof(muscleAtributesStructure));
	cudaMalloc((void**)&MuscleGPU, NumberOfMuscles*sizeof(muscleAtributesStructure));
	cudaErrorCheck(__FILE__, __LINE__);

	// Setting all muscles to their default settings; 
	for(int i = 0; i < NumberOfMuscles; i++)
	{
		Muscle[i].nodeA = -1;
		Muscle[i].nodeB = -1;
		Muscle[i].apNode = -1;
		Muscle[i].on = false;
		Muscle[i].disabled = false;
		Muscle[i].timer = -1.0;
		Muscle[i].mass = -1.0;
		Muscle[i].naturalLength = -1.0;
		Muscle[i].relaxedStrength = -1.0;
		Muscle[i].compresionStopFraction = -1.0;
		Muscle[i].conductionVelocity = -1.0;
		Muscle[i].conductionDuration = -1.0;
		Muscle[i].contractionDuration = -1.0;
		Muscle[i].contractionStrength = -1.0;
		Muscle[i].rechargeDuration = -1.0;
		
		// Setting all muscle colors to ready (red)
		Muscle[i].color.x = 1.0;
		Muscle[i].color.y = 0.0;
		Muscle[i].color.z = 0.0;
		Muscle[i].color.w = 0.0;
	}
	
	// Reading in from the blender file what two nodes the muscle connect.
	for(int i = 0; i < NumberOfMuscles; i++)
	{
		fscanf(inFile, "%d", &id);
		fscanf(inFile, "%d", &idNode1);
		fscanf(inFile, "%d", &idNode2);
		
		if(NumberOfMuscles <= id)
		{
			printf("\n You are trying to create a muscle that is out of bounds.\n");
			exit(0);
		}
		if(NumberOfNodes <= idNode1 || NumberOfNodes <= idNode2)
		{
			printf("\n You are trying to conect to a node that is out of bounds.\n");
			exit(0);
		}
		Muscle[id].nodeA = idNode1;
		Muscle[id].nodeB = idNode2;
	}
	
	fclose(inFile);
	printf("\n Blender generated muscles have been created.");
}

// This code connects the muscles to the nodes.
void linkNodesToMuscles()
{	
	int k;
	// Each node will have a list of muscles they are attached to.
	for(int i = 0; i < NumberOfNodes; i++)
	{
		k = 0;
		for(int j = 0; j < NumberOfMuscles; j++)
		{
			if(Muscle[j].nodeA == i || Muscle[j].nodeB == i)
			{
				if(MUSCLES_PER_NODE < k)
				{
					printf("\n Number of muscles connected to node %d larger than the allowed number of", i);
					printf("\n connected to a single node.");
					printf("\n If this is not a mistake increase MUSCLES_PER_NODE in the hearer.h file.");
					exit(0);
				}
				Node[i].muscle[k] = j;
				//printf("\n node id = %d, node muscle id = %d , muscle id = %d", i, k, j);
				k++;
			}
		}
	}
	printf("\n Nodes have been linked to muscles");
}

double getLogNormal()
{
	//time_t t;
	// Seading the random number generater.
	//srand((unsigned) time(&t));
	double temp1, temp2;
	double randomNumber;
	int test;
	
	// Getting two uniform random numbers in [0,1]
	temp1 = ((double) rand() / (RAND_MAX));
	temp2 = ((double) rand() / (RAND_MAX));
	test = 0;
	while(test ==0)
	{
		// Getting ride of the end points so now random number is in (0,1)
		if(temp1 == 0 || temp1 == 1 || temp2 == 0 || temp2 == 1) 
		{
			test = 0;
		}
		else
		{
			// Using Box-Muller to get a standard normal random number.
			randomNumber = cos(2.0*PI*temp2)*sqrt(-2 * log(temp1));
			// Creating a log-normal distrobution from the normal randon number.
			randomNumber = exp(randomNumber);
			test = 1;
		}

	}
	return(randomNumber);	
}

void setMuscleAttributesAndNodeMasses()
{	
	float dx, dy, dz, d;
	float sum, totalLengthOfAllMuscles;
	float totalSurfaceAreaUsed, totalMassUsed;
	int count;
	float averageRadius, areaSum, areaAdjustment;
	int k;
	int muscleNumber;
	time_t t;
	int muscleTest, muscleTryCount, muscleTryCountMax;
	
	muscleTryCountMax = 100;
	
	// Seading the random number generater.
	srand((unsigned) time(&t));
	
	// Mogy need to work on this ??????
	// Getting some method of asigning an area to a node so we can get a force from the pressure.
	// We are letting the area be the circle made from the average radius out from a the node in question.
	// This will leave some area left out so we will perportionatly distribute this extra area out to the nodes as well.
	// If shape is a circle first we divide by the number of divsions to get the surface area of a great circler.
	// Then scale by the ratio of the circle compared to a great circle.
	// Circles seem to handle less pressure with this sceam so we downed the pressure by 2/3
	// On the atria1 shape we took out the area that the superiorVenaCava and inferiorVenaCava cover. 
	totalSurfaceAreaUsed = 4.0*PI*RadiusOfAtria*RadiusOfAtria;
	
	// Now we are finding the average radius from a node out to all it's nieghbor nodes.
	// Then setting its area to a circle of half this radius.
	areaSum = 0.0;
	for(int i = 0; i < NumberOfNodes; i++)
	{
		averageRadius = 0.0;
		count = 0;
		for(int j = 0; j < MUSCLES_PER_NODE; j++)
		{
			if(NumberOfNodes*MUSCLES_PER_NODE <= (i*MUSCLES_PER_NODE + j))
			{
				printf("\n TSU Error: number of ConnectingNodes is out of bounds in function setMuscleAttributesAndNodeMasses\n");
				exit(0);
			}
			
			muscleNumber = Node[i].muscle[j];
			if(muscleNumber != -1)
			{
				// The muscle is connected to two nodes. One to me and one to you. Need to find out who you are and not connect to myself.
				k = Muscle[muscleNumber].nodeA;
				if(k == i) k = Muscle[muscleNumber].nodeB;
				dx = Node[k].position.x - Node[i].position.x;
				dy = Node[k].position.y - Node[i].position.y;
				dz = Node[k].position.z - Node[i].position.z;
				averageRadius += sqrt(dx*dx + dy*dy + dz*dz);
				count++;
			}
		}
		if(count != 0) 
		{
			averageRadius /= count; // Getting the average radius; 
			averageRadius /= 2.0; // taking half that radius; 
			Node[i].area = PI*averageRadius*averageRadius; 
		}
		else
		{
			Node[i].area = 0.0; 
		}
		areaSum += Node[i].area;
	}
	
	areaAdjustment = totalSurfaceAreaUsed - areaSum;
	if(0.0 < areaAdjustment)
	{
		for(int i = 0; i < NumberOfNodes; i++)
		{
			if(areaSum < 0.00001)
			{
				printf("\n TSU Error: areaSum is too small (%f)\n", areaSum);
				exit(0);
			}
			else 
			{
				Node[i].area += areaAdjustment*Node[i].area/areaSum;
			}
		}
	}
	
	// Need to work on this Mogy ?????????
	// Setting the total mass used. If it is a sphere it is just the mass of the atria.
	// If shape is a circle first we divide by the number of divsions to get the mass a great circler.
	// Then scale by the ratio of the circle compared to a great circle.
	totalMassUsed = MassOfAtria; 
	// Taking out the mass of the two vena cava holes. It should be the same ration as the ratio of the surface areas.
	totalMassUsed *= totalSurfaceAreaUsed/(4.0*PI*RadiusOfAtria*RadiusOfAtria);
	
	//Finding the length of each muscle and the total length of all muscles.
	totalLengthOfAllMuscles = 0.0;
	for(int i = 0; i < NumberOfMuscles; i++)
	{	
		dx = Node[Muscle[i].nodeA].position.x - Node[Muscle[i].nodeB].position.x;
		dy = Node[Muscle[i].nodeA].position.y - Node[Muscle[i].nodeB].position.y;
		dz = Node[Muscle[i].nodeA].position.z - Node[Muscle[i].nodeB].position.z;
		d = sqrt(dx*dx + dy*dy + dz*dz);
		Muscle[i].naturalLength = d;
		totalLengthOfAllMuscles += d;
	}
	
	// Setting the mass of all muscles.
	for(int i = 0; i < NumberOfMuscles; i++)
	{	
		Muscle[i].mass = totalMassUsed*(Muscle[i].naturalLength/totalLengthOfAllMuscles);
	}
	
	// Setting muscle timing functions
	for(int i = 0; i < NumberOfMuscles; i++)
	{
		Muscle[i].timer = 0.0;
		
		muscleTest = 0;
		muscleTryCount = 0;
		while(muscleTest == 0)
		{
			Muscle[i].conductionVelocity = BaseMuscleConductionVelocity;
			Muscle[i].conductionDuration = Muscle[i].naturalLength/Muscle[i].conductionVelocity;
			
			Muscle[i].contractionDuration = BaseMuscleContractionDuration;
			
			Muscle[i].rechargeDuration = BaseMuscleRechargeDuration;
			
			// If it takes the electrical wave longer to cross the muscle than it does to get ready 
			// to fire a muscle could excite itself.
			if(Muscle[i].conductionDuration < Muscle[i].contractionDuration + Muscle[i].rechargeDuration)
			{
				muscleTest = 1;
			}
			
			muscleTryCount++;
			if(muscleTryCountMax < muscleTryCount)
			{
				printf(" \n You have tried to create muscle %d over %d times. You need to reset your muscle timing settings.\n", i, muscleTryCountMax);
				printf(" Good Bye\n");
				exit(0);
			}
		}
	}
	
	// Setting strength functions.
	for(int i = 0; i < NumberOfMuscles; i++)
	{	
		Muscle[i].contractionStrength = MyocyteForcePerMassMultiplier*MyocyteForcePerMass*Muscle[i].mass;
		
		Muscle[i].relaxedStrength = BaseMuscleRelaxedStrengthFraction*Muscle[i].contractionStrength;
		
		// Making sure the muscle will not contract too much or get longer when it is suppose to shrink.
		muscleTest = 0;
		muscleTryCount = 0;
		while(muscleTest == 0)
		{
			Muscle[i].compresionStopFraction = BaseMuscleCompresionStopFraction;
			
			if(0.5 < Muscle[i].compresionStopFraction && Muscle[i].compresionStopFraction < 1.0)
			{
				muscleTest = 1;
			}
			
			muscleTryCount++;
			if(muscleTryCountMax < muscleTryCount)
			{
				printf(" \n You have tried to create muscle %d over %d times. You need to reset your muscle contraction length settings.\n", i, muscleTryCountMax);
				printf(" Good Bye\n");
				exit(0);
			}
		}
	}
	
	// Setting the node masses
	for(int i = 0; i < NumberOfNodes; i++)
	{
		sum = 0.0;
		for(int j = 0; j < MUSCLES_PER_NODE; j++)
		{
			if(Node[i].muscle[j] != -1)
			{
				sum += Muscle[Node[i].muscle[j]].mass;
			}
		}
		Node[i].mass = sum/2.0;
	}
	printf("\n Muscle Attributes And Node Masses have been set");
}

