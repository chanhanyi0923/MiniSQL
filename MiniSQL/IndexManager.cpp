#include "IndexManager.h"

#include <fstream>
#include "DataC.h"
#include "DataF.h"
#include "DataI.h"

#define INDEX 1

extern BufferBlock buffer;

using namespace std;

IndexManager::IndexManager()
{
}


IndexManager::~IndexManager()
{
}

void IndexManager::Init(string filename, int keyType)
{
	fstream f;
	fname = filename;
	f.open(filename, ios::in | ios::binary);
	//尚未创建该index，需要初始化filehead
	if (!f.is_open()) {
		f.open(filename, ios::out | ios::binary);
		f.write("$", 1);
		size = 0;
		///////////////////////////////////
		//do some init for a new root
		rootOffset = ROOTFATHER;
		Ktype = keyType;
		fname = filename;
		if (Ktype == 0) Klen = sizeof(int);
		else if (Ktype == 1) Klen = sizeof(float);
		else if (Ktype == 2) Klen = MAX_STRING_LENGTH;
		else throw("IndexManager::Init(): wrong Ktype");
		//////////////////////////////////
		f.close();
		size++;
		return;
	}
	else {
		f.seekp(0, ios::end);
		size = f.tellg() / BLOCK_SIZE;
		f.close();
		if (size == 0)
			throw("IndexManager::Init():already exit an empty file name as " + filename);
	}

	int headBlockNo = buffer.read_block(fname, 0, INDEX);
	char* p = BufferBlock::m_blocks[headBlockNo].address;
	rootOffset = *(int*)p;
	Ktype = keyType;
//	Ktype = *(int*)(p + 4);
	if (Ktype == 0) Klen = sizeof(int);
	else if (Ktype == 1) Klen = sizeof(float);
	else if (Ktype == 2) Klen = MAX_STRING_LENGTH;
	else throw("IndexManager::Init(): wrong Ktype");
	BufferBlock::m_blocks[headBlockNo].not_being_used();
}

void IndexManager::WriteBack()
{
	unsigned int fileHead = buffer.read_block(fname, 0, INDEX);
	char* p = BufferBlock::m_blocks[fileHead].address;
	*(int*)p = rootOffset;
	*(int*)(p + 4) = Ktype;
	BufferBlock::m_blocks[fileHead].written();
	BufferBlock::m_blocks[fileHead].not_being_used();
}

unsigned int IndexManager::findLeaf(Data* key)			//根据K值，返回K值所在的叶子节点offset
{
	unsigned int current_block = buffer.read_block(fname, rootOffset, INDEX);	//从根节点开始
	if (size == 2) return BufferBlock::m_blocks[current_block].offset;			//size==2说明只有一个文件头和一个根节点，根节点就是叶节点
	char* p = BufferBlock::m_blocks[current_block].address;
	if (*p == 'l') return current_block;
	int gap = Klen + 4;
	int Knumbers = *(int*)(p + 5);
	int count = 0;
	while (1) {
		bool compres;
		if (*p == INTERNAL) {				//是内部节点
			p += blockHead;
			if (Ktype == 0)
				compres = *((int*)(p + 4)) >= ((DataI*)key)->x;
			else if (Ktype == 1)
				compres = *((float*)(p + 4)) >= ((DataF*)key)->x;
			else if (Ktype == 2)
				compres = (strcmp(((DataC*)key)->x.c_str(), (char*)(p + 4)) >= 0);
			if (compres == false) {
				current_block = buffer.read_block(fname, *((int*)p), INDEX);
				p = BufferBlock::m_blocks[current_block].address;
				if (*p == 'l') {
					BufferBlock::m_blocks[current_block].not_being_used();
					return BufferBlock::m_blocks[current_block].offset;
				}
				Knumbers = *(int*)(p + 5);
				count = 0;
			}
			else {
				if (count == Knumbers) {
					current_block = buffer.read_block(fname, *((int*)(p + gap)), INDEX);
				}
				p += gap;
				count++;
				continue;
			}
		}
	}
}

int IndexManager::find_pos_in_leafBlock(Data* key, unsigned int leafBlockOffset)
{
	unsigned int bufferBlockNo = buffer.read_block(fname, leafBlockOffset, INDEX);
	char* p = BufferBlock::m_blocks[bufferBlockNo].address;
	int Knumber = *(int*)(p + 5);
	int count = Knumber;
	int current_cell_offset = blockHead;
	int gap = Klen + 4;				//gap:一个key加一个地址的大小
	bool full = false;
	if (Knumber*gap + blockHead > BLOCK_SIZE - gap)	//插不下下一个元素
		full = true;
	//判断是否比第一个元素还小
	bool compres = false;
	char* cp = p + blockHead;
	if (Ktype == 0)
		compres = *((int*)(cp)) > ((DataI*)key)->x;
	else if (Ktype == 1)
		compres = *((float*)(cp)) > ((DataF*)key)->x;
	else if (Ktype == 2)
		compres = (strcmp((char*)(cp), ((DataC*)key)->x.c_str()) == 1);
	if (compres) {
		return 0;
	}

	while (1)
	{
		if (current_cell_offset > BLOCK_SIZE || count <= 0) {
			current_cell_offset -= gap;
			break;
		}
		bool compres = false;
		char* cp = p + current_cell_offset;
		if (Ktype == 0)
			compres = *((int*)(cp)) < ((DataI*)key)->x;
		else if (Ktype == 1)
			compres = *((float*)(cp)) < ((DataF*)key)->x;
		else if (Ktype == 2)
			compres = (strcmp(((DataC*)key)->x.c_str(), (char*)(cp)) == 1);
		if (compres == true) {				//继续向后查找
			current_cell_offset += gap;
			count--;
			continue;
		}
		else  break;
	}
	BufferBlock::m_blocks[bufferBlockNo].not_being_used();
	if (full) return -current_cell_offset;
	else return current_cell_offset;
}

int IndexManager::Find(string file, Data* key)		//未找到返回-1，空index返回-2
{
	if (size == 0) return -2;
	unsigned int bufferBlockOffset = findLeaf(key);
	unsigned int bufferBlockNo = buffer.read_block(fname, bufferBlockOffset, INDEX);
	int offset_in_block = abs(find_pos_in_leafBlock(key, bufferBlockOffset));
	char* p = BufferBlock::m_blocks[bufferBlockNo].address;
	char* Kp = p + offset_in_block;
	bool is_equal = false;
	if (Ktype == 0)
		is_equal = *((int*)(Kp)) == ((DataI*)key)->x;
	else if (Ktype == 1)
		is_equal = *((float*)(Kp)) == ((DataF*)key)->x;
	else if (Ktype == 2)
		is_equal = (strcmp(((DataC*)key)->x.c_str(), (char*)(Kp + 4)) == 0);
	BufferBlock::m_blocks[bufferBlockNo].not_being_used();
	if (is_equal) return *(int*)(Kp + Klen);
	else return -1;
}

void IndexManager::Insert(string filename, Data* key, int addr)
{
	if (size == 1 && rootOffset == ROOTFATHER)		//文件只有文件头，还未插入数据
	{
		unsigned int rootBufferNo = buffer.write_block(fname, INDEX);
		rootOffset = BufferBlock::m_blocks[rootBufferNo].offset;
		BufferBlock::m_blocks[rootBufferNo].name = filename;
		char *p = BufferBlock::m_blocks[rootBufferNo].address;
		*p = LEAF;
		*(int*)(p + 1) = Ktype;
		*(int*)(p + 5) = 1;
		*(int*)(p + 9) = ROOTFATHER;
		*(int*)(p + 13) = ROOTFATHER;
		if (Ktype == 0) *(int*)(p + blockHead) = ((DataI*)key)->x;
		else if (Ktype == 1) *(float*)(p + blockHead) = ((DataF*)key)->x;
		else if (Ktype == 2) memcpy(p + blockHead, ((DataC*)key)->x.c_str(), Klen);
		*(int*)(p + blockHead + Klen) = addr;
		size++;
		BufferBlock::m_blocks[rootBufferNo].not_being_used();
	}
	else {
		insertRecursively(rootOffset, key, addr);
	}
}

void IndexManager::insertRecursively(unsigned int offsetOfRootBlock, Data* key, int addr)
{
	unsigned int rootBufferNo = buffer.read_block(fname, offsetOfRootBlock, INDEX);
	BufferBlock::m_blocks[rootBufferNo].written();
	char* p = BufferBlock::m_blocks[rootBufferNo].address;
	char blockType = *p;
	int rootKnumber = *(int*)(p + 5);
	int gap = Klen + 4;
	unsigned int father;
	if (blockType == INTERNAL&&rootKnumber * gap + blockHead + 4 > BLOCK_SIZE - gap) {		//遇到满内节点
		father = blockSplit(rootBufferNo);
		insertRecursively(father, key, addr);
	}
	else if (blockType == LEAF && rootKnumber*gap + blockHead > BLOCK_SIZE - gap) {		//遇到满叶子节点
		father = blockSplit(rootBufferNo);
		insertRecursively(father, key, addr);
	}
	else {					//未满
		if (blockType == INTERNAL) {
			//找到插入路径的下一层
			int count = 0;
			unsigned descentBlock;
			char* currentp = p;
			while (1) {
				bool compres;
				currentp += blockHead;
				if (Ktype == 0)
					compres = *((int*)(currentp + 4)) >= ((DataI*)key)->x;
				else if (Ktype == 1)
					compres = *((float*)(currentp + 4)) >= ((DataF*)key)->x;
				else if (Ktype == 2)
					compres = (strcmp(((DataC*)key)->x.c_str(), (char*)(currentp + 4)) >= 0);
				if (compres == false) {
					descentBlock = buffer.read_block(fname, *((int*)currentp), INDEX);
					break;
				}
				else {
					if (count == rootKnumber) {
						descentBlock = buffer.read_block(fname, *((int*)(currentp + gap)), INDEX);
					}
				}
			}
			insertRecursively(descentBlock, key, addr);
		}
		else if (blockType == 'l') {
			int offsetInLeaf = find_pos_in_leafBlock(key, offsetOfRootBlock) + Klen + 4;
			//判断是否重复插入
			bool flag = false;
			if (Ktype == 0) flag = *(int*)(p + offsetInLeaf - gap) == ((DataI*)key)->x;
			else if (Ktype == 1) flag = *(float*)(p + offsetInLeaf - gap) == ((DataF*)key)->x;
			else if (Ktype == 2) flag = (strcmp(p + offsetInLeaf - gap, ((DataC*)key)->x.c_str()) == 0);
			if (flag == 1) throw("IndexManager::Insert:insert an already exit element");
			char* temp = new char[BLOCK_SIZE];
			memcpy(temp, p, BLOCK_SIZE);
			memcpy(p + offsetInLeaf + gap, temp + offsetInLeaf, BLOCK_SIZE - offsetInLeaf - gap);
			delete[] temp;

			if (Ktype == 0) *(int*)(p + offsetInLeaf) = ((DataI*)key)->x;
			else if (Ktype == 1) *(float*)(p + offsetInLeaf) = ((DataF*)key)->x;
			else if (Ktype == 2) memcpy(p + offsetInLeaf, ((DataC*)key)->x.c_str(), MAX_STRING_LENGTH);
			*(int*)(p + offsetInLeaf + Klen) = addr;
			*(int*)(p + 5) = *(int*)(p + 5) + 1;

		}
		else throw("IndexManager::insertRecursively:undefined blocktype value" + blockType);
	}
	BufferBlock::m_blocks[rootBufferNo].not_being_used();
}

unsigned int IndexManager::blockSplit(unsigned int splitedBlockOffset)
{
	unsigned int bufferBlockNo = buffer.read_block(fname, splitedBlockOffset, INDEX);
	char* p = BufferBlock::m_blocks[bufferBlockNo].address;
	int gap = Klen + 4;
	int Knumber = *(int*)(p + 5);
	char blockType = *p;
	unsigned int fatherOffset;			//返回值
	if (blockHead + Knumber*gap < BLOCK_SIZE - gap - 4) return -1;
	unsigned int father_offset = *(int*)(p + 9);	//原始的fatheroffset
	unsigned int newBlockNo = buffer.write_block(fname, INDEX);
	int left_number = ceil(Knumber / 2.0), right_number = floor(Knumber / 2.0);
	//key：父亲节点插入的数据

	void* key = p + blockHead + left_number*gap;

	char* newp = BufferBlock::m_blocks[newBlockNo].address;
	*newp = *p;				//节点类型应当与分列前一样
	*(int*)(newp + 1) = *(int*)(p + 1);	//key类型应当一样
	*(int*)(p + 5) = left_number; *(int*)(newp + 1) = right_number;
	*(int*)(newp + 9) = *(int*)(p + 9);	//father应该一样
	*(int*)(newp + 13) = *(int*)(p + 13);
	*(int*)(p + 13) = BufferBlock::m_blocks[newBlockNo].offset;
	memcpy(newp + blockHead, p + blockHead + left_number*gap, right_number*gap);		//将data转移一半到新block
	memset(p + blockHead + left_number*gap, '$', right_number*gap);					//旧block原先的位置设为默认值'$'																		

																					//顶上没有节点，b+树长高一层
	if (father_offset == ROOTFATHER) {
		unsigned int newRootNo = buffer.write_block(fname, INDEX);
		if (blockType == INTERNAL) {			//分裂的是中间节点
			char* rp = BufferBlock::m_blocks[newRootNo].address;
			*rp = INTERNAL;
			*(int*)(p + 1) = Ktype;
			*(int*)(p + 5) = 1;
			*(int*)(p + 9) = ROOTFATHER;
			*(int*)(p + 13) = 0;
			memcpy(rp + blockHead + 4, newp + blockHead + 4, Klen);
			*(int*)(rp + blockHead) = BufferBlock::m_blocks[bufferBlockNo].offset;
			*(int*)(rp + blockHead + gap) = BufferBlock::m_blocks[newBlockNo].offset;
		}
		else {		//分裂的是叶子节点
			char* rp = BufferBlock::m_blocks[newRootNo].address;
			*rp = INTERNAL;
			*(int*)(p + 1) = Ktype;
			*(int*)(p + 5) = 1;
			*(int*)(p + 9) = ROOTFATHER;
			*(int*)(p + 13) = 0;
			memcpy(rp + blockHead + 4, newp + blockHead, Klen);
			*(int*)(rp + blockHead) = BufferBlock::m_blocks[bufferBlockNo].offset;
			*(int*)(rp + blockHead + gap) = BufferBlock::m_blocks[newBlockNo].offset;
		}
		fatherOffset = BufferBlock::m_blocks[newRootNo].offset;
		rootOffset = BufferBlock::m_blocks[newRootNo].offset;			//更改结构中的根节点指针
		BufferBlock::m_blocks[newRootNo].not_being_used();
	}

	//上面还有节点，不用长高
	else {
		unsigned int fatherBlockNo = buffer.read_block(fname, father_offset, INDEX);		//从buffer中获取fatherblock
		char* fp = BufferBlock::m_blocks[fatherBlockNo].address;
		int fatherKnumber = *(int*)(fp + 5);
		if (blockHead + fatherKnumber*gap > BLOCK_SIZE - gap - 4) throw("IndexManager::blockSplit: father is full, this is invalid");

		//找插入位置
		int count = 0;
		unsigned int current_offset = blockHead;
		while (1) {
			bool compres;
			if (Ktype == 0)
				compres = *((int*)(fp + current_offset + 4)) >= *(int*)key;
			else if (Ktype == 1)
				compres = *((float*)(fp + current_offset + 4)) >= *(float*)key;
			else if (Ktype == 2)
				compres = (strcmp((char*)key, (char*)(fp + current_offset + 4)) >= 0);
			if (compres == true) break;
			else {
				current_offset += gap;
				count++;
				if (count == fatherKnumber) break;			//已经找完所有father中所有key，且还有空位置，则在最后插入。
			}
		}
		//插入过程
		BufferBlock::m_blocks[fatherBlockNo].written();
		if (blockType == INTERNAL) {
			memcpy(fp + blockHead + 4 + gap*left_number, newp + blockHead + 4, Klen);
			*(int*)(fp + blockHead * gap*left_number) = BufferBlock::m_blocks[bufferBlockNo].offset;
			*(int*)(fp + blockHead + gap + gap*left_number) = BufferBlock::m_blocks[newBlockNo].offset;
		}
		else {
			memcpy(fp + blockHead + 4 + gap*left_number, newp + blockHead, Klen);
			*(int*)(fp + blockHead * gap*left_number) = BufferBlock::m_blocks[bufferBlockNo].offset;
			*(int*)(fp + blockHead + gap + gap*left_number) = BufferBlock::m_blocks[newBlockNo].offset;
		}
		fatherOffset = BufferBlock::m_blocks[fatherBlockNo].offset;
		BufferBlock::m_blocks[fatherBlockNo].not_being_used();
	}
	BufferBlock::m_blocks[bufferBlockNo].not_being_used();
	BufferBlock::m_blocks[newBlockNo].not_being_used();
	size++;
	return fatherOffset;
}

int* IndexManager::Range(string file, Data*key1, Data*key2)
{
	int* re = new int[MAX_RANGE];
	int sp = 0;
	int gap = Klen + 4;
	//找到比key1大的最近的元素
	unsigned int startBlockOffset, endBlockOffset;
	int offsetInStartBlock, offsetInEndBlock;
	//找开始位置
	unsigned int bufferBlockOffset = findLeaf(key1);
	unsigned int bufferBlockNo = buffer.read_block(fname, bufferBlockOffset, INDEX);
	int offset_in_block = find_pos_in_leafBlock(key1, bufferBlockNo);
	if (offset_in_block<0) {				//从下个块的第一个元素开始
		startBlockOffset = *(int*)(BufferBlock::m_blocks[bufferBlockNo].address + 13);
		if (startBlockOffset == ROOTFATHER) { *re = -1; return re; }		//没有下一个块，说明开始位置已经大于最大
		offsetInStartBlock = blockHead;
	}
	else if (offset_in_block == 0) {			//比最小块最小元素还小
		startBlockOffset = bufferBlockOffset;
		offsetInStartBlock = blockHead;
	}
	else {
		startBlockOffset = bufferBlockOffset;
		offsetInStartBlock = offset_in_block;
	}
	BufferBlock::m_blocks[bufferBlockNo].not_being_used();
	unsigned int currentOffset, nextOffset, bufferNo;
	int Knumbers;
	currentOffset = startBlockOffset;
	unsigned int currentNo = buffer.read_block(fname, currentOffset, INDEX);
	nextOffset = *(int*)(BufferBlock::m_blocks[currentNo].address + 13);
	Knumbers = *(int*)(BufferBlock::m_blocks[currentNo].address + 5);
	for (int i = offsetInStartBlock; i <= blockHead + Knumbers*gap; i += gap) {
		//已经大于截止元素
		bool compres = false;
		if (Ktype == 0)
			compres = *((int*)(BufferBlock::m_blocks[currentNo].address + i)) > ((DataI*)key2)->x;
		else if (Ktype == 1)
			compres = *((float*)(BufferBlock::m_blocks[currentNo].address + i)) > ((DataF*)key2)->x;
		else if (Ktype == 2)
			compres = (strcmp((char*)(BufferBlock::m_blocks[currentNo].address + i), ((DataC*)key2)->x.c_str()) == 1);
		if (compres) {
			BufferBlock::m_blocks[currentNo].not_being_used();
			re[sp++] = -1;
			return re;
		}
		re[sp++] = *(int*)(BufferBlock::m_blocks[currentNo].address + i + Klen);
		if (sp >= MAX_RANGE) throw("IndexManager::Range: numbers of elements is greater than MAX_RANGE");
	}
	BufferBlock::m_blocks[currentNo].not_being_used();
	while (1) {
		if (nextOffset == ROOTFATHER) {
			re[sp++] = -1;
			return re;
		}
		currentOffset = nextOffset;
		currentNo = buffer.read_block(fname, currentOffset, INDEX);
		nextOffset = *(int*)(BufferBlock::m_blocks[currentNo].address + 13);
		Knumbers = *(int*)(BufferBlock::m_blocks[currentNo].address + 5);
		for (int i = offsetInStartBlock; i < blockHead + Knumbers*gap; i += gap) {
			//已经大于截止元素
			bool compres = false;
			if (Ktype == 0)
				compres = *((int*)(BufferBlock::m_blocks[currentNo].address + i)) >((DataI*)key2)->x;
			else if (Ktype == 1)
				compres = *((float*)(BufferBlock::m_blocks[currentNo].address + i)) > ((DataF*)key2)->x;
			else if (Ktype == 2)
				compres = (strcmp((char*)(BufferBlock::m_blocks[currentNo].address + i), ((DataC*)key2)->x.c_str()) == 1);
			if (compres) {
				BufferBlock::m_blocks[currentNo].not_being_used();
				re[sp++] = -1;
				return re;
			}
			re[sp++] = *(int*)(BufferBlock::m_blocks[currentNo].address + i + Klen);
			if (sp + 1 >= MAX_RANGE) throw("IndexManager::Range: numbers of elements is greater than MAX_RANGE");
		}
		BufferBlock::m_blocks[currentNo].not_being_used();
	}
}

void IndexManager::Drop(string file)
{
	remove(file.c_str());
}

int IndexManager::Delete(string file, Data* key)
{
	unsigned int leafOffset = findLeaf(key);
	unsigned int leafNo = buffer.read_block(file, leafOffset, INDEX);
	int pos = find_pos_in_leafBlock(key, leafOffset);
	if (pos <= 0) return -1;			//叶子节点中不存在该元素
	int gap = Klen + 4;
	char* p = BufferBlock::m_blocks[leafNo].address;
	bool flag = false;
	if (Ktype == 0) flag = *(int*)(p + pos) == ((DataI*)key)->x;
	else if (Ktype == 1) flag = *(float*)(p + pos) == ((DataF*)key)->x;
	else if (Ktype == 2) flag = (strcmp(p + pos, ((DataC*)key)->x.c_str()) == 0);
	if (flag == false) return -1;
	else {
		char* temp1 = new char[BLOCK_SIZE];
		memcpy(temp1, p, BLOCK_SIZE);
		memcpy(p + pos, temp1 + pos + gap, BLOCK_SIZE - pos - gap);
		memset(p + BLOCK_SIZE - gap, '$', gap);
		delete[] temp1;
		*(int*)(p + 5) = *(int*)(p + 5) - 1;
		return 1;
	}
}

