#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TranspersonalGame.h"
#include "Core_CharacterPhysics.generated.h"

UENUM(BlueprintType)
enum class ECore_CharacterPhysicsState : uint8
{
    Normal          UMETA(DisplayName = "Normal"),
    Ragdoll         UMETA(DisplayName = "Ragdoll"),
    Swimming        UMETA(DisplayName = "Swimming"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Falling         UMETA(DisplayName = "Falling"),
    Stunned         UMETA(DisplayName = "Stunned")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_CharacterPhysicsConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float JumpVelocity = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float AirControl = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GroundFriction = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float BrakingDeceleration = 2048.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollImpulseThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollRecoveryTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
    float SwimSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
    float BuoyancyForce = 980.0f;

    FCore_CharacterPhysicsConfig()
    {
        // Default values already set above
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsImpactData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FVector ImpactNormal = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float ImpactForce = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    AActor* ImpactActor = nullptr;

    UPROPERTY(BlueprintReadOnly)
    float ImpactTime = 0.0f;

    FCore_PhysicsImpactData()
    {
        // Default values already set above
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_CharacterPhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CharacterPhysics();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Config")
    FCore_CharacterPhysicsConfig PhysicsConfig;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    ECore_CharacterPhysicsState CurrentPhysicsState = ECore_CharacterPhysicsState::Normal;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    bool bIsGrounded = true;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    bool bIsInWater = false;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    float CurrentVelocityMagnitude = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    FVector CurrentAcceleration = FVector::ZeroVector;

    // Movement Functions
    UFUNCTION(BlueprintCallable, Category = "Character Physics")
    void SetMovementSpeed(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category = "Character Physics")
    void ApplyImpulse(const FVector& Impulse, bool bVelChange = false);

    UFUNCTION(BlueprintCallable, Category = "Character Physics")
    void SetPhysicsState(ECore_CharacterPhysicsState NewState);

    UFUNCTION(BlueprintCallable, Category = "Character Physics")
    void EnableRagdoll(float Duration = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Character Physics")
    void DisableRagdoll();

    UFUNCTION(BlueprintCallable, Category = "Character Physics")
    void HandleImpact(const FCore_PhysicsImpactData& ImpactData);

    // Swimming Functions
    UFUNCTION(BlueprintCallable, Category = "Swimming")
    void EnterWater();

    UFUNCTION(BlueprintCallable, Category = "Swimming")
    void ExitWater();

    UFUNCTION(BlueprintCallable, Category = "Swimming")
    void ApplyBuoyancy(float DeltaTime);

    // Ground Detection
    UFUNCTION(BlueprintCallable, Category = "Ground Detection")
    bool CheckGroundContact();

    UFUNCTION(BlueprintCallable, Category = "Ground Detection")
    FVector GetGroundNormal();

    UFUNCTION(BlueprintCallable, Category = "Ground Detection")
    float GetDistanceToGround();

    // Physics Validation
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool ValidatePhysicsState();

    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    void ResetPhysicsState();

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Physics Events")
    void OnPhysicsStateChanged(ECore_CharacterPhysicsState OldState, ECore_CharacterPhysicsState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Physics Events")
    void OnImpactReceived(const FCore_PhysicsImpactData& ImpactData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Physics Events")
    void OnRagdollActivated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Physics Events")
    void OnRagdollDeactivated();

private:
    // Component References
    UPROPERTY()
    ACharacter* OwnerCharacter = nullptr;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent = nullptr;

    UPROPERTY()
    USkeletalMeshComponent* MeshComponent = nullptr;

    UPROPERTY()
    UCapsuleComponent* CapsuleComponent = nullptr;

    // Internal State
    float RagdollTimer = 0.0f;
    float RagdollDuration = 0.0f;
    bool bRagdollActive = false;
    ECore_CharacterPhysicsState PreviousPhysicsState = ECore_CharacterPhysicsState::Normal;

    // Ground Detection
    FVector LastGroundNormal = FVector::UpVector;
    float LastGroundDistance = 0.0f;
    bool bGroundContactValid = false;

    // Water Physics
    float WaterLevel = 0.0f;
    float SubmergedDepth = 0.0f;

    // Internal Functions
    void InitializeComponents();
    void UpdatePhysicsState(float DeltaTime);
    void UpdateGroundDetection();
    void UpdateWaterPhysics(float DeltaTime);
    void ProcessRagdoll(float DeltaTime);
    void ValidateMovementSettings();
    
    // Physics Calculations
    FVector CalculateBuoyancyForce() const;
    float CalculateWaterResistance() const;
    bool ShouldActivateRagdoll(const FCore_PhysicsImpactData& ImpactData) const;
};