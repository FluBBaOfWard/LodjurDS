//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                              Keith Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Generic error handler class                                              //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class provides error handler facilities for the Lynx emulator, I    //
// shamelessly lifted most of the code from Stella by Brad Mott.            //
//                                                                          //
// Keith Wilkins                                                            //
// August 1997                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 01Aug1997 KW Document header added & class documented.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef ERROR_H
#define ERROR_H

class CLynxException : public CException
{
	public:
		// Constructor
		CLynxException() {}
 
		// Copy Constructor
		CLynxException(CLynxException& err)
		{
			int MsgCount,DescCount;

			MsgCount = err.Message().pcount() + 1;
			DescCount = err.Description().pcount() + 1;

			LPTSTR msg = mMsg.GetBuffer(MsgCount);
			LPTSTR desc = mDesc.GetBuffer(DescCount);

			lstrcpyn(msg,err.Message().str(),MsgCount);
			lstrcpyn(desc,err.Description().str(),DescCount);
		}
 
		// Destructor
		virtual ~CLynxException()
		{
			mMsg.ReleaseBuffer();
			mMsgStream.rdbuf()->freeze(0);
			mDesc.ReleaseBuffer();
			mDescStream.rdbuf()->freeze(0);
		}

  public:
		// Answer stream which should contain the one line error message
		ostrstream& Message() { return mMsgStream; }

		// Answer stream which should contain the multiple line description
		ostrstream& Description() { return mDescStream; }

  public:
		// Overload the assignment operator
		CLynxException& operator=(CLynxException& err)
		{
			mMsgStream.seekp(0);

			mMsgStream.write(err.Message().str(), err.Message().pcount());
			err.Message().rdbuf()->freeze(0);

			mDescStream.seekp(0);

			mDescStream.write(err.Description().str(), err.Description().pcount());
			err.Description().rdbuf()->freeze(0);

			return *this;
		}

		// Overload the I/O output operator
		friend ostream& operator<<(ostream& out, CLynxException& err)
		{
			out.write(err.Message().str(), err.Message().pcount());
			err.Message().rdbuf()->freeze(0);

			if (err.Description().pcount() != 0)
			{
				out << endl << endl;

				out.write(err.Description().str(), err.Description().pcount());
				err.Description().rdbuf()->freeze(0);
			}

			return out;
		}

  private:
		// Contains the one line error code message
		ostrstream mMsgStream;

		// Contains a multiple line description of the error and ways to 
		// solve the problem
		ostrstream mDescStream;

  public:
		// CStrings to hold the data after its been thrown

		CString	mMsg;
		CString mDesc;
};
#endif
