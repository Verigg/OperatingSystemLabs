#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <string>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
HANDLE log_mutex;
HANDLE counter_mutex;
#else
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <semaphore.h>
sem_t* log_semaphore;
sem_t* counter_semaphore;
#endif

// Shared variables in shared memory
std::atomic<int>* shared_counter = nullptr;
std::atomic<bool>* is_leader = nullptr;

// Local atomic flags
std::atomic<bool> is_leader_instance(false);
std::atomic<bool> copy1_running(false);
std::atomic<bool> copy2_running(false);
std::atomic<bool> stop_flag(false);

// Thread management
std::vector<std::thread> threads;
std::ofstream log_file;

/**
 * Sets up shared memory for the counter and leader flag.
 */
void setup_shared_memory() {
#ifdef _WIN32
    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "SharedCounter");
    if (hMapFile == NULL) { // leader instance, creating variables
        hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(std::atomic<int>) + sizeof(std::atomic<bool>), "SharedCounter");
        if (hMapFile == NULL) {
            std::cerr << "Could not create file mapping object: " << GetLastError() << std::endl;
            exit(1);
        }
        void* base_address = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(std::atomic<int>) + sizeof(std::atomic<bool>));
        shared_counter = new (base_address) std::atomic<int>(0);
        is_leader = new (reinterpret_cast<void*>(reinterpret_cast<char*>(base_address) + sizeof(std::atomic<int>))) std::atomic<bool>(true);
        if (!shared_counter || !is_leader) {
            std::cerr << "Could not map view of file: " << GetLastError() << std::endl;
            CloseHandle(hMapFile);
            exit(1);
        }
        is_leader_instance = true;
    } else { // additional instance
        void* base_address = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(std::atomic<int>) + sizeof(std::atomic<bool>));
        shared_counter = reinterpret_cast<std::atomic<int>*>(base_address);
        is_leader = reinterpret_cast<std::atomic<bool>*>(reinterpret_cast<char*>(base_address) + sizeof(std::atomic<int>));
        if (!shared_counter || !is_leader) {
            std::cerr << "Could not map view of file: " << GetLastError() << std::endl;
            CloseHandle(hMapFile);
            exit(1);
        }
    }
#else
    const char* shared_memory_name = "/SharedCounter";
    const size_t memory_size = sizeof(std::atomic<int>) + sizeof(std::atomic<bool>);
    int fd = shm_open(shared_memory_name, O_RDWR, 0666);

    if (fd == -1) { // leader instance, creating variables
        fd = shm_open(shared_memory_name, O_CREAT | O_RDWR, 0666);
        if (fd == -1) {
            perror("Could not create shared memory");
            exit(1);
        }
        if (ftruncate(fd, memory_size) == -1) {
            perror("Could not set size for shared memory");
            shm_unlink(shared_memory_name);
            exit(1);
        }
        void* addr = mmap(nullptr, memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            perror("Could not map shared memory");
            shm_unlink(shared_memory_name);
            exit(1);
        }
        shared_counter = new (addr) std::atomic<int>(0);
        is_leader = new (reinterpret_cast<void*>(reinterpret_cast<char*>(addr) + sizeof(std::atomic<int>))) std::atomic<bool>(true);
        is_leader_instance = true;
    } else { // additional instance
        void* addr = mmap(nullptr, memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            perror("Could not map shared memory");
            exit(1);
        }
        shared_counter = reinterpret_cast<std::atomic<int>*>(addr);
        is_leader = reinterpret_cast<std::atomic<bool>*>(reinterpret_cast<char*>(addr) + sizeof(std::atomic<int>));
    }

    close(fd);
#endif
}

/**
 * Sets up synchronization mechanisms for logging.
 */
void setup_log_synchronization() {
#ifdef _WIN32
    log_mutex = CreateMutexA(NULL, FALSE, "GlobalLogMutex");
    if (!log_mutex) {
        std::cerr << "Failed to create log mutex." << std::endl;
        exit(1);
    }
    counter_mutex = CreateMutexA(NULL, FALSE, "GlobalCounterMutex");
    if (!counter_mutex) {
        std::cerr << "Failed to create counter mutex." << std::endl;
        exit(1);
    }
#else
    log_semaphore = sem_open("/log_semaphore", O_CREAT, 0644, 1);
    if (log_semaphore == SEM_FAILED) {
        std::cerr << "Failed to create log semaphore." << std::endl;
        exit(1);
    }
    counter_semaphore = sem_open("/counter_semaphore", O_CREAT, 0644, 1);
    if (counter_semaphore == SEM_FAILED) {
        std::cerr << "Failed to create counter semaphore." << std::endl;
        exit(1);
    }
#endif
}

/**
 * Cleans up shared memory resources.
 */
void cleanup_shared_memory() {
#ifdef _WIN32
    if (shared_counter || is_leader) {
        UnmapViewOfFile(shared_counter);
        UnmapViewOfFile(is_leader);
    }
#else
    if (shared_counter || is_leader) {
        munmap(shared_counter, sizeof(std::atomic<int>) + sizeof(std::atomic<bool>));
    }
    shm_unlink("/SharedCounter");
#endif
}

/**
 * Cleans up synchronization resources for logging.
 */
void cleanup_log_synchronization() {
#ifdef _WIN32
    CloseHandle(log_mutex);
    CloseHandle(counter_mutex);
#else
    sem_close(log_semaphore);
    sem_unlink("/log_semaphore");
    sem_close(counter_semaphore);
    sem_unlink("/counter_semaphore");
#endif
}

/**
 * Function to terminate threads
 */
void terminate_threads() {
    std::cout << "Terminate threads...\n";
    stop_flag = true;

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::cout << "All threads are terminated.\n";
}

/**
 * Utility function to get current time
 */
std::string get_current_time() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    char buffer[30];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);

    return std::string(buffer) + "." + std::to_string(ms.count());
}

/**
 * Logs a message with a timestamp.
 */
void log_message(const std::string& message) {
#ifdef _WIN32
    WaitForSingleObject(log_mutex, INFINITE);
#else
    sem_wait(log_semaphore);
#endif

    log_file << get_current_time() << " - " << message << std::endl;
    log_file.flush();

#ifdef _WIN32
    ReleaseMutex(log_mutex);
#else
    sem_post(log_semaphore);
#endif
}


void safe_set_counter(int value) {
#ifdef _WIN32
    WaitForSingleObject(counter_mutex, INFINITE);
#else
    sem_wait(counter_semaphore);
#endif

    (*shared_counter) = value;  

#ifdef _WIN32
    ReleaseMutex(counter_mutex);
#else
    sem_post(counter_semaphore);
#endif
}

/**
 * Function to handle program exit
 */
void on_exit() {

    if (is_leader_instance) {
        std::cout << "Releasing leader flag...\n";
        is_leader->store(false, std::memory_order_release);
        auto start_time = std::chrono::steady_clock::now();
        while (!is_leader->load(std::memory_order_acquire)) {
            if (std::chrono::steady_clock::now() - start_time > std::chrono::milliseconds(100)) {
                break;
            }
            std::this_thread::yield();
        }

        if (!is_leader->load(std::memory_order_acquire)) {
            cleanup_log_synchronization();
            cleanup_shared_memory();
        }
    }

    terminate_threads();

    exit(0);
}

/**
 * Function to handle spawning childs
 */
void spawn_child_process(int id) {
#ifdef _WIN32
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);
    std::string command = "Timer.exe " + std::to_string(id); 
    if (!CreateProcessA(NULL, const_cast<char*>(command.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        log_message("Failed to create child process.");
        return;
    }

    if (id == 1) {
        copy1_running = true;
    } else if (id == 2) {
        copy2_running = true;
    }

    std::thread child([pi, id]() {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (id == 1) {
            copy1_running = false;
        } else if (id == 2) {
            copy2_running = false;
        }
    });
    threads.push_back(std::move(child));
#else
    std::string command = "./Timer " + std::to_string(id);
    pid_t pid = fork();
    if (pid < 0) {
        log_message("Failed to fork child process.");
        return;
    }

    if (pid == 0) {
        // Child process
        execlp("sh", "sh", "-c", command.c_str(), (char*)NULL);
        log_message("Failed to execute child process.");
        std::exit(1);
    } else {
        // Parent process
        if (id == 1) {
            copy1_running = true;
        } else if (id == 2) {
            copy2_running = true;
        }

        std::thread child([pid, id]() {
            int status;
            waitpid(pid, &status, 0);

            if (id == 1) {
                copy1_running = false;
            } else if (id == 2) {
                copy2_running = false;
            }
        });
        threads.push_back(std::move(child));
    }
#endif
}

/**
 * Thread for logging
 */
void log_counter_thread() {
    while (!stop_flag) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        log_message("Main instance log: Counter value: " + std::to_string((*shared_counter)));
    }
}

/**
 * Thread for counter increment
 */
void counter_increment_thread() {
    while (!stop_flag) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        safe_set_counter((*shared_counter) + 1);
    }
}

/**
 * Thread for user input
 */
void user_input_thread() {
    while (!stop_flag) {
        std::string input;
        std::cout << "Enter command (set <value>, get, stop): " << std::endl;
        std::getline(std::cin, input);

        if (!std::getline(std::cin, input)) {
            std::cout << "Input stream closed. Stopping thread." << std::endl;
            break;
        }

        if (input.rfind("set", 0) == 0) { 
            try {
                int value = std::stoi(input.substr(4)); 
                safe_set_counter(value);
                log_message("Counter set to: " + std::to_string(value));
                std::cout << "Counter set to: " << value << std::endl;
            } catch (...) {
                std::cout << "Invalid command. Use: set <value>" << std::endl;
            }
        } else if (input == "get") {
            int current_value = *shared_counter;
            std::cout << "Current counter value: " << current_value << std::endl;
        } else {
            std::cout << "Unknown command. Available commands: set <value>, get, stop." << std::endl;
        }
    }
}

void child_instance_behavior(int id, pid_t child_pid){
    std::string start_message = "Child process " + std::to_string(id) + " started. PID: " + std::to_string(child_pid);
    log_message(start_message);
    
    if (id == 1) {
        safe_set_counter((*shared_counter) + 10);
        log_message("Child 1 incremented counter by 10. Exiting.");
    } else if (id == 2) {
        safe_set_counter((*shared_counter) * 2);
        log_message("Child 2 doubled counter. Sleeping for 2 seconds.");
        std::this_thread::sleep_for(std::chrono::seconds(2));
        safe_set_counter((*shared_counter) / 2);
        log_message("Child 2 halved counter. Exiting.");
    }

    std::string end_message = "Child process " + std::to_string(id) + " exiting. PID: " + std::to_string(child_pid);
    log_message(end_message);
}

void additional_instance_behavior(){
    std::cout << "This is additional instance, affects only counter" << std::endl;
    while (!is_leader_instance) {
        if (!is_leader->load(std::memory_order_acquire)) {
            is_leader->store(true, std::memory_order_release);
            is_leader_instance = true;
        }
    }
    std::cout << "Leader instance was closed" << std::endl;
}

void leader_instance_behavior(){
    std::cout << "This instance is leader" << std::endl;

    std::thread log_counter(log_counter_thread);
    threads.push_back(std::move(log_counter));

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(3));

        if (!copy1_running && !copy2_running) {
            log_message("Spawning child processes.");

            spawn_child_process(1);
            spawn_child_process(2);
        } else {
            log_message("Child processes still running. Skipping spawn.");
        }

    }
}

void parent_instance_behavior() {

    std::thread user_input(user_input_thread);
    threads.push_back(std::move(user_input));

    std::thread counter_increment(counter_increment_thread);
    threads.push_back(std::move(counter_increment));

    while(true) {
        if (is_leader_instance) {
            leader_instance_behavior();
        } else {
            additional_instance_behavior();
        }
    }
}

#ifdef _WIN32
    BOOL WINAPI ConsoleEventHandler(DWORD event) {
        if (event == CTRL_CLOSE_EVENT || event == CTRL_C_EVENT){
            on_exit();
            return TRUE;
        }
        return FALSE;
    }
#else
    void signalHandler(int signal) {
        if (signal == SIGHUP || signal == SIGINT || signal == SIGTERM) {
            on_exit();
        }
        std::exit(signal);
    }
#endif

int main(int argc, char* argv[]) {
    setup_shared_memory();
    setup_log_synchronization();

    auto pid =
#ifdef _WIN32
        GetCurrentProcessId();
#else
        getpid();
#endif

    std::filesystem::create_directories("../logs");
    log_file.open("../logs/timer.log", std::ios::app);
    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file." << std::endl;
        cleanup_log_synchronization();
        return 1;
    }

    if (argc > 1) {
        int id = std::stoi(argv[1]);
        child_instance_behavior(id, pid);
        return 0;
    }

#ifdef _WIN32
    SetConsoleCtrlHandler(ConsoleEventHandler, TRUE);
#else
    std::signal(SIGINT, signalHandler);
    std::signal(SIGHUP, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::atexit(on_exit);
#endif

    log_message("Program started. PID: " + std::to_string(pid));

    parent_instance_behavior();

    return 0;
}