#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "SharedTypes.h"
#include "Core_PhysicsImpactSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnImpactProcessed, AActor*, Actor, FVector, ImpactLocation, float, ImpactForce, ECore_ImpactType, ImpactType);

/**
 * Specialized physics impact system for handling collision impacts, falls, and explosions
 * Focuses on realistic impact physics for dinosaur survival gameplay
 * Handles footprint creation, object destruction, and environmental deformation
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UCore_PhysicsImpactSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsImpactSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Impact processing methods
    UFUNCTION(BlueprintCallable, Category = "Physics Impact")
    void ProcessFootstepImpact(AActor* Actor, const FVector& FootLocation, float ActorMass, const FVector& Velocity);

    UFUNCTION(BlueprintCallable, Category = "Physics Impact")
    void ProcessFallImpact(AActor* Actor, const FVector& ImpactLocation, float FallHeight, float ImpactVelocity);

    UFUNCTION(BlueprintCallable, Category = "Physics Impact")
    void ProcessCollisionImpact(AActor* ActorA, AActor* ActorB, const FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Physics Impact")
    void ProcessExplosionImpact(const FVector& ExplosionCenter, float ExplosionRadius, float ExplosionForce);

    // Impact force calculation
    UFUNCTION(BlueprintCallable, Category = "Physics Impact", BlueprintPure)
    float CalculateImpactForce(float Mass, const FVector& Velocity, float ContactArea = 1.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Physics Impact", BlueprintPure)
    float CalculateFallDamage(float FallHeight, float ActorMass, ECore_SurfaceType SurfaceType) const;

    UFUNCTION(BlueprintCallable, Category = "Physics Impact", BlueprintPure)
    ECore_ImpactType DetermineImpactType(float ImpactForce, const FVector& ImpactVelocity) const;

    // Environmental impact effects
    UFUNCTION(BlueprintCallable, Category = "Physics Impact")
    void CreateFootprint(const FVector& Location, float FootSize, float Depth, AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Impact")
    void CreateImpactCrater(const FVector& Location, float CraterRadius, float CraterDepth, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Physics Impact")
    void CreateDebrisField(const FVector& Location, float DebrisRadius, int32 DebrisCount, float DebrisForce);

    // Impact response configuration
    UFUNCTION(BlueprintCallable, Category = "Physics Impact")
    void SetImpactThresholds(float MinForce, float MaxForce, float MinVelocity, float MaxVelocity);

    UFUNCTION(BlueprintCallable, Category = "Physics Impact")
    void SetSurfaceProperties(ECore_SurfaceType SurfaceType, float Hardness, float Absorption, float Restitution);

    // Impact statistics
    UFUNCTION(BlueprintCallable, Category = "Physics Impact", BlueprintPure)
    int32 GetTotalImpactsProcessed() const { return TotalImpactsProcessed; }

    UFUNCTION(BlueprintCallable, Category = "Physics Impact", BlueprintPure)
    float GetAverageImpactForce() const { return AverageImpactForce; }

    UFUNCTION(BlueprintCallable, Category = "Physics Impact", BlueprintPure)
    float GetLargestImpactForce() const { return LargestImpactForce; }

    // Event delegates
    UPROPERTY(BlueprintAssignable, Category = "Physics Impact Events")
    FOnImpactProcessed OnImpactProcessed;

protected:
    // Impact thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings", meta = (AllowPrivateAccess = "true"))
    float MinImpactForceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings", meta = (AllowPrivateAccess = "true"))
    float MaxImpactForceProcessed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings", meta = (AllowPrivateAccess = "true"))
    float MinImpactVelocityThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings", meta = (AllowPrivateAccess = "true"))
    float MaxImpactVelocityProcessed;

    // Footprint settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint Settings", meta = (AllowPrivateAccess = "true"))
    float MinFootprintForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint Settings", meta = (AllowPrivateAccess = "true"))
    float MaxFootprintDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint Settings", meta = (AllowPrivateAccess = "true"))
    float FootprintDecayTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprint Settings", meta = (AllowPrivateAccess = "true"))
    bool bCreateDinosaurFootprints;

    // Crater settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater Settings", meta = (AllowPrivateAccess = "true"))
    float MinCraterForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater Settings", meta = (AllowPrivateAccess = "true"))
    float MaxCraterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater Settings", meta = (AllowPrivateAccess = "true"))
    float MaxCraterDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crater Settings", meta = (AllowPrivateAccess = "true"))
    bool bCreateImpactCraters;

    // Debris settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris Settings", meta = (AllowPrivateAccess = "true"))
    float MinDebrisForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris Settings", meta = (AllowPrivateAccess = "true"))
    int32 MaxDebrisCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris Settings", meta = (AllowPrivateAccess = "true"))
    float DebrisLifetime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris Settings", meta = (AllowPrivateAccess = "true"))
    bool bCreateImpactDebris;

    // Surface properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Properties", meta = (AllowPrivateAccess = "true"))
    TMap<ECore_SurfaceType, float> SurfaceHardness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Properties", meta = (AllowPrivateAccess = "true"))
    TMap<ECore_SurfaceType, float> SurfaceAbsorption;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Properties", meta = (AllowPrivateAccess = "true"))
    TMap<ECore_SurfaceType, float> SurfaceRestitution;

    // Impact statistics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Impact Statistics", meta = (AllowPrivateAccess = "true"))
    int32 TotalImpactsProcessed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Impact Statistics", meta = (AllowPrivateAccess = "true"))
    float AverageImpactForce;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Impact Statistics", meta = (AllowPrivateAccess = "true"))
    float LargestImpactForce;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Impact Statistics", meta = (AllowPrivateAccess = "true"))
    TArray<FVector> RecentImpactLocations;

private:
    // Internal processing methods
    void ProcessImpactEffects(const FVector& Location, float ImpactForce, ECore_ImpactType ImpactType, AActor* CausingActor);
    ECore_SurfaceType DetermineSurfaceType(const FVector& Location) const;
    float GetSurfaceProperty(ECore_SurfaceType SurfaceType, const FString& PropertyName) const;
    
    // Environmental modification
    void ModifyTerrain(const FVector& Location, float Radius, float Depth);
    void SpawnDebrisActors(const FVector& Location, int32 Count, float Force);
    
    // Statistics tracking
    void UpdateImpactStatistics(float ImpactForce, const FVector& Location);
    void CleanupOldImpacts();

    // Internal state
    float ImpactForceAccumulator;
    int32 ImpactFrameCounter;
    TArray<AActor*> SpawnedDebrisActors;
    TMap<FVector, float> ActiveFootprints;
    TMap<FVector, float> ActiveCraters;
};