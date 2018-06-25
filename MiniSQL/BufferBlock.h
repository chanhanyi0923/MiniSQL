#pragma once
#include <string>
class BufferBlock
{
private:
	static const int MAX_BLOCKS = 1000;
	static const int BLOCK_SIZE = 4096;
	static const int INDEX = 2;
	static const int TABLE = 1;

public:

	static BufferBlock* m_blocks;

	char* address;		//块的首地址
	std::string name;	//index或table的名字
	unsigned int offset;		//文件中第几个块
	short int table_index;	//该块是存的是index还是table。是index值为index，是table值为table，空块值为0
	bool dirty_bit;		//被修改过为1，未修改过为0
	bool being_used;	//正在使用该块则为1
	unsigned int count;	//用于实现LRU

	BufferBlock();
	~BufferBlock();





	static void initiate_blocks();
	static void flush_all();

	unsigned int write_block(std::string filename, short table_index);

	//如果没有该文件 返回-1
	unsigned int read_block(std::string filename, unsigned int offset, short table_index);
	

	void not_being_used();
	void written();

	//private:

	static void using_block(unsigned int number);
	unsigned int get_blank(std::string filename);
	unsigned int get_blank(unsigned int offset);
	void flush_block();
	static unsigned int max_count_block();
	std::string getvalues(int startpos, int endpos);


};
