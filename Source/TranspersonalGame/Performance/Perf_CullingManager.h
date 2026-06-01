#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Perf_CullingManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_CullingType : uint8
{
    Distance UMETA(DisplayName = "Distance Culling"),
    Frustum UMETA(DisplayName = "Frustum Culling"),
    Occlusion UMETA(DisplayName = "Occlusion Culling"),
    LOD UMETA(DisplayName = "LOD Culling"),
    Performance UMETA(DisplayName = "Performance Culling")
};

USTRUCT(BlueprintType)
struct FPerf_CullingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float MaxDrawDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float FrustumCullingMargin = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    int32 MaxVisibleActors = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float PerformanceCullingThreshold = 30.0f;
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_CullingManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_CullingManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling Settings")
    FPerf_CullingSettings CullingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling Settings")
    bool bEnableCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling Settings")
    float CullingUpdateInterval = 0.1f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 CulledActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 VisibleActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float LastCullingTime = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void UpdateCulling();

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void SetCullingEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Culling")
    bool IsActorCulled(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void CullActor(AActor* Actor, EPerf_CullingType CullingType);

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void UnCullActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Culling")
    TArray<AActor*> GetCulledActors() const;

    UFUNCTION(BlueprintCallable, Category = "Culling")
    TArray<AActor*> GetVisibleActors() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCullingEfficiency() const;

private:
    float LastUpdateTime = 0.0f;
    TArray<AActor*> CulledActorsList;
    TArray<AActor*> VisibleActorsList;
    
    bool ShouldCullByDistance(AActor* Actor) const;
    bool ShouldCullByFrustum(AActor* Actor) const;
    bool ShouldCullByOcclusion(AActor* Actor) const;
    bool ShouldCullByPerformance(AActor* Actor) const;
    
    void UpdateActorVisibility(AActor* Actor, bool bVisible);
    FVector GetPlayerLocation() const;
};