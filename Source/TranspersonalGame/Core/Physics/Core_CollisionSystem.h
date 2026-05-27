#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Collision/CollisionQueryParams.h"
#include "Core_CollisionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_CollisionType : uint8
{
    None            UMETA(DisplayName = "None"),
    Player          UMETA(DisplayName = "Player"),
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    Environment     UMETA(DisplayName = "Environment"),
    Projectile      UMETA(DisplayName = "Projectile"),
    Debris          UMETA(DisplayName = "Debris"),
    Trigger         UMETA(DisplayName = "Trigger"),
    Water           UMETA(DisplayName = "Water"),
    Vegetation      UMETA(DisplayName = "Vegetation")
};

USTRUCT(BlueprintType)
struct FCore_CollisionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    ECore_CollisionType CollisionType = ECore_CollisionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float ImpactForce = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FVector ImpactNormal = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    AActor* HitActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    UPrimitiveComponent* HitComponent = nullptr;

    FCore_CollisionData()
    {
        CollisionType = ECore_CollisionType::None;
        ImpactForce = 0.0f;
        ImpactLocation = FVector::ZeroVector;
        ImpactNormal = FVector::ZeroVector;
        HitActor = nullptr;
        HitComponent = nullptr;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollisionDetected, const FCore_CollisionData&, CollisionData, const FHitResult&, HitResult);

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_CollisionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CollisionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Collision detection methods
    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool PerformLineTrace(const FVector& Start, const FVector& End, FHitResult& OutHit, ECore_CollisionType TraceType = ECore_CollisionType::Environment);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool PerformSphereTrace(const FVector& Center, float Radius, FHitResult& OutHit, ECore_CollisionType TraceType = ECore_CollisionType::Environment);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool PerformBoxTrace(const FVector& Center, const FVector& HalfExtents, const FRotator& Rotation, FHitResult& OutHit, ECore_CollisionType TraceType = ECore_CollisionType::Environment);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void ProcessCollisionEvent(const FHitResult& HitResult, float ImpactForce);

    // Collision configuration
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetCollisionType(ECore_CollisionType NewType);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    ECore_CollisionType GetCollisionType() const { return CurrentCollisionType; }

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void EnableCollisionDetection(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool IsCollisionDetectionEnabled() const { return bCollisionEnabled; }

    // Physics interaction
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void ApplyImpactForce(UPrimitiveComponent* Component, const FVector& Force, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetPhysicsProperties(UPrimitiveComponent* Component, float Mass, float LinearDamping, float AngularDamping);

    // Event delegates
    UPROPERTY(BlueprintAssignable, Category = "Collision Events")
    FOnCollisionDetected OnCollisionDetected;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    ECore_CollisionType CurrentCollisionType = ECore_CollisionType::Environment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    bool bCollisionEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    float MinImpactForceThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    float MaxTraceDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings")
    bool bDebugTraces = false;

private:
    // Internal collision handling
    void HandleCollisionResponse(const FCore_CollisionData& CollisionData);
    ECollisionChannel GetCollisionChannelFromType(ECore_CollisionType Type);
    FCollisionQueryParams CreateQueryParams(ECore_CollisionType TraceType);
};