#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Perf_CullingManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_CullingType : uint8
{
    Distance        UMETA(DisplayName = "Distance Culling"),
    Frustum         UMETA(DisplayName = "Frustum Culling"),
    Occlusion       UMETA(DisplayName = "Occlusion Culling"),
    LOD             UMETA(DisplayName = "LOD Culling")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CullingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableDistanceCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float MaxRenderDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableFrustumCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float OcclusionCheckFrequency;

    FPerf_CullingSettings()
    {
        bEnableDistanceCulling = true;
        MaxRenderDistance = 10000.0f;
        bEnableFrustumCulling = true;
        bEnableOcclusionCulling = true;
        OcclusionCheckFrequency = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_CullingManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_CullingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterActorForCulling(AActor* Actor, float CullingDistance = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterActorFromCulling(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCullingSettings(const FPerf_CullingSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_CullingSettings GetCullingSettings() const { return CullingSettings; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceUpdateCulling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetCulledActorCount() const { return CulledActors.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetVisibleActorCount() const { return RegisteredActors.Num() - CulledActors.Num(); }

protected:
    void UpdateDistanceCulling();
    void UpdateFrustumCulling();
    void UpdateOcclusionCulling();
    
    bool IsActorInCameraFrustum(AActor* Actor) const;
    bool IsActorOccluded(AActor* Actor) const;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_CullingSettings CullingSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<AActor*> RegisteredActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TMap<AActor*, float> ActorCullingDistances;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<AActor*> CulledActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastOcclusionCheck;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CullingUpdateCounter;
};