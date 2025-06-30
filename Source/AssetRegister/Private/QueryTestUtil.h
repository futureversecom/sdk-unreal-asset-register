#pragma once

namespace QueryTestUtil
{
	inline FString RemoveAllWhitespace(const FString& Input)
	{
		FString Result;
	
		for (TCHAR Char : Input)
		{
			if (!FChar::IsWhitespace(Char))
			{
				Result.AppendChar(Char);
			}
		}

		return Result;
	}
}
