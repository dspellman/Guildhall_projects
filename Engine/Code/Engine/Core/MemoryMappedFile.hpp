#pragma once
#include <string>
#include <xiosbase>

// MemoryMappedFile reads an entire binary file into memory in a single read operation and provides vector like access to the data
class MemoryMappedFile
{
public:
	~MemoryMappedFile();
	MemoryMappedFile(char const* fileName) noexcept;	// reads file into buffer
	MemoryMappedFile(const MemoryMappedFile& ) = delete;
	MemoryMappedFile& operator=(const MemoryMappedFile& ) = delete;

	bool IsGood();										// returns true if no error detected when file was read (data is valid to process)
	bool IsBad();										// returns whether the badbit was set as the error
	bool IsFail();										// returns whether the failbit was set as the error
	uint8_t* data();									// returns pointer to first byte of data
	uint8_t* begin();									// returns pointer to first byte of data
	uint8_t* end();										// returns pointer to byte after last byte of data
	uint8_t operator[](size_t index);					// returns byte of data at specified index in buffer (index out-of-bounds is undefined)
	// I really don't think implementing at() without throwing an exception is
	// a good idea, but at worst it behaves like operator[].  Having to test for
	// an error that is different than expected behavior defeats the API purpose.
	uint8_t at(size_t index);							// returns byte of data at specified index (must check IsGood() afterwards for error)
	uint8_t operator[](size_t index) const;				// returns byte of data at specified index in buffer (index out-of-bounds is undefined)
 	uint8_t at(size_t index) const;						// returns byte of data at specified index (must check IsGood() afterwards for error)
	uint8_t front();									// returns first byte of data
	uint8_t back();										// returns last byte of data
	size_t size();										// returns number of bytes of data in buffer

private:
	MemoryMappedFile& operator=(MemoryMappedFile&& lhs);

	errno_t m_error = 5;								// initialize to I/O error until proved otherwise
	size_t m_size = 0;									// the number of bytes stored in the buffer (the buffer length)
	uint8_t* m_buffer = nullptr;						// the data storage buffer (allocated and deleted by the class internally)
	mutable std::ios_base::iostate m_iostate = std::ios_base::goodbit;
};

void TestDriver(char const* fileName);					// test driver that prints results of unit tests to screen
