#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Perf_CullingOptimizer.generated.h"

USTRUCT(BlueprintType)
struct FPerf_CullingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float MaxVisibilityDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float FrustumCullingMargin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableDistanceCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableFrustumCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float CullingUpdateFrequency;

    FPerf_CullingSettings()
    {
        MaxVisibilityDistance = 10000.0f;
        FrustumCullingMargin = 500.0f;
        bEnableDistanceCulling = true;
        bEnableFrustumCulling = true;
        bEnableOcclusionCulling = false;
        CullingUpdateFrequency = 0.5f;
    }
};

UCLASS(BlueprintType, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_CullingOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_CullingOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartCulling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopCulling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCullingSettings(const FPerf_CullingSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_CullingSettings GetCullingSettings() const { return CullingSettings; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetCulledActorCount() const { return CulledActorCount; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetVisibleActorCount() const { return VisibleActorCount; }

protected:
    void UpdateCulling();
    bool ShouldCullActor(AActor* Actor, const FVector& ViewLocation, const FVector& ViewDirection) const;
    bool IsActorInFrustum(AActor* Actor, const FVector& ViewLocation, const FVector& ViewDirection) const;
    float GetDistanceToActor(AActor* Actor, const FVector& ViewLocation) const;

private:
    UPROPERTY(EditAnywhere, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_CullingSettings CullingSettings;

    UPROPERTY()
    bool bIsCullingActive;

    UPROPERTY()
    int32 CulledActorCount;

    UPROPERTY()
    int32 VisibleActorCount;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> ManagedActors;

    FTimerHandle CullingTimerHandle;
    float TimeSinceLastUpdate;
};