#include "BufferBlock.h"

#include <fstream>

BufferBlock::BufferBlock()
{
}


BufferBlock::~BufferBlock()
{
}


void BufferBlock::initiate_blocks()
{
	m_blocks = new BufferBlock[MAX_BLOCKS];
	unsigned int i;
	for (i = 0; i < MAX_BLOCKS; i++) {
		m_blocks[i].address = NULL;
		m_blocks[i].being_used = 0;
		m_blocks[i].dirty_bit = 0;
		m_blocks[i].table_index = 0;
		m_blocks[i].count = 0;
	}
}

void BufferBlock::flush_all()		//写回所有块，并释放空间
{
	unsigned int i;
	for (i = 0; i < MAX_BLOCKS; i++) {
		m_blocks[i].flush_block();
		if (m_blocks[i].address != NULL)
			delete[] m_blocks[i].address;
	}
}

unsigned int BufferBlock::write_block(std::string filename, short table_index)
{
	unsigned int number = get_blank(filename);
	m_blocks[number].written();
	m_blocks[number].table_index = table_index;
	return number;
}

unsigned int BufferBlock::read_block(std::string filename, unsigned int offset, short table_index)
{
	for (int i = 0; i < MAX_BLOCKS; i++) {
		if (m_blocks[i].name == filename && m_blocks[i].offset == offset && m_blocks[i].table_index == table_index) { //找到了所请求的块
			int number = i;
			using_block(number);
			return number;
		}
	}
	//未找到块，需要从文件中读取
	unsigned int number = get_blank(offset);
	m_blocks->table_index = table_index;
	m_blocks[number].name = filename;
	m_blocks[number].table_index = table_index;
	std::ifstream f;
	f.open(filename, std::ifstream::binary | std::ifstream::in);
	if (!f.is_open()) return -1;
	f.seekg(offset*BLOCK_SIZE);
	f.read(m_blocks[number].address, BLOCK_SIZE);
	f.close();

	using_block(number);
	return number;

}

unsigned int BufferBlock::get_blank(std::string filename)
{
	unsigned int number, i;
	for (i = 0; i < MAX_BLOCKS; i++) {
		if (m_blocks[i].table_index == 0 && !m_blocks[i].being_used) break;;
	}
	if (i < MAX_BLOCKS) {		//有未使用的空块
		number = i;
		m_blocks[i].address = new char[BLOCK_SIZE];
	}
	else {				//没有空块，需要将最少使用的写回
		number = max_count_block();
		m_blocks[number].flush_block();
	}
	unsigned int ofst = 0;
	std::fstream f;
	f.open(filename, std::ios::in | std::ios::out | std::ios::binary);
	if (!f.is_open()) {
		f.open(filename, std::ios::out);
		f.close();
		f.open(filename, std::ios::in | std::ios::out | std::ios::binary);
	}
	f.seekp(0, std::ios::end);
	unsigned int end = f.tellg();
	char c = '$';
	while (1) {
		if (ofst >= end) break;

		f.seekg(ofst);
		f.read(&c, 1);
		if (c == '!') break;
		ofst += BLOCK_SIZE;
	}
	char*p;
	for (p = m_blocks[i].address; p < m_blocks[i].address + BLOCK_SIZE; p++) {
		*p = '$';
	}
	using_block(number);
	m_blocks[number].offset = ofst / BLOCK_SIZE;
	f.seekp(ofst);
	f.write("$", 1);
	f.close();
	m_blocks[number].name = filename;
	return number;
}

unsigned int BufferBlock::get_blank(unsigned int offset)
{
	unsigned int number, i;
	for (i = 0; i < MAX_BLOCKS; i++) {
		if (m_blocks[i].table_index == 0 && !m_blocks[i].being_used) break;;
	}
	if (i < MAX_BLOCKS) {		//有未使用的空块
		number = i;
		m_blocks[i].address = new char[BLOCK_SIZE];
	}
	else {				//没有空块，需要将最少使用的写回
		number = max_count_block();
		m_blocks[number].flush_block();
	}
	m_blocks[number].offset = offset;
	char*p;
	for (p = m_blocks[i].address; p < m_blocks[i].address + BLOCK_SIZE; p++) {
		*p = '$';
	}
	using_block(number);
	return number;
}

void BufferBlock::using_block(unsigned int number)
{
	unsigned int i;
	for (i = 0; i < MAX_BLOCKS; i++) {
		if (i != number && !m_blocks[i].being_used)		//所有其他未使用的块count+1
			m_blocks[i].count++;
		else if (i == number) m_blocks[i].being_used = 1;
	}
}

void BufferBlock::flush_block()
{
	std::ofstream f;
	if (dirty_bit && table_index) {
		f.open(name, std::ios::binary | std::ios::in | std::ios::out);
		if (!f.is_open()) throw("open file" + name + "failed");
		f.seekp(offset*BLOCK_SIZE);
		f.write(address, BLOCK_SIZE);
		f.close();
	}
	offset = 0;
	being_used = 0;
	dirty_bit = 0;
	table_index = 0;
	count = 0;
	name = "";

}

unsigned int BufferBlock::max_count_block()		//返回buffer中count最大的一个块
{
	unsigned int i;
	unsigned int max_count = m_blocks[0].count, max_index = 0;
	for (i = 0; i < MAX_BLOCKS; i++) {
		if (m_blocks[i].count > max_count) {
			max_count = m_blocks[i].count;
			max_index = i;
		}
	}
	return max_index;
}


void BufferBlock::not_being_used()
{
	being_used = 0;
}


void BufferBlock::written()
{
	dirty_bit = 1;
}



std::string BufferBlock::getvalues(int startpos, int endpos)
{
	using std::string;
	string tmpt = "";
	if (startpos >= 0 && startpos <= endpos && endpos <= BLOCK_SIZE)
		for (int i = startpos; i < endpos; i++)
			tmpt += address[i];
	return tmpt;
}



