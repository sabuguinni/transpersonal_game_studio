#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Templates/SharedPointer.h"
#include "AssetProductionManager.generated.h"

UENUM(BlueprintType)
enum class EAssetType : uint8
{
    StaticMesh,
    SkeletalMesh,
    Texture,
    Material,
    Animation,
    Sound,
    Blueprint,
    Level,
    DataAsset,
    Video
};

UENUM(BlueprintType)
enum class EAssetStatus : uint8
{
    Planned,
    InProduction,
    Review,
    Approved,
    Integrated,
    Deprecated
};

USTRUCT(BlueprintType)
struct FAssetProductionItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString AssetID;

    UPROPERTY(BlueprintReadOnly)
    FString AssetName;

    UPROPERTY(BlueprintReadOnly)
    EAssetType Type;

    UPROPERTY(BlueprintReadOnly)
    EAssetStatus Status;

    UPROPERTY(BlueprintReadOnly)
    int32 ProducingAgentID;

    UPROPERTY(BlueprintReadOnly)
    FString FilePath;

    UPROPERTY(BlueprintReadOnly)
    FString Description;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Tags;

    UPROPERTY(BlueprintReadOnly)
    float ProductionProgress;

    UPROPERTY(BlueprintReadOnly)
    FString LastModified;

    FAssetProductionItem()
    {
        Type = EAssetType::DataAsset;
        Status = EAssetStatus::Planned;
        ProducingAgentID = 0;
        ProductionProgress = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FAssetProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 TotalAssets;

    UPROPERTY(BlueprintReadOnly)
    int32 CompletedAssets;

    UPROPERTY(BlueprintReadOnly)
    int32 InProductionAssets;

    UPROPERTY(BlueprintReadOnly)
    int32 PendingReviewAssets;

    UPROPERTY(BlueprintReadOnly)
    float OverallProgress;

    UPROPERTY(BlueprintReadOnly)
    TMap<EAssetType, int32> AssetCountByType;

    FAssetProductionMetrics()
    {
        TotalAssets = 0;
        CompletedAssets = 0;
        InProductionAssets = 0;
        PendingReviewAssets = 0;
        OverallProgress = 0.0f;
    }
};

/**
 * Asset Production Manager
 * Tracks and manages all assets being created by the 19-agent production pipeline
 * Integrates with HeyGen, image generation, and UE5 asset systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAssetProductionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAssetProductionManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Asset Registration
    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    FString RegisterAsset(const FString& AssetName, EAssetType Type, int32 ProducingAgentID, const FString& Description = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    void UpdateAssetStatus(const FString& AssetID, EAssetStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    void UpdateAssetProgress(const FString& AssetID, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    void SetAssetFilePath(const FString& AssetID, const FString& FilePath);

    // Asset Queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Asset Production")
    TArray<FAssetProductionItem> GetAssetsByAgent(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Asset Production")
    TArray<FAssetProductionItem> GetAssetsByType(EAssetType Type) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Asset Production")
    TArray<FAssetProductionItem> GetAssetsByStatus(EAssetStatus Status) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Asset Production")
    FAssetProductionItem GetAsset(const FString& AssetID) const;

    // Production Tools Integration
    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    FString RegisterHeyGenVideo(const FString& VideoID, const FString& Title, int32 ProducingAgentID);

    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    FString RegisterGeneratedImage(const FString& ImageURL, const FString& Description, int32 ProducingAgentID);

    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    FString RegisterUE5Asset(const FString& AssetPath, EAssetType Type, int32 ProducingAgentID);

    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    FString RegisterSoundAsset(const FString& SoundPath, const FString& Description, int32 ProducingAgentID);

    // Asset Dependencies
    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    void AddAssetDependency(const FString& AssetID, const FString& DependencyAssetID);

    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    bool ValidateAssetDependencies(const FString& AssetID) const;

    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    TArray<FString> GetBlockedAssets() const;

    // Asset Review and Approval
    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    void SubmitForReview(const FString& AssetID, const FString& ReviewNotes = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    void ApproveAsset(const FString& AssetID, const FString& ApprovalNotes = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    void RejectAsset(const FString& AssetID, const FString& RejectionReason);

    // Metrics and Reporting
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Asset Production")
    FAssetProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    void GenerateProductionReport() const;

    // Asset Cleanup
    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    void DeprecateAsset(const FString& AssetID, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Asset Production")
    void CleanupDeprecatedAssets();

protected:
    // Asset storage
    UPROPERTY()
    TArray<FAssetProductionItem> ProductionAssets;

    // Asset lookup maps
    TMap<FString, int32> AssetIDToIndex;
    TMap<int32, TArray<FString>> AgentAssets;
    TMap<EAssetType, TArray<FString>> TypeAssets;

    // Internal functions
    void UpdateMetrics();
    FString GenerateAssetID(const FString& AssetName, EAssetType Type);
    void LogAssetEvent(const FString& AssetID, const FString& Event);
    bool IsAssetIDValid(const FString& AssetID) const;
    int32 GetAssetIndex(const FString& AssetID) const;

private:
    // Cached metrics
    mutable FAssetProductionMetrics CachedMetrics;
    mutable bool bMetricsCacheDirty;
};