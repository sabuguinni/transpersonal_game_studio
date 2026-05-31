#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perf_LODManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableAutomaticLOD = true;

    FPerf_LODSettings()
    {
        HighDetailDistance = 2000.0f;
        MediumDetailDistance = 5000.0f;
        LowDetailDistance = 10000.0f;
        CullDistance = 20000.0f;
        bEnableAutomaticLOD = true;
    }
};

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    High        UMETA(DisplayName = "High Detail"),
    Medium      UMETA(DisplayName = "Medium Detail"),
    Low         UMETA(DisplayName = "Low Detail"),
    Culled      UMETA(DisplayName = "Culled")
};

/**
 * Level of Detail (LOD) Management System
 * Automatically adjusts mesh detail based on distance from player
 * Manages culling for performance optimization
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_LODManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void UpdateLODForActor(AActor* Actor, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void UpdateAllActorLODs(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    EPerf_LODLevel CalculateLODLevel(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    FPerf_LODSettings GetLODSettings() const { return LODSettings; }

    // Culling Management
    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void CullActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void UnCullActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    bool IsActorCulled(AActor* Actor) const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance|Stats")
    int32 GetManagedActorCount() const { return ManagedActors.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Performance|Stats")
    int32 GetCulledActorCount() const { return CulledActors.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Performance|Stats")
    float GetAverageLODUpdateTime() const { return AverageLODUpdateTime; }

protected:
    // LOD Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    FPerf_LODSettings LODSettings;

    // Managed Actors
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> ManagedActors;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> CulledActors;

    // Performance Tracking
    UPROPERTY()
    float AverageLODUpdateTime;

    UPROPERTY()
    TArray<float> LODUpdateTimes;

    // Internal Methods
    void ApplyLODToStaticMesh(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel);
    void ApplyLODToSkeletalMesh(USkeletalMeshComponent* MeshComp, EPerf_LODLevel LODLevel);
    void RegisterActor(AActor* Actor);
    void UnregisterActor(AActor* Actor);
    void UpdatePerformanceStats(float UpdateTime);

private:
    // Timer for periodic LOD updates
    FTimerHandle LODUpdateTimer;
    void PeriodicLODUpdate();
};