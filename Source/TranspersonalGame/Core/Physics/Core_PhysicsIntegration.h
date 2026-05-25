#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/HitResult.h"
#include "Core_PhysicsIntegration.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCore_PhysicsIntegration, Log, All);

/**
 * Enhanced physics integration for TranspersonalCharacter
 * Handles advanced physics interactions, terrain adaptation, and collision responses
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsIntegration();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Physics Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    float TerrainAdaptationStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    float CollisionResponseMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    float MaxTerrainSlope = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    bool bEnableAdvancedPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    bool bEnableTerrainIK = true;

    // Physics State
    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    FVector CurrentGroundNormal;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    float CurrentTerrainSlope;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    bool bIsOnValidTerrain;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    bool bIsOnUnevenTerrain;

    // Physics Functions
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdateTerrainAdaptation();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ApplyPhysicsCorrection(const FVector& CorrectionVector);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    bool PerformTerrainTrace(FHitResult& OutHit, const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    FVector CalculateTerrainNormal(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void HandleCollisionResponse(const FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void EnablePhysicsSimulation(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetPhysicsProperties(float Mass, float LinearDamping, float AngularDamping);

    // Enhanced Physics Features
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ApplyImpulseAtLocation(const FVector& Impulse, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetCollisionResponseToChannel(ECollisionChannel Channel, ECollisionResponse Response);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    bool CheckPhysicsStability();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void OptimizePhysicsPerformance();

    // Validation Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Integration")
    void ValidatePhysicsSetup();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics Integration")
    void TestPhysicsIntegration();

private:
    // Internal physics state
    float LastValidationTime;
    FVector LastKnownGoodPosition;
    bool bPhysicsValidated;

    // Helper functions
    void UpdatePhysicsState();
    void ValidateTerrainInteraction();
    bool IsPhysicsComponentValid() const;
    void LogPhysicsState() const;
};