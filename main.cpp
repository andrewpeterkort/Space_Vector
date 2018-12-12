/*************************************************
 * Space Vector
 * Started Dec. 11th 2018
 * Andrew Peterkort
 *************************************************/
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <unistd.h>//This is a unix/linux library, need replacement

/*************************************************
 * OBJECT: BLOCK 
 * uses a 64 bit seed number to create a 32x32 bit array of ones and zeros
 * each block represents a grid of astroids in space and is generated with
 * Conway's Game of Life like algorithims *************************************************/ class Block {

	public:
		uint64_t seed; //stores the initial seed for random generation
		uint32_t* block; //stores the current block of 32x32 bit astroid layout
		void printBlock();
		void playConway();//prints out the progression of generations
		Block(uint64_t);
		~Block();

	private:
		uint32_t* drawBlock; //this is the buffer block that has the new fram written to it
		bool readCell(int, int, uint32_t*); //reads if a cell exits at x,y
		void writeCell(int, int, uint32_t*); //writes a cell to x,y
		void deleteCell(int, int, uint32_t*); //deletes cell at x,y
		int countNeighbors(int, int, uint32_t*); //countes the total neghboring cells to x,y
		void cycleConway(); //plays one frame of the game
		void clearScreen();

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
void Block::cycleConway(){

	int numNeighbors;

	for( int y = 0; y < 32; y++ ){

		for( int x = 0; x < 32; x++ ){

			numNeighbors = countNeighbors(x,y,block);

			if( numNeighbors == 3 ){

				writeCell(x,y,drawBlock);

			}else if( (numNeighbors < 2) || (numNeighbors > 3) ){

				deleteCell(x,y,drawBlock);

			}

		}

	}

	memcpy(block,drawBlock,128);

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

void Block::playConway(){

	for(;;){ //forever

		clearScreen();
		printBlock();
		cycleConway();
		usleep(100000); //this call needs to be replaced in the future for cross platform ability

	}

}

Block::Block(uint64_t seed){

	this->seed = seed;
	srand(seed); //a different rand algo needs to be considered too
	block = new uint32_t [32];
	drawBlock = new uint32_t [32];

	for( int i = 0; i < 100; i++ )
		writeCell(rand()%32, rand()%32, block);

	memcpy(drawBlock,block,128);

}

Block::~Block(){

	delete[] block;
	delete[] drawBlock;

}

int main() {

	uint32_t seed = time(NULL);
	Block* newBlock = new Block(seed);
	newBlock->playConway(); //the program actually stops here
	delete newBlock;

	return 1;

}
