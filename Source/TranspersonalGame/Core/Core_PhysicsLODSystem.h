#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Core_PhysicsLODSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsLODLevel : uint8
{
    LOD0_Full       UMETA(DisplayName = "LOD 0 - Full Physics"),
    LOD1_Reduced    UMETA(DisplayName = "LOD 1 - Reduced Physics"),
    LOD2_Simple     UMETA(DisplayName = "LOD 2 - Simple Physics"),
    LOD3_Static     UMETA(DisplayName = "LOD 3 - Static Only")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD0Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD1Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD2Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD3Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bUseDistanceCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bUseFrustumCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float UpdateFrequency;

    FCore_PhysicsLODSettings()
        : LOD0Distance(500.0f)
        , LOD1Distance(1000.0f)
        , LOD2Distance(2000.0f)
        , LOD3Distance(5000.0f)
        , bUseDistanceCulling(true)
        , bUseFrustumCulling(true)
        , UpdateFrequency(0.1f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsLODObject
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "LOD Object")
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Object")
    ECore_PhysicsLODLevel CurrentLOD;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Object")
    float DistanceToViewer;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Object")
    bool bIsInFrustum;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Object")
    bool bPhysicsEnabled;

    FCore_PhysicsLODObject()
        : Actor(nullptr)
        , CurrentLOD(ECore_PhysicsLODLevel::LOD0_Full)
        , DistanceToViewer(0.0f)
        , bIsInFrustum(true)
        , bPhysicsEnabled(true)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_PhysicsLODSystem : public AActor
{
    GENERATED_BODY()

public:
    ACore_PhysicsLODSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // LOD System Management
    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void EnableLODSystem(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    bool IsLODSystemEnabled() const;

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void RegisterPhysicsObject(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void UnregisterPhysicsObject(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void SetLODSettings(const FCore_PhysicsLODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    FCore_PhysicsLODSettings GetLODSettings() const;

    // LOD Level Management
    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void SetActorLODLevel(AActor* Actor, ECore_PhysicsLODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    ECore_PhysicsLODLevel GetActorLODLevel(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void ForceUpdateLOD();

    // Debug and Statistics
    UFUNCTION(BlueprintCallable, Category = "Physics LOD", CallInEditor)
    void LogLODStatistics();

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    int32 GetRegisteredObjectCount() const;

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    int32 GetActivePhysicsObjectCount() const;

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void OptimizeForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void OptimizeForQuality();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    FCore_PhysicsLODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    bool bLODSystemEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    bool bAutoRegisterActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float MaxProcessingTimePerFrame;

    UPROPERTY(BlueprintReadOnly, Category = "LOD System")
    TArray<FCore_PhysicsLODObject> RegisteredObjects;

    UPROPERTY(BlueprintReadOnly, Category = "LOD System")
    float TimeSinceLastUpdate;

private:
    // Core LOD functions
    void UpdateLODLevels();
    void ProcessLODObject(FCore_PhysicsLODObject& LODObject);
    ECore_PhysicsLODLevel CalculateLODLevel(const FCore_PhysicsLODObject& LODObject) const;
    
    // Distance and culling calculations
    float CalculateDistanceToViewer(AActor* Actor) const;
    bool IsActorInFrustum(AActor* Actor) const;
    FVector GetViewerLocation() const;
    
    // Physics state management
    void ApplyLODLevel(AActor* Actor, ECore_PhysicsLODLevel LODLevel);
    void EnableFullPhysics(AActor* Actor);
    void EnableReducedPhysics(AActor* Actor);
    void EnableSimplePhysics(AActor* Actor);
    void DisablePhysics(AActor* Actor);
    
    // Auto-registration
    void AutoRegisterNearbyActors();
    bool ShouldAutoRegisterActor(AActor* Actor) const;
    
    // Performance tracking
    int32 ObjectsProcessedThisFrame;
    float FrameProcessingTime;
    
    // Cached viewer data
    FVector CachedViewerLocation;
    float ViewerLocationCacheTime;
    float ViewerLocationCacheInterval;
};