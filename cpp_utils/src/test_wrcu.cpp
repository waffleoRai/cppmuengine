//============================================================================
// Name        : 
// Author      : Blythe Hospelhorn
// Version     :
//============================================================================

#include "FileStreamer.h"

#include <iostream>
#include "unicode/ustdio.h"
#include "unicode/ustream.h"
#include <io.h>
#include <fcntl.h>

//using namespace std;
using namespace waffleoRai_Utils;

void testFileInStream(FileInputStreamer& fis){
	cout << "isOpen: " << fis.isOpen() << "\n";
	cout << "atEnd: " << fis.streamEnd() << "\n";
	cout << "isGood: " << fis.isGood() << "\n";
	cout << "isBad: " << fis.isBad() << "\n";
	cout << "isFail: " << fis.isFail() << "\n";
	printf("Remaining: 0x%llx\n", fis.remaining());
	printf("Read: 0x%llx\n", fis.bytesRead());
	printf("File Size: 0x%llx\n", fis.fileSize());
}

void testInput(DataInputStreamer& fis){
	//8 4 2 3 1
	printf("Long (Unsigned): 0x%016llx\n", fis.nextUnsignedLong());
	printf("Int (Unsigned): 0x%08x\n", fis.nextUnsignedInt());
	printf("Short (Unsigned): 0x%04x\n", fis.nextUnsignedShort());
	printf("24 (Unsigned): 0x%06x\n", fis.nextUnsigned24());
	printf("Byte (Unsigned): 0x%02x\n", fis.nextByte());
}

void testFileInput(string& filepath, Endianness e, u64 offset, u64 skip) {
	FileInputStreamer fis = FileInputStreamer(filepath);
	DataInputStreamer dis = DataInputStreamer(fis, e);

	testFileInStream(fis);
	cout << "Opening Stream...\n";
	fis.open(offset);
	cout << "Stream open!\n";
	testFileInStream(fis);
	testInput(dis);
	testFileInStream(fis);
	fis.jumpTo(skip);
	cout << "Jumped!\n";
	testInput(dis);
	testFileInStream(fis);
	cout << "Closing Stream...\n";
	dis.close();
	fis.close();
	testFileInStream(fis);
	cout << "Stream test complete!\n";
}

void testFileOutput(string& filepath, Endianness e, bool append) {
	FileOutputStreamer fos = FileOutputStreamer(filepath);
	DataOutputStreamer dos = DataOutputStreamer(fos, e);
	cout << "Opening Stream...\n";
	if (append) fos.openForAppending();
	else fos.open();
	cout << "Stream open!\n";
	//01
	cout << "Testing 8-bit put...\n";
	dos.putByte(0x01);
	//02 03 04
	cout << "Testing 24-bit put...\n";
	dos.put24(0x020304);
	//05 06
	cout << "Testing 16-bit put...\n";
	dos.putUnsignedShort(0x0506);
	//07 08
	dos.putUnsignedShort(0x0708);
	//09 0a 0b 0c
	cout << "Testing 32-bit put...\n";
	dos.putUnsignedInt(0x090a0b0c);
	//0d 0e 0f 10
	dos.putUnsignedInt(0x0d0e0f10);
	cout << "Testing 64-bit put...\n";
	dos.putUnsignedLong(0x1020304050607080ULL); //Might not work on windows
	dos.putUnsignedLong(0x90a0b0c0d0e0f000ULL); //Might not work on windows
	cout << "Testing put bytes...\n";
	ubyte btest[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
					0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	fos.addBytes(btest, 16);
	cout << "Closing Stream...\n";
	dos.close();
	fos.close();
	cout << "Stream test complete!\n";
	cout << "Output test written to: " << filepath + "\n";
}

void testUtilities() {

	//some mundane Endian-ness tests
	if (wrcu_sys_big_endian()) cout << "This system is Big-Endian\n";
	else cout << "This system is Little-Endian\n";
	uint64_t val8 = 0x0123456789abcdefULL;
	uint32_t val4 = 0x01234567;
	uint16_t val2 = 0x0123;

	printf("0x%04x byte reverses to ", val2);
	wrcu_reverseBytes(reinterpret_cast<uint8_t*>(&val2), 2);
	printf("0x%04x\n", val2);

	printf("0x%08x byte reverses to ", val4);
	wrcu_reverseBytes(reinterpret_cast<uint8_t*>(&val4), 4);
	printf("0x%08x\n", val4);

	printf("0x%016llx byte reverses to ", val8);
	wrcu_reverseBytes(reinterpret_cast<uint8_t*>(&val8), 8);
	printf("0x%016llx\n", val8);

	//num to str tests
	char dstr[6];
	char hstr[5];
	u16_to_decstr(val2, dstr, 5);
	u16_to_hexstr(val2, hstr, 4);

	cout << "Test decimal string conversion: " << dstr << "\n";
	cout << "Test hex string conversion: " << hstr << "\n";

	cout << "Utility Test End\n\n";
}

void testUnicodePaths(string& testdir) {

	string unicodelist = testdir + "\\filenames.txt";
	UFILE* file = u_fopen(unicodelist.c_str(), "r", NULL, "UTF8");
	UnicodeString ustr1 = UnicodeString(512, 0, 0);

	//First line
	int i;
	for (i = 0; i < testdir.length(); i++) {
		ustr1.append(static_cast<char16_t>(testdir[i]));
	}
	ustr1.append('\\');
	UChar c = '\0';
	while ((c = u_fgetc(file)) != -1) {
		if (c == '\n' || c == '\0') break;
		ustr1.append(c);
		printf("%04x ", c);
	}
	printf("\n");
	ustr1.append('\0');

	//Second line
	UnicodeString ustr2 = UnicodeString(512, 0, 0);
	for (i = 0; i < testdir.length(); i++) {
		ustr2.append(static_cast<char16_t>(testdir[i]));
	}
	ustr2.append('\\');
	while ((c = u_fgetc(file)) != -1) {
		if (c == '\n' || c == '\0') break;
		ustr2.append(c);
	}
	ustr2.append('\0');

	u_fclose(file);

	//Print results - looks like we can write this to cout using ustream
	cout << "Path 1 Read: " << ustr1 << "\n";
	cout << "Path 2 Read: " << ustr2 << "\n";

	//Try to open these files TODO
	cout << "--> Unicode Path Input\n";
	try
	{
		const char16_t* buff = ustr1.getTerminatedBuffer();
		while(*(buff) != '\0') printf("%04x ", *(buff++));
		printf("\n");

		//filesystem::path tpath = filesystem::path(buff);

		FileInputStreamer fis = FileInputStreamer(ustr1.getTerminatedBuffer());
		DataInputStreamer dis = DataInputStreamer(fis, Endianness::little_endian);

		testFileInStream(fis);
		cout << "Opening Stream...\n";
		fis.open();
		cout << "Stream open!\n";
		testFileInStream(fis);
		testInput(dis);
		testFileInStream(fis);
		cout << "Closing Stream...\n";
		dis.close();
		fis.close();
		testFileInStream(fis);
		cout << "Stream test complete!\n";
	}
	catch (InputException& e)
	{
		cout << "Exception caught! :|\n";
		cout << e.what() << "\n";
	}

	cout << "--> Unicode Path Output\n";
	try {
		FileOutputStreamer fos = FileOutputStreamer(ustr2.getTerminatedBuffer());
		DataOutputStreamer dos = DataOutputStreamer(fos, Endianness::little_endian);
		cout << "Opening Stream...\n";
		fos.open();
		cout << "Stream open!\n";
		//01
		cout << "Testing 8-bit put...\n";
		dos.putByte(0x01);
		//02 03 04
		cout << "Testing 24-bit put...\n";
		dos.put24(0x020304);
		//05 06
		cout << "Testing 16-bit put...\n";
		dos.putUnsignedShort(0x0506);
		//07 08
		dos.putUnsignedShort(0x0708);
		//09 0a 0b 0c
		cout << "Testing 32-bit put...\n";
		dos.putUnsignedInt(0x090a0b0c);
		//0d 0e 0f 10
		dos.putUnsignedInt(0x0d0e0f10);
		cout << "Testing 64-bit put...\n";
		dos.putUnsignedLong(0x1020304050607080ULL); //Might not work on windows
		dos.putUnsignedLong(0x90a0b0c0d0e0f000ULL); //Might not work on windows
		cout << "Testing put bytes...\n";
		ubyte btest[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
						0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
		fos.addBytes(btest, 16);
		cout << "Closing Stream...\n";
		dos.close();
		fos.close();
		cout << "Stream test complete!\n";
		cout << "Output test written to: " << ustr2 << "\n";
	}
	catch (OutputException& e) { cout << "Exception caught! :|\n"; cout << e.what() << "\n"; }

}

void testFileStreamer(string& testdir)
{
	string filepath1 = testdir + "\\06_StoneTowerTemple.mid";
	string fakefilepath = testdir + "\\dne.bin";
	string outpathb = testdir + "\\outbe.bin";
	string outpathl = testdir + "\\outle.bin";
	string strfilepath = testdir + "\\uniintest_le.bin";
	string strfilepath2 = testdir + "\\uniouttest_le.bin";
	u32 oob1 = 0x10000; //Past end of midi
	u32 offset = 0x30a; //Start of second midi track - for checking opening middle of file

	//--Test input
	cout << "--> Input (BE) - Offset 0x00\n";
	testFileInput(filepath1, Endianness::big_endian, 0, offset);

	//Repeat with LE
	cout << "--> Input (LE) - Offset 0x00\n";
	testFileInput(filepath1, Endianness::little_endian, 0, offset);

	//Repeat opening at offset
	printf("--> Input (BE) - Offset 0x%x\n", offset);
	testFileInput(filepath1, Endianness::big_endian, offset, offset);

	//Try opening past end of file
	printf("--> Input (BE) - Offset 0x%x\n", oob1);
	try
	{
		testFileInput(filepath1, Endianness::big_endian, oob1, offset);
	}
	catch(InputException& e)
	{
		cout << "Exception caught! :)\n";
		cout << e.what() << "\n";
	}

	//Try jumping past end of file
	printf("--> Input (LE) - Offset 0x%x (OB Jump)\n", offset);
	try
	{
		testFileInput(filepath1, Endianness::little_endian, offset, oob1);
	}
	catch(InputException& e)
	{
		cout << "Exception caught! :)\n";
		cout << e.what() << "\n";
	}

	//Try file that doesn't exist
	printf("--> Bad File Input - Offset 0x00\n");
	try
	{
		testFileInput(fakefilepath, Endianness::little_endian, 0, offset);
	}
	catch(InputException& e)
	{
		cout << "Exception caught! :)\n";
		cout << e.what() << "\n";
	}

	//--Test output
	cout << "--> Output (BE)\n";
	try{testFileOutput(outpathb, Endianness::big_endian, false);}
	catch(OutputException& e){cout << "Exception caught! :<\n"; cout << e.what() << "\n";}

	cout << "--> Output (LE)\n";
	try{testFileOutput(outpathl, Endianness::little_endian, false);}
	catch(OutputException& e){cout << "Exception caught! :<\n"; cout << e.what() << "\n";}

	cout << "--> Output Append (LE to BE)\n";
	try{testFileOutput(outpathb, Endianness::little_endian, true);}
	catch(OutputException& e){cout << "Exception caught! :<\n"; cout << e.what() << "\n";}

	//Test Unicode Paths
	cout << "--> Unicode Paths\n";
	testUnicodePaths(testdir);

	//Test Strings
	cout << "--> Unicode/ASCII Strings\n";
	try
	{
		FileInputStreamer fis = FileInputStreamer(strfilepath);
		DataInputStreamer dis = DataInputStreamer(fis, Endianness::little_endian);
		fis.open();

		FileOutputStreamer fos = FileOutputStreamer(strfilepath2);
		DataOutputStreamer dos = DataOutputStreamer(fos, Endianness::little_endian);
		fos.open();

		//Plain ASCII
		size_t strsz = static_cast<size_t>(dis.nextUnsignedShort());
		string asciistr = string();
		asciistr.reserve(strsz + 1);
		size_t read = dis.readASCIIString(asciistr, strsz);
		cout << "String read: " << asciistr << "\n";
		cout << "Bytes read: " << read << "\n";
		//Write Back...
		dos.putASCIIString(asciistr, true, true);

		//UTF8 ASCII
		strsz = static_cast<size_t>(dis.nextUnsignedShort());
		int32_t asz = static_cast<int32_t>(strsz) + 1;
		UnicodeString unistr = UnicodeString(asz, ' ', 0);
		read = dis.readUTF8String(unistr, strsz);
		cout << "String read: " << unistr << "\n";
		cout << "Bytes read: " << read << "\n";
		dos.putUTF8String(unistr, true, true);

		//UTF16 BOM ASCII
		strsz = static_cast<size_t>(dis.nextUnsignedShort());
		asz = (static_cast<int32_t>(strsz) + 1) >> 1;
		unistr = UnicodeString(asz, ' ', 0);
		read = dis.readUTF16String(unistr, strsz, true);
		cout << "String read: " << unistr << "\n";
		cout << "Bytes read: " << read << "\n";
		dos.putUTF16String(unistr, true, true);

		//UTF16 BE ASCII
		strsz = static_cast<size_t>(dis.nextUnsignedShort());
		dis.setEndianness(Endianness::big_endian);
		dos.setEndianness(Endianness::big_endian);
		asz = (static_cast<int32_t>(strsz) + 1) >> 1;
		unistr = UnicodeString(asz, ' ', 0);
		read = dis.readUTF16String(unistr, strsz, false);
		cout << "String read: " << unistr << "\n";
		cout << "Bytes read: " << read << "\n";
		dos.putUTF16String(unistr, true, false);

		//UTF16 LE ASCII
		dis.setEndianness(Endianness::little_endian);
		dos.setEndianness(Endianness::little_endian);
		strsz = static_cast<size_t>(dis.nextUnsignedShort());
		asz = (static_cast<int32_t>(strsz) + 1) >> 1;
		unistr = UnicodeString(asz, ' ', 0);
		read = dis.readUTF16String(unistr, strsz, false);
		cout << "String read: " << unistr << "\n";
		cout << "Bytes read: " << read << "\n";
		dos.putUTF16String(unistr, true, false);
		
		//UTF8 Uni
		strsz = static_cast<size_t>(dis.nextUnsignedShort());
		asz = static_cast<int32_t>(strsz) + 1;
		unistr = UnicodeString(asz, ' ', 0);
		read = dis.readUTF8String(unistr, strsz);
		cout << "String read: " << unistr << "\n";
		cout << "Bytes read: " << read << "\n";
		dos.putUTF8String(unistr, true, true);

		//UTF16 BOM Uni
		strsz = static_cast<size_t>(dis.nextUnsignedShort());
		asz = (static_cast<int32_t>(strsz) + 1) >> 1;
		unistr = UnicodeString(asz, ' ', 0);
		read = dis.readUTF16String(unistr, strsz, true);
		cout << "String read: " << unistr << "\n";
		cout << "Bytes read: " << read << "\n";
		dos.putUTF16String(unistr, true, true);

		//UTF16 BE Uni
		strsz = static_cast<size_t>(dis.nextUnsignedShort());
		dis.setEndianness(Endianness::big_endian);
		dos.setEndianness(Endianness::big_endian);
		asz = (static_cast<int32_t>(strsz) + 1) >> 1;
		unistr = UnicodeString(asz, ' ', 0);
		read = dis.readUTF16String(unistr, strsz, false);
		cout << "String read: " << unistr << "\n";
		cout << "Bytes read: " << read << "\n";
		dos.putUTF16String(unistr, true, false);

		//UTF16 LE Uni
		dis.setEndianness(Endianness::little_endian);
		dos.setEndianness(Endianness::little_endian);
		strsz = static_cast<size_t>(dis.nextUnsignedShort());
		asz = (static_cast<int32_t>(strsz) + 1) >> 1;
		unistr = UnicodeString(asz, ' ', 0);
		read = dis.readUTF16String(unistr, strsz, false);
		cout << "String read: " << unistr << "\n";
		cout << "Bytes read: " << read << "\n";
		dos.putUTF16String(unistr, true, false);

		dos.close();
		fos.close();

		dis.close();
		fis.close();
	}
	catch (InputException& e)
	{
		cout << "Exception caught (input)! :<\n";
		cout << e.what() << "\n";
	}
	catch (OutputException& e)
	{
		cout << "Exception caught (output)! :<\n";
		cout << e.what() << "\n";
	}

}

int main(void)
{
	//_setmode(_fileno(stdout), _O_U16TEXT); //https://stackoverflow.com/questions/2492077/output-unicode-strings-in-windows-console-app
	string testdir = "D:\\usr\\bghos\\code\\test";
	try{
		testUtilities();
		testFileStreamer(testdir);
	}
	catch(exception& e){cout << "Uncaught exception: \n" << e.what() << "\n"; return 1;}

	return 0;
}
