
#ifndef ISTEAMREMOTESTORAGE002_H
#define ISTEAMREMOTESTORAGE002_H
#ifdef STEAM_WIN32
#pragma once
#endif


//-----------------------------------------------------------------------------
// Purpose: Functions for accessing, reading and writing files stored remotely 
//			and cached locally
//-----------------------------------------------------------------------------
class ISteamRemoteStorage002
{
public:
	// NOTE
	//
	// Filenames are case-insensitive, and will be converted to lowercase automatically.
	// So "foo.bar" and "Foo.bar" are the same file, and if you write "Foo.bar" then
	// iterate the files, the filename returned will be "foo.bar".
	//

	// file operations
	virtual bool	FileWrite( const char *pchFile, const void *pvData, int32 cubData ) = 0;
	virtual int32	GetFileSize( const char *pchFile ) = 0;
	virtual int32	FileRead( const char *pchFile, void *pvData, int32 cubDataToRead ) = 0;
	virtual bool	FileExists( const char *pchFile ) = 0;

	// iteration
	virtual int32 GetFileCount() = 0;
	virtual const char *GetFileNameAndSize( int iFile, int32 *pnFileSizeInBytes ) = 0;

	// quota management
	virtual bool GetQuota( int32 *pnTotalBytes, int32 *puAvailableBytes ) = 0;
};

#endif // ISTEAMREMOTESTORAGE_H
