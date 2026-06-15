#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Perf_CullingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CullingStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    int32 VisibleActors;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    int32 FrustumCulledActors;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    int32 DistanceCulledActors;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    int32 OcclusionCulledActors;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    float CullingTimeMS;

    FPerf_CullingStats()
    {
        TotalActors = 0;
        VisibleActors = 0;
        FrustumCulledActors = 0;
        DistanceCulledActors = 0;
        OcclusionCulledActors = 0;
        CullingTimeMS = 0.0f;
    }
};

UENUM(BlueprintType)
enum class EPerf_CullingMode : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    FrustumOnly     UMETA(DisplayName = "Frustum Only"),
    DistanceOnly    UMETA(DisplayName = "Distance Only"),
    FrustumDistance UMETA(DisplayName = "Frustum + Distance"),
    Full            UMETA(DisplayName = "Full Culling")
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_CullingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_CullingManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Culling control
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCullingMode(EPerf_CullingMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_CullingMode GetCullingMode() const { return CurrentCullingMode; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCullingDistances(float NearDistance, float FarDistance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetFrustumCullingEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOcclusionCullingEnabled(bool bEnabled);

    // Culling execution
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void PerformCullingPass();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_CullingStats GetLastCullingStats() const { return LastCullingStats; }

    // Auto-culling
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartAutoCulling(float UpdateInterval = 0.1f);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopAutoCulling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsAutoCullingActive() const { return bIsAutoCullingActive; }

    // Performance targets
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(int32 MaxVisibleActors, float TargetFrameTimeMS);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    EPerf_CullingMode CurrentCullingMode;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    float NearCullingDistance;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    float FarCullingDistance;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    bool bFrustumCullingEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    bool bOcclusionCullingEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    bool bIsAutoCullingActive;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    int32 MaxVisibleActors;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    float TargetFrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    FPerf_CullingStats LastCullingStats;

    // Auto-culling timer
    FTimerHandle AutoCullingTimerHandle;

    // Culling methods
    void PerformFrustumCulling(const TArray<AActor*>& Actors, TArray<AActor*>& VisibleActors);
    void PerformDistanceCulling(const TArray<AActor*>& Actors, TArray<AActor*>& VisibleActors);
    void PerformOcclusionCulling(const TArray<AActor*>& Actors, TArray<AActor*>& VisibleActors);
    
    // Utility methods
    FVector GetCameraLocation();
    FVector GetCameraForward();
    bool IsActorInFrustum(AActor* Actor);
    float GetDistanceToCamera(AActor* Actor);
    void SetActorVisibility(AActor* Actor, bool bVisible);
    void AdaptiveCulling();
};