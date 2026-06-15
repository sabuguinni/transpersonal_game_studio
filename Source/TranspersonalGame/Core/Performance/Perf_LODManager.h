#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "SharedTypes.h"
#include "Perf_LODManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    LOD_High        UMETA(DisplayName = "High Detail"),
    LOD_Medium      UMETA(DisplayName = "Medium Detail"),
    LOD_Low         UMETA(DisplayName = "Low Detail"),
    LOD_Culled      UMETA(DisplayName = "Culled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighDetailDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumDetailDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowDetailDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableFrustumCulling = true;

    FPerf_LODSettings()
    {
        HighDetailDistance = 2000.0f;
        MediumDetailDistance = 5000.0f;
        LowDetailDistance = 10000.0f;
        CullDistance = 15000.0f;
        bEnableDistanceCulling = true;
        bEnableFrustumCulling = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_ActorLODData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor LOD")
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor LOD")
    EPerf_LODLevel CurrentLOD = EPerf_LODLevel::LOD_High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor LOD")
    float LastDistanceToPlayer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor LOD")
    bool bIsVisible = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor LOD")
    bool bIsCulled = false;

    FPerf_ActorLODData()
    {
        Actor = nullptr;
        CurrentLOD = EPerf_LODLevel::LOD_High;
        LastDistanceToPlayer = 0.0f;
        bIsVisible = true;
        bIsCulled = false;
    }
};

/**
 * Performance LOD Manager - Manages Level of Detail and culling for optimal frame rates
 * Targets: 60fps on high-end PC, 30fps on console
 * Automatically adjusts LOD based on distance to player and performance metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_LODManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_LODManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core LOD Management
    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void RegisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void UnregisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void UpdateLODForActor(AActor* Actor, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void UpdateAllActorLODs();

    // LOD Level Management
    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    EPerf_LODLevel CalculateLODLevel(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void SetActorLODLevel(AActor* Actor, EPerf_LODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void ApplyLODToStaticMesh(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void ApplyLODToSkeletalMesh(USkeletalMeshComponent* MeshComp, EPerf_LODLevel LODLevel);

    // Culling Management
    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void CullActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void UnCullActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    bool ShouldCullActor(AActor* Actor, float DistanceToPlayer) const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    int32 GetRegisteredActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    int32 GetCulledActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    float GetAverageDistanceToPlayer() const;

    // Settings Management
    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    FPerf_LODSettings GetLODSettings() const;

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Performance LOD", CallInEditor)
    void DebugDrawLODInfo();

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void LogLODStatistics();

protected:
    // LOD Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings", meta = (AllowPrivateAccess = "true"))
    FPerf_LODSettings LODSettings;

    // Registered Actors
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LOD Data", meta = (AllowPrivateAccess = "true"))
    TArray<FPerf_ActorLODData> RegisteredActors;

    // Performance Metrics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 TotalRegisteredActors = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 CulledActorCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float LastUpdateTime = 0.0f;

    // Internal Methods
    AActor* GetPlayerActor() const;
    FVector GetPlayerLocation() const;
    void CleanupInvalidActors();
    FPerf_ActorLODData* FindActorLODData(AActor* Actor);
};