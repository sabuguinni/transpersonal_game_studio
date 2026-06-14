#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "Perf_CullingSystem.generated.h"

UENUM(BlueprintType)
enum class EPerf_CullingType : uint8
{
    Distance = 0,
    Frustum = 1,
    Occlusion = 2,
    LOD = 3,
    Importance = 4
};

USTRUCT(BlueprintType)
struct FPerf_CullingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableDistanceCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float MaxDrawDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableFrustumCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableLODCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    int32 MaxVisibleActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float CullingUpdateInterval;

    FPerf_CullingSettings()
    {
        bEnableDistanceCulling = true;
        MaxDrawDistance = 10000.0f;
        bEnableFrustumCulling = true;
        bEnableOcclusionCulling = false;
        bEnableLODCulling = true;
        MaxVisibleActors = 1000;
        CullingUpdateInterval = 0.1f;
    }
};

USTRUCT(BlueprintType)
struct FPerf_CullableActor
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    float DistanceToCamera;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    bool bIsVisible;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    bool bIsCulled;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    EPerf_CullingType CullingReason;

    UPROPERTY(BlueprintReadOnly, Category = "Culling")
    int32 ImportanceScore;

    FPerf_CullableActor()
    {
        Actor = nullptr;
        DistanceToCamera = 0.0f;
        bIsVisible = true;
        bIsCulled = false;
        CullingReason = EPerf_CullingType::Distance;
        ImportanceScore = 0;
    }
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
    // Culling management
    UFUNCTION(BlueprintCallable, Category = "Culling")
    void RegisterCullableActor(AActor* Actor, int32 ImportanceScore = 0);

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void UnregisterCullableActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void UpdateCulling();

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void SetCullingSettings(const FPerf_CullingSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Culling")
    FPerf_CullingSettings GetCullingSettings() const { return CullingSettings; }

    UFUNCTION(BlueprintCallable, Category = "Culling")
    int32 GetVisibleActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Culling")
    int32 GetCulledActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Culling")
    TArray<FPerf_CullableActor> GetCullableActors() const { return CullableActors; }

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void EnableCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Culling")
    bool IsCullingEnabled() const { return bCullingEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void ForceUpdateCulling();

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void LogCullingStats();

private:
    UPROPERTY(EditAnywhere, Category = "Culling", meta = (AllowPrivateAccess = "true"))
    FPerf_CullingSettings CullingSettings;

    UPROPERTY(EditAnywhere, Category = "Culling", meta = (AllowPrivateAccess = "true"))
    bool bCullingEnabled;

    UPROPERTY(EditAnywhere, Category = "Culling", meta = (AllowPrivateAccess = "true"))
    bool bDebugDrawCulling;

    UPROPERTY(BlueprintReadOnly, Category = "Culling", meta = (AllowPrivateAccess = "true"))
    TArray<FPerf_CullableActor> CullableActors;

    float LastCullingUpdate;
    FVector LastCameraLocation;
    FRotator LastCameraRotation;

    // Culling methods
    bool IsActorInCameraFrustum(AActor* Actor, const FVector& CameraLocation, const FRotator& CameraRotation);
    bool IsActorOccluded(AActor* Actor, const FVector& CameraLocation);
    bool ShouldCullByDistance(AActor* Actor, const FVector& CameraLocation);
    bool ShouldCullByImportance(const FPerf_CullableActor& CullableActor, int32 VisibleCount);
    
    void UpdateActorVisibility(FPerf_CullableActor& CullableActor, bool bShouldBeVisible);
    void CalculateImportanceScores();
    
    FVector GetCameraLocation() const;
    FRotator GetCameraRotation() const;
    
    void DebugDrawCulling();
};