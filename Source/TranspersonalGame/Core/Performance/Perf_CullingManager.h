#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Perf_CullingManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_CullingMethod : uint8
{
    Distance        UMETA(DisplayName = "Distance Based"),
    Frustum         UMETA(DisplayName = "Frustum Culling"),
    Occlusion       UMETA(DisplayName = "Occlusion Culling"),
    LOD             UMETA(DisplayName = "LOD Based"),
    Hybrid          UMETA(DisplayName = "Hybrid Method")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CullingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float MaxDrawDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float CullingUpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    EPerf_CullingMethod CullingMethod = EPerf_CullingMethod::Hybrid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableFrustumCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    int32 MaxVisibleActors = 2000;
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_CullingManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_CullingManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_CullingSettings CullingSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentVisibleActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentCulledActors = 0;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateCulling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCullingMethod(EPerf_CullingMethod NewMethod);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMaxDrawDistance(float NewDistance);

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetCullingEfficiency() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void OptimizeSceneCulling();

private:
    UPROPERTY()
    TArray<AActor*> CachedActors;

    UPROPERTY()
    TArray<AActor*> VisibleActors;

    float LastUpdateTime = 0.0f;
    
    void PerformDistanceCulling();
    void PerformFrustumCulling();
    void PerformOcclusionCulling();
    void PerformHybridCulling();
    void UpdateActorVisibility(AActor* Actor, bool bShouldBeVisible);
    bool IsActorInFrustum(AActor* Actor) const;
    float GetDistanceToActor(AActor* Actor) const;
};