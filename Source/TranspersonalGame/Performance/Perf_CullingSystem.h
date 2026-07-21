#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Perf_CullingSystem.generated.h"

UENUM(BlueprintType)
enum class EPerf_CullingMode : uint8
{
    None            UMETA(DisplayName = "No Culling"),
    Distance        UMETA(DisplayName = "Distance Based"),
    Frustum         UMETA(DisplayName = "Frustum Culling"),
    Occlusion       UMETA(DisplayName = "Occlusion Culling"),
    Combined        UMETA(DisplayName = "Combined Culling")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CullingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    EPerf_CullingMode CullingMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = "100.0"))
    float MaxRenderDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = "50.0"))
    float LOD1Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = "100.0"))
    float LOD2Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (ClampMin = "200.0"))
    float LOD3Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableOcclusionCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableFrustumCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingUpdateFrequency;

    FPerf_CullingSettings()
    {
        CullingMode = EPerf_CullingMode::Combined;
        MaxRenderDistance = 10000.0f;
        LOD1Distance = 2000.0f;
        LOD2Distance = 5000.0f;
        LOD3Distance = 8000.0f;
        bEnableOcclusionCulling = true;
        bEnableFrustumCulling = true;
        CullingUpdateFrequency = 0.1f; // 10 times per second
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CullingStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 VisibleActors;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 CulledByDistance;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 CulledByFrustum;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 CulledByOcclusion;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float PerformanceGainPercent;

    FPerf_CullingStats()
    {
        TotalActors = 0;
        VisibleActors = 0;
        CulledByDistance = 0;
        CulledByFrustum = 0;
        CulledByOcclusion = 0;
        PerformanceGainPercent = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_CullingSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCullingSettings(const FPerf_CullingSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_CullingSettings GetCullingSettings() const { return CullingSettings; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateCulling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_CullingStats GetCullingStats() const { return CullingStats; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterActorForCulling(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterActorFromCulling(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetActorCullingDistance(AActor* Actor, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsActorVisible(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCullingForBiome(const FString& BiomeName);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    FPerf_CullingSettings CullingSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    FPerf_CullingStats CullingStats;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    bool bCullingEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    TArray<TWeakObjectPtr<AActor>> RegisteredActors;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    TMap<TWeakObjectPtr<AActor>, float> ActorCullingDistances;

private:
    FTimerHandle CullingUpdateTimer;

    void OnCullingUpdate();
    bool ShouldCullByDistance(AActor* Actor, const FVector& ViewLocation) const;
    bool ShouldCullByFrustum(AActor* Actor) const;
    bool ShouldCullByOcclusion(AActor* Actor) const;
    void SetActorVisibility(AActor* Actor, bool bVisible);
    void UpdateLODForActor(AActor* Actor, float Distance);
    FVector GetPlayerViewLocation() const;
};