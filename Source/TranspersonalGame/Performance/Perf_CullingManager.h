#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Perf_CullingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CullingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float MaxDrawDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float FrustumCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableDistanceCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    int32 MaxVisibleActors;

    FPerf_CullingSettings()
    {
        MaxDrawDistance = 50000.0f;
        FrustumCullingDistance = 30000.0f;
        bEnableOcclusionCulling = true;
        bEnableDistanceCulling = true;
        MaxVisibleActors = 2000;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_ActorCullingData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    float DistanceToPlayer;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    bool bIsVisible;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    bool bIsCulled;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    float LastUpdateTime;

    FPerf_ActorCullingData()
    {
        DistanceToPlayer = 0.0f;
        bIsVisible = true;
        bIsCulled = false;
        LastUpdateTime = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_CullingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_CullingManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void RegisterActorForCulling(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void UnregisterActorFromCulling(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void SetCullingSettings(const FPerf_CullingSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    FPerf_CullingSettings GetCullingSettings() const { return CullingSettings; }

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void EnableCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    bool IsCullingEnabled() const { return bCullingEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    int32 GetRegisteredActorCount() const { return CullingData.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    int32 GetVisibleActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    int32 GetCulledActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void ForceUpdateCulling();

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void LogCullingStats();

protected:
    void UpdateCulling(float DeltaTime);
    void UpdateActorCulling(FPerf_ActorCullingData& ActorData);
    bool ShouldCullActor(AActor* Actor, float DistanceToPlayer);
    void SetActorVisibility(AActor* Actor, bool bVisible);
    FVector GetPlayerLocation();

private:
    UPROPERTY()
    FPerf_CullingSettings CullingSettings;

    UPROPERTY()
    TArray<FPerf_ActorCullingData> CullingData;

    UPROPERTY()
    bool bCullingEnabled;

    UPROPERTY()
    float CullingUpdateInterval;

    UPROPERTY()
    float LastCullingUpdateTime;

    UPROPERTY()
    int32 ActorsProcessedPerFrame;

    UPROPERTY()
    int32 CurrentProcessingIndex;
};