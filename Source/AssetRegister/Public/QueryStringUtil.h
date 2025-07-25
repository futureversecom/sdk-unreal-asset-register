#pragma once
#include "JsonObjectConverter.h"
#include "AssetRegisterLog.h"

namespace QueryStringUtil
{
	static FString QueryNameMetadata = TEXT("QueryName");

	inline FString ConvertJsonToGraphQLFriendlyString(const FString& Json)
	{
		FString TrimmedJson = Json;
            
		// remove the outermost brackets if possible
		int32 FirstBrace, LastBrace;
		if (Json.FindChar('{', FirstBrace) && Json.FindLastChar('}', LastBrace) && FirstBrace == 0 && LastBrace > FirstBrace)
		{
			TrimmedJson = Json.Mid(FirstBrace + 1, LastBrace - FirstBrace - 1).TrimStartAndEnd();
		}

		// Use regex to remove quotes around keys
		// Match: "key": → Replace with: key:
		const FRegexPattern KeyPattern(TEXT("\"([^\"]+)\":"));
		FRegexMatcher Matcher(KeyPattern, TrimmedJson);

		FString FinalJson;
		int32 LastPos = 0;

		while (Matcher.FindNext())
		{
			int32 MatchStart = Matcher.GetMatchBeginning();
			int32 MatchEnd = Matcher.GetMatchEnding();
			FString KeyName = Matcher.GetCaptureGroup(1);

			// Append everything before the match
			FinalJson += TrimmedJson.Mid(LastPos, MatchStart - LastPos);
			// Append modified key
			FinalJson += FString::Printf(TEXT("%s:"), *KeyName);

			LastPos = MatchEnd;
		}
        
		// Append the rest of the string
		FinalJson += TrimmedJson.Mid(LastPos);
        
		return FinalJson;
	}
	
	inline FString ToQueryName(const FString& OriginalName, const FString& PrefixChar, const bool bToCamelCase = true)
	{
		FString OutString = OriginalName;
		if (!PrefixChar.IsEmpty() && OriginalName.StartsWith(PrefixChar) && FChar::IsUpper(OriginalName[1]))
		{
			OutString = OriginalName.RightChop(1);		
		}
		
		// To camel Case
		if (bToCamelCase && FChar::IsUpper(OutString[0]))
		{
			OutString[0] = FChar::ToLower(OriginalName[0]);
		}
		
		return OutString;
	}
	
	template<typename TClass, typename TField, std::enable_if_t<std::is_base_of_v<UObject, TClass>, int> = 0>
	FString GetQueryName(TField TClass::* FieldPtr)
	{
		UClass* Class = std::remove_pointer_t<TClass>::StaticClass();
		const uint64 Offset = (uint64)&(reinterpret_cast<TClass*>(0)->*FieldPtr);

		for (TFieldIterator<FProperty> It(Class); It; ++It)
		{
			if (It->GetOffset_ForUFunction() == Offset)
			{
				return ToQueryName(It->GetName(), TEXT(""));
			}
		}
		return TEXT("<UnknownField>");
	}
	
	template<typename TStruct, typename TField, std::enable_if_t<TStruct::StaticStruct != nullptr, int> = 0>
	FString GetQueryName(TField TStruct::* FieldPtr)
	{
		const UStruct* Struct = TBaseStructure<TStruct>::Get();
		const uint64 Offset = (uint64)&(reinterpret_cast<TStruct*>(0)->*FieldPtr);

		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			if (It->GetOffset_ForUFunction() == Offset)
			{
				return ToQueryName(It->GetName(), TEXT(""));
			}
		}
		return TEXT("<UnknownField>");
	}

	template<typename TStruct, std::enable_if_t<TStruct::StaticStruct != nullptr, int> = 0>
	FString GetQueryName(const bool bToCamelCase = true)
	{
		const UStruct* Struct = TStruct::StaticStruct();
		
		FString StructName = Struct->GetFName().ToString();
		return ToQueryName(StructName, TEXT("F"), bToCamelCase);
	}
	
	// UObject class pointer version
	template<typename TClass, std::enable_if_t<std::is_base_of_v<UObject, TClass>, int> = 0>
	FString GetQueryName(const bool bToCamelCase = true)
	{
		UClass* Class = TClass::StaticClass();

		FString ClassName = Class->GetFName().ToString();
		return ToQueryName(ClassName, TEXT("U"), bToCamelCase);
	}

	inline void FindAllFieldsRecursively(const TSharedPtr<FJsonObject>& JsonObject, const FString& TargetField, TArray<TSharedPtr<FJsonValue>>& OutValues)
	{
		if (!JsonObject.IsValid()) return;

		for (const auto& Pair : JsonObject->Values)
		{
			if (Pair.Key == TargetField)
			{
				OutValues.Add(Pair.Value);
			}

			if (Pair.Value->Type == EJson::Object)
			{
				FindAllFieldsRecursively(Pair.Value->AsObject(), TargetField, OutValues);
			}
			else if (Pair.Value->Type == EJson::Array)
			{
				for (const auto& Element : Pair.Value->AsArray())
				{
					if (Element->Type == EJson::Object)
					{
						FindAllFieldsRecursively(Element->AsObject(), TargetField, OutValues);
					}
				}
			}
		}
	}

	inline TSharedPtr<FJsonValue> FindFieldRecursively(const TSharedPtr<FJsonObject>& JsonObject, const FString& TargetField)
	{
		if (!JsonObject.IsValid()) return nullptr;

		for (const auto& Pair : JsonObject->Values)
		{
			// Direct match
			if (Pair.Key == TargetField)
			{
				return Pair.Value;
			}

			// If value is an object, search inside it
			if (Pair.Value->Type == EJson::Object)
			{
				TSharedPtr<FJsonObject> SubObject = Pair.Value->AsObject();
				if (SubObject.IsValid())
				{
					TSharedPtr<FJsonValue> Found = FindFieldRecursively(SubObject, TargetField);
					if (Found.IsValid())
					{
						return Found;
					}
				}
			}

			// If value is an array, search inside any object elements
			else if (Pair.Value->Type == EJson::Array)
			{
				const TArray<TSharedPtr<FJsonValue>>& Array = Pair.Value->AsArray();
				for (const TSharedPtr<FJsonValue>& Element : Array)
				{
					if (Element.IsValid() && Element->Type == EJson::Object)
					{
						TSharedPtr<FJsonObject> ElementObj = Element->AsObject();
						TSharedPtr<FJsonValue> Found = FindFieldRecursively(ElementObj, TargetField);
						if (Found.IsValid())
						{
							return Found;
						}
					}
				}
			}
		}

		return nullptr;
	}

	template<typename TModel>
	bool TryGetModel(const FString& JsonString, TModel& OutStruct)
	{
		TSharedPtr<FJsonObject> RootObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

		const FString ModelName = GetQueryName<TModel>();
		if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
		{
			UE_LOG(LogAssetRegister, Error, TEXT("Failed to deserialize string: %s."), *JsonString);
			return false;
		}
		const TSharedPtr<FJsonValue> TargetField = FindFieldRecursively(RootObject, ModelName);
		if (!TargetField)
		{
			UE_LOG(LogAssetRegister, Error, TEXT("Failed to find field '%s' in Json string: %s"), *ModelName, *JsonString);
			return false;
		}
		
		return FJsonObjectConverter::JsonObjectToUStruct<TModel>(TargetField->AsObject().ToSharedRef(), &OutStruct);
	}

	template<typename TStruct>
	bool TryGetModelField(const FString& JsonString, const FString& TargetFieldName, TStruct& OutStruct)
	{
		TSharedPtr<FJsonObject> RootObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	
		if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
		{
			UE_LOG(LogAssetRegister, Error, TEXT("Failed to parse JSON string."));
			return false;
		}
		
		const TSharedPtr<FJsonValue> TargetField = FindFieldRecursively(RootObject, TargetFieldName);
		if (!TargetField)
		{
			UE_LOG(LogAssetRegister, Verbose, TEXT("Failed to find field '%s' in Json string: %s"), *TargetFieldName, *JsonString);
			return false;
		}

		return FJsonObjectConverter::JsonObjectToUStruct<TStruct>(TargetField->AsObject().ToSharedRef(), &OutStruct);
	}
};
