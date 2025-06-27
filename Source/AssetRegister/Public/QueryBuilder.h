#pragma once

#include "QueryNode.h"
#include "Schemas/Asset.h"
#include "Schemas/Inputs/AssetInput.h"

class FAssetRegister
{
public:
	FAssetRegister() {}

	static TSharedPtr<FQueryNode<FAsset>> AddAssetQuery(const FAssetInput& Input)
	{
		const auto Node= MakeShared<FQueryNode<FAsset>>(QueryStringUtil::GetQueryName<FAsset>());
		Node->AddArgument(Input);
		return Node;
	}
};
