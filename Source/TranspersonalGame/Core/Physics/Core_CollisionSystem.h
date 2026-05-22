#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Core_CollisionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_CollisionType : uint8
{
    None = 0,
    Static = 1,
    Dynamic = 2,
    Kinematic = 3,
    Trigger = 4,
    Destructible = 5
};

USTRUCT(BlueprintType)
struct FCore_CollisionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    ECore_CollisionType CollisionType = ECore_CollisionType::Static;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float Mass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bGenerateHitEvents = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bCanEverAffectNavigation = true;

    FCore_CollisionData()
    {
        CollisionType = ECore_CollisionType::Static;
        Mass = 1.0f;
        Friction = 0.7f;
        Restitution = 0.3f;
        bGenerateHitEvents = true;
        bCanEverAffectNavigation = true;
    }
};

USTRUCT(BlueprintType)
struct FCore_CollisionEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Event")
    AActor* HitActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Event")
    FVector ImpactPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Event")
    FVector ImpactNormal = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Event")
    float ImpactForce = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Event")
    float Timestamp = 0.0f;

    FCore_CollisionEvent()
    {
        HitActor = nullptr;
        ImpactPoint = FVector::ZeroVector;
        ImpactNormal = FVector::ZeroVector;
        ImpactForce = 0.0f;
        Timestamp = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCore_CollisionEvent, const FCore_CollisionEvent&, CollisionEvent, AActor*, OtherActor);

/**
 * Core Collision System - Manages collision detection and response for all game objects
 * Handles static/dynamic collisions, trigger events, and physics interactions
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_CollisionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CollisionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Collision Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System")
    FCore_CollisionData CollisionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System")
    bool bEnableCollisionSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System")
    float CollisionCheckRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System")
    float MaxCollisionForce = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision System")
    TArray<TEnumAsByte<EObjectTypeQuery>> CollisionObjectTypes;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Collision Events")
    FOnCore_CollisionEvent OnCollisionDetected;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void InitializeCollisionSystem();

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void SetCollisionType(ECore_CollisionType NewType);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    bool CheckCollisionAtLocation(const FVector& Location, float Radius = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    TArray<AActor*> GetOverlappingActors(float Radius = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void ApplyImpactForce(const FVector& ImpactPoint, const FVector& Force);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void RegisterCollisionEvent(AActor* HitActor, const FVector& ImpactPoint, const FVector& ImpactNormal, float Force);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void EnableCollisionSystem(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Collision System")
    void UpdateCollisionProperties();

    // Collision Response
    UFUNCTION(BlueprintCallable, Category = "Collision Response")
    void HandleStaticCollision(const FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Collision Response")
    void HandleDynamicCollision(const FHitResult& HitResult, float ImpactVelocity);

    UFUNCTION(BlueprintCallable, Category = "Collision Response")
    void HandleTriggerCollision(AActor* TriggerActor);

protected:
    // Internal collision tracking
    UPROPERTY()
    TArray<FCore_CollisionEvent> RecentCollisions;

    UPROPERTY()
    float LastCollisionTime = 0.0f;

    UPROPERTY()
    int32 CollisionEventCount = 0;

    // Helper functions
    void ProcessCollisionQueue();
    void CleanupOldCollisions();
    float CalculateImpactForce(const FVector& Velocity, float Mass);
    bool IsValidCollisionTarget(AActor* Actor);
};