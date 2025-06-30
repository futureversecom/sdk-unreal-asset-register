#pragma once

#include "QueryNode.h"
#include "Schemas/Asset.h"
#include "Schemas/Assets.h"
#include "Schemas/Inputs/AssetConnection.h"
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

	static TSharedPtr<FQueryNode<FAssets>> AddAssetsQuery(const FAssetConnection& Input)
	{
		const auto Node= MakeShared<FQueryNode<FAssets>>(QueryStringUtil::GetQueryName<FAssets>());
		Node->AddArgument(Input);
		return Node;
	}
};
