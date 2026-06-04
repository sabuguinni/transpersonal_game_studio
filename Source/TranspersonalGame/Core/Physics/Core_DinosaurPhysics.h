#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "SharedTypes.h"
#include "Core_DinosaurPhysics.generated.h"

UENUM(BlueprintType)
enum class ECore_DinosaurSpecies : uint8
{
    TRex           UMETA(DisplayName = "Tyrannosaurus Rex"),
    Raptor         UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Stegosaurus    UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class ECore_DinosaurMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Charging    UMETA(DisplayName = "Charging"),
    Eating      UMETA(DisplayName = "Eating"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct FCore_DinosaurPhysicsConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxWalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxRunSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxChargeSpeed = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float TurnRate = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float StepHeight = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GroundFriction = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float BrakingDeceleration = 2048.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float FleeThreshold = 30.0f;

    FCore_DinosaurPhysicsConfig()
    {
        MaxWalkSpeed = 300.0f;
        MaxRunSpeed = 600.0f;
        MaxChargeSpeed = 1000.0f;
        TurnRate = 90.0f;
        Mass = 1000.0f;
        StepHeight = 45.0f;
        GroundFriction = 8.0f;
        BrakingDeceleration = 2048.0f;
        AttackRange = 200.0f;
        AttackDamage = 50.0f;
        DetectionRadius = 1500.0f;
        FleeThreshold = 30.0f;
    }
};

USTRUCT(BlueprintType)
struct FCore_DinosaurSpeciesData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    ECore_DinosaurSpecies Species = ECore_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FString DisplayName = TEXT("Unknown Dinosaur");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float BaseHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float BaseSize = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FCore_DinosaurPhysicsConfig PhysicsConfig;

    FCore_DinosaurSpeciesData()
    {
        Species = ECore_DinosaurSpecies::TRex;
        DisplayName = TEXT("Unknown Dinosaur");
        BaseHealth = 100.0f;
        BaseSize = 1.0f;
        bIsCarnivore = true;
        bIsPackHunter = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_DinosaurPhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_DinosaurPhysics();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Species Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Species")
    ECore_DinosaurSpecies DinosaurSpecies = ECore_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Species")
    FCore_DinosaurSpeciesData SpeciesData;

    // Movement State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement State")
    ECore_DinosaurMovementState CurrentMovementState = ECore_DinosaurMovementState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement State")
    FVector CurrentVelocity = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement State")
    float CurrentSpeed = 0.0f;

    // Physics Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bUseCustomPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGroundAdaptation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float FootIKRange = 50.0f;

    // Health and Status
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    bool bIsAlive = true;

    // Component References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* MeshComponent;

    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void SetMovementState(ECore_DinosaurMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void SetSpecies(ECore_DinosaurSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void ApplyMovementInput(FVector InputDirection, float InputMagnitude);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void StartCharging();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void StopCharging();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void Die();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void Respawn();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Dinosaur Physics")
    ECore_DinosaurSpecies GetSpecies() const { return DinosaurSpecies; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur Physics")
    ECore_DinosaurMovementState GetMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur Physics")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur Physics")
    bool IsMoving() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur Physics")
    bool IsCharging() const;

    // Validation Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    void ValidatePhysicsSetup();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    void TestMovementStates();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    void TestSpeciesConfiguration();

private:
    void InitializeSpeciesData();
    void UpdateMovementPhysics(float DeltaTime);
    void UpdateGroundAdaptation();
    void UpdateFootIK();
    void ApplyPhysicsConfiguration();
    FCore_DinosaurSpeciesData GetDefaultSpeciesData(ECore_DinosaurSpecies Species);

    // Internal State
    bool bIsCharging = false;
    float ChargingStartTime = 0.0f;
    FVector LastGroundNormal = FVector::UpVector;
    TArray<FVector> FootIKTargets;
};