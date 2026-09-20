#pragma once
#include <chrono>
#include <ctime>
#include <vector>
#include <Windows.h>
#include <tlhelp32.h>
#include <fstream>
#include <winternl.h>
#define _NTDEF_
#include <ntsecapi.h>
#undef _NTDEF_
#include <cstdint>
#include <DbgHelp.h>
#include <thread>
#include <functional>
#include <map>
#include <atlbase.h>
#include <string>
#include <memory>
#include <random>
#include <ntstatus.h>
#include <dia2.h>
#include <diacreate.h>
#include <array>
#include <unordered_map>
#include <atomic>
#include <numbers>
#include <unordered_set>
#include <aclapi.h>
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "diaguids.lib")
#pragma comment(lib, "Urlmon.lib")
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "ntdll.lib")
namespace ch = std::chrono;

#include <impl/ia32/ia32.h>

#include <deps/oxorany/include.h>

#include <core/utility/utility.h>

#include <impl/pdb/pdb.h>
inline auto g_pdb = std::make_shared<pdb::c_pdb>( oxorany( "ntoskrnl.exe" ) );

#include <deps/service/bytes.h>
#include <deps/service/service.h>

#include <core/loader/memory/paging/paging.h>
inline auto g_paging = std::make_unique< paging::c_paging >( );

#include <core/loader/memory/memory.h>

#include <core/loader/memory/syscall/syscall.h>
inline auto g_syscall = std::make_unique< syscall::c_syscall >( );

#include <core/loader/process/process.h>
inline auto g_process = std::make_unique< process::c_process >( ::GetCurrentProcessId( ) );

#include <core/loader/memory/physical/physical.h>
inline auto g_physical = std::make_unique< physical::c_physical >( );

#include <deps/service/driver/driver.h>
inline auto g_driver = std::make_shared<driver::c_driver>( );

namespace memory {
    inline std::function<bool( std::uint64_t, void*, std::size_t )> m_read_physical =
        [ ]( std::uint64_t addr, void* buffer, std::size_t size ) -> bool {
        return g_driver->read_physical_memory( addr, buffer, size );
        };
    inline std::function<bool( std::uint64_t, void*, std::size_t )> m_write_physical =
        [ ]( std::uint64_t addr, void* buffer, std::size_t size ) -> bool {
        return g_driver->write_physical_memory( addr, buffer, size );
        };
}

#include <deps/service/cache/cache.h>
#include <deps/service/startup.h>
inline auto g_service = std::make_shared<service::c_service>( );

#include <core/loader/authority/authority.h>
inline auto g_authority = std::make_unique< authority::c_authority >( );

#include <core/utility/crash.h>