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
	
	template<typename TClass, typename TField, std::enable_if_t<std::is_base_of_v<UObject, TClass>, int> = 0>
	FString GetQueryName(TField TClass::* FieldPtr)
	{
		UClass* Class = std::remove_pointer_t<TClass>::StaticClass();
		const uint64 Offset = (uint64)&(reinterpret_cast<TClass*>(0)->*FieldPtr);

		for (TFieldIterator<FProperty> It(Class); It; ++It)
		{
			if (It->GetOffset_ForUFunction() == Offset)
			{
				if (It->HasMetaData(*QueryNameMetadata))
					return It->GetMetaData(*QueryNameMetadata);
				return It->GetName();
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
				if (It->HasMetaData(*QueryNameMetadata))
					return It->GetMetaData(*QueryNameMetadata);
				return It->GetName();
			}
		}
		return TEXT("<UnknownField>");
	}

	template<typename TStruct, std::enable_if_t<TStruct::StaticStruct != nullptr, int> = 0>
	FString GetQueryName()
	{
		const UStruct* Struct = TStruct::StaticStruct();
		if (Struct->HasMetaData(*QueryNameMetadata))
		{
			return Struct->GetMetaData(*QueryNameMetadata);
		}

		FString StructName = Struct->GetFName().ToString();
		if (StructName.StartsWith(TEXT("F")) && FChar::IsUpper(StructName[1]))
		{
			StructName = StructName.RightChop(1);
		}
		return StructName;
	}
	
	// UObject class pointer version
	template<typename TClass, std::enable_if_t<std::is_base_of_v<UObject, TClass>, int> = 0>
	FString GetQueryName()
	{
		UClass* Class = TClass::StaticClass();

		if (Class->HasMetaData(*QueryNameMetadata))
		{
			return Class->GetMetaData(*QueryNameMetadata);
		}

		FString ClassName = Class->GetFName().ToString();
		if (ClassName.StartsWith(TEXT("U")) && FChar::IsUpper(ClassName[1]))
		{
			ClassName = ClassName.RightChop(1);
		}
		return ClassName;
	}
	
	template<typename TModel>
	bool TryGetModel(const FString& JsonString, TModel& OutStruct)
	{
		TSharedPtr<FJsonObject> RootObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	
		if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
		{
			UE_LOG(LogAssetRegister, Error, TEXT("Failed to parse JSON string."));
			return false;
		}
	
		const TSharedPtr<FJsonObject>* DataObject;
		if (!RootObject->TryGetObjectField(TEXT("data"), DataObject))
		{
			UE_LOG(LogAssetRegister, Error, TEXT("Missing 'data' field in the response."));
			return false;
		}
		
		const TSharedPtr<FJsonObject>* TargetObject;
		if (!(*DataObject)->TryGetObjectField(GetQueryName<TModel>(), TargetObject))
		{
			UE_LOG(LogAssetRegister, Error, TEXT("Missing target field '%s' in 'data'."), *GetQueryName<TModel>());
			return false;
		}
	
		return FJsonObjectConverter::JsonObjectToUStruct<TModel>(TargetObject->ToSharedRef(), &OutStruct);
	}

	template<typename TModel, typename TStruct>
	bool TryGetModelField(const FString& JsonString, const FString& TargetFieldName, TStruct& OutStruct)
	{
		TSharedPtr<FJsonObject> RootObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	
		if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
		{
			UE_LOG(LogAssetRegister, Error, TEXT("Failed to parse JSON string."));
			return false;
		}
	
		const TSharedPtr<FJsonObject>* DataObject;
		if (!RootObject->TryGetObjectField(TEXT("data"), DataObject))
		{
			UE_LOG(LogAssetRegister, Error, TEXT("Missing 'data' field in the response."));
			return false;
		}
		
		const TSharedPtr<FJsonObject>* ModelObject;
		if (!(*DataObject)->TryGetObjectField(GetQueryName<TModel>(), ModelObject))
		{
			UE_LOG(LogAssetRegister, Error, TEXT("Missing model field '%s' in 'data'."), *GetQueryName<TModel>());
			return false;
		}
		
		const TSharedPtr<FJsonObject>* FieldObject;
		if (!(*ModelObject)->TryGetObjectField(TargetFieldName, FieldObject))
		{
			UE_LOG(LogAssetRegister, Error, TEXT("Missing target field '%s' in '%s'."), *TargetFieldName, *GetQueryName<TModel>());
			return false;
		}

		return FJsonObjectConverter::JsonObjectToUStruct<TStruct>(FieldObject->ToSharedRef(), &OutStruct);
	}
};
