#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "Core_CollisionSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_CollisionProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FName ProfileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionResponse> ObjectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bGenerateOverlapEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bCanCharacterStepUpOn;

    FCore_CollisionProfile()
    {
        ProfileName = NAME_None;
        CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
        ObjectType = ECR_WorldStatic;
        bGenerateOverlapEvents = true;
        bCanCharacterStepUpOn = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_CollisionEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    AActor* Actor1;

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    AActor* Actor2;

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    FVector ImpactPoint;

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    FVector ImpactNormal;

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    float ImpactVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Collision")
    float Timestamp;

    FCore_CollisionEvent()
    {
        Actor1 = nullptr;
        Actor2 = nullptr;
        ImpactPoint = FVector::ZeroVector;
        ImpactNormal = FVector::UpVector;
        ImpactVelocity = 0.0f;
        Timestamp = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollisionEvent, const FCore_CollisionEvent&, CollisionEvent);

/**
 * Core Collision System - Manages collision detection, response, and optimization
 * Handles collision profiles, event broadcasting, and performance monitoring
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_CollisionSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCore_CollisionSystem();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Collision profile management
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void RegisterCollisionProfile(const FCore_CollisionProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    bool ApplyCollisionProfile(UPrimitiveComponent* Component, const FName& ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    FCore_CollisionProfile GetCollisionProfile(const FName& ProfileName) const;

    // Collision detection
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    bool LineTrace(const FVector& Start, const FVector& End, FHitResult& OutHit, 
                   ECollisionChannel TraceChannel = ECC_Visibility, 
                   const TArray<AActor*>& ActorsToIgnore = TArray<AActor*>()) const;

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    bool SphereTrace(const FVector& Start, const FVector& End, float Radius, 
                     FHitResult& OutHit, ECollisionChannel TraceChannel = ECC_Visibility,
                     const TArray<AActor*>& ActorsToIgnore = TArray<AActor*>()) const;

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    bool BoxTrace(const FVector& Start, const FVector& End, const FVector& HalfSize,
                  const FRotator& Orientation, FHitResult& OutHit,
                  ECollisionChannel TraceChannel = ECC_Visibility,
                  const TArray<AActor*>& ActorsToIgnore = TArray<AActor*>()) const;

    // Overlap detection
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    TArray<AActor*> GetOverlappingActors(const FVector& Location, float Radius,
                                         ECollisionChannel ObjectType = ECC_Pawn) const;

    // Event handling
    UPROPERTY(BlueprintAssignable, Category = "Collision System")
    FOnCollisionEvent OnCollisionDetected;

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void BroadcastCollisionEvent(const FCore_CollisionEvent& Event);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void EnableCollisionOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void SetCollisionLODDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    int32 GetActiveCollisionCount() const;

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    float GetCollisionPerformanceMetric() const;

    // Debugging
    UFUNCTION(BlueprintCallable, Category = "Collision System", CallInEditor = true)
    void DebugDrawCollisionShapes(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void LogCollisionStats() const;

protected:
    // Collision profiles registry
    UPROPERTY()
    TMap<FName, FCore_CollisionProfile> CollisionProfiles;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bOptimizationEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float CollisionLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxActiveCollisions;

    // Statistics
    UPROPERTY(BlueprintReadOnly, Category = "Statistics", meta = (AllowPrivateAccess = "true"))
    int32 CollisionEventsThisFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics", meta = (AllowPrivateAccess = "true"))
    float AverageCollisionProcessingTime;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics", meta = (AllowPrivateAccess = "true"))
    int32 TotalCollisionEvents;

    // Debug settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
    bool bDebugDrawEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
    float DebugDrawDuration;

private:
    // Internal methods
    void InitializeDefaultProfiles();
    void UpdatePerformanceMetrics(float DeltaTime);
    void OptimizeCollisions();
    bool ShouldProcessCollision(const FVector& Location) const;

    // Performance tracking
    float CollisionProcessingTimeAccumulator;
    int32 CollisionProcessingFrameCount;
    TArray<float> RecentCollisionTimes;
};