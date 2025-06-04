#pragma once
#include "AssetRegisterLog.h"

class FQueryRegistry
{
public:
    FQueryRegistry() {}
    
    template<typename RootStruct, typename... MemberPointers>
    void RegisterField(MemberPointers... Pointers)
    {
        RegisterNestedField<RootStruct>(TArray<SIZE_T>{ GetMemberOffset(Pointers)... });
    }
    
    template<typename RootStruct>
    void RegisterNestedField(const TArray<SIZE_T>& Offsets)
    {
        const UStruct* Struct = RootStruct::StaticStruct();
        const FName StructName = Struct->GetFName();

        FString InternalPath;
        FString QueryPath;

        // Add root struct QueryName
        if (Struct->HasMetaData(*QueryNameMetadata))
        {
            QueryPath = Struct->GetMetaData(*QueryNameMetadata);
        }
        else
        {
            QueryPath = Struct->GetName();
            if (QueryPath.StartsWith(TEXT("F")) && FChar::IsUpper(QueryPath[1]))
            {
                QueryPath = QueryPath.RightChop(1);
            }
        }

        const UStruct* CurrentStruct = Struct;
        int32 Depth = 0;

        for (const SIZE_T Offset : Offsets)
        {
            FProperty* Property = FindPropertyByOffset(CurrentStruct, Offset);
            if (!Property)
            {
                UE_LOG(LogAssetRegister, Warning, TEXT("Failed to find property at offset %llu in struct %s"), Offset, *CurrentStruct->GetName());
                return;
            }

            FString PropertyName = Property->GetName();
            FString QueryName = Property->HasMetaData(*QueryNameMetadata)
                ? Property->GetMetaData(*QueryNameMetadata)
                : PropertyName;

            InternalPath = InternalPath.IsEmpty() ? PropertyName : InternalPath + TEXT(".") + PropertyName;
            QueryPath = QueryPath + TEXT(".") + QueryName;

            // If it's a nested struct, go deeper
            if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
            {
                CurrentStruct = StructProp->Struct;
            }

            Depth++;
        }

        Registry.FindOrAdd(StructName).Add(InternalPath, QueryPath);
    }

protected:
    TMap<FName, TMap<FString, FString>> Registry;
    
private:
    template<typename StructType, typename MemberType>
    static SIZE_T GetMemberOffset(MemberType StructType::* Member)
    {
        return reinterpret_cast<SIZE_T>(&(reinterpret_cast<StructType*>(0)->*Member));
    }

    static FProperty* FindPropertyByOffset(const UStruct* Struct, const SIZE_T Offset)
    {
        for (TFieldIterator<FProperty> It(Struct); It; ++It)
        {
            if (It->GetOffset_ForInternal() == Offset)
            {
                return *It;
            }
        }
        return nullptr;
    }
    
    FString QueryNameMetadata = TEXT("QueryName");
};
