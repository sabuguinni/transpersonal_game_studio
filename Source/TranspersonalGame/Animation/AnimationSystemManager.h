#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "AnimationSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Combat      UMETA(DisplayName = "Combat"),
    Injured     UMETA(DisplayName = "Injured")
};

UENUM(BlueprintType)
enum class EAnim_ActionType : uint8
{
    None            UMETA(DisplayName = "None"),
    SpearThrow      UMETA(DisplayName = "Spear Throw"),
    ToolCrafting    UMETA(DisplayName = "Tool Crafting"),
    FireMaking      UMETA(DisplayName = "Fire Making"),
    RockClimbing    UMETA(DisplayName = "Rock Climbing"),
    DinosaurTaming  UMETA(DisplayName = "Dinosaur Taming"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Building        UMETA(DisplayName = "Building")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    EAnim_MovementState MovementState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsOnGround;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TerrainSlope;

    FAnim_MotionMatchingData()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        MovementState = EAnim_MovementState::Idle;
        bIsOnGround = true;
        TerrainSlope = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector LeftFootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector RightFootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator LeftFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator RightFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float LeftFootIKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float RightFootIKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector LookAtTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float LookAtAlpha;

    FAnim_IKData()
    {
        LeftFootLocation = FVector::ZeroVector;
        RightFootLocation = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        LeftFootIKAlpha = 0.0f;
        RightFootIKAlpha = 0.0f;
        LookAtTarget = FVector::ZeroVector;
        LookAtAlpha = 0.0f;
    }
};

/**
 * Animation System Manager - Handles all character animation systems
 * Manages Motion Matching, IK, Montages, and Procedural Animation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Motion Matching System
    UFUNCTION(BlueprintCallable, Category = "Animation|Motion Matching")
    void UpdateMotionMatchingData(ACharacter* Character, FAnim_MotionMatchingData& OutData);

    UFUNCTION(BlueprintCallable, Category = "Animation|Motion Matching")
    void FindBestMotionMatch(const FAnim_MotionMatchingData& InputData, FString& OutAnimationName);

    // IK System
    UFUNCTION(BlueprintCallable, Category = "Animation|IK")
    void UpdateFootIK(ACharacter* Character, FAnim_IKData& OutIKData);

    UFUNCTION(BlueprintCallable, Category = "Animation|IK")
    void CalculateFootPlacement(ACharacter* Character, bool bLeftFoot, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha);

    UFUNCTION(BlueprintCallable, Category = "Animation|IK")
    void SetLookAtTarget(ACharacter* Character, const FVector& Target, float Alpha);

    // Animation Montage System
    UFUNCTION(BlueprintCallable, Category = "Animation|Montages")
    bool PlayActionMontage(ACharacter* Character, EAnim_ActionType ActionType);

    UFUNCTION(BlueprintCallable, Category = "Animation|Montages")
    void StopActionMontage(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Animation|Montages")
    bool IsPlayingActionMontage(ACharacter* Character) const;

    // Procedural Animation
    UFUNCTION(BlueprintCallable, Category = "Animation|Procedural")
    void UpdateProceduralBreathing(ACharacter* Character, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation|Procedural")
    void ApplyEnvironmentalReactions(ACharacter* Character, float Temperature, float Humidity, bool bIsRaining);

    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Animation|State")
    void SetMovementState(ACharacter* Character, EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation|State")
    EAnim_MovementState GetMovementState(ACharacter* Character) const;

    // Terrain Adaptation
    UFUNCTION(BlueprintCallable, Category = "Animation|Terrain")
    void UpdateTerrainAdaptation(ACharacter* Character, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation|Terrain")
    float CalculateTerrainSlope(ACharacter* Character) const;

protected:
    // Motion Matching Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Motion Matching")
    TMap<FString, FAnim_MotionMatchingData> MotionDatabase;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Assets")
    TMap<EAnim_ActionType, TSoftObjectPtr<UAnimMontage>> ActionMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Assets")
    TSoftObjectPtr<UBlendSpace> LocomotionBlendSpace;

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK")
    float FootIKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK")
    float LookAtInterpSpeed;

    // Character State Tracking
    UPROPERTY()
    TMap<TWeakObjectPtr<ACharacter>, EAnim_MovementState> CharacterStates;

    UPROPERTY()
    TMap<TWeakObjectPtr<ACharacter>, FAnim_IKData> CharacterIKData;

private:
    // Helper functions
    void InitializeMotionDatabase();
    void LoadAnimationAssets();
    bool PerformFootTrace(ACharacter* Character, const FVector& FootLocation, FHitResult& OutHit) const;
    FVector GetCharacterVelocity(ACharacter* Character) const;
    float CalculateMovementDirection(ACharacter* Character) const;
};