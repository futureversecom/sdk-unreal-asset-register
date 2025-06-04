#include "AssetRegisterLog.h"

DEFINE_LOG_CATEGORY(LogAssetRegister);

namespace AssetRegisterLogging
{
	static TAutoConsoleVariable<bool> CVarAssetRegisterDebugLogging(
TEXT("AssetRegister.Logging.DebugLogging"),
false,
TEXT("Enable verbose debug logging for AssetRegister SDK."));

	bool DebugLoggingEnabled()
	{
		return CVarAssetRegisterDebugLogging.GetValueOnAnyThread();
	}
}
