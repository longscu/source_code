#ifndef BUFFERS_H_
#define BUFFERS_H_

#include <algorithm>
#include <iostream>

const size_t default_buffer_blocks = 16;
const size_t default_block_size = 4096;

template <class T>
class general_buffer
{
	protected:
	FILE *_file;
	size_t _buffer_size;
	size_t _cur_buf_size;
	size_t _buf_offset;
	T *_buffer;
	size_t _count;
	
	public:
	general_buffer(FILE *file, size_t buf_size)
	{
		_file = file;
		if(buf_size == 0)
			buf_size = default_buffer_blocks;
		buf_size *= default_block_size;
		
		_buffer_size = buf_size/sizeof(T);
		_buffer = new T[_buffer_size];
		_count = 0;
	}
	
	bool valid()
	{
		return _buffer != 0;
	}
	
	size_t mem_consumption()
	{
		return _buffer_size *sizeof(T);
	}
	
	size_t get_count()
	{
		return _count;
	}
	
	void release()
	{
		if(_buffer != 0)
			delete []_buffer;
		buffer = 0;
	}
	
	~general_buffer()
	{
		release();
	}	
};

template <class T>
class input_buffer:public general_buffer<T>
{
	private:
	size_t _offset_in_file;
	size_t _remain;
	
	size_t _first_fetch_offset;
	
	bool get_next_blocks()
	{
		if(remain <= 0)
			return false;
			
		size_t real_offset = ftell(_file);
		if(real_offset == size_t(-1))
			return false;
		if(real_offset != _offset_in_file)
		{
			if(!fseek(file, _offset_in_file))
				return false;
		}
		
		size_t next_fetch = std::min(_remain + _first_fetch_offset, _buffer_size)*sizeof(T);
		if(!fread(buffer, 1, next_fetch, file))
			return false;
		else
		{
			_offset_in_file += next_fetch;
			_cur_buf_size = next_fetch/sizeof(T);
			_buf_offset = _first_fetch_offset;
			_first_fetch_offset = 0;
			return true;
		}
	}
	
	public:
		input_buffer(FILE *file, size_t buf_size = 0, size_t start_pos = 0):general_buffer<T>(file, bufsize)
		{
			_cur_buf_size = 0;
			_buf_offset = 0;
			
			size_t sblock_id = start_pos/_buffer_size;
			size_t sblock_offset = start_pos%_buffer_size;
			
			_first_fetch_offset = sblock_offset;
			_offset_in_file = sblock_id *_buffer_size*sizeof(T);
			_remain = get_file_size(file)/sizeof(T) - start_pos;
		}
		
		bool get_top(T &out)
		{
			if(_buf_offset == _cur_buf_size)
			{
				if(!get_next_blocks())
					return false;
			}
			out = _buffer[_buf_offset];
			return true;
		}
		
		bool pop_top(T &out)
		{
			if(!get_top(out))
				return false;
			if(!pop())
				return false;
			return true;
		}
		
		bool pop(size_t n = 1)
		{
			if(n > _remain)
				return false;
				
			size_t delta1 = _cur_buf_size - _buf_offset;
			if(n > delta1)
			{
				n -= delta1;
				remain -= delta1;
				_buf_offset += delta1;
				
				size_t sblock_delta = n/_buffer_size;
				size_t sblock_offset = n % _buffer_size;
				
				_offset_in_file += sblock_delta * _buffer_size * sizeof(T);
				remain -= sblock_delta * _buffer_size;
				if(!get_next_block())
					return false;
					
				n -= sblock_delta * _buffer_size;
			}
			
			_buf_offset += n;
			_remain -= n;
			_count += n;
			return true;
		}
		
		
		size_t get_remain()
		{
			return _remain;
		}
		
		bool empty()
		{
			return remain == 0;
		}
	
};
#endif