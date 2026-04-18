#include "AssetProductionManager.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Guid.h"

UAssetProductionManager::UAssetProductionManager()
    : bMetricsCacheDirty(true)
{
}

void UAssetProductionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("AssetProductionManager: Initializing asset tracking system"));
    
    // Initialize storage containers
    ProductionAssets.Empty();
    AssetIDToIndex.Empty();
    AgentAssets.Empty();
    TypeAssets.Empty();
    
    bMetricsCacheDirty = true;
    
    UE_LOG(LogTemp, Log, TEXT("AssetProductionManager: Asset tracking system initialized"));
}

void UAssetProductionManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("AssetProductionManager: Shutting down asset tracking system"));
    
    // Generate final production report
    GenerateProductionReport();
    
    // Clear all data
    ProductionAssets.Empty();
    AssetIDToIndex.Empty();
    AgentAssets.Empty();
    TypeAssets.Empty();
    
    Super::Deinitialize();
}

FString UAssetProductionManager::RegisterAsset(const FString& AssetName, EAssetType Type, int32 ProducingAgentID, const FString& Description)
{
    if (AssetName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("AssetProductionManager: Cannot register asset with empty name"));
        return FString();
    }
    
    // Generate unique asset ID
    FString AssetID = GenerateAssetID(AssetName, Type);
    
    // Create new asset item
    FAssetProductionItem NewAsset;
    NewAsset.AssetID = AssetID;
    NewAsset.AssetName = AssetName;
    NewAsset.Type = Type;
    NewAsset.Status = EAssetStatus::Planned;
    NewAsset.ProducingAgentID = ProducingAgentID;
    NewAsset.Description = Description;
    NewAsset.ProductionProgress = 0.0f;
    NewAsset.LastModified = FDateTime::Now().ToString();
    
    // Add to storage
    int32 AssetIndex = ProductionAssets.Add(NewAsset);
    AssetIDToIndex.Add(AssetID, AssetIndex);
    
    // Update lookup maps
    AgentAssets.FindOrAdd(ProducingAgentID).Add(AssetID);
    TypeAssets.FindOrAdd(Type).Add(AssetID);
    
    bMetricsCacheDirty = true;
    
    LogAssetEvent(AssetID, FString::Printf(TEXT("Registered by Agent %d"), ProducingAgentID));
    
    UE_LOG(LogTemp, Log, TEXT("AssetProductionManager: Registered asset '%s' (ID: %s) by Agent %d"), 
           *AssetName, *AssetID, ProducingAgentID);
    
    return AssetID;
}

void UAssetProductionManager::UpdateAssetStatus(const FString& AssetID, EAssetStatus NewStatus)
{
    int32 AssetIndex = GetAssetIndex(AssetID);
    if (AssetIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("AssetProductionManager: Asset ID '%s' not found for status update"), *AssetID);
        return;
    }
    
    EAssetStatus OldStatus = ProductionAssets[AssetIndex].Status;
    ProductionAssets[AssetIndex].Status = NewStatus;
    ProductionAssets[AssetIndex].LastModified = FDateTime::Now().ToString();
    
    bMetricsCacheDirty = true;
    
    LogAssetEvent(AssetID, FString::Printf(TEXT("Status changed from %d to %d"), 
                                          static_cast<int32>(OldStatus), static_cast<int32>(NewStatus)));
    
    UE_LOG(LogTemp, Log, TEXT("AssetProductionManager: Updated status for asset '%s' from %d to %d"), 
           *AssetID, static_cast<int32>(OldStatus), static_cast<int32>(NewStatus));
}

void UAssetProductionManager::UpdateAssetProgress(const FString& AssetID, float Progress)
{
    int32 AssetIndex = GetAssetIndex(AssetID);
    if (AssetIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("AssetProductionManager: Asset ID '%s' not found for progress update"), *AssetID);
        return;
    }
    
    float ClampedProgress = FMath::Clamp(Progress, 0.0f, 100.0f);
    float OldProgress = ProductionAssets[AssetIndex].ProductionProgress;
    
    ProductionAssets[AssetIndex].ProductionProgress = ClampedProgress;
    ProductionAssets[AssetIndex].LastModified = FDateTime::Now().ToString();
    
    // Auto-update status based on progress
    if (ClampedProgress >= 100.0f && ProductionAssets[AssetIndex].Status == EAssetStatus::InProduction)
    {
        ProductionAssets[AssetIndex].Status = EAssetStatus::Review;
    }
    else if (ClampedProgress > 0.0f && ProductionAssets[AssetIndex].Status == EAssetStatus::Planned)
    {
        ProductionAssets[AssetIndex].Status = EAssetStatus::InProduction;
    }
    
    bMetricsCacheDirty = true;
    
    LogAssetEvent(AssetID, FString::Printf(TEXT("Progress updated from %.1f%% to %.1f%%"), OldProgress, ClampedProgress));
}

void UAssetProductionManager::SetAssetFilePath(const FString& AssetID, const FString& FilePath)
{
    int32 AssetIndex = GetAssetIndex(AssetID);
    if (AssetIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("AssetProductionManager: Asset ID '%s' not found for file path update"), *AssetID);
        return;
    }
    
    ProductionAssets[AssetIndex].FilePath = FilePath;
    ProductionAssets[AssetIndex].LastModified = FDateTime::Now().ToString();
    
    LogAssetEvent(AssetID, FString::Printf(TEXT("File path set to: %s"), *FilePath));
}

TArray<FAssetProductionItem> UAssetProductionManager::GetAssetsByAgent(int32 AgentID) const
{
    TArray<FAssetProductionItem> Result;
    
    if (const TArray<FString>* AssetIDs = AgentAssets.Find(AgentID))
    {
        for (const FString& AssetID : *AssetIDs)
        {
            int32 AssetIndex = GetAssetIndex(AssetID);
            if (AssetIndex != INDEX_NONE)
            {
                Result.Add(ProductionAssets[AssetIndex]);
            }
        }
    }
    
    return Result;
}

TArray<FAssetProductionItem> UAssetProductionManager::GetAssetsByType(EAssetType Type) const
{
    TArray<FAssetProductionItem> Result;
    
    if (const TArray<FString>* AssetIDs = TypeAssets.Find(Type))
    {
        for (const FString& AssetID : *AssetIDs)
        {
            int32 AssetIndex = GetAssetIndex(AssetID);
            if (AssetIndex != INDEX_NONE)
            {
                Result.Add(ProductionAssets[AssetIndex]);
            }
        }
    }
    
    return Result;
}

TArray<FAssetProductionItem> UAssetProductionManager::GetAssetsByStatus(EAssetStatus Status) const
{
    TArray<FAssetProductionItem> Result;
    
    for (const FAssetProductionItem& Asset : ProductionAssets)
    {
        if (Asset.Status == Status)
        {
            Result.Add(Asset);
        }
    }
    
    return Result;
}

FAssetProductionItem UAssetProductionManager::GetAsset(const FString& AssetID) const
{
    int32 AssetIndex = GetAssetIndex(AssetID);
    if (AssetIndex != INDEX_NONE)
    {
        return ProductionAssets[AssetIndex];
    }
    
    return FAssetProductionItem(); // Return default-constructed item
}

FString UAssetProductionManager::RegisterHeyGenVideo(const FString& VideoID, const FString& Title, int32 ProducingAgentID)
{
    FString Description = FString::Printf(TEXT("HeyGen Video: %s"), *VideoID);
    return RegisterAsset(Title, EAssetType::Video, ProducingAgentID, Description);
}

FString UAssetProductionManager::RegisterGeneratedImage(const FString& ImageURL, const FString& Description, int32 ProducingAgentID)
{
    FString AssetName = FString::Printf(TEXT("Generated_Image_%s"), *FGuid::NewGuid().ToString(EGuidFormats::Short));
    FString FullDescription = FString::Printf(TEXT("Generated Image: %s (URL: %s)"), *Description, *ImageURL);
    return RegisterAsset(AssetName, EAssetType::Texture, ProducingAgentID, FullDescription);
}

FString UAssetProductionManager::RegisterUE5Asset(const FString& AssetPath, EAssetType Type, int32 ProducingAgentID)
{
    FString AssetName = FPaths::GetBaseFilename(AssetPath);
    FString AssetID = RegisterAsset(AssetName, Type, ProducingAgentID, FString::Printf(TEXT("UE5 Asset: %s"), *AssetPath));
    SetAssetFilePath(AssetID, AssetPath);
    return AssetID;
}

FString UAssetProductionManager::RegisterSoundAsset(const FString& SoundPath, const FString& Description, int32 ProducingAgentID)
{
    FString AssetName = FPaths::GetBaseFilename(SoundPath);
    FString AssetID = RegisterAsset(AssetName, EAssetType::Sound, ProducingAgentID, Description);
    SetAssetFilePath(AssetID, SoundPath);
    return AssetID;
}

void UAssetProductionManager::AddAssetDependency(const FString& AssetID, const FString& DependencyAssetID)
{
    int32 AssetIndex = GetAssetIndex(AssetID);
    if (AssetIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("AssetProductionManager: Asset ID '%s' not found for dependency addition"), *AssetID);
        return;
    }
    
    if (!IsAssetIDValid(DependencyAssetID))
    {
        UE_LOG(LogTemp, Warning, TEXT("AssetProductionManager: Dependency asset ID '%s' not found"), *DependencyAssetID);
        return;
    }
    
    ProductionAssets[AssetIndex].Dependencies.AddUnique(DependencyAssetID);
    ProductionAssets[AssetIndex].LastModified = FDateTime::Now().ToString();
    
    LogAssetEvent(AssetID, FString::Printf(TEXT("Added dependency: %s"), *DependencyAssetID));
}

bool UAssetProductionManager::ValidateAssetDependencies(const FString& AssetID) const
{
    int32 AssetIndex = GetAssetIndex(AssetID);
    if (AssetIndex == INDEX_NONE)
    {
        return false;
    }
    
    const FAssetProductionItem& Asset = ProductionAssets[AssetIndex];
    
    for (const FString& DependencyID : Asset.Dependencies)
    {
        int32 DependencyIndex = GetAssetIndex(DependencyID);
        if (DependencyIndex == INDEX_NONE)
        {
            return false; // Dependency not found
        }
        
        const FAssetProductionItem& Dependency = ProductionAssets[DependencyIndex];
        if (Dependency.Status != EAssetStatus::Approved && Dependency.Status != EAssetStatus::Integrated)
        {
            return false; // Dependency not ready
        }
    }
    
    return true;
}

TArray<FString> UAssetProductionManager::GetBlockedAssets() const
{
    TArray<FString> BlockedAssets;
    
    for (const FAssetProductionItem& Asset : ProductionAssets)
    {
        if (Asset.Status == EAssetStatus::InProduction || Asset.Status == EAssetStatus::Review)
        {
            if (!ValidateAssetDependencies(Asset.AssetID))
            {
                BlockedAssets.Add(Asset.AssetID);
            }
        }
    }
    
    return BlockedAssets;
}

void UAssetProductionManager::SubmitForReview(const FString& AssetID, const FString& ReviewNotes)
{
    UpdateAssetStatus(AssetID, EAssetStatus::Review);
    LogAssetEvent(AssetID, FString::Printf(TEXT("Submitted for review: %s"), *ReviewNotes));
}

void UAssetProductionManager::ApproveAsset(const FString& AssetID, const FString& ApprovalNotes)
{
    UpdateAssetStatus(AssetID, EAssetStatus::Approved);
    UpdateAssetProgress(AssetID, 100.0f);
    LogAssetEvent(AssetID, FString::Printf(TEXT("Approved: %s"), *ApprovalNotes));
}

void UAssetProductionManager::RejectAsset(const FString& AssetID, const FString& RejectionReason)
{
    UpdateAssetStatus(AssetID, EAssetStatus::InProduction);
    LogAssetEvent(AssetID, FString::Printf(TEXT("Rejected: %s"), *RejectionReason));
}

FAssetProductionMetrics UAssetProductionManager::GetProductionMetrics() const
{
    if (bMetricsCacheDirty)
    {
        UpdateMetrics();
    }
    
    return CachedMetrics;
}

void UAssetProductionManager::GenerateProductionReport() const
{
    FAssetProductionMetrics Metrics = GetProductionMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("=== ASSET PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Assets: %d"), Metrics.TotalAssets);
    UE_LOG(LogTemp, Log, TEXT("Completed Assets: %d"), Metrics.CompletedAssets);
    UE_LOG(LogTemp, Log, TEXT("In Production: %d"), Metrics.InProductionAssets);
    UE_LOG(LogTemp, Log, TEXT("Pending Review: %d"), Metrics.PendingReviewAssets);
    UE_LOG(LogTemp, Log, TEXT("Overall Progress: %.1f%%"), Metrics.OverallProgress);
    
    UE_LOG(LogTemp, Log, TEXT("Assets by Type:"));
    for (const auto& TypeCount : Metrics.AssetCountByType)
    {
        UE_LOG(LogTemp, Log, TEXT("  Type %d: %d assets"), static_cast<int32>(TypeCount.Key), TypeCount.Value);
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== END REPORT ==="));
}

void UAssetProductionManager::DeprecateAsset(const FString& AssetID, const FString& Reason)
{
    UpdateAssetStatus(AssetID, EAssetStatus::Deprecated);
    LogAssetEvent(AssetID, FString::Printf(TEXT("Deprecated: %s"), *Reason));
}

void UAssetProductionManager::CleanupDeprecatedAssets()
{
    int32 CleanedCount = 0;
    
    for (int32 i = ProductionAssets.Num() - 1; i >= 0; --i)
    {
        if (ProductionAssets[i].Status == EAssetStatus::Deprecated)
        {
            FString AssetID = ProductionAssets[i].AssetID;
            
            // Remove from lookup maps
            AssetIDToIndex.Remove(AssetID);
            
            // Remove from agent assets
            for (auto& AgentPair : AgentAssets)
            {
                AgentPair.Value.Remove(AssetID);
            }
            
            // Remove from type assets
            for (auto& TypePair : TypeAssets)
            {
                TypePair.Value.Remove(AssetID);
            }
            
            // Remove from main array
            ProductionAssets.RemoveAt(i);
            CleanedCount++;
        }
    }
    
    // Rebuild index map
    AssetIDToIndex.Empty();
    for (int32 i = 0; i < ProductionAssets.Num(); ++i)
    {
        AssetIDToIndex.Add(ProductionAssets[i].AssetID, i);
    }
    
    bMetricsCacheDirty = true;
    
    UE_LOG(LogTemp, Log, TEXT("AssetProductionManager: Cleaned up %d deprecated assets"), CleanedCount);
}

void UAssetProductionManager::UpdateMetrics() const
{
    CachedMetrics = FAssetProductionMetrics();
    CachedMetrics.TotalAssets = ProductionAssets.Num();
    
    float TotalProgress = 0.0f;
    
    for (const FAssetProductionItem& Asset : ProductionAssets)
    {
        switch (Asset.Status)
        {
        case EAssetStatus::Approved:
        case EAssetStatus::Integrated:
            CachedMetrics.CompletedAssets++;
            break;
        case EAssetStatus::InProduction:
            CachedMetrics.InProductionAssets++;
            break;
        case EAssetStatus::Review:
            CachedMetrics.PendingReviewAssets++;
            break;
        default:
            break;
        }
        
        TotalProgress += Asset.ProductionProgress;
        
        // Count by type
        int32& TypeCount = CachedMetrics.AssetCountByType.FindOrAdd(Asset.Type);
        TypeCount++;
    }
    
    if (CachedMetrics.TotalAssets > 0)
    {
        CachedMetrics.OverallProgress = TotalProgress / CachedMetrics.TotalAssets;
    }
    
    bMetricsCacheDirty = false;
}

FString UAssetProductionManager::GenerateAssetID(const FString& AssetName, EAssetType Type)
{
    FString TypePrefix;
    switch (Type)
    {
    case EAssetType::StaticMesh: TypePrefix = TEXT("SM"); break;
    case EAssetType::SkeletalMesh: TypePrefix = TEXT("SK"); break;
    case EAssetType::Texture: TypePrefix = TEXT("T"); break;
    case EAssetType::Material: TypePrefix = TEXT("M"); break;
    case EAssetType::Animation: TypePrefix = TEXT("A"); break;
    case EAssetType::Sound: TypePrefix = TEXT("S"); break;
    case EAssetType::Blueprint: TypePrefix = TEXT("BP"); break;
    case EAssetType::Level: TypePrefix = TEXT("L"); break;
    case EAssetType::Video: TypePrefix = TEXT("V"); break;
    default: TypePrefix = TEXT("DA"); break;
    }
    
    FString CleanName = AssetName;
    CleanName = CleanName.Replace(TEXT(" "), TEXT("_"));
    CleanName = CleanName.Replace(TEXT("-"), TEXT("_"));
    
    return FString::Printf(TEXT("%s_%s_%s"), *TypePrefix, *CleanName, *FGuid::NewGuid().ToString(EGuidFormats::Short));
}

void UAssetProductionManager::LogAssetEvent(const FString& AssetID, const FString& Event)
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("AssetProductionManager: [%s] %s"), *AssetID, *Event);
}

bool UAssetProductionManager::IsAssetIDValid(const FString& AssetID) const
{
    return AssetIDToIndex.Contains(AssetID);
}

int32 UAssetProductionManager::GetAssetIndex(const FString& AssetID) const
{
    if (const int32* Index = AssetIDToIndex.Find(AssetID))
    {
        return *Index;
    }
    return INDEX_NONE;
}