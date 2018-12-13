/*************************************************
 * Space Vector
 * Started Dec. 11th 2018
 * Andrew Peterkort
 *************************************************/
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <unistd.h>//This is a unix/linux library, need replacement
#include <random>
#include <ctime>//needs replacement

/*************************************************
 * OBJECT: BLOCK 
 * uses a 64 bit seed number to create a 32x32 bit array of ones and zeros
 * each block represents a grid of astroids in space and is generated with
 * Conway's Game of Life like algorithims
 *************************************************/
class Block {

	public:
		uint32_t seed; //stores the initial seed for random generation
		uint32_t* block; //stores the current block of 32x32 bit astroid layout
		void printBlock();
		void generateBlock();//prints out the progression of generations
		Block(uint32_t,std::mt19937*);
		~Block();

	private:
		uint32_t* drawBlock; //this is the buffer block that has the new fram written to it
		bool readCell(int, int, uint32_t*); //reads if a cell exits at x,y
		void writeCell(int, int, uint32_t*); //writes a cell to x,y
		void deleteCell(int, int, uint32_t*); //deletes cell at x,y
		int countNeighbors(int, int, uint32_t*); //countes the total neghboring cells to x,y
		bool cycleConway(); //plays one frame of the game
		void clearScreen();
		std::mt19937* randGen;

};

bool Block::readCell(int x, int y, uint32_t* outBlock){ 

	return (bool)((outBlock[y] >> x) & (uint32_t)1);

}

void Block::writeCell(int x, int y, uint32_t* inBlock){

	uint32_t numAdded = 1;
	numAdded = numAdded << x;
	inBlock[y] = inBlock[y] | numAdded;

}

void Block::deleteCell(int x, int y, uint32_t* inBlock){

	uint32_t numAdded = 1;
	numAdded = numAdded << x;
	inBlock[y] = inBlock[y] & ~numAdded;

}

int Block::countNeighbors(int x, int y, uint32_t* inBlock){

	int count = 0;
	//these limits are set as limits for the differences to count
	//around x,y.
	int xLowLimit = -1;
	int xHighLimit = 1;
	int yLowLimit = -1;
	int yHighLimit = 1;

	//these limits are adjusted to deal with corners and edges to prevent
	//the program from trying to access memory outside the allocated portion
	if(x == 0)
		xLowLimit = 0;
	if(x == 31)
		xHighLimit = 0;
	if(y == 0)
		yLowLimit = 0;
	if(y == 31)
		yHighLimit = 0;

	//the limits are then applied to counting around the cell
	for( int xOffset = xLowLimit; xOffset <= xHighLimit; xOffset++ ){

		for( int yOffset = yLowLimit; yOffset <= yHighLimit; yOffset++ ){

			if((xOffset == 0) && (yOffset == 0)) //we don't want to count ourselves
				continue;

			if(readCell(x + xOffset,y + yOffset,inBlock) == true)
				count++;

		}

	}

	return count;

}

//each position needs to have the cells around it counted so it can
//decide the fait of the cell or no-cell at the current position
//drawBlock starts each new cycle as a copy of block so cells that
//stay allive with value 2 are left alone... etc.
bool Block::cycleConway(){

	uint32_t maskMod32 = 31;
	int numNeighbors;
	int totalNeighbors = 0;

	for( int y = 0; y < 32; y++ ){

		for( int x = 0; x < 32; x++ ){

			numNeighbors = countNeighbors(x,y,block);
			totalNeighbors += numNeighbors; //add the total neighbors to see if there are any cells in the block

			//more entropy is added through randGen to make sure the algo does not make common identifiable
			//shapes and instead the astroid groups look more random. Uses maskMod32 for more effecient bit masking vs moding
			if( (numNeighbors == 3) || ((numNeighbors == 2) && (((*randGen)() & maskMod32) == 1)) ){

				writeCell(x,y,drawBlock);

			}else if( numNeighbors < 2 ){

				deleteCell(x,y,drawBlock);

			}

		}

	}

	memcpy(block,drawBlock,128);
	
	if( totalNeighbors == 0 ) //if there are no cells in teh block, return false to tell the lower function
		return false;

	return true;

}

void Block::clearScreen(){

	for( int i = 0; i < 100; i++ ){

		printf("\n");

	}

}

void Block::printBlock(){

	for( int y = 0; y < 32; y++ ){

		for( int x = 0; x < 32; x++ ){

			printf(" ");

			if( readCell(x,y,block) == true ){

				printf("*");

			}else{

				printf(" ");

			}

		}

		printf("\n");

	}

}

void Block::generateBlock(){

	for( int i = 0; i < 20; i++ ){

		clearScreen();
		printBlock();

		if( cycleConway() == false ) //if cycleConway detects no cells in the block
			break;

		usleep(100000); //this call needs to be replaced in the future for cross platform ability

	}

}

Block::Block(uint32_t seed, std::mt19937* randGenerator){

	uint32_t maskMod32 = 31;
	this->seed = seed;
	this->randGen = randGenerator;
	randGenerator->seed(seed);
	block = new uint32_t [32];
	drawBlock = new uint32_t [32];

	for( int i = 0; i < 50; i++ )
		writeCell((*this->randGen)() & maskMod32, (*this->randGen)() & maskMod32, block); //masking to make clear that we are not creating
	//and statistical non-uniformaties with our random numbers

	memcpy(drawBlock,block,128);

}

Block::~Block(){

	delete[] block;
	delete[] drawBlock;

}

int main() {

	uint32_t seed = time(NULL); //this will be replaced eventually
	std::mt19937* randGenerator = new std::mt19937; //only init mt19937 once because it takes time
	Block* newBlock = new Block(seed,randGenerator);
	newBlock->generateBlock(); //the program actually stops here
	delete newBlock;
	delete randGenerator;

	return 1;

}
