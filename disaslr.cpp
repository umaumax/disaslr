#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <vector>

#if __linux__
#include <sys/personality.h>
#endif

bool IsExecutableFile(std::string filepath) {
  struct stat sb;
  return (stat(filepath.c_str(), &sb) == 0) && (sb.st_mode & S_IXOTH);
}
std::string GetCommandFullPath(std::string command) {
  std::string delimiter = ":";
  std::string path =
      std::string(getenv("PATH")) + ":" + std::string(getenv("PWD")) + ":";
  if (command.find('/', 0) != std::string::npos) {
    if (IsExecutableFile(command)) {
      return command;
    }
  } else {
    size_t start_pos = 0, end_pos = 0;
    while ((end_pos = path.find(':', start_pos)) != std::string::npos) {
      std::string target_directory =
          path.substr(start_pos, end_pos - start_pos);
      if (target_directory.empty()) {
        continue;
      }
      std::string target_path = target_directory + "/" + command;
      if (IsExecutableFile(target_path)) {
        return target_path;
      }
      start_pos = end_pos + 1;
    }
  }
  return "";
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "%s [filepath] [args]\n", argv[0]);
    return 1;
  }
  std::string command_name      = std::string(argv[1]);
  std::string command_full_path = GetCommandFullPath(command_name);
  if (command_full_path.empty()) {
    std::cerr << "cannot find executable command: " << command_name
              << std::endl;
    return 1;
  }

  // preparate char** args
  std::vector<char*> args_vec;
  args_vec.emplace_back(const_cast<char*>(command_full_path.c_str()));
  for (int i = 2; i < argc; i++) {
    args_vec.emplace_back(argv[i]);
  }
  args_vec.emplace_back(nullptr);
  char** args = args_vec.data();

  short ps_flags = 0;
  posix_spawn_file_actions_t actions;
  posix_spawnattr_t attrs;

  posix_spawn_file_actions_init(&actions);
  posix_spawnattr_init(&attrs);

  int ret;
#if __APPLE__
#ifndef _POSIX_SPAWN_DISABLE_ASLR
#define _POSIX_SPAWN_DISABLE_ASLR 0x0100
#endif
  ps_flags |= POSIX_SPAWN_SETEXEC;        // Mac OS X only
  ps_flags |= _POSIX_SPAWN_DISABLE_ASLR;  // Mac OS X only

  ret = posix_spawnattr_setflags(&attrs, ps_flags);
  if (ret != 0) {
    std::cerr << "failed posix_spawnattr_setflags: " << std::strerror(ret)
              << std::endl;
    return 1;
  }
#elif __linux__
  // get original setting
  int personality_orig = personality(0xffffffff);
  if (personality_orig == -1) {
    std::cerr << "failed posix_spawn: " << std::strerror(errno) << std::endl;
    return 1;
  }
  int no_aslr_personality = personality_orig | ADDR_NO_RANDOMIZE;
  ret                     = personality(no_aslr_personality);
  if (ret == -1) {
    std::cerr << "failed posix_spawn: " << std::strerror(errno) << std::endl;
    return 1;
  }
#endif

  pid_t pid;
  ret = posix_spawn(&pid, args[0], &actions, &attrs, args, nullptr);
  if (ret != 0) {
    std::cerr << "failed posix_spawn: " << std::strerror(ret) << std::endl;
  }
  ret = posix_spawnattr_destroy(&attrs);
  if (ret != 0) {
    std::cerr << "failed posix_spawnattr_destroy: " << std::strerror(ret)
              << std::endl;
  }

  ret = posix_spawn_file_actions_destroy(&actions);
  if (ret != 0) {
    std::cerr << "failed posix_spawn_file_actions_destroy: "
              << std::strerror(ret) << std::endl;
  }
  return 0;
}
