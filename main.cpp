/*************************************************
 * Space Vector
 * Started Dec. 11th 2018
 * Andrew Peterkort
 *************************************************/
#include <cstdint>
#include <cstdio>
#include <unistd.h>//This is a unix/linux library, need replacement
#include <random>
#include <ctime>//needs replacement

//LONG TERM PROBLEMS
//Obviouse limit flaws to finding seed differences and get out of range
//of any affect for our limited 32 bit seed consider some kind of mask
//to make generation infinite..... testing it would be hard

/*************************************************
 * OBJECT: BLOCK 
 * uses a 64 bit seed number to create a 32x32 bit array of ones and zeros
 * each block represents a grid of astroids in space and is generated with * Conway's Game of Life like algorithims
 *************************************************/
class Block {

	public:
		uint32_t seed; //stores the initial seed for random generation
		void printBlock();
		void generateBlock();//prints out the progression of generations
		int64_t xBlock;
		int64_t yBlock;
		Block(uint32_t,std::mt19937*,int64_t,int64_t);
		~Block();

	private:
		uint8_t** block; //stores the current block of 96x96 bit astroid layout
		uint8_t** drawBlock; //this is the buffer block that has the new frame written to it.
		bool readCell(int, int, uint8_t**); //reads if a cell exits at x,y
		void writeCell(int, int, uint8_t**); //writes a cell to x,y
		void deleteCell(int, int, uint8_t**); //deletes cell at x,y
		int countNeighbors(int, int, uint8_t**); //countes the total neghboring cells to x,y
		bool cycleConway(); //plays one frame of the game
		void copyArray(uint8_t**, uint8_t**); //copies array two too array one
		void clearScreen();
		uint64_t getNaturalCordinate(int64_t);
		uint32_t getSeedDiff(int64_t,int64_t);
		std::mt19937* randGen;
};

bool Block::readCell(int x, int y, uint8_t** outBlock){ 

	return (outBlock[y][x] == 1);
}

void Block::writeCell(int x, int y, uint8_t** inBlock){

	inBlock[y][x] = 1;
}

void Block::deleteCell(int x, int y, uint8_t** inBlock){

	inBlock[y][x] = 0;
}

void Block::copyArray(uint8_t** dest, uint8_t** input){

	for( int i = 0; i < 96; i++ ){

		for( int j = 0; j < 96; j++ ){

			dest[i][j] = input[i][j];
		}
	}
}

uint64_t Block::getNaturalCordinate(int64_t cord){

	uint64_t natCord = 0;

	if( cord > 0 )
		natCord = cord * 2;

	if( cord < 0 )
		natCord = (cord * -2) - 1;

	return natCord;
}

uint32_t Block::getSeedDiff(int64_t xBlock, int64_t yBlock){

	uint64_t natVectX = getNaturalCordinate(xBlock);
	uint64_t natVectY = getNaturalCordinate(yBlock);

	return (uint32_t)(((natVectX + natVectY)*(natVectX + natVectY + 1)/2) + natVectX);
	//this is the Cantor pairing algo. Inputs need to be put onto the natural number line
	//because the cantor pairing formula changes (N,N) -> N 
}

int Block::countNeighbors(int x, int y, uint8_t** inBlock){

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
	if(x == 95)
		xHighLimit = 0;
	if(y == 0)
		yLowLimit = 0;
	if(y == 95)
		yHighLimit = 0;

	//the limits are then applied to counting around the cell
	for( int xOffset = xLowLimit; xOffset <= xHighLimit; xOffset++ ){

		for( int yOffset = yLowLimit; yOffset <= yHighLimit; yOffset++ ){

			if((xOffset == 0) && (yOffset == 0)) //we don't want to count ourselves
				continue;

			if(readCell(x + xOffset, y + yOffset, inBlock) == true)
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

	uint8_t blockSeed = 0;
	uint32_t maskMod32 = 31;
	int numNeighbors;
	int runRand;
	int totalNeighbors = 0;

	for( int y = 0; y < 96; y++ ){

		for( int x = 0; x < 96; x++ ){

			blockSeed = (x / 32) + (3 * (y / 32)); //this is the algo that splits between the different seeds **don't think this is bugged 
			numNeighbors = countNeighbors(x,y,block);
			totalNeighbors += numNeighbors; //add the total neighbors to see if there are any cells in the block

			//more entropy is added through randGen to make sure the algo does not make common identifiable
			//shapes and instead the astroid groups look more random. Uses maskMod64 for more effecient bit masking vs moding
			runRand = (randGen[blockSeed])() & maskMod32;

			if( (numNeighbors == 3) || ((numNeighbors == 2) && (runRand == 1) )){

				writeCell(x,y,drawBlock);

			}else if( numNeighbors < 2 ){

				deleteCell(x,y,drawBlock);
			}
		}
	}

	copyArray(block,drawBlock);

	//might want to cut this if check for a little bit more gas if we know probabilisticly, there will
	//always be some astroids in a 9x9
	if( totalNeighbors == 0 ) //if there are no cells in the block, return false to tell the lower function
		return false;

	return true;

}

void Block::clearScreen(){

	for( int i = 0; i < 100; i++ ){

		printf("\n");

	}

}

//it should be noted that because the astroid feilds print out in this way
//the astroids themselves are navigated in standard computer graphics format
//with (0,0) being the top left hand corner and both axes going positive
//like this x+ <right>  y+ <down>
void Block::printBlock(){

	for( int y = 32; y < 64; y++ ){

		for( int x = 32; x < 64; x++ ){

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

	for( int i = 0; i < 20; i++ ){//notice only 20 cycles of Conway

		clearScreen();
		printBlock();

		if( cycleConway() == false ) //if cycleConway detects no cells in the block
			break;

		usleep(100000); //this call needs to be replaced in the future for cross platform ability

	}

}

Block::Block(uint32_t seed, std::mt19937* randGenerators, int64_t xBlock, int64_t yBlock){

	/*
		 basiclly this is the 96x96 uint8_t memory block:

	 *------*------*------*
	 |      |      |      |
	 |  0   |  1   |  2   |
	 *------*------*------*
	 |      |      |      |
	 |  3   |  4   |  5   |
	 *------*------*------*
	 |      |      |      |
	 |  6   |  7   |  8   |
	 *------*------*------*

	 The numbers inside are their seed number in the seed array

	 the 32x32 block in the middle is the block we are actually generating
	 but we run our simulation with other 32x32 blocks on the outside
	 edges with the correct seeds to smooth edges and avoid potential non cordination
	 between adjecent blocks
	 */

	uint8_t seedCount = 0;
	uint32_t maskMod32 = 31;

	this->seed = seed;
	this->xBlock = xBlock;
	this->yBlock = yBlock;
	this->randGen = randGenerators;

	//these loops are in this specific order to give the serounding blocks
	//the right seed when they are looped through in this exact order latter
	for( int y = 1; y >= -1; y-- ){

		for( int x = -1; x <= 1; x++ ){

			randGen[seedCount].seed(seed + getSeedDiff(xBlock + x, yBlock + y));
			seedCount++;

			//the (0,0) block should give 0 from the getSeedDiff function. making the 0,0 block
			//the base block generated from the base seed

		}
	}

	block = new uint8_t* [96];
	drawBlock = new uint8_t* [96];

	for( int i = 0; i < 96; i++ ){

		block[i] = new uint8_t [96];
		drawBlock[i] = new uint8_t [96];
	}

	seedCount = 0;

	for( int y = 0; y <= 64; y += 32 ) {

		for( int x = 0; x <= 64; x += 32 ) { //loop through the blocks

			for( int j = 0; j < 50; j++ ){ //how many cordinates to generate per block

				//in the block like scetch shown above
				writeCell(((randGen[seedCount])() & maskMod32) + x, ((randGen[seedCount])() & maskMod32) + y, block); 
			}

			seedCount++;
		}
	}

	copyArray(drawBlock,block);
	//masking to make clear that we are not creating
	//statistical non-uniformaties with our random numbers
	//populating all nine blocks to allow for clean block boarders
	//around the middle block which will be cut out to make the finale block
	//population proceeds across borders during generation
	//we make a point not to use some equal prob. distribution function because
	//those function change depending on which machine we compile for so we have
	//to keep our grids base two numbers so we can mask and have even distribution
}

Block::~Block(){

	for( int i = 0; i < 96; i++ ){

		delete[] block [i];
		delete[] drawBlock [i];
	}

	delete[] block;
	delete[] drawBlock;
}

int main() {

	uint32_t seed = 0; //this will be replaced eventually ... hopefully

	//we want to keep all 9 in memory to speed up generating new blocks
	std::mt19937* randNumGenerators = new std::mt19937 [9];

	Block* newBlock = new Block(seed,randNumGenerators,0,0);
	newBlock->generateBlock();

	delete newBlock;
	delete[] randNumGenerators;	

	return 1;
}
