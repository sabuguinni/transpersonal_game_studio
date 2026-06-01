#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "Core_CollisionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_CollisionType : uint8
{
    None            UMETA(DisplayName = "No Collision"),
    Block           UMETA(DisplayName = "Block"),
    Overlap         UMETA(DisplayName = "Overlap"),
    Ignore          UMETA(DisplayName = "Ignore")
};

UENUM(BlueprintType)
enum class ECore_SurfaceType : uint8
{
    Default         UMETA(DisplayName = "Default"),
    Rock            UMETA(DisplayName = "Rock"),
    Dirt            UMETA(DisplayName = "Dirt"),
    Grass           UMETA(DisplayName = "Grass"),
    Water           UMETA(DisplayName = "Water"),
    Wood            UMETA(DisplayName = "Wood"),
    Metal           UMETA(DisplayName = "Metal"),
    Flesh           UMETA(DisplayName = "Flesh"),
    Bone            UMETA(DisplayName = "Bone"),
    Sand            UMETA(DisplayName = "Sand"),
    Mud             UMETA(DisplayName = "Mud"),
    Snow            UMETA(DisplayName = "Snow"),
    Ice             UMETA(DisplayName = "Ice")
};

USTRUCT(BlueprintType)
struct FCore_CollisionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FVector ImpactPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FVector ImpactNormal = FVector::UpVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float ImpactForce = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    ECore_SurfaceType SurfaceType = ECore_SurfaceType::Default;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    AActor* HitActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    UPrimitiveComponent* HitComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float PenetrationDepth = 0.0f;

    FCore_CollisionData()
    {
        ImpactPoint = FVector::ZeroVector;
        ImpactNormal = FVector::UpVector;
        ImpactForce = 0.0f;
        SurfaceType = ECore_SurfaceType::Default;
        HitActor = nullptr;
        HitComponent = nullptr;
        PenetrationDepth = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCore_OnCollisionDetected, const FCore_CollisionData&, CollisionData, float, DeltaTime);

/**
 * Core Collision System - Advanced collision detection and response
 * Handles surface type detection, collision filtering, and impact calculations
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_CollisionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CollisionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Collision Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    bool bEnableAdvancedCollision = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    float CollisionScanRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    float MinImpactForceThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    TArray<TEnumAsByte<EObjectTypeQuery>> CollisionObjectTypes;

    // Surface Detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Detection")
    TMap<FString, ECore_SurfaceType> SurfaceTypeMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Detection")
    bool bAutoDetectSurfaceTypes = true;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FCore_OnCollisionDetected OnCollisionDetected;

    // Collision Detection Functions
    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool PerformLineTrace(const FVector& Start, const FVector& End, FCore_CollisionData& OutCollisionData);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool PerformSphereTrace(const FVector& Start, const FVector& End, float Radius, FCore_CollisionData& OutCollisionData);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool PerformBoxTrace(const FVector& Start, const FVector& End, const FVector& HalfSize, FCore_CollisionData& OutCollisionData);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    TArray<FCore_CollisionData> GetNearbyCollisions(const FVector& Center, float Radius);

    // Surface Type Functions
    UFUNCTION(BlueprintCallable, Category = "Surface")
    ECore_SurfaceType GetSurfaceTypeAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Surface")
    void RegisterSurfaceType(const FString& MaterialName, ECore_SurfaceType SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "Surface")
    float GetSurfaceFriction(ECore_SurfaceType SurfaceType) const;

    UFUNCTION(BlueprintCallable, Category = "Surface")
    float GetSurfaceBounciness(ECore_SurfaceType SurfaceType) const;

    // Impact Calculation
    UFUNCTION(BlueprintCallable, Category = "Impact")
    float CalculateImpactForce(const FVector& Velocity, float Mass, const FVector& Normal);

    UFUNCTION(BlueprintCallable, Category = "Impact")
    FVector CalculateReflectionVector(const FVector& IncomingVector, const FVector& Normal, float Bounciness);

    UFUNCTION(BlueprintCallable, Category = "Impact")
    void ProcessCollisionImpact(const FCore_CollisionData& CollisionData, AActor* ImpactActor);

    // Collision Filtering
    UFUNCTION(BlueprintCallable, Category = "Filtering")
    void SetCollisionResponseToChannel(ECollisionChannel Channel, ECore_CollisionType Response);

    UFUNCTION(BlueprintCallable, Category = "Filtering")
    void AddIgnoredActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Filtering")
    void RemoveIgnoredActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Filtering")
    void ClearIgnoredActors();

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugDrawCollisionSphere();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogCollisionStatistics();

private:
    // Internal data
    TArray<AActor*> IgnoredActors;
    TArray<FCore_CollisionData> RecentCollisions;
    
    // Performance tracking
    int32 CollisionChecksThisFrame = 0;
    float LastCollisionTime = 0.0f;
    
    // Helper functions
    void InitializeSurfaceTypes();
    ECore_SurfaceType DetermineSurfaceType(const FHitResult& HitResult);
    FCollisionQueryParams BuildQueryParams() const;
    void ProcessHitResult(const FHitResult& HitResult, FCore_CollisionData& OutCollisionData);
    void UpdateCollisionStatistics();
};