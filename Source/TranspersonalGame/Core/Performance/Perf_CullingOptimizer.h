#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "Perf_CullingOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_CullingMode : uint8
{
    Distance        UMETA(DisplayName = "Distance Culling"),
    Frustum         UMETA(DisplayName = "Frustum Culling"),
    Occlusion       UMETA(DisplayName = "Occlusion Culling"),
    Hybrid          UMETA(DisplayName = "Hybrid Culling")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CullingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float MaxDrawDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float MinScreenSize = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableFrustumCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    int32 MaxVisibleObjects = 5000;

    FPerf_CullingSettings()
    {
        MaxDrawDistance = 10000.0f;
        MinScreenSize = 0.01f;
        bEnableOcclusionCulling = true;
        bEnableFrustumCulling = true;
        MaxVisibleObjects = 5000;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_CullingOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_CullingOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling Settings")
    FPerf_CullingSettings CullingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling Settings")
    EPerf_CullingMode CullingMode = EPerf_CullingMode::Hybrid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.1f;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 TotalObjects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 VisibleObjects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 CulledObjects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    float CullingEfficiency = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void UpdateCulling();

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void SetCullingMode(EPerf_CullingMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void SetMaxDrawDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void EnableOcclusionCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    float GetCullingEfficiency() const { return CullingEfficiency; }

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    int32 GetVisibleObjectCount() const { return VisibleObjects; }

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    int32 GetCulledObjectCount() const { return CulledObjects; }

private:
    float TimeSinceLastUpdate = 0.0f;
    TArray<TWeakObjectPtr<AActor>> TrackedActors;
    
    void GatherActorsToTrack();
    void PerformDistanceCulling();
    void PerformFrustumCulling();
    void PerformOcclusionCulling();
    void UpdateStatistics();
    bool IsActorInViewFrustum(AActor* Actor) const;
    float GetDistanceToPlayer(AActor* Actor) const;
    void SetActorVisibility(AActor* Actor, bool bVisible);
};