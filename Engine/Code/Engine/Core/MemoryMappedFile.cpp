#include "MemoryMappedFile.hpp"
#include <cstdio>
#include <cerrno>
#include <iostream>
#include <xiosbase>
#include <ios>
#include <fstream>
#include <iosfwd>
#include <span>

//------------------------------------------------------------------------
MemoryMappedFile::~MemoryMappedFile()
{
	if (m_buffer)
	{
		delete[] m_buffer;
	}
}

//------------------------------------------------------------------------
MemoryMappedFile::MemoryMappedFile(char const* fileName) noexcept
{
	m_iostate = std::ios_base::goodbit;
	// open the file
	FILE* fp = NULL;
	m_error = fopen_s(&fp, fileName, "rb");
	if (m_error || !fp)
	{
		m_iostate = std::ios_base::badbit;
		return;
	}
	// get the file length and allocate the buffer
	m_error = fseek(fp, 0L, SEEK_END);
	if (m_error)
	{
		m_iostate = std::ios_base::failbit;
		m_error = fclose(fp);
		return;
	}
	m_size = ftell(fp);
	if (m_size == -1)
	{
		m_iostate = std::ios_base::failbit;
		m_error = fclose(fp);
		return;
	}
//	m_buffer = new (std::nothrow) uint8_t[m_size * 100000000000]; // test new allocation error
	m_buffer = new (std::nothrow) uint8_t[m_size];
	if (!m_buffer)
	{
		m_iostate = std::ios_base::failbit;
		m_error = fclose(fp);
		return;
	}
	m_error = fseek(fp, 0L, SEEK_SET); // rewind with error checking
	if (m_error)
	{
		m_iostate = std::ios_base::failbit;
		m_error = fclose(fp);
		return;
	}
	// read the data into the buffer in a single read operation
	if (m_buffer && m_iostate == std::ios_base::goodbit)
	{
		size_t size = fread(m_buffer, 1, m_size, fp);
		if (size != m_size)
		{
			m_iostate = std::ios_base::failbit;
		}
	}
	m_error = fclose(fp);
	if (m_error)
	{
		m_iostate = std::ios_base::badbit;
	}
}

//------------------------------------------------------------------------
bool MemoryMappedFile::IsGood()
{
	return m_iostate == std::ios_base::goodbit;
}

//------------------------------------------------------------------------
bool MemoryMappedFile::IsBad()
{
	return m_iostate == std::ios_base::badbit;
}

//------------------------------------------------------------------------
bool MemoryMappedFile::IsFail()
{
	return m_iostate == std::ios_base::failbit;
}

//------------------------------------------------------------------------
uint8_t* MemoryMappedFile::data()
{
	return m_buffer;
}

//------------------------------------------------------------------------
uint8_t* MemoryMappedFile::begin()
{
	return m_buffer;
}

//------------------------------------------------------------------------
uint8_t* MemoryMappedFile::end()
{
	return m_buffer + m_size;
}

//------------------------------------------------------------------------
uint8_t MemoryMappedFile::operator[](size_t index)
{
	return m_buffer[index];
}

//------------------------------------------------------------------------
// I really don't think implementing at() without throwing an exception is
// a good idea, but at worst it behaves like operator[].  Having to test for
// an error that is different than expected behavior defeats the API purpose.
uint8_t MemoryMappedFile::at(size_t index)
{
	if (index > m_size)
	{
		m_iostate = std::ios_base::failbit;
	}
	return m_buffer[index];
}

//------------------------------------------------------------------------
uint8_t MemoryMappedFile::operator[](size_t index) const
{
	return m_buffer[index];
}

//------------------------------------------------------------------------
// I really don't think implementing at() without throwing an exception is
// a good idea, but at worst it behaves like operator[].  Having to test for
// an error that is different than expected behavior defeats the API purpose.
uint8_t MemoryMappedFile::at(size_t index) const
{
	if (index > m_size)
	{
		m_iostate = std::ios_base::failbit;
	}
	return m_buffer[index];
}

//------------------------------------------------------------------------
uint8_t MemoryMappedFile::front()
{
	return *m_buffer;
}

//------------------------------------------------------------------------
uint8_t MemoryMappedFile::back()
{
	return *(m_buffer + m_size - 1);
}

//------------------------------------------------------------------------
size_t MemoryMappedFile::size()
{
	return m_size;
}

//------------------------------------------------------------------------
MemoryMappedFile& MemoryMappedFile::operator=(MemoryMappedFile&& lhs)
{
	m_buffer = std::move(lhs.m_buffer);
	return *this;
}

//------------------------------------------------------------------------
void TestDriver(char const* fileName)
{
	// Error tests
	MemoryMappedFile bad("badfile.txt");
	if (bad.IsGood() == false)
	{
		std::cout << "File error reading non-existent file (test): " << "\n\n";
	}
	MemoryMappedFile mmf(fileName);
	if (mmf.IsGood() == false)
	{
		std::cout << "File error (for real): " << "\n";
		return; // file read failed
	}
	
	std::cout << "File size() is: " << mmf.size() << " bytes\n\n";

	// Iterator and access tests
	uint8_t* index = mmf.begin();
	std::cout << "Contents of file " << fileName << " read using begin() and end() iterators is:\n";
	while (index != mmf.end())
	{
		std::cout << (char)(*index++);
	}
	std::cout << "\n\n";
	std::cout << "Contents of file " << fileName << " read using size() and operator[] iterators is:\n";
	for (size_t index = 0; index < mmf.size(); index++)
	{
		std::cout << (char)(mmf[index]);
	}
	std::cout << "\n\n";
	std::cout << "Read mmf[8]: '" << (char)mmf[8] << "'\n";
	std::cout << "Read mmf.front(): '" << (char)mmf.front() << "' and mmf.back(): '" << (char)mmf.back() << "'\n\n";

	// bounds tests
	std::cout << "Read byte " << (int)(mmf[90]) << " without error\n";
	std::cout << "Read byte " << (int)(mmf.at(90)) << " with error detected: " << (mmf.IsGood() ? "false" : "true") << "\n";
	std::cout << "\n";
	std::cout << "All tests completed.\n";
}

//------------------------------------------------------------------------
int main()
{
	std::cout << "Running test driver\n\n";
	std::string filename = "test.txt";
	TestDriver(filename.c_str());
}