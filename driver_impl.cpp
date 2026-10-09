#include "../../include/driver/idriver.hpp"
#include "../../include/utils/logger.hpp"
#include "../../include/driver/driver_data.h"
#include "../../include/driver/wdk.h"
#include <ntstatus.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string.h>
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <sstream>
#include <format>

namespace vex::driver {

    /**
     * @brief Concrete driver implementation
     */
    class DriverImpl : public IDriver {
    public:
        DriverImpl() : m_process_id(0), m_base_address(0), m_dtb(0), m_driver_handle(INVALID_HANDLE_VALUE) {}

        ~DriverImpl() override {
            unload();
        }

        // Setup and management
        bool setup() override {
            LOG_INFO("Setting up driver...");

            // Stop and delete existing service
            system("sc stop gdrv_svc");
            system("sc delete gdrv_svc");

            // Copy driver to System32
            std::ofstream driver("C:\\Windows\\System32\\drivers\\gdrv_svc.sys", std::ios::binary);
            if (!driver.is_open()) {
                LOG_ERROR("Failed to create driver file");
                return false;
            }

            driver.write(reinterpret_cast<const char*>(driver_data), sizeof(driver_data));
            driver.close();

            // Create and start service
            system("sc create gdrv_svc type= kernel start= demand binPath= C:\\Windows\\System32\\drivers\\gdrv_svc.sys");
            system("sc start gdrv_svc");

            // Get computer name
            WCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
            DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
            GetComputerNameW(computerName, &size);

            // Create device path
            WCHAR devicePath[MAX_PATH] = { 0 };
            swprintf_s(devicePath, L"\\\\.\\%s", computerName);

            // Open driver handle
            m_driver_handle = CreateFileW(devicePath, GENERIC_READ | GENERIC_WRITE,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                        OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);

            if (m_driver_handle == INVALID_HANDLE_VALUE) {
                LOG_ERROR("Failed to open driver handle");
                return false;
            }

            return true;
        }

        void unload() override {
            if (m_driver_handle != INVALID_HANDLE_VALUE) {
                CloseHandle(m_driver_handle);
                m_driver_handle = INVALID_HANDLE_VALUE;
            }
        }

        bool is_valid() const override {
            return m_driver_handle != INVALID_HANDLE_VALUE;
        }

        // Process management
        bool attach_process(const std::wstring& process_name) override {
            m_process_id = get_process_id_by_name(process_name);
            if (!m_process_id) {
                LOG_ERROR("Failed to find process: " + std::string(process_name.begin(), process_name.end()));
                return false;
            }

            m_base_address = get_base_address();
            if (!m_base_address) {
                LOG_ERROR("Failed to get base address");
                return false;
            }

            m_dtb = get_dtb(m_base_address);
            if (!m_dtb) {
                LOG_ERROR("Failed to get DTB");
                return false;
            }

            return true;
        }

        bool attach_process(uint32_t process_id) override {
            m_process_id = process_id;
            if (!m_process_id) {
                LOG_ERROR("Invalid process ID");
                return false;
            }

            m_base_address = get_base_address();
            if (!m_base_address) {
                LOG_ERROR("Failed to get base address");
                return false;
            }

            m_dtb = get_dtb(m_base_address);
            if(!m_dtb) {
                LOG_ERROR("Failed to get DTB");
                return false;
			}

            return true;
        }

        uint32_t get_process_id() const override { return m_process_id; }
        uintptr_t get_base_address() const override { return m_base_address; }
        uintptr_t get_dtb() const override { return m_dtb; }

        bool move_mouse(uint32_t x, uint32_t y, uint16_t button_flags) {
            s_command_data payload{};
            payload.params.mouse.x = x;
            payload.params.mouse.y = y;
            payload.params.mouse.button_flags = button_flags;

            auto result = send_command(e_command_type::mouse_move, payload);
            if (result.get_status() > 0) {
                return false;
            }
            return true;
        }

        // Memory operations
        bool read_memory(uintptr_t target, void* dest, size_t size) override {
            s_command_data payload{};
            payload.params.rw.process_id = this->m_process_id;
            payload.params.rw.size = size;
            payload.params.rw.src = (void*)target;
            payload.params.rw.dst = dest;
            payload.params.rw.write = 0;
            payload.params.rw.dirbase = (void*)this->m_dirbase;
            auto result = send_command(e_command_type::rw_phys_memory, payload);
            if (result.get_status() > 0) {
                return false;
            }
            return true;
        }

        bool write_memory(void* dest, void* src, std::size_t size) override {
            s_command_data payload{};
            payload.params.rw.process_id = this->m_process_id;
            payload.params.rw.size = size;
            payload.params.rw.src = (void*)dest;
            payload.params.rw.dst = (void*)src;
            payload.params.rw.write = 1;
            payload.params.rw.dirbase = (void*)this->m_dirbase;
            auto result = send_command(e_command_type::rw_phys_memory, payload);
            if (result.get_status() > 0) {
                return false;
            }
            return true;
		}

        bool stream_mode(HWND hwnd, uint32_t flag) {
            s_command_data payload{};
            payload.params.stream_mode.hwnd = reinterpret_cast<uint64_t>(hwnd);
            payload.params.stream_mode.flag = flag;
            auto result = send_command(e_command_type::stream_mode, payload);
            if (result.get_status() > 0) {
                return false;
			}
			return true;
        }

        // System information
        uintptr_t get_kernel_base(const std::string& module_name) override {
            s_command_data payload{};
			strncpy_s(payload.params.kernel_base.module_name, module_name.c_str(), sizeof(payload.params.kernel_base.module_name) - 1);

            auto result = send_command(e_command_type::get_kernel_base, payload);
            if (!NT_SUCCESS(result.get_status()))
            {
                return 0;
            }

            return (uintptr_t)result.get_data().params.kernel_base.address;
        }

        // Utilities
        void* find_pattern(uintptr_t base, const char* pattern, const char* mask) override {
            if (!is_valid() || !pattern || !mask) return nullptr;

            const size_t CHUNK_SIZE = 4096;
            const size_t MAX_SCAN_SIZE = 0x10000000; // 256MB max limit
            const size_t pattern_length = strlen(mask);

            if (pattern_length == 0) return nullptr;

            std::vector<uint8_t> chunk_data(CHUNK_SIZE);
            std::vector<uint8_t> pattern_bytes;
            std::vector<bool> pattern_mask;

            // Convert pattern string to bytes
            if (!parse_pattern(pattern, mask, pattern_bytes, pattern_mask)) {
                return nullptr;
            }

            for (size_t offset = 0; offset < MAX_SCAN_SIZE; offset += CHUNK_SIZE) {
                uintptr_t current_address = base + offset;

                // Read current chunk
                if (!read_memory(current_address, chunk_data.data(), CHUNK_SIZE)) {
                    // If read failed, may have reached end of valid region
                    break;
                }

                // Search pattern in current chunk
                for (size_t i = 0; i <= CHUNK_SIZE - pattern_length; i++) {
                    if (match_pattern(chunk_data.data() + i, pattern_bytes, pattern_mask)) {
                        return reinterpret_cast<void*>(current_address + i);
                    }
                }

                // If not last possible chunk, read a bit more to avoid missing patterns
                // that are at chunk boundaries
                if (offset + CHUNK_SIZE < MAX_SCAN_SIZE && pattern_length > 1) {
                    // Read next partial chunk to check boundary
                    std::vector<uint8_t> overlap_data(pattern_length - 1);
                    if (read_memory(current_address + CHUNK_SIZE, overlap_data.data(), pattern_length - 1)) {
                        // Check pattern at boundary
                        for (size_t i = CHUNK_SIZE - pattern_length + 1; i < CHUNK_SIZE; i++) {
                            std::vector<uint8_t> border_data;
                            border_data.insert(border_data.end(),
                                             chunk_data.begin() + i, chunk_data.end());
                            border_data.insert(border_data.end(),
                                             overlap_data.begin(),
                                             overlap_data.begin() + (pattern_length - (CHUNK_SIZE - i)));

                            if (match_pattern(border_data.data(), pattern_bytes, pattern_mask)) {
                                return reinterpret_cast<void*>(current_address + i);
                            }
                        }
                    }
                }
            }

            return nullptr;
        }

        void set_dir_base(void* dir) override {
            this->m_dirbase = dir;
        }

    private:
        uint32_t    m_process_id;
        uintptr_t   m_base_address;
        uintptr_t   m_dtb;
        void*       m_dirbase = 0;
        HANDLE      m_driver_handle;

        uint32_t get_process_id_by_name(const std::wstring& process_name) {
            auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (snapshot == INVALID_HANDLE_VALUE) return 0;

            PROCESSENTRY32W process_entry{};
            process_entry.dwSize = sizeof(process_entry);

            if (!Process32FirstW(snapshot, &process_entry)) {
                CloseHandle(snapshot);
                return 0;
            }

            do {
                if (process_name == process_entry.szExeFile) {
                    CloseHandle(snapshot);
                    return process_entry.th32ProcessID;
                }
            } while (Process32NextW(snapshot, &process_entry));

            CloseHandle(snapshot);
            return 0;
        }

        uintptr_t get_base_address() {
            s_command_data payload{};
            payload.params.base_address.process_id = this->m_process_id;
            auto result = send_command(e_command_type::get_base_address, payload);
            if (!NT_SUCCESS(result.get_status())) {
                return 0;
            }
            return result.get_data().params.base_address.base_address;
        }

        uintptr_t get_dtb(uintptr_t base_address) {
            s_command_data payload{};
            payload.params.dtb.address = base_address;
            auto result = send_command(e_command_type::get_dtb, payload);
            if (!NT_SUCCESS(result.get_status())) {
                return 0;
            }
            return result.get_data().params.dtb.address2;
        }

        // Helper to convert pattern string to bytes
        bool parse_pattern(const char* pattern, const char* mask,
                          std::vector<uint8_t>& pattern_bytes, std::vector<bool>& pattern_mask) {
            const size_t length = strlen(mask);
            pattern_bytes.reserve(length);
            pattern_mask.reserve(length);

            std::istringstream pattern_stream(pattern);
            std::string byte_str;
            size_t mask_index = 0;

            while (pattern_stream >> byte_str && mask_index < length) {
                if (mask[mask_index] == '?') {
                    // Wildcard - any byte
                    pattern_bytes.push_back(0x00);
                    pattern_mask.push_back(false);
                } else {
                    // Specific byte
                    try {
                        uint8_t byte_val = static_cast<uint8_t>(std::stoi(byte_str, nullptr, 16));
                        pattern_bytes.push_back(byte_val);
                        pattern_mask.push_back(true);
                    } catch (const std::exception&) {
                        return false; // Parsing error
                    }
                }
                mask_index++;
            }

            return mask_index == length;
        }

        // Helper to check if bytes match pattern
        bool match_pattern(const uint8_t* data, const std::vector<uint8_t>& pattern_bytes,
                          const std::vector<bool>& pattern_mask) {
            for (size_t i = 0; i < pattern_bytes.size(); i++) {
                if (pattern_mask[i] && data[i] != pattern_bytes[i]) {
                    return false;
                }
            }
            return true;
        }

        template<typename Payload>
        c_command<Payload> send_command(e_command_type type, const Payload& data, std::uint32_t timeout_ms = 2000) {
            using command_t = c_command<Payload>;

            if (this->m_driver_handle == INVALID_HANDLE_VALUE) {
                command_t err{ type, data };
                err.set_status(STATUS_UNSUCCESSFUL);
                return err;
            }

            command_t cmd{ type, data };


            IO_STATUS_BLOCK block;
            auto result =
                direct_device_control(
                    this->m_driver_handle,
                    nullptr,
                    nullptr,
                    nullptr,
                    &block,
                    0,
                    &cmd,
                    static_cast<std::size_t>(sizeof(cmd)),
                    &cmd,
                    static_cast<std::size_t>(sizeof(cmd)));

            if (!NT_SUCCESS(result) || !NT_SUCCESS(block.Status)) {
                cmd.set_status(STATUS_UNSUCCESSFUL);
                return cmd;
            }

            if (block.Information != sizeof(command_t)) {
                DWORD error = GetLastError();
                cmd.set_status(STATUS_UNSUCCESSFUL);
                return cmd;
            }

            return cmd;
        }
    };

    // Factory to create driver instances
    std::shared_ptr<IDriver> create_driver() {
        return std::make_shared<DriverImpl>();
    }

} // namespace vex::driver