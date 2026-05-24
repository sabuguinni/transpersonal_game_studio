#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Perf_CullingSystem.generated.h"

UENUM(BlueprintType)
enum class EPerf_CullingMethod : uint8
{
    Distance        UMETA(DisplayName = "Distance Culling"),
    Frustum         UMETA(DisplayName = "Frustum Culling"),
    Occlusion       UMETA(DisplayName = "Occlusion Culling"),
    Combined        UMETA(DisplayName = "Combined Culling")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CullingStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Culling Stats")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Culling Stats")
    int32 VisibleActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Culling Stats")
    int32 CulledActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Culling Stats")
    float CullingEfficiency = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Culling Stats")
    float LastCullingTime = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_ActorCullingData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Culling Data")
    AActor* Actor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Culling Data")
    float DistanceToCamera = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Culling Data")
    bool bIsVisible = true;

    UPROPERTY(BlueprintReadWrite, Category = "Culling Data")
    bool bWasCulled = false;

    UPROPERTY(BlueprintReadWrite, Category = "Culling Data")
    float LastUpdateTime = 0.0f;
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_CullingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_CullingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Main culling functions
    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void PerformCulling();

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void SetCullingMethod(EPerf_CullingMethod NewMethod);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    FPerf_CullingStats GetCullingStats() const;

    // Distance culling
    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void SetDistanceCullingThresholds(float NearDistance, float FarDistance);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    bool IsActorWithinCullingDistance(AActor* Actor, float Distance) const;

    // Frustum culling
    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    bool IsActorInCameraFrustum(AActor* Actor) const;

    // Actor management
    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void RegisterActorForCulling(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void UnregisterActorFromCulling(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void ClearCullingRegistry();

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Performance|Culling", CallInEditor)
    void DebugDrawCullingInfo();

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void EnableCullingDebug(bool bEnable);

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling Settings")
    EPerf_CullingMethod CullingMethod = EPerf_CullingMethod::Combined;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling Settings")
    float CullingUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Culling")
    float NearCullingDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Culling")
    float FarCullingDistance = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustum Culling")
    float FrustumPadding = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerFrame = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableOcclusionCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowCullingDebug = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bLogCullingStats = false;

private:
    // Internal state
    TArray<FPerf_ActorCullingData> CullingRegistry;
    FPerf_CullingStats CurrentStats;
    float LastCullingUpdate = 0.0f;
    int32 CurrentFrameIndex = 0;

    // Camera reference
    UPROPERTY()
    UCameraComponent* CachedCameraComponent = nullptr;

    // Internal functions
    void UpdateCullingRegistry();
    void PerformDistanceCulling();
    void PerformFrustumCulling();
    void PerformOcclusionCulling();
    void UpdateCullingStats();
    void ApplyCullingToActor(AActor* Actor, bool bShouldCull);
    UCameraComponent* GetPlayerCamera();
    FVector GetCameraLocation() const;
    FRotator GetCameraRotation() const;
};