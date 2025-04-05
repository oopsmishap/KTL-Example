#include <ktl/chrono.hpp>
#include <ktl/string.hpp>
#include <ktl/fmt.hpp>

#include <ntddk.h>

#include <log.hpp>

EXTERN_C NTSTATUS driver_entry(
    [[maybe_unused]] DRIVER_OBJECT* driver_object, [[maybe_unused]] UNICODE_STRING* registry_path
) noexcept
try
{
    log::info("Hello, world!");
    log::debug("Driver object:  {:#x}", (uintptr_t)driver_object);
    log::debug(L"Registry path: {}", registry_path->Buffer);

    return STATUS_SUCCESS;
}
catch (const ktl::exception& exc)
{
    DbgPrint("Unhandled exception caught: %s with code %x\n", exc.what(), exc.code());
    return exc.code();
}