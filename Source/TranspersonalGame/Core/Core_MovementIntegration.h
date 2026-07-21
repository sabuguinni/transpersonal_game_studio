#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core_MovementIntegration.generated.h"

class UCore_TerrainPhysics;
class UCore_CollisionSystem;
class UCore_PhysicsManager;

/**
 * Movement Integration System
 * Integrates character movement with physics systems for realistic prehistoric survival gameplay
 * Handles terrain adaptation, collision response, and physics-based movement
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API ACore_MovementIntegration : public AActor
{
    GENERATED_BODY()

public:
    ACore_MovementIntegration();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core physics system references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Integration")
    UCore_TerrainPhysics* TerrainPhysics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Integration")
    UCore_CollisionSystem* CollisionSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Integration")
    UCore_PhysicsManager* PhysicsManager;

    // Movement integration settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float TerrainAdaptationSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float SlopeLimit = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float UnstableTerrainSpeedMultiplier = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    bool bEnableRealisticPhysics = true;

    // Character tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Tracking")
    TArray<ACharacter*> TrackedCharacters;

public:
    // Movement integration functions
    UFUNCTION(BlueprintCallable, Category = "Movement Integration")
    void RegisterCharacter(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Movement Integration")
    void UnregisterCharacter(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Movement Integration")
    bool IsTerrainStable(const FVector& Location, float Radius = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Movement Integration")
    float GetTerrainSlope(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Movement Integration")
    FVector GetTerrainNormal(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Movement Integration")
    void ApplyTerrainMovementModifier(ACharacter* Character, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement Integration")
    void HandleUnstableTerrain(ACharacter* Character, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Movement Integration")
    void UpdateCharacterMovement(ACharacter* Character, float DeltaTime);

    // Physics integration
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void IntegrateWithPhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ValidatePhysicsIntegration();

    // Editor testing functions
    UFUNCTION(CallInEditor, Category = "Testing")
    void TestMovementIntegration();

    UFUNCTION(CallInEditor, Category = "Testing")
    void SpawnTestCharacter();

private:
    // Internal movement calculations
    void CalculateTerrainInfluence(ACharacter* Character, float DeltaTime);
    void ApplyPhysicsCorrections(ACharacter* Character, float DeltaTime);
    void HandleCollisionResponse(ACharacter* Character, const FHitResult& Hit);
    
    // Validation helpers
    bool ValidateSystemReferences();
    void LogMovementState(ACharacter* Character);
};