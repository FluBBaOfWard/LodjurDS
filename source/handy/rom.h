
#ifndef ROM_H
#define ROM_H

#define ROM_SIZE				0x200

class CRom : public CLynxBase
{
	// Function members

	public:
		/** Supply the Lynx boot image, is 512 bytes long. */
		CRom(void);

	public:
		void	Reset(void);

		void	Poke(ULONG addr,UBYTE data) { };
		UBYTE	Peek(ULONG addr) { return(0);};
		ULONG	ObjectSize(void) {return ROM_SIZE;};

	// Data members

	private:
		UBYTE	mRomData[ROM_SIZE];
};

#endif
