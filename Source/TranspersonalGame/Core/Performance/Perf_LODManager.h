#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "Perf_LODManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    LOD_High    UMETA(DisplayName = "High Quality"),
    LOD_Medium  UMETA(DisplayName = "Medium Quality"),
    LOD_Low     UMETA(DisplayName = "Low Quality"),
    LOD_Culled  UMETA(DisplayName = "Culled")
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighQualityDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumQualityDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowQualityDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableFrustumCulling = true;

    FPerf_LODSettings()
    {
        HighQualityDistance = 1000.0f;
        MediumQualityDistance = 3000.0f;
        LowQualityDistance = 8000.0f;
        CullDistance = 15000.0f;
        bEnableDistanceCulling = true;
        bEnableFrustumCulling = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_LODManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_LODManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODForActor(AActor* Actor, APawn* ViewerPawn);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateAllLODs();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_LODLevel GetLODLevelForDistance(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_LODSettings GetLODSettings() const { return LODSettings; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetRegisteredActorCount() const { return RegisteredActors.Num(); }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_LODSettings LODSettings;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> RegisteredActors;

    UPROPERTY()
    float LastUpdateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.1f; // Update every 100ms

    void ApplyLODToStaticMesh(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel);
    void ApplyLODToSkeletalMesh(USkeletalMeshComponent* MeshComp, EPerf_LODLevel LODLevel);
    bool IsActorInViewFrustum(AActor* Actor, APawn* ViewerPawn) const;
    float GetDistanceToViewer(AActor* Actor, APawn* ViewerPawn) const;

    FTimerHandle LODUpdateTimer;
    void PerformLODUpdate();
};