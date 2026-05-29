// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CharacterMovementAnimationSystemV44.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMovementAnimationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsAccelerating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float LeanAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float TurnInPlaceAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementPlayRate;

    FMovementAnimationData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        bIsMoving = false;
        bIsAccelerating = false;
        bIsInAir = false;
        bIsCrouching = false;
        LeanAmount = 0.0f;
        TurnInPlaceAngle = 0.0f;
        MovementPlayRate = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCharacterArchetypeMovementSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FString ArchetypeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    float WalkSpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    float RunSpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    float CrouchSpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float AnimationPlayRateMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float TurnInPlaceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float LeanIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bEnableRootMotion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional")
    float FearMovementModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional")
    float CalmMovementModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional")
    float AlertnessMovementModifier;

    FCharacterArchetypeMovementSettings()
    {
        WalkSpeedMultiplier = 1.0f;
        RunSpeedMultiplier = 1.0f;
        CrouchSpeedMultiplier = 1.0f;
        AnimationPlayRateMultiplier = 1.0f;
        TurnInPlaceThreshold = 45.0f;
        LeanIntensity = 1.0f;
        bEnableRootMotion = true;
        FearMovementModifier = 0.8f;
        CalmMovementModifier = 1.0f;
        AlertnessMovementModifier = 1.2f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEmotionalMovementState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float CalmLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float AlertnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float StressLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float FatigueLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float InjuryLevel;

    FEmotionalMovementState()
    {
        FearLevel = 0.0f;
        CalmLevel = 1.0f;
        AlertnessLevel = 0.5f;
        StressLevel = 0.0f;
        HungerLevel = 0.0f;
        FatigueLevel = 0.0f;
        InjuryLevel = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementStateChangedDelegate, const FMovementAnimationData&, MovementData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmotionalStateChangedDelegate, const FEmotionalMovementState&, EmotionalState);

/**
 * Character Movement Animation System V44
 * Handles movement animation data calculation with archetype and emotional support
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterMovementAnimationSystemV44 : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterMovementAnimationSystemV44();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    TArray<FCharacterArchetypeMovementSettings> ArchetypeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    FString CurrentArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float MovementSmoothingSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float DirectionSmoothingSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float LeanSmoothingSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float MinMovementThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
    float MaxLeanAngle;

    // Emotional system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional System")
    FEmotionalMovementState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional System")
    bool bEnableEmotionalModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional System")
    float EmotionalTransitionSpeed;

    // Turn in place
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn in Place")
    bool bEnableTurnInPlace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn in Place")
    float TurnInPlaceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn in Place")
    float TurnInPlacePlayRate;

    // Root motion
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Root Motion")
    bool bEnableRootMotion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Root Motion")
    float RootMotionScale;

    // Performance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseAsyncUpdates;

    // Debug
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bEnableDebugVisualization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowMovementVectors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowEmotionalState;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnMovementStateChangedDelegate OnMovementStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnEmotionalStateChangedDelegate OnEmotionalStateChanged;

    // Current movement data
    UPROPERTY(BlueprintReadOnly, Category = "Movement Data")
    FMovementAnimationData CurrentMovementData;

    // Blueprint Interface
    UFUNCTION(BlueprintCallable, Category = "Movement Animation")
    void UpdateMovementData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement Animation")
    FMovementAnimationData GetMovementData() const;

    UFUNCTION(BlueprintCallable, Category = "Movement Animation")
    void SetArchetype(const FString& ArchetypeName);

    UFUNCTION(BlueprintCallable, Category = "Movement Animation")
    FCharacterArchetypeMovementSettings GetCurrentArchetypeSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void SetEmotionalState(const FEmotionalMovementState& NewEmotionalState);

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void ModifyEmotionalState(float Fear, float Calm, float Alertness, float Stress);

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void SetSurvivalState(float Hunger, float Fatigue, float Injury);

    UFUNCTION(BlueprintCallable, Category = "Movement Animation")
    float GetEffectiveSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "Movement Animation")
    float GetEffectiveDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Movement Animation")
    float GetEffectivePlayRate() const;

    UFUNCTION(BlueprintCallable, Category = "Movement Animation")
    bool ShouldTurnInPlace() const;

    UFUNCTION(BlueprintCallable, Category = "Movement Animation")
    float GetTurnInPlaceAngle() const;

    UFUNCTION(BlueprintCallable, Category = "Movement Animation")
    FVector GetLeanVector() const;

    // Advanced features
    UFUNCTION(BlueprintCallable, Category = "Advanced Movement")
    void EnableEmotionalModifiers(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Advanced Movement")
    void SetMovementSmoothingParameters(float MovementSmoothing, float DirectionSmoothing, float LeanSmoothing);

    UFUNCTION(BlueprintCallable, Category = "Advanced Movement")
    void AddArchetypeSettings(const FCharacterArchetypeMovementSettings& NewArchetypeSettings);

    UFUNCTION(BlueprintCallable, Category = "Advanced Movement")
    bool RemoveArchetypeSettings(const FString& ArchetypeName);

protected:
    // Internal functions
    void InitializeDefaultArchetypes();
    void CalculateMovementData(float DeltaTime);
    void ApplyArchetypeModifiers();
    void ApplyEmotionalModifiers();
    void CalculateDirection();
    void CalculateLean(float DeltaTime);
    void CalculateTurnInPlace();
    void UpdateEmotionalTransitions(float DeltaTime);
    
    FCharacterArchetypeMovementSettings* GetArchetypeSettings(const FString& ArchetypeName);
    float CalculateEmotionalSpeedModifier() const;
    float CalculateEmotionalAnimationModifier() const;
    
    void DrawDebugVisualization() const;

    // Component references
    UPROPERTY()
    UCharacterMovementComponent* CharacterMovementComponent;

    UPROPERTY()
    APawn* OwnerPawn;

    // Runtime data
    UPROPERTY(Transient)
    FVector PreviousVelocity;

    UPROPERTY(Transient)
    FRotator PreviousRotation;

    UPROPERTY(Transient)
    float TimeSinceLastUpdate;

    UPROPERTY(Transient)
    FEmotionalMovementState TargetEmotionalState;

    UPROPERTY(Transient)
    bool bIsInitialized;

    // Performance optimization
    UPROPERTY(Transient)
    int32 UpdateCounter;

    UPROPERTY(Transient)
    float CachedSpeed;

    UPROPERTY(Transient)
    float CachedDirection;

    UPROPERTY(Transient)
    float CachedLeanAmount;
};