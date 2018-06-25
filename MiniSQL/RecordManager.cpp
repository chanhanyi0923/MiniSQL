﻿#include "RecordManager.h"

#include <fstream>
#include <string>

#include "CatalogManager.h"
#include "IndexManager.h"
#include "InsertPos.h"
#include "Data.h"
#include "DataI.h"
#include "DataF.h"
#include "DataC.h"

using namespace std;

RecordManager::RecordManager()
{
}


RecordManager::RecordManager(BufferBlock bf):
	buffer(bf)
{
}


RecordManager::~RecordManager()
{
}


bool RecordManager::CreateTable(Table& tableIn, BufferBlock buffer)
{
	string filename = tableIn.getname() + ".table";
	fstream fout(filename.c_str(), ios::out);
	fout.close();
	tableIn.blockNum = 1;
	CatalogManager Ca;
	Ca.changeblock(tableIn.getname(), tableIn.blockNum);
	return true;
}

Tuple* RecordManager::Char2Tuple(Table& tableIn, char* Tuplestring)
{
	Tuple* temp_tuple;
	temp_tuple = new Tuple;
	//string Tuplestr = Tuplestring;
	if (Tuplestring[0] == EMPTY) return temp_tuple;//该行是空的
	int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
	for (int attr_index = 0; attr_index < tableIn.getattribute().num; attr_index++) {
		if (tableIn.getattribute().flag[attr_index] == -1) {//是一个整数
			int value;
			memcpy(&value, &(Tuplestring[c_pos]), sizeof(int));
			c_pos += sizeof(int);
			temp_tuple->addData(new DataI(value));
		}
		else if (tableIn.getattribute().flag[attr_index] == 0) {//float
			float value;
			memcpy(&value, &(Tuplestring[c_pos]), sizeof(float));
			c_pos += sizeof(float);
			temp_tuple->addData(new DataF(value));
		}
		else {
			char value[MAXSTRINGLEN];
			int strLen = tableIn.getattribute().flag[attr_index] + 1;
			memcpy(value, &(Tuplestring[c_pos]), strLen);
			c_pos += strLen;
			temp_tuple->addData(new DataC(string(value)));
		}
	}//以上内容先从文件中生成一行tuper
	return temp_tuple;
}

char* RecordManager::Tuple2Char(Table& tableIn, Tuple& singleTuple)
{
	char* ptrRes;
	int pos = 0;//当前的插入位置
				//cout << tableIn.dataSize() << endl;
	ptrRes = new char[(tableIn.dataSize() + 1) * sizeof(char)];
	for (int i = 0; i < tableIn.getattribute().num; i++) {
		if (tableIn.getattribute().flag[i] == -1) { //int
			int value = ((DataI*)singleTuple[i])->x;
			memcpy(ptrRes + pos, &value, sizeof(int));
			pos += sizeof(int);
		}
		else if (tableIn.getattribute().flag[i] == 0) {
			float value = ((DataF*)singleTuple[i])->x;
			memcpy(ptrRes + pos, &value, sizeof(float));
			pos += sizeof(float);
		}
		else { //string
			string value(((DataC*)singleTuple[i])->x);
			int strLen = tableIn.getattribute().flag[i] + 1;
			memcpy(ptrRes + pos, value.c_str(), strLen);//多加1，拷贝最后的'\0';
			pos += strLen;
		}
	}
	//字符串最后加一个'\0'表示这条Tuple的字符串结束
	ptrRes[tableIn.dataSize()] = '\0';
	return ptrRes;
}

Tuple RecordManager::String2Tuple(Table& tableIn, string stringRow)
{
	Tuple temp_tuple;
	//if (stringRow.c_str()[0] == EMPTY) return temp_tuple;//该行是空的
	int c_pos = 0;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
	for (int attr_index = 0; attr_index < tableIn.getattribute().num; attr_index++) {
		if (tableIn.getattribute().flag[attr_index] == -1) {//是一个整数
			int value;
			memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
			printf("value:%08x\n", value);
			c_pos += sizeof(int);
			temp_tuple.addData(new DataI(value));
		}
		else if (tableIn.getattribute().flag[attr_index] == 0) {//float
			float value;
			memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
			c_pos += sizeof(float);
			temp_tuple.addData(new DataF(value));
		}
		else {
			char value[MAXSTRINGLEN];
			int strLen = tableIn.getattribute().flag[attr_index] + 1;
			memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
			c_pos += strLen;
			temp_tuple.addData(new DataC(string(value)));
		}
	}
	//以上内容先从文件中生成一行tuple
	return temp_tuple;
}

//int RecordManager::FindWithIndex(Table& tableIn, Tuple& row, int mask) {
//	IndexManager indexMA;
//	for (int i = 0; i < tableIn.index.num; i++) {
//		if (tableIn.index.location[i] == mask) { //找到索引
//			Data* ptrData;
//			ptrData = row[mask];
//			int pos = indexMA.Find(tableIn.getname() + to_string(mask) + ".index", ptrData);
//			return pos;
//		}
//	}
//	return -1;
//}

void RecordManager::Insert(Table& tableIn, Tuple& singleTuple)
{
	/////////////////////////////////////////////
	//check Redundancy using index
	//for (int i = 0; i < tableIn.attr.num; i++) {
	//	if (tableIn.attr.unique[i] == 1) {
	//		int addr = FindWithIndex(tableIn, singleTuple, i);
	//		if (addr >= 0) { //already in the table 
	//			throw ("Unique Value Redundancy occurs, thus insertion failed");
	//			return;
	//		}
	//	}
	//}
	///////////////////////////////////////////////
	for (int i = 0; i < tableIn.attr.num; i++) {
		if (tableIn.attr.unique[i]) {
			vector<Where> w;
			vector<int> mask;
			Where *uni_w = new Where;
			uni_w->flag = Where::eq;
			switch (singleTuple[i]->flag) {
			case -1:uni_w->d = new DataI(((DataI*)singleTuple[i])->x); break;
			case 0:uni_w->d = new DataF(((DataF*)singleTuple[i])->x); break;
			default:uni_w->d = new DataC(((DataC*)singleTuple[i])->x); break;
			}
			w.push_back(*uni_w);
			mask.push_back(i);
			//Table temp_table = Select(tableIn, mask, mask, w);
			//if (temp_table.T.size() != 0) {
			//throw ("Unique Value Redundancy occurs, thus insertion failed");
			//}
			//判断是否unique
			if (!UNIQUE(tableIn, w[0], i)) {
				throw ("Unique Value Redundancy occurs, thus insertion failed");
			}

			delete uni_w->d;
			delete uni_w;
		}
	}

	char *charTuple;
	charTuple = Tuple2Char(tableIn, singleTuple);//把一个元组转换成字符串
												 //获取插入位置
	InsertPos iPos;
	if (tableIn.blockNum == 0) { //new file and no block exist 
		iPos.bufferNUM = addBlockInFile(tableIn);
		iPos.position = 0;
	}
	else {
		int bufferNum;
		int blockOffset;
		int length;
		int recordNum;
		int flag = 0;
		for (int i = 0; i < tableIn.blockNum; i++) {
			bufferNum = buffer.read_block(tableIn.Tname, i, 1);
			tableIn.linklist[i] = bufferNum;
		}
		//在每一个储存有数据的block块都比较
		for (int i = 0; i < tableIn.blockNum; i++) {
			string filename = tableIn.getname() + ".table";
			length = tableIn.dataSize() + 1; //多余的一位放在开头，表示是否有效
											 //blockOffset = tableIn.linklist[i];
			bufferNum = tableIn.linklist[i];
			recordNum = BLOCK_SIZE / length;
			for (int offset = 0; offset < recordNum; offset++) {
				int position = offset * length;
				char isEmpty = buffer.m_blocks[bufferNum].address[position];//检查第一位是否有效，判断该行是否有内容
				if (isEmpty == EMPTY) {//find an empty space
					iPos.bufferNUM = bufferNum;
					iPos.position = position;
					flag = 1;
					break;
				}
			}
			if (flag == 1) break;
		}
		//所有block已经装满，新开一个block
		if (flag == 0) {
			iPos.bufferNUM = addBlockInFile(tableIn);
			iPos.position = 0;
		}
	}

	buffer.m_blocks[iPos.bufferNUM].address[iPos.position] = NOTEMPTY;
	memcpy(&(buffer.m_blocks[iPos.bufferNUM].address[iPos.position + 1]), charTuple, tableIn.dataSize());
	int length = tableIn.dataSize() + 1; //一个元组的信息在文档中的长度
	//////////////////////////////////////////////
	//insert tuple into index file
	//IndexManager indexMA;
	//int blockCapacity = BLOCK_SIZE / length;
	//for (int i = 0; i < tableIn.index.num; i++) {
	//	int tuperAddr = buffer.m_blocks[iPos.bufferNUM].offset*blockCapacity + iPos.position / length; //the tuper's addr in the data file
	//	for (int j = 0; j < tableIn.index.num; j++) {
	//		indexMA.Insert(tableIn.getname() + to_string(tableIn.index.location[j]) + ".index", singleTuple[tableIn.index.location[i]], tuperAddr);
	//	}
	//}
	//////////////////////////////////////////////
	buffer.m_blocks[iPos.bufferNUM].written();
	delete[] charTuple;
}

int RecordManager::Delete(Table& tableIn, vector<int>mask, vector<Where> w) {
	string filename = tableIn.getname() + ".table";
	string stringRow;

	int count = 0;
	int length = tableIn.dataSize() + 1;
	const int recordNum = BLOCK_SIZE / length;
	int bufferNum;
	for (int i = 0; i < tableIn.blockNum; i++) {
		bufferNum = buffer.read_block(tableIn.Tname, i, 1);
		tableIn.linklist[i] = bufferNum;
	}
	for (int blockOffset = 0; blockOffset < tableIn.blockNum; blockOffset++) {
		/*	int bufferNum = buf_ptr->getIfIsInBuffer(filename, blockOffset);
		if (bufferNum == -1) {
		bufferNum = buf_ptr->getEmptyBuffer();
		buf_ptr->readBlock(filename, blockOffset, bufferNum);
		}*/
		bufferNum = tableIn.linklist[blockOffset];
		for (int offset = 0; offset < recordNum; offset++) {
			int position = offset * length;
			stringRow = buffer.m_blocks[bufferNum].getvalues(position, position + length);
			if (stringRow.c_str()[0] == EMPTY) continue;
			int c_pos = 1;
			Tuple *temp_tuple = new Tuple;
			for (int attr_index = 0; attr_index < tableIn.getattribute().num; attr_index++) {
				if (tableIn.getattribute().flag[attr_index] == -1) {//int
					int value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
					c_pos += sizeof(int);
					temp_tuple->addData(new DataI(value));
				}
				else if (tableIn.getattribute().flag[attr_index] == 0) {//float
					float value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
					c_pos += sizeof(float);
					temp_tuple->addData(new DataF(value));
				}
				else {//char
					char value[MAXSTRINGLEN];
					int strLen = tableIn.getattribute().flag[attr_index] + 1;
					memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
					c_pos += strLen;
					temp_tuple->addData(new DataC(string(value)));
				}
			}

			if (isSatisfied(tableIn, *temp_tuple, mask, w)) {
				buffer.m_blocks[bufferNum].address[position] = DELETED; //DELETED==EMYTP
				buffer.m_blocks[bufferNum].written();
				count++;
			}
		}
	}
	return count;
}

bool RecordManager::DropTable(Table& tableIn) {
	string filename = tableIn.getname() + ".table";
	if (remove(filename.c_str()) != 0) {
		throw ("Can't delete the file!\n");
	}
	else {
		for (int i = 0; i < MAXBLOCKNUM; i++) {
			if (buffer.m_blocks[i].name == filename) {
				buffer.m_blocks[i].not_being_used();
				buffer.m_blocks[i].dirty_bit = 0;
			}
		}
	}
	return true;
}

Table RecordManager::SelectProject(Table& tableIn, vector<int>attrSelect) {
	//tableIn是要选择的表，即目标表
	//vector<int>attrSelect是需要映射的属性
	Attribute AtbOut;
	Tuple *ptrTuple = NULL;
	//将要选择的属性都存在一个attribute实例中
	AtbOut.num = attrSelect.size();
	for (int i = 0; i < AtbOut.num; i++) {
		AtbOut.flag[i] = tableIn.getattribute().flag[attrSelect[i]];
		AtbOut.name[i] = tableIn.getattribute().name[attrSelect[i]];
		AtbOut.unique[i] = tableIn.getattribute().unique[attrSelect[i]];
	}
	Table tableOut(tableIn.getname(), AtbOut, tableIn.blockNum);
	int choose;
	for (int i = 0; i < tableIn.T.size(); i++) {//tuple的个数
		ptrTuple = new Tuple;
		for (int j = 0; j < AtbOut.num; j++) {
			choose = attrSelect[j];
			Data *res_add = NULL;
			if (tableIn.T[i]->operator [](choose)->flag == -1) {
				res_add = new DataI((*((DataI*)tableIn.T[i]->operator [](choose))).x);
			}
			else if (tableIn.T[i]->operator [](choose)->flag == 0) {
				res_add = new DataF((*((DataF*)tableIn.T[i]->operator [](choose))).x);
			}
			else if (tableIn.T[i]->operator [](choose)->flag>0) {
				res_add = new DataC((*((DataC*)tableIn.T[i]->operator [](choose))).x);
			}

			ptrTuple->addData(res_add);//bug

		}
		tableOut.addData(ptrTuple);
	}

	return tableOut;
}

bool RecordManager::isSatisfied(Table& tableinfor, Tuple& row, vector<int> mask, vector<Where> w) {
	bool res = true;//？？？？？？？？？？？？？？？
	for (int i = 0; i < mask.size(); i++) {
		if (w[i].d == NULL) { //不存在Where条件
			continue;
		}
		else if (row[mask[i]]->flag == -1) { //int
			switch (w[i].flag) {
			case Where::eq: if (!(((DataI*)row[mask[i]])->x == ((DataI*)w[i].d)->x)) return false; break;
			case Where::leq: if (!(((DataI*)row[mask[i]])->x <= ((DataI*)w[i].d)->x)) return false; break;
			case Where::l: if (!(((DataI*)row[mask[i]])->x < ((DataI*)w[i].d)->x)) return false; break;
			case Where::geq: if (!(((DataI*)row[mask[i]])->x >= ((DataI*)w[i].d)->x)) return false; break;
			case Where::g: if (!(((DataI*)row[mask[i]])->x >((DataI*)w[i].d)->x)) return false; break;
			case Where::neq: if (!(((DataI*)row[mask[i]])->x != ((DataI*)w[i].d)->x)) return false; break;
			default:;
			}
		}
		else if (row[mask[i]]->flag == 0) { //Float
			switch (w[i].flag) {
			case Where::eq: if (!(abs(((DataF*)row[mask[i]])->x - ((DataF*)w[i].d)->x)<MIN_Theta)) return false; break;
			case Where::leq: if (!(((DataF*)row[mask[i]])->x <= ((DataF*)w[i].d)->x)) return false; break;
			case Where::l: if (!(((DataF*)row[mask[i]])->x < ((DataF*)w[i].d)->x)) return false; break;
			case Where::geq: if (!(((DataF*)row[mask[i]])->x >= ((DataF*)w[i].d)->x)) return false; break;
			case Where::g: if (!(((DataF*)row[mask[i]])->x >((DataF*)w[i].d)->x)) return false; break;
			case Where::neq: if (!(((DataF*)row[mask[i]])->x != ((DataF*)w[i].d)->x)) return false; break;
			default:;
			}
		}
		else if (row[mask[i]]->flag > 0) { //string
			switch (w[i].flag) {
			case Where::eq: if (!(((DataC*)row[mask[i]])->x == ((DataC*)w[i].d)->x)) return false; break;
			case Where::leq: if (!(((DataC*)row[mask[i]])->x <= ((DataC*)w[i].d)->x)) return false; break;
			case Where::l: if (!(((DataC*)row[mask[i]])->x < ((DataC*)w[i].d)->x)) return false; break;
			case Where::geq: if (!(((DataC*)row[mask[i]])->x >= ((DataC*)w[i].d)->x)) return false; break;
			case Where::g: if (!(((DataC*)row[mask[i]])->x >((DataC*)w[i].d)->x)) return false; break;
			case Where::neq: if (!(((DataC*)row[mask[i]])->x != ((DataC*)w[i].d)->x)) return false; break;
			default:;
			}
		}
		//else { //just for debug
		//	cout << "Error in RecordManager in function is satisified!" << endl;
		//	system("pause");
		//}
	}
	return res;
}

Table RecordManager::Select(Table& tableIn, vector<int>attrSelect, vector<int>mask, vector<Where>& w) {
	if (mask.size() == 0) {
		return Select(tableIn, attrSelect);
	}
	string stringRow;
	string filename = tableIn.getname() + ".table";
	string indexfilename;
	///////////////为什么+1？:第一位为有效位
	int length = tableIn.dataSize() + 1;
	const int recordNum = BLOCK_SIZE / length;

	
	for (int blockOffset = 0; blockOffset < tableIn.blockNum; blockOffset++) {

		//返回一个
		int bufferNum = buffer.read_block(filename, blockOffset, 1);
		//tableIn.linklist.push_back(bufferNum);
		for (int offset = 0; offset < recordNum; offset++) {
			int position = offset * length;
			stringRow = buffer.m_blocks->m_blocks[bufferNum].getvalues(position, position + length);
			if (stringRow.c_str()[0] == EMPTY) continue;//该行是空的
			int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
			Tuple *temp_tuple = new Tuple;
			for (int attr_index = 0; attr_index < tableIn.getattribute().num; attr_index++) {
				if (tableIn.getattribute().flag[attr_index] == -1) {//是一个整数
					int value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
					c_pos += sizeof(int);
					temp_tuple->addData(new DataI(value));
				}
				else if (tableIn.getattribute().flag[attr_index] == 0) {//float
					float value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
					c_pos += sizeof(float);
					temp_tuple->addData(new DataF(value));
				}
				else {
					char value[MAXSTRINGLEN];
					int strLen = tableIn.getattribute().flag[attr_index] + 1;
					memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
					c_pos += strLen;
					temp_tuple->addData(new DataC(string(value)));
				}
			}//以上内容先从文件中生成一行tuper，一下判断是否满足要求

			if (isSatisfied(tableIn, *temp_tuple, mask, w)) {
				tableIn.addData(temp_tuple); //可能会存在问题;solved!
			}
			else delete temp_tuple;
		}
	}
	return SelectProject(tableIn, attrSelect);
}

Table RecordManager::Select(Table& tableIn, vector<int>attrSelect) {
	string stringRow;
	string filename = tableIn.getname() + ".table";
	Tuple* temp_tuple;
	int length = tableIn.dataSize() + 1; //一个元组的信息在文档中的长度
	const int recordNum = BLOCK_SIZE / length; //一个block中存储的记录条数
	for (int blockOffset = 0; blockOffset < tableIn.blockNum; blockOffset++) {//读取整个文件中的所有内容
																			  //int bufferNum = buf_ptr->getIfIsInBuffer(filename, blockOffset);
																			  //if (bufferNum == -1) { //该块不再内存中，读取之
																			  //	bufferNum = buf_ptr->getEmptyBuffer();
																			  //	buf_ptr->readBlock(filename, blockOffset, bufferNum);
																			  //}
		int bufferNum = buffer.read_block(filename, blockOffset, 1);
		for (int offset = 0; offset < recordNum; offset++) {
			int position = offset * length;
			stringRow = buffer.m_blocks[bufferNum].getvalues(position, position + length);
			if (stringRow.c_str()[0] == EMPTY) continue;//该行是空的
			int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
			temp_tuple = new Tuple;
			for (int attr_index = 0; attr_index < tableIn.getattribute().num; attr_index++) {
				if (tableIn.getattribute().flag[attr_index] == -1) {//是一个整数
					int value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
					c_pos += sizeof(int);
					temp_tuple->addData(new DataI(value));
				}
				else if (tableIn.getattribute().flag[attr_index] == 0) {//float
					float value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
					c_pos += sizeof(float);
					temp_tuple->addData(new DataF(value));
				}
				else {
					char value[MAXSTRINGLEN];
					int strLen = tableIn.getattribute().flag[attr_index] + 1;
					memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
					c_pos += strLen;
					temp_tuple->addData(new DataC(string(value)));
				}
			}
			tableIn.addData(temp_tuple); //可能会存在问题;solved!
		}
	}
	return SelectProject(tableIn, attrSelect);
}

bool RecordManager::UNIQUE(Table& tableIn, Where w, int loca) {
	int length = tableIn.dataSize() + 1; //一个元组的信息在文档中的长度
	const int recordNum = BLOCK_SIZE / length; //一个block中存储的记录条数
	string stringRow;
	string filename = tableIn.getname() + ".table";
	int attroff = 1;//从1开始因为0是有效位
	for (int i = 0; i<loca - 1; i++) {
		if (tableIn.attr.flag[i] == -1) {
			attroff += sizeof(int);
		}
		else if (tableIn.attr.flag[i] == 0) {
			attroff += sizeof(float);
		}
		else {
			attroff += sizeof(char)*tableIn.attr.flag[i];
		}
	}
	int inflag = tableIn.attr.flag[loca];
	for (int blockOffset = 0; blockOffset < tableIn.blockNum; blockOffset++) {
		//读取整个文件中的所有内容
		int bufferNum = buffer.read_block(filename, blockOffset, 1);
		for (int offset = 0; offset < recordNum; offset++) {
			int position = offset * length + attroff;
			if (inflag == -1) {
				int value;
				memcpy(&value, &(buffer.m_blocks[bufferNum].address[position + 4]), sizeof(int));
				if (value == ((DataI*)(w.d))->x)
					return false;
			}
			else if (inflag == 0) {
				float value;
				memcpy(&value, &(buffer.m_blocks[bufferNum].address[position + 4]), sizeof(float));
				if (value == ((DataF*)(w.d))->x)
					return false;
			}
			else {
				char value[MAXSTRINGLEN];
				memcpy(value, &(buffer.m_blocks[bufferNum].address[position + 4]), tableIn.attr.flag[loca] + 1);
				if (string(value) == ((DataC*)(w.d))->x)
					return false;
			}
		}
	}
	return true;
}

int RecordManager::addBlockInFile(Table& tableinfor)
{
	string filename = tableinfor.getname() + ".table";

	int bufferNum = buffer.get_blank(filename);
	tableinfor.linklist[tableinfor.blockNum] = bufferNum;
	buffer.m_blocks[bufferNum].flush_block();
	buffer.m_blocks[bufferNum].name = tableinfor.getname() + ".table";
	buffer.m_blocks[bufferNum].offset = tableinfor.blockNum++;
	CatalogManager ca;
	ca.changeblock(tableinfor.getname(), tableinfor.blockNum);
	return bufferNum;
}

