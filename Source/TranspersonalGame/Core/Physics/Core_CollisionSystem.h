// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "Core_CollisionSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCollisionSystem, Log, All);

/**
 * Collision Event Types for the Core Collision System
 */
UENUM(BlueprintType)
enum class ECore_CollisionEventType : uint8
{
    None            UMETA(DisplayName = "None"),
    CharacterHit    UMETA(DisplayName = "Character Hit"),
    DinosaurImpact  UMETA(DisplayName = "Dinosaur Impact"),
    ObjectCollision UMETA(DisplayName = "Object Collision"),
    TerrainContact  UMETA(DisplayName = "Terrain Contact"),
    WaterEntry      UMETA(DisplayName = "Water Entry"),
    FallDamage      UMETA(DisplayName = "Fall Damage")
};

/**
 * Collision Response Types
 */
UENUM(BlueprintType)
enum class ECore_CollisionResponse : uint8
{
    Ignore      UMETA(DisplayName = "Ignore"),
    Block       UMETA(DisplayName = "Block"),
    Overlap     UMETA(DisplayName = "Overlap"),
    Damage      UMETA(DisplayName = "Damage"),
    Ragdoll     UMETA(DisplayName = "Trigger Ragdoll"),
    Destruction UMETA(DisplayName = "Trigger Destruction")
};

/**
 * Collision Data Structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_CollisionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    ECore_CollisionEventType EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    ECore_CollisionResponse ResponseType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FVector ImpactPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FVector ImpactNormal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float ImpactForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    AActor* HitActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    UPrimitiveComponent* HitComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float Timestamp;

    FCore_CollisionData()
    {
        EventType = ECore_CollisionEventType::None;
        ResponseType = ECore_CollisionResponse::Ignore;
        ImpactPoint = FVector::ZeroVector;
        ImpactNormal = FVector::ZeroVector;
        ImpactForce = 0.0f;
        HitActor = nullptr;
        HitComponent = nullptr;
        Timestamp = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollisionEvent, const FCore_CollisionData&, CollisionData);

/**
 * Core Collision System Component
 * 
 * Handles all collision detection, response, and damage calculation for the game.
 * Integrates with physics system for realistic collision responses.
 * 
 * Features:
 * - Multi-layered collision detection (character, dinosaur, environment)
 * - Dynamic collision response based on impact force and object types
 * - Integration with ragdoll and destruction systems
 * - Performance-optimized collision queries with spatial partitioning
 * - Realistic fall damage and impact calculations
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_CollisionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CollisionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Collision Detection Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    bool bEnableCollisionDetection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    float CollisionCheckRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    float MinImpactForceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    float MaxImpactForceThreshold;

    // Damage Calculation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Settings")
    float FallDamageThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Settings")
    float FallDamageMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Settings")
    float ImpactDamageMultiplier;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxCollisionChecksPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CollisionUpdateInterval;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCollisionEvent OnCollisionDetected;

    // Core Collision Functions
    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool DetectCollision(AActor* Actor, FCore_CollisionData& OutCollisionData);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void ProcessCollisionResponse(const FCore_CollisionData& CollisionData);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    float CalculateImpactForce(const FVector& Velocity, float Mass, const FVector& Normal);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    float CalculateFallDamage(float FallHeight, float CharacterMass);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void RegisterCollisionEvent(const FCore_CollisionData& CollisionData);

    // Collision Query Functions
    UFUNCTION(BlueprintCallable, Category = "Collision Query")
    TArray<AActor*> GetNearbyActors(const FVector& Location, float Radius, TSubclassOf<AActor> ActorClass = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Collision Query")
    bool LineTraceForCollision(const FVector& Start, const FVector& End, FHitResult& OutHit, bool bIgnoreSelf = true);

    UFUNCTION(BlueprintCallable, Category = "Collision Query")
    bool SphereTraceForCollision(const FVector& Start, const FVector& End, float Radius, FHitResult& OutHit);

    // Collision Response Functions
    UFUNCTION(BlueprintCallable, Category = "Collision Response")
    void ApplyCollisionDamage(AActor* Actor, float Damage, const FVector& ImpactPoint);

    UFUNCTION(BlueprintCallable, Category = "Collision Response")
    void TriggerRagdollFromCollision(AActor* Actor, const FVector& ImpactForce, const FVector& ImpactPoint);

    UFUNCTION(BlueprintCallable, Category = "Collision Response")
    void TriggerDestructionFromCollision(AActor* Actor, float DestructionThreshold);

    // System Management
    UFUNCTION(BlueprintCallable, Category = "System")
    void SetCollisionEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "System")
    void ResetCollisionSystem();

    UFUNCTION(BlueprintCallable, Category = "System")
    void UpdateCollisionSettings(float NewRadius, float NewThreshold);

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void DebugDrawCollisionSphere(float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogCollisionStats();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Collision")
    int32 GetActiveCollisionCount() const { return ActiveCollisions.Num(); }

    UFUNCTION(BlueprintPure, Category = "Collision")
    float GetLastCollisionTime() const { return LastCollisionTime; }

    UFUNCTION(BlueprintPure, Category = "Collision")
    bool IsCollisionSystemEnabled() const { return bEnableCollisionDetection; }

protected:
    // Internal collision tracking
    UPROPERTY()
    TArray<FCore_CollisionData> ActiveCollisions;

    UPROPERTY()
    TArray<FCore_CollisionData> CollisionHistory;

    // Performance tracking
    float LastUpdateTime;
    float LastCollisionTime;
    int32 CollisionChecksThisFrame;

    // Internal Functions
    void UpdateCollisionDetection(float DeltaTime);
    void ProcessActiveCollisions();
    void CleanupOldCollisions();
    bool ShouldProcessCollision(const FCore_CollisionData& CollisionData);
    ECore_CollisionResponse DetermineCollisionResponse(AActor* Actor1, AActor* Actor2, float ImpactForce);
    void OptimizeCollisionQueries();

    // Collision Type Specific Functions
    void HandleCharacterCollision(const FCore_CollisionData& CollisionData);
    void HandleDinosaurCollision(const FCore_CollisionData& CollisionData);
    void HandleEnvironmentCollision(const FCore_CollisionData& CollisionData);
    void HandleTerrainCollision(const FCore_CollisionData& CollisionData);

    // Performance Optimization
    void UpdateSpatialPartitioning();
    bool IsWithinPerformanceBudget() const;
};