#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Eng_MovementArchitecture.generated.h"

/**
 * Movement Architecture System for Milestone 1
 * Defines the core movement patterns and physics integration
 * for the prehistoric survival game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_MovementArchitecture : public AActor
{
    GENERATED_BODY()

public:
    AEng_MovementArchitecture();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core movement configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Architecture")
    float BaseWalkSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Architecture")
    float BaseRunSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Architecture")
    float JumpHeight = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Architecture")
    float MaxStamina = 100.0f;

    // Terrain adaptation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MaxWalkableAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float TerrainAdaptationSpeed = 5.0f;

    // Physics integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AirControl = 0.2f;

    // Collision configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CharacterRadius = 34.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CharacterHeight = 180.0f;

    // Movement validation functions
    UFUNCTION(BlueprintCallable, Category = "Movement Architecture")
    bool ValidateMovementSettings();

    UFUNCTION(BlueprintCallable, Category = "Movement Architecture")
    void ApplyMovementSettings();

    UFUNCTION(BlueprintCallable, Category = "Movement Architecture")
    void ResetToDefaults();

    // Architecture validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void ValidateArchitecture();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void GenerateArchitectureReport();

private:
    // Internal validation state
    bool bArchitectureValid = false;
    float LastValidationTime = 0.0f;

    // Helper functions
    void ValidateTerrainSettings();
    void ValidatePhysicsSettings();
    void ValidateCollisionSettings();
};

/**
 * Movement Architecture Component
 * Attached to characters to provide movement architecture integration
 */
UCLASS(BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_MovementArchitectureComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_MovementArchitectureComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Movement architecture reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TObjectPtr<AEng_MovementArchitecture> MovementArchitecture;

    // Component-specific settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    bool bUseArchitectureSettings = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    float MovementMultiplier = 1.0f;

    // Integration functions
    UFUNCTION(BlueprintCallable, Category = "Movement Architecture")
    void ApplyArchitectureToCharacter();

    UFUNCTION(BlueprintCallable, Category = "Movement Architecture")
    void UpdateMovementFromArchitecture();

private:
    void FindMovementArchitecture();
    void ValidateArchitectureConnection();
};