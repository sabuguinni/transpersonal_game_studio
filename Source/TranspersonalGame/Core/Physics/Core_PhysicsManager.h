#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodySetup.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core_PhysicsManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsProfile : uint8
{
    Default         UMETA(DisplayName = "Default"),
    Character       UMETA(DisplayName = "Character"),
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    Environment     UMETA(DisplayName = "Environment"),
    Projectile      UMETA(DisplayName = "Projectile"),
    Debris          UMETA(DisplayName = "Debris"),
    Ragdoll         UMETA(DisplayName = "Ragdoll")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bSimulatePhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Friction = 0.7f;

    FCore_PhysicsSettings()
    {
        Mass = 1.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.0f;
        bEnableGravity = true;
        bSimulatePhysics = true;
        Restitution = 0.3f;
        Friction = 0.7f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCore_OnPhysicsImpact, AActor*, Actor, FVector, ImpactLocation, float, ImpactForce);

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCore_PhysicsManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Physics profile management
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyPhysicsProfile(UPrimitiveComponent* Component, ECore_PhysicsProfile Profile);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    FCore_PhysicsSettings GetPhysicsProfileSettings(ECore_PhysicsProfile Profile) const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsProfileSettings(ECore_PhysicsProfile Profile, const FCore_PhysicsSettings& Settings);

    // Physics simulation control
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void EnablePhysicsSimulation(AActor* Actor, bool bEnable = true);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetActorMass(AActor* Actor, float NewMass);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void AddImpulseToActor(AActor* Actor, FVector Impulse, bool bVelChange = false);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void AddForceToActor(AActor* Actor, FVector Force, bool bAccelChange = false);

    // Collision detection
    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool LineTracePhysics(const FVector& Start, const FVector& End, FHitResult& HitResult, bool bTraceComplex = false);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool SphereTracePhysics(const FVector& Start, const FVector& End, float Radius, FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    TArray<AActor*> GetOverlappingActors(const FVector& Location, float Radius);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics")
    int32 GetActivePhysicsActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    float GetPhysicsFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsSubsteps(int32 MaxSubsteps, float FixedTimeStep);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Physics")
    FCore_OnPhysicsImpact OnPhysicsImpact;

    // Validation and debugging
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics")
    void ValidatePhysicsSetup();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics")
    void OptimizePhysicsPerformance();

protected:
    UPROPERTY()
    TMap<ECore_PhysicsProfile, FCore_PhysicsSettings> PhysicsProfiles;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;

    // Internal methods
    void InitializePhysicsProfiles();
    void UpdatePhysicsMetrics();
    void OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

    // Performance tracking
    float LastPhysicsFrameTime;
    int32 ActivePhysicsActorCount;

    // Timer handles
    FTimerHandle MetricsUpdateTimer;
};