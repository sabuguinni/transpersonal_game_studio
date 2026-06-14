#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Perf_LODManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighQualityDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumQualityDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowQualityDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    int32 MaxVisibleActors = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float UpdateFrequency = 0.1f;

    FPerf_LODSettings()
    {
        HighQualityDistance = 2000.0f;
        MediumQualityDistance = 5000.0f;
        LowQualityDistance = 10000.0f;
        CullDistance = 15000.0f;
        MaxVisibleActors = 500;
        UpdateFrequency = 0.1f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_ActorLODData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    float DistanceToPlayer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    int32 CurrentLODLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    bool bIsVisible = true;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    bool bIsCulled = false;

    FPerf_ActorLODData()
    {
        Actor = nullptr;
        DistanceToPlayer = 0.0f;
        CurrentLODLevel = 0;
        bIsVisible = true;
        bIsCulled = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_LODManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_LODManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core LOD management
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void UpdateLODSystem();

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void RegisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void UnregisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Performance|LOD")
    FPerf_LODSettings GetLODSettings() const { return LODSettings; }

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void SetEnabled(bool bNewEnabled);

    UFUNCTION(BlueprintPure, Category = "Performance|LOD")
    bool IsEnabled() const { return bEnabled; }

    // LOD level management
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void SetActorLODLevel(AActor* Actor, int32 LODLevel);

    UFUNCTION(BlueprintPure, Category = "Performance|LOD")
    int32 GetActorLODLevel(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void CullActor(AActor* Actor, bool bShouldCull);

    // Statistics and monitoring
    UFUNCTION(BlueprintPure, Category = "Performance|LOD")
    int32 GetManagedActorCount() const { return ManagedActors.Num(); }

    UFUNCTION(BlueprintPure, Category = "Performance|LOD")
    int32 GetVisibleActorCount() const;

    UFUNCTION(BlueprintPure, Category = "Performance|LOD")
    int32 GetCulledActorCount() const;

    UFUNCTION(BlueprintPure, Category = "Performance|LOD")
    TArray<FPerf_ActorLODData> GetLODStatistics() const;

    // Auto-registration
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void AutoRegisterAllActors();

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void AutoRegisterActorsOfClass(UClass* ActorClass);

protected:
    // Internal methods
    void UpdateActorLOD(FPerf_ActorLODData& ActorData);
    float CalculateDistanceToPlayer(AActor* Actor) const;
    int32 DetermineLODLevel(float Distance) const;
    void ApplyLODToStaticMesh(UStaticMeshComponent* MeshComp, int32 LODLevel);
    void ApplyLODToSkeletalMesh(USkeletalMeshComponent* MeshComp, int32 LODLevel);
    APawn* GetPlayerPawn() const;

    // Timer management
    void StartLODUpdateTimer();
    void StopLODUpdateTimer();

private:
    UPROPERTY()
    FPerf_LODSettings LODSettings;

    UPROPERTY()
    TArray<FPerf_ActorLODData> ManagedActors;

    UPROPERTY()
    bool bEnabled = true;

    FTimerHandle LODUpdateTimerHandle;
};