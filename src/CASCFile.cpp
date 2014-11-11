/*
 * CASCFile.cpp
 *
 *  Created on: 23 oct. 2014
 *      Author: Jerome
 */

#include "CASCFile.h"
#include "CASCFolder.h"
#include "globalvars.h"
#include "modelviewer.h"
#include <iostream>

#include "logger/Logger.h"
//#define DEBUG_READ

CASCFile::CASCFile()
: GameFile(),m_handle(NULL)
{
#ifdef DEBUG_READ
  std::cout << __FUNCTION__ << " 2" << std::endl;
#endif
}

CASCFile::CASCFile(const std::string & path)
 : GameFile(), m_filePath(path)
{
#ifdef DEBUG_READ
  std::cout << __FUNCTION__ << " 1" << std::endl;
#endif
  openFile(path);
}


bool CASCFile::open()
{
  bool result = false;
  if(CASCFOLDER.hStorage)
  {
    result = CascOpenFile(CASCFOLDER.hStorage, m_filePath.c_str(), CASCFOLDER.CASCLocale(), 0, &m_handle);
    if(!result)
      LOG_ERROR << "Opening" << m_filePath.c_str() << "failed." << "Error" << GetLastError();
  }
  else
    m_handle = NULL;
#ifdef DEBUG_READ
  std::cout << __FUNCTION__ << " result => " << std::boolalpha << result << std::endl;
#endif
  return result;
}

bool CASCFile::close()
{
  return CascCloseFile(m_handle);
}

void CASCFile::openFile(std::string filename)
{
#ifdef DEBUG_READ
  std::cout << __FUNCTION__ << " opening => " << filename << std::endl;
#endif
  m_filePath = filename;
  if(open())
  {
    DWORD nbBytesRead = 0;
    size = CascGetFileSize(m_handle,0);
    buffer = new unsigned char[size];
    CascReadFile(m_handle, buffer, size, &nbBytesRead);
    if(nbBytesRead != 0)
      eof = false;
#ifdef DEBUG_READ
    std::cout << __FUNCTION__ <<  " | " << m_filePath << " nb bytes read => " << nbBytesRead << " / " << size << std::endl;
#endif
  }
}
