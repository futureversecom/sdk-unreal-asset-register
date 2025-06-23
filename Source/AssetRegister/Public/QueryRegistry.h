#pragma once
#include "AssetRegisterLog.h"
#include "JsonObjectConverter.h"

template<typename TModel>
class FQueryRegistry
{
public:
    FQueryRegistry() {}
  
    FString GetModelString() const
    {
        const UStruct* Struct = TModel::StaticStruct();
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
    
    template<typename T>
    void RegisterArgument(const T& Argument)
    {
        const UScriptStruct* StructDef = TBaseStructure<T>::Get();
        T DefaultStruct; // a default instance to compare against
        
        for (TFieldIterator<FProperty> It(StructDef); It; ++It)
        {
            const FProperty* Property = *It;
            
            // Compare current vs default values
            if (!Property->Identical(Property->ContainerPtrToValuePtr<void>(&Argument),
                                     Property->ContainerPtrToValuePtr<void>(&DefaultStruct)))
            {
                if (Property->HasMetaData(*QueryNameMetadata))
                {
                    FieldString += FString::Printf(TEXT("  %s\n"), *Property->GetMetaData(*QueryNameMetadata));
                }
            }
        }
        
        const UScriptStruct* ArgumentDefinition = TBaseStructure<T>::Get();

        FString OutJson;
        if (FJsonObjectConverter::UStructToJsonObjectString(ArgumentDefinition, &Argument, OutJson,
            0,0,0, nullptr, false))
        {
            Arguments.Add(OutJson);
        }
    }

    void RegisterArgument(const FString& Name, const FString& Value)
    {
        Arguments.Add(FString::Printf(TEXT("%s:\"%s\""), *Name, *Value));
    }

    void RegisterArgument(const FString& Name, const int32 Value)
    {
        Arguments.Add(FString::Printf(TEXT("%s:%s"), *Name, *FString::FromInt(Value)));
    }

    void RegisterArgument(const FString& Name, const bool Value)
    {
        Arguments.Add(FString::Printf(TEXT("%s:%s"), *Name, Value ? TEXT("true") : TEXT("false")));
    }
    
    FString GetArgumentsString() const
    {
        TArray<FString> ArgumentStrings;
        for (const FString& ArgumentString : Arguments)
        {
            ArgumentStrings.Add(ConvertJsonToGraphQLFriendlyString(ArgumentString));
        }
        
        return FString::Join(ArgumentStrings, TEXT(","));
    }

    void RegisterField(const TModel& Struct)
    {
        const UScriptStruct* StructType = TBaseStructure<TModel>::Get();
        FieldString = StructToQueryRecursive(StructType, &Struct);
    }
    
    FString StructToQueryRecursive(const UScriptStruct* StructType, const void* StructPtr)
    {
        void* DefaultData = FMemory_Alloca(StructType->GetStructureSize());
        StructType->InitializeStruct(DefaultData);

        FString OutString = TEXT("\n{");

        for (TFieldIterator<FProperty> It(StructType); It; ++It)
        {
            const FProperty* Property = *It;

            const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(StructPtr);
            const void* DefaultPtr = Property->ContainerPtrToValuePtr<void>(DefaultData);

            // Skip default values
            if (Property->Identical(ValuePtr, DefaultPtr))
                continue;
            
            if (Property->HasMetaData(*QueryNameMetadata))
            {
                OutString += FString::Printf(TEXT("\n  %s"), *Property->GetMetaData(*QueryNameMetadata));
            }
            if (const FStructProperty* StructProp = CastField<FStructProperty>(Property))
            {
                OutString += StructToQueryRecursive(StructProp->Struct, ValuePtr);
            }
        }

        StructType->DestroyStruct(DefaultData);
        OutString +=TEXT("\n}");

        return OutString;
    }

    
    static FString ConvertJsonToGraphQLFriendlyString(const FString& Json)
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
    
    FString GetFieldString() const
    {
        return FieldString;
    }
    
    FString GetQueryString() const
    {
        FString QueryString = "query {\n";
        QueryString += GetModelString() + "(" + GetArgumentsString() + ")";
        QueryString += GetFieldString() + "\n }";
      
        return QueryString;
    }
    
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
        if (!(*DataObject)->TryGetObjectField(GetModelString(), TargetObject))
        {
            UE_LOG(LogAssetRegister, Error, TEXT("Missing target field '%s' in 'data'."), *GetModelString());
            return false;
        }
   
        return FJsonObjectConverter::JsonObjectToUStruct<TModel>(TargetObject->ToSharedRef(), &OutStruct);
    }

protected:
    FString FieldString;
    TArray<FString> Arguments;
    
private:
    FString QueryNameMetadata = TEXT("QueryName");
};
