#include "Engine/Core/FileUtils.hpp"
#include "ErrorWarningAssert.hpp"
#include <cstdio>
#include <cerrno>

int FileReadToBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename)
{
	FILE* fp = NULL;
	errno_t error = fopen_s(&fp, filename.c_str(), "rb");
	if (!fp || error)
	{
		return 5;
		// ERROR_AND_DIE("Failed to open binary file for reading (%i)");
	}
	fseek(fp, 0L, SEEK_END);
	int size = ftell(fp);
	if ((int)outBuffer.size() < size)
	{
		outBuffer.resize(size);
	}
	rewind(fp);
	fread(outBuffer.data(), 1, size, fp);
	fclose(fp);
	return 0;
}

int FileReadToString(std::string& outString, const std::string& filename)
{
	FILE* fp = NULL;
	errno_t error = fopen_s(&fp, filename.c_str(), "rb");
	if (!fp || error)
	{
		return 5;
		// ERROR_AND_DIE("Failed to open binary file for reading (%i)");
	}
	std::fseek(fp, 0, SEEK_END);
	outString.resize(std::ftell(fp));
	std::rewind(fp);
	std::fread(&outString[0], 1, outString.size(), fp);
	std::fclose(fp);
	return 0;
}

int FileWriteBinaryBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename)
{
	FILE* fp = NULL;
	errno_t error = fopen_s(&fp, filename.c_str(), "wb");
	if (!fp || error)
	{
		ERROR_AND_DIE("Failed to open binary file for reading (%i)");
	}
	fwrite(outBuffer.data(), 1, outBuffer.size(), fp);
	fclose(fp);
	return 0;
}

void CheckErroNo()
{
	/*
char errorMsg[43][3] = {
	{"0",	"0",			"0"},
	{"1",	"EPERM		",	"Operation not permitted."},
	{"2",	"ENOENT		",	"No such file or directory."},
	{"3",	"ESRCH		",	"No such process."},
	{"4",	"EINTR		",	"Interrupted function."},
	{"5",	"EIO		",	"I/O error."},
	{"6",	"ENXIO		",	"No such device or address."},
	{"7",	"E2BIG		",	"Argument list too long."},
	{"8",	"ENOEXEC	",	"Exec format error."},
	{"9",	"EBADF		",	"Bad file number."},
	{"10",	"ECHILD		",	"No spawned processes."},
	{"11",	"EAGAIN		",	"No more processes or not enough memory or maximum nesting level reached."},
	{"12",	"ENOMEM		",	"Not enough memory is available for the attempted operator."},
	{"13",	"EACCES		",	"Permission denied."},
	{"14",	"EFAULT		",	"Bad address."},
	{"80",	"STRUNCATE	",	"A string copy or concatenation resulted in a truncated string."},
	{"16",	"EBUSY		",	"Device or resource busy."},
	{"17",	"EEXIST		",	"Files exists.An attempt has been made to create a file that already exists."},
	{"18",	"EXDEV		",	"Cross - device link."},
	{"19",	"ENODEV		",	"No such device."},
	{"20",	"ENOTDIR	",	"Not a directory."},
	{"21",	"EISDIR		",	"Is a directory."},
	{"22",	"EINVAL		",	"Invalid argument."},
	{"23",	"ENFILE		",	"Too many files open in system."},
	{"24",	"EMFILE		",	"Too many open files."},
	{"25",	"ENOTTY		",	"Inappropriate I / O control operation."},
	{"0",	"0			",	"0"},
	{"27",	"EFBIG		",	"File too large."},
	{"28",	"ENOSPC		",	"No space left on device."},
	{"29",	"ESPIPE		",	"Invalid seek."},
	{"30",	"EROFS		",	"Read only file system."},
	{"31",	"EMLINK		",	"Too many links."},
	{"32",	"EPIPE		",	"Broken pipe."},
	{"33",	"EDOM		",	"The argument to a math function is not in the domain of the function."},
	{"34",	"ERANGE		",	"Result too large."},
	{"0",	"0			",	"0"},
	{"36",	"EDEADLK	",	"Resource deadlock would occur."},
	{"0",	"0			",	"0"},
	{"38",	"ENAMETOOLONG",	"Filename too long."},
	{"39",	"ENOLCK		",	"No locks available."},
	{"40",	"ENOSYS		",	"Function not supported."},
	{"41",	"ENOTEMPTY	",	"Directory not empty."},
	{"42",	"EILSEQ		",	"Illegal sequence of bytes(for example, in an MBCS string)."}
}
*/

}

void TestDriver()
{
//	std::string filename = "d:\\untitled.png";
	std::string filename = "d:\\test.txt";
	std::vector<uint8_t> outBuffer;
	std::string outString;
//	int FileWriteBinaryBuffer(std::vector<uint8_t>&outBuffer, const std::string & filename);
	FileReadToBuffer(outBuffer, filename);
	FileReadToString(outString, filename);
}