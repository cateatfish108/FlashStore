/*****************************************************************************
 * Copyright [2017] [taurus.ai]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

/**
 * Page Utility functions.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * for memory access, yjj-format file name, page number, etc.
 */

// #include <iostream>
#include "yijinjing/journal/PageUtil.h"

#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <experimental/filesystem>
#include <regex>
#include <sstream>

#include "yijinjing/journal/PageHeader.h"

USING_YJJ_NAMESPACE

string PageUtil::GenPageFileName(const string& jname, short pageNum) {
  string filename(JOURNAL_PREFIX);
  filename.push_back('.');
  filename += jname;
  filename.push_back('.');
  filename += std::to_string(pageNum);
  filename.push_back('.');
  filename += JOURNAL_SUFFIX;
  return filename;
  //    return JOURNAL_PREFIX + "." + jname + "." + std::to_string(pageNum) + "." + JOURNAL_SUFFIX;
}

string PageUtil::GenPageFullPath(const string& dir, const string& jname, short pageNum) {
  //    std::stringstream ss;
  //    ss << dir << "/" << GenPageFileName(jname, pageNum);
  //    return ss.str();

  string filename(dir);
  filename.push_back('/');
  filename += GenPageFileName(jname, pageNum);
  return filename;
}

string PageUtil::GetPageFileNamePattern(const string& jname) {
  return JOURNAL_PREFIX + "\\." + jname + "\\.[0-9]+\\." + JOURNAL_SUFFIX;
}

short PageUtil::ExtractPageNum(const string& filename, const string& jname) {
  string numStr = filename.substr(JOURNAL_PREFIX.length() + jname.length() + 2,
                                  filename.length() - JOURNAL_SUFFIX.length());
  return (short)atoi(numStr.c_str());
}

vector<short> PageUtil::GetPageNums(const string& dir, const string& jname) {
  const string namePattern = GetPageFileNamePattern(jname);
  std::experimental::filesystem::path p(dir);
  std::regex pattern(namePattern);
  vector<short> res;
  for (auto& file : std::experimental::filesystem::directory_iterator(p)) {
    const string filename = file.path().filename().string();
    if (std::regex_match(filename.begin(), filename.end(), pattern))
      res.push_back(PageUtil::ExtractPageNum(filename, jname));
  }
  std::sort(res.begin(), res.end());
  return res;
}

void PageUtil::RemoveJournal(const string& dir, const string& jname) {
  const string namePattern = GetPageFileNamePattern(jname);
  std::experimental::filesystem::path p(dir);
  std::regex pattern(namePattern);
  vector<string> res;
  for (auto& file : std::experimental::filesystem::directory_iterator(p)) {
    const string filename = file.path().filename().string();
    if (std::regex_match(filename.begin(), filename.end(), pattern))
      res.push_back(file.path().string());
  }
  for (auto& file : res) {
    ///        printf("removing journal: %s\n", file.c_str());
    std::experimental::filesystem::remove(file);
  }
}

short PageUtil::GetPageNumWithTime(const string& dir, const string& jname, int64_t time) {
  vector<short> pageNums = GetPageNums(dir, jname);
  for (int idx = pageNums.size() - 1; idx >= 0; idx--) {
    PageHeader header = GetPageHeader(dir, jname, pageNums[idx]);
    if (header.start_nano < time) return pageNums[idx];
  }
  return 1;
}

PageHeader PageUtil::GetPageHeader(const string& dir, const string& jname, short pageNum) {
  PageHeader header;
  string path = PageUtil::GenPageFullPath(dir, jname, pageNum);
  FILE* pfile = fopen(path.c_str(), "rb");
  if (pfile == nullptr) perror("cannot open file in PageUtil::GetPageNumWithTime");
  size_t length = fread(&header, 1, sizeof(PageHeader), pfile);
  if (length != sizeof(PageHeader))
    perror("cannot get page header in PageUtil::GetPageNumWithTime");
  fclose(pfile);
  return header;
}

/*
 * memory manipulation (no service)
 */

void* PageUtil::LoadPageBuffer(const string& path, int size, bool isWriting, bool quickMode) {
  std::experimental::filesystem::path page_path = path;
  std::experimental::filesystem::path page_folder_path = page_path.parent_path();
  if (!std::experimental::filesystem::exists(page_folder_path)) {
    std::experimental::filesystem::create_directories(page_folder_path);
  }

  int fd = open(path.c_str(), (isWriting) ? (O_RDWR | O_CREAT) : O_RDONLY, (mode_t)0600);
  if (fd < 0) {
    if (!isWriting) return nullptr;
    perror("Cannot create/write the file");
    exit(EXIT_FAILURE);
  }

  if (/*!quickMode &&*/ isWriting) {
    if (lseek(fd, size - 1, SEEK_SET) == -1) {
      close(fd);
      perror("Error calling lseek() to 'stretch' the file");
      exit(EXIT_FAILURE);
    }
    if (write(fd, "", 1) == -1) {
      close(fd);
      perror("Error writing last byte of the file");
      exit(EXIT_FAILURE);
    }
  }

  void* buffer =
      mmap(0, size, (isWriting) ? (PROT_READ | PROT_WRITE) : PROT_READ, MAP_SHARED, fd, 0);

  if (buffer == MAP_FAILED) {
    close(fd);
    perror("Error mapping file to buffer");
    exit(EXIT_FAILURE);
  }

  if (!quickMode && madvise(buffer, size, MADV_RANDOM) != 0 && mlock(buffer, size) != 0) {
    munmap(buffer, size);
    close(fd);
    perror("Error in madvise or mlock");
    exit(EXIT_FAILURE);
  }

  ///    std::cout<<"Path:"<<path <<"addr:"<<buffer<<" isWriting:"<<isWriting<<std::endl;
  close(fd);
  return buffer;
}

void PageUtil::ReleasePageBuffer(void* buffer, int size, bool quickMode) {
  // unlock and unmap
  if (!quickMode && munlock(buffer, size) != 0) {
    perror("ERROR in munlock");
    exit(EXIT_FAILURE);
  }

  if (munmap(buffer, size) != 0) {
    perror("ERROR in munmap");
    exit(EXIT_FAILURE);
  }
}

bool PageUtil::FileExists(const string& filename) {
  int fd = open(filename.c_str(), O_RDONLY, (mode_t)0600);
  if (fd >= 0) {
    close(fd);
    return true;
  }
  return false;
}
