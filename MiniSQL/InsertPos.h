#pragma once
class InsertPos
{
public:
	//在内存中的第几个区块
	int bufferNUM;

	//在区块中的位置
	int position;

	InsertPos();
	~InsertPos();
};

