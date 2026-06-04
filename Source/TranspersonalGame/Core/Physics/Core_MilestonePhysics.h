#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "Core_MilestonePhysics.generated.h"

/**
 * MILESTONE 1 PHYSICS INTEGRATION COMPONENT
 * 
 * Implements core physics requirements for "WALK AROUND" milestone:
 * - ThirdPersonCharacter movement validation (WASD, camera, jump)
 * - Terrain interaction physics (height adaptation, slope handling)
 * - Basic collision detection for dinosaur meshes
 * - Physics performance monitoring for 60fps target
 * 
 * This component integrates with Engine Architect's MilestoneValidator
 * to provide real-time physics validation for Milestone 1 completion.
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_MovementValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bWASDMovementActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement") 
    bool bCameraControlActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bJumpFunctional = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float MovementSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float CameraYaw = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float CameraPitch = 0.0f;

    FCore_MovementValidation()
    {
        bWASDMovementActive = false;
        bCameraControlActive = false;
        bJumpFunctional = false;
        MovementSpeed = 0.0f;
        CameraYaw = 0.0f;
        CameraPitch = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainPhysicsData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    bool bLandscapeDetected = false;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    float TerrainHeightVariation = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    float CurrentSlope = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    bool bCharacterGrounded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    FVector GroundNormal = FVector::ZeroVector;

    FCore_TerrainPhysicsData()
    {
        bLandscapeDetected = false;
        TerrainHeightVariation = 0.0f;
        CurrentSlope = 0.0f;
        bCharacterGrounded = false;
        GroundNormal = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsPerformance
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CollisionChecksPerFrame = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPerformanceTargetMet = false;

    FCore_PhysicsPerformance()
    {
        CurrentFPS = 0.0f;
        PhysicsTickTime = 0.0f;
        ActiveRigidBodies = 0;
        CollisionChecksPerFrame = 0;
        bPerformanceTargetMet = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_MilestonePhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_MilestonePhysics();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // MILESTONE 1 VALIDATION METHODS
    
    /** Validate character movement system for WALK AROUND milestone */
    UFUNCTION(BlueprintCallable, Category = "Milestone Physics")
    bool ValidateCharacterMovement(ACharacter* Character);

    /** Validate terrain physics interaction */
    UFUNCTION(BlueprintCallable, Category = "Milestone Physics")
    bool ValidateTerrainPhysics(const FVector& CharacterLocation);

    /** Validate dinosaur collision meshes */
    UFUNCTION(BlueprintCallable, Category = "Milestone Physics")
    bool ValidateDinosaurCollision();

    /** Check physics performance meets 60fps target */
    UFUNCTION(BlueprintCallable, Category = "Milestone Physics")
    bool ValidatePhysicsPerformance();

    /** Get comprehensive milestone physics status */
    UFUNCTION(BlueprintCallable, Category = "Milestone Physics")
    float GetMilestonePhysicsProgress();

    // REAL-TIME MONITORING

    /** Get current movement validation data */
    UFUNCTION(BlueprintPure, Category = "Milestone Physics")
    FCore_MovementValidation GetMovementValidation() const { return MovementValidation; }

    /** Get current terrain physics data */
    UFUNCTION(BlueprintPure, Category = "Milestone Physics")
    FCore_TerrainPhysicsData GetTerrainPhysicsData() const { return TerrainPhysicsData; }

    /** Get current physics performance data */
    UFUNCTION(BlueprintPure, Category = "Milestone Physics")
    FCore_PhysicsPerformance GetPhysicsPerformance() const { return PhysicsPerformance; }

    // MILESTONE INTEGRATION

    /** Called by MilestoneValidator to check physics requirements */
    UFUNCTION(BlueprintCallable, Category = "Milestone Physics")
    bool CheckMilestone1Requirements();

    /** Get detailed physics status for milestone reporting */
    UFUNCTION(BlueprintCallable, Category = "Milestone Physics")
    FString GetPhysicsStatusReport();

protected:
    // VALIDATION DATA
    
    UPROPERTY(BlueprintReadOnly, Category = "Milestone Physics", meta = (AllowPrivateAccess = "true"))
    FCore_MovementValidation MovementValidation;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone Physics", meta = (AllowPrivateAccess = "true"))
    FCore_TerrainPhysicsData TerrainPhysicsData;

    UPROPERTY(BlueprintReadOnly, Category = "Milestone Physics", meta = (AllowPrivateAccess = "true"))
    FCore_PhysicsPerformance PhysicsPerformance;

    // VALIDATION SETTINGS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone Physics", meta = (AllowPrivateAccess = "true"))
    float ValidationInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone Physics", meta = (AllowPrivateAccess = "true"))
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone Physics", meta = (AllowPrivateAccess = "true"))
    float MinTerrainHeightVariation = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone Physics", meta = (AllowPrivateAccess = "true"))
    int32 MinDinosaurMeshes = 3;

    // INTERNAL STATE

    UPROPERTY()
    float LastValidationTime = 0.0f;

    UPROPERTY()
    TWeakObjectPtr<ACharacter> CachedCharacter;

    UPROPERTY()
    TWeakObjectPtr<ALandscape> CachedLandscape;

    UPROPERTY()
    TArray<TWeakObjectPtr<AStaticMeshActor>> CachedDinosaurMeshes;

private:
    // INTERNAL VALIDATION METHODS
    
    void UpdateMovementValidation();
    void UpdateTerrainPhysicsData();
    void UpdatePhysicsPerformance();
    void CacheWorldReferences();
    bool FindCharacterInWorld();
    bool FindLandscapeInWorld();
    bool FindDinosaurMeshes();
    float CalculateTerrainHeightVariation(const FVector& Center, float Radius);
    FVector GetGroundNormal(const FVector& Location);
    bool IsValidDinosaurMesh(AStaticMeshActor* MeshActor);
};