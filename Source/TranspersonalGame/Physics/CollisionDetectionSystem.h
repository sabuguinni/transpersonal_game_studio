#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "CollisionQueryParams.h"
#include "WorldCollision.h"
#include "CollisionDetectionSystem.generated.h"

// Forward declarations
class UStaticMeshComponent;
class USkeletalMeshComponent;
class UBoxComponent;
class USphereComponent;
class UCapsuleComponent;

// Enums for collision detection
UENUM(BlueprintType)
enum class ECollisionTraceType : uint8
{
    Line            UMETA(DisplayName = "Line Trace"),
    Sphere          UMETA(DisplayName = "Sphere Trace"),
    Box             UMETA(DisplayName = "Box Trace"),
    Capsule         UMETA(DisplayName = "Capsule Trace"),
    Multi           UMETA(DisplayName = "Multi Trace")
};

UENUM(BlueprintType)
enum class ECollisionChannel : uint8
{
    WorldStatic     UMETA(DisplayName = "World Static"),
    WorldDynamic    UMETA(DisplayName = "World Dynamic"),
    Pawn            UMETA(DisplayName = "Pawn"),
    Visibility      UMETA(DisplayName = "Visibility"),
    Camera          UMETA(DisplayName = "Camera"),
    PhysicsBody     UMETA(DisplayName = "Physics Body"),
    Vehicle         UMETA(DisplayName = "Vehicle"),
    Destructible    UMETA(DisplayName = "Destructible"),
    Custom          UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class ECollisionPriority : uint8
{
    Low             UMETA(DisplayName = "Low Priority"),
    Medium          UMETA(DisplayName = "Medium Priority"),
    High            UMETA(DisplayName = "High Priority"),
    Critical        UMETA(DisplayName = "Critical Priority")
};

// Structs for collision configuration
USTRUCT(BlueprintType)
struct FCollisionTraceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    ECollisionTraceType TraceType = ECollisionTraceType::Line;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    float TraceDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    FVector TraceExtent = FVector(5.0f, 5.0f, 5.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    bool bTraceComplex = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    bool bIgnoreSelf = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    ECollisionPriority Priority = ECollisionPriority::Medium;
};

USTRUCT(BlueprintType)
struct FCollisionFilterSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter Settings")
    TArray<ECollisionChannel> IgnoreChannels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter Settings")
    TArray<AActor*> IgnoreActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter Settings")
    TArray<UClass*> IgnoreClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter Settings")
    float MinimumImpactVelocity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter Settings")
    bool bFilterByMass = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter Settings")
    float MinimumMass = 1.0f;
};

USTRUCT(BlueprintType)
struct FCollisionResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Collision Result")
    bool bHit = false;

    UPROPERTY(BlueprintReadOnly, Category = "Collision Result")
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Collision Result")
    FVector HitNormal = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Collision Result")
    AActor* HitActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Collision Result")
    UPrimitiveComponent* HitComponent = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Collision Result")
    float Distance = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Collision Result")
    FVector ImpactVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Collision Result")
    float ImpactForce = 0.0f;
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollisionDetected, const FCollisionResult&, CollisionResult, float, DeltaTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHighSpeedCollision, AActor*, Actor1, AActor*, Actor2, float, ImpactForce);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollisionEnter, UPrimitiveComponent*, MyComponent, UPrimitiveComponent*, OtherComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollisionExit, UPrimitiveComponent*, MyComponent, UPrimitiveComponent*, OtherComponent);

/**
 * Advanced collision detection system for prehistoric world interactions
 * Handles complex collision scenarios including dinosaur-environment, player-world, and object-object collisions
 * Optimized for performance with LOD-based collision complexity
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCollisionDetectionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCollisionDetectionSystem();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Detection")
    FCollisionTraceSettings TraceSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Detection")
    FCollisionFilterSettings FilterSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Detection")
    bool bEnableContinuousDetection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Detection")
    float DetectionFrequency = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseLODBasedDetection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1 = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance3 = 2000.0f;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Collision Events")
    FOnCollisionDetected OnCollisionDetected;

    UPROPERTY(BlueprintAssignable, Category = "Collision Events")
    FOnHighSpeedCollision OnHighSpeedCollision;

    UPROPERTY(BlueprintAssignable, Category = "Collision Events")
    FOnCollisionEnter OnCollisionEnter;

    UPROPERTY(BlueprintAssignable, Category = "Collision Events")
    FOnCollisionExit OnCollisionExit;

    // Public Methods
    UFUNCTION(BlueprintCallable, Category = "Collision Detection")
    FCollisionResult PerformLineTrace(const FVector& Start, const FVector& End);

    UFUNCTION(BlueprintCallable, Category = "Collision Detection")
    FCollisionResult PerformSphereTrace(const FVector& Start, const FVector& End, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Collision Detection")
    FCollisionResult PerformBoxTrace(const FVector& Start, const FVector& End, const FVector& HalfExtent);

    UFUNCTION(BlueprintCallable, Category = "Collision Detection")
    FCollisionResult PerformCapsuleTrace(const FVector& Start, const FVector& End, float Radius, float HalfHeight);

    UFUNCTION(BlueprintCallable, Category = "Collision Detection")
    TArray<FCollisionResult> PerformMultiTrace(const FVector& Start, const FVector& End);

    UFUNCTION(BlueprintCallable, Category = "Collision Detection")
    void SetTraceSettings(const FCollisionTraceSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Collision Detection")
    void SetFilterSettings(const FCollisionFilterSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Collision Detection")
    void AddIgnoreActor(AActor* ActorToIgnore);

    UFUNCTION(BlueprintCallable, Category = "Collision Detection")
    void RemoveIgnoreActor(AActor* ActorToRemove);

    UFUNCTION(BlueprintCallable, Category = "Collision Detection")
    void ClearIgnoreActors();

    UFUNCTION(BlueprintCallable, Category = "Collision Query")
    bool IsActorInRange(AActor* TargetActor, float Range) const;

    UFUNCTION(BlueprintCallable, Category = "Collision Query")
    TArray<AActor*> GetActorsInRadius(const FVector& Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Collision Query")
    AActor* GetNearestActor(const FVector& Location, float MaxRange = 1000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODDistances(float LOD1, float LOD2, float LOD3);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetCurrentLODLevel() const;

protected:
    // Component references
    UPROPERTY()
    TObjectPtr<UPrimitiveComponent> OwnerPrimitiveComponent;

    // Internal state
    UPROPERTY()
    float LastDetectionTime = 0.0f;

    UPROPERTY()
    int32 CurrentLODLevel = 0;

    UPROPERTY()
    TArray<FCollisionResult> PreviousCollisions;

    UPROPERTY()
    TSet<TObjectPtr<AActor>> CurrentlyOverlappingActors;

    // Core methods
    void InitializeCollisionSystem();
    void UpdateCollisionDetection(float DeltaTime);
    void UpdateLODLevel();
    
    // Trace methods
    FCollisionResult ExecuteTrace(const FVector& Start, const FVector& End, ECollisionTraceType TraceType);
    FCollisionQueryParams BuildQueryParams() const;
    FCollisionObjectQueryParams BuildObjectQueryParams() const;
    
    // Event processing
    void ProcessCollisionResult(const FCollisionResult& Result, float DeltaTime);
    void CheckForHighSpeedCollisions(const FCollisionResult& Result);
    void UpdateOverlapTracking();
    
    // Performance optimization
    bool ShouldPerformDetection(float DeltaTime) const;
    float GetLODBasedFrequency() const;
    
    // Utility methods
    FCollisionResult ConvertHitResult(const FHitResult& HitResult) const;
    float CalculateImpactForce(const FVector& Velocity, float Mass) const;
    bool PassesFilter(const FHitResult& HitResult) const;
    
    // Event handlers
    UFUNCTION()
    void OnOwnerHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION()
    void OnOwnerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOwnerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);
};