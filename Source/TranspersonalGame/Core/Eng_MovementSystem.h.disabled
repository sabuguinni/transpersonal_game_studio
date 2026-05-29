#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/ActorComponent.h"
#include "Eng_MovementSystem.generated.h"

/**
 * Movement System Architecture for Transpersonal Game
 * Defines standardized movement patterns for characters and dinosaurs
 * Ensures consistent physics and animation integration
 */

UENUM(BlueprintType)
enum class EEng_MovementType : uint8
{
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Falling     UMETA(DisplayName = "Falling")
};

UENUM(BlueprintType)
enum class EEng_TerrainType : uint8
{
    Flat        UMETA(DisplayName = "Flat"),
    Slope       UMETA(DisplayName = "Slope"),
    Rough       UMETA(DisplayName = "Rough"),
    Mud         UMETA(DisplayName = "Mud"),
    Sand        UMETA(DisplayName = "Sand"),
    Rock        UMETA(DisplayName = "Rock"),
    Water       UMETA(DisplayName = "Water")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_MovementSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SprintSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float CrouchSpeed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float JumpHeight = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float StaminaDrainRate = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float TerrainSpeedMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_MovementState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EEng_MovementType CurrentMovementType = EEng_MovementType::Walking;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EEng_TerrainType CurrentTerrain = EEng_TerrainType::Flat;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float CurrentSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsOnGround = true;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bCanSprint = true;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float StaminaLevel = 100.0f;
};

/**
 * Movement System Component
 * Standardizes movement behavior across all characters
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_MovementSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_MovementSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Movement Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    FEng_MovementSettings MovementSettings;

    // Current Movement State
    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    FEng_MovementState MovementState;

    // Movement Functions
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetMovementType(EEng_MovementType NewMovementType);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateTerrainType(EEng_TerrainType NewTerrain);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    float GetCurrentSpeedLimit() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool CanPerformMovement(EEng_MovementType MovementType) const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void RegenerateStamina(float DeltaTime);

    // Architecture Validation
    UFUNCTION(CallInEditor, Category = "Architecture")
    void ValidateMovementArchitecture();

protected:
    // Internal movement logic
    void UpdateMovementSpeed();
    void CheckTerrainUnderfoot();
    float CalculateTerrainSpeedModifier() const;

private:
    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // Internal state
    float LastStaminaRegenTime = 0.0f;
    bool bInitialized = false;
};

/**
 * Movement Architecture Manager
 * Oversees all movement systems in the game
 */
UCLASS()
class TRANSPERSONALGAME_API UEng_MovementArchitectureManager : public UObject
{
    GENERATED_BODY()

public:
    // Singleton access
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    static UEng_MovementArchitectureManager* GetInstance();

    // Architecture validation
    UFUNCTION(CallInEditor, Category = "Architecture")
    void ValidateAllMovementSystems();

    UFUNCTION(CallInEditor, Category = "Architecture")
    void GenerateMovementReport();

    // Movement system registration
    void RegisterMovementSystem(UEng_MovementSystem* MovementSystem);
    void UnregisterMovementSystem(UEng_MovementSystem* MovementSystem);

protected:
    // Registered movement systems
    UPROPERTY()
    TArray<UEng_MovementSystem*> RegisteredSystems;

private:
    static UEng_MovementArchitectureManager* Instance;
};