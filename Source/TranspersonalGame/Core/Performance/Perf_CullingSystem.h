#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Perf_CullingSystem.generated.h"

UENUM(BlueprintType)
enum class EPerf_CullingType : uint8
{
    None        UMETA(DisplayName = "No Culling"),
    Distance    UMETA(DisplayName = "Distance Culling"),
    Frustum     UMETA(DisplayName = "Frustum Culling"),
    Occlusion   UMETA(DisplayName = "Occlusion Culling"),
    Combined    UMETA(DisplayName = "Combined Culling")
};

USTRUCT(BlueprintType)
struct FPerf_CullingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    EPerf_CullingType CullingType = EPerf_CullingType::Combined;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float MaxCullingDistance = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float MinCullingDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableFrustumCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float CullingUpdateFrequency = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    int32 MaxActorsPerFrame = 50;

    FPerf_CullingSettings()
    {
        CullingType = EPerf_CullingType::Combined;
        MaxCullingDistance = 20000.0f;
        MinCullingDistance = 100.0f;
        bEnableOcclusionCulling = true;
        bEnableFrustumCulling = true;
        CullingUpdateFrequency = 0.2f;
        MaxActorsPerFrame = 50;
    }
};

USTRUCT(BlueprintType)
struct FPerf_CullingStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 CulledActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 VisibleActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float LastUpdateTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float AverageUpdateTime = 0.0f;

    FPerf_CullingStats()
    {
        TotalActors = 0;
        CulledActors = 0;
        VisibleActors = 0;
        LastUpdateTime = 0.0f;
        AverageUpdateTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_CullingSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_CullingSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterActorForCulling(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterActorFromCulling(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateCulling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldCullActor(AActor* Actor, APawn* ViewerPawn) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCullingSettings(const FPerf_CullingSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_CullingSettings GetCullingSettings() const { return CullingSettings; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_CullingStats GetCullingStats() const { return CullingStats; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetActorCulled(AActor* Actor, bool bCulled);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetRegisteredActorCount() const { return RegisteredActors.Num(); }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_CullingSettings CullingSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_CullingStats CullingStats;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> RegisteredActors;

    UPROPERTY()
    TMap<TWeakObjectPtr<AActor>, bool> ActorCullingState;

    FTimerHandle CullingUpdateTimer;
    int32 CurrentActorIndex = 0;

    void PerformCullingUpdate();
    bool IsActorInFrustum(AActor* Actor, APawn* ViewerPawn) const;
    bool IsActorOccluded(AActor* Actor, APawn* ViewerPawn) const;
    float GetDistanceToViewer(AActor* Actor, APawn* ViewerPawn) const;
    void UpdateCullingStats();
};