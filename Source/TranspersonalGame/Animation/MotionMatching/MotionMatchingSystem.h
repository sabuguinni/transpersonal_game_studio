#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/PoseSearchDatabase.h"
#include "Animation/PoseSearchSchema.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionMatchingSystem.generated.h"

/**
 * Sistema de Motion Matching para animações fluídas e naturais
 * Baseado na filosofia de Richard Williams: cada movimento tem peso e história
 * Inspirado no RDR2: cada personagem tem uma linguagem corporal única
 */

UENUM(BlueprintType)
enum class EMotionMatchingState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Sneaking        UMETA(DisplayName = "Sneaking"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Swimming        UMETA(DisplayName = "Swimming"),
    Falling         UMETA(DisplayName = "Falling"),
    Landing         UMETA(DisplayName = "Landing"),
    Turning         UMETA(DisplayName = "Turning"),
    Interaction     UMETA(DisplayName = "Interaction"),
    Fear            UMETA(DisplayName = "Fear"),
    Caution         UMETA(DisplayName = "Caution"),
    Exhaustion      UMETA(DisplayName = "Exhaustion")
};

UENUM(BlueprintType)
enum class ECharacterPersonality : uint8
{
    Cautious        UMETA(DisplayName = "Cautious - Moves carefully, always alert"),
    Confident       UMETA(DisplayName = "Confident - Bold movements, upright posture"),
    Nervous         UMETA(DisplayName = "Nervous - Fidgety, quick glances"),
    Tired           UMETA(DisplayName = "Tired - Slower movements, slouched"),
    Injured         UMETA(DisplayName = "Injured - Favoring one side, limping"),
    Predator        UMETA(DisplayName = "Predator - Stalking, focused movements"),
    Prey            UMETA(DisplayName = "Prey - Jumpy, ready to flee")
};

USTRUCT(BlueprintType)
struct FMotionMatchingQuery
{
    GENERATED_BODY()

    // Trajectory data
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CurrentVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector DesiredVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FuturePosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator FutureRotation = FRotator::ZeroRotator;

    // Character state
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMotionMatchingState CurrentState = EMotionMatchingState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterPersonality Personality = ECharacterPersonality::Cautious;

    // Environmental context
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GroundSlope = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOnUnstableGround = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNearPredator = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearLevel = 0.0f;

    // Animation quality modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WeightShift = 0.0f; // Como o peso do corpo está distribuído

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Tension = 0.0f; // Tensão muscular (0 = relaxado, 1 = muito tenso)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Fatigue = 0.0f; // Nível de cansaço
};

USTRUCT(BlueprintType)
struct FCharacterAnimationProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ProfileName = "Default";

    // Databases específicos para este perfil
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UPoseSearchDatabase> IdleDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UPoseSearchDatabase> LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UPoseSearchDatabase> ActionDatabase;

    // Modificadores de personalidade
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float MovementSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float TurnRateMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BaseAnxietyLevel = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PosturalTension = 0.3f;

    // Blend times únicos para este personagem
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.05", ClampMax = "1.0"))
    float QuickBlendTime = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float NormalBlendTime = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.2", ClampMax = "3.0"))
    float SlowBlendTime = 0.8f;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMotionMatchingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UMotionMatchingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Sistema principal de Motion Matching
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatching(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetCharacterProfile(const FCharacterAnimationProfile& NewProfile);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetPersonality(ECharacterPersonality NewPersonality);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateEnvironmentalContext(float GroundSlope, bool bUnstableGround, bool bNearThreat);

    // Query system
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FMotionMatchingQuery BuildCurrentQuery();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UPoseSearchDatabase* SelectBestDatabase(const FMotionMatchingQuery& Query);

    // Emotional state system
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetFearLevel(float NewFearLevel);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddTension(float TensionAmount, float Duration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddFatigue(float FatigueAmount);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    EMotionMatchingState GetCurrentState() const { return CurrentQuery.CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetCurrentBlendTime() const;

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsInTransition() const { return bIsInTransition; }

protected:
    // Core components
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    UPROPERTY()
    TObjectPtr<UAnimInstance> AnimInstance;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FCharacterAnimationProfile CharacterProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchSchema> DefaultSchema;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FMotionMatchingQuery CurrentQuery;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FMotionMatchingQuery PreviousQuery;

    // Transition system
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    bool bIsInTransition = false;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float TransitionTimeRemaining = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float TransitionDuration = 0.3f;

    // Personality modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    ECharacterPersonality CurrentPersonality = ECharacterPersonality::Cautious;

    // Temporal state tracking
    UPROPERTY()
    float TensionDecayTimer = 0.0f;

    UPROPERTY()
    float CurrentTension = 0.0f;

    UPROPERTY()
    float CurrentFatigue = 0.0f;

    // Performance tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDebugDrawing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float QueryUpdateFrequency = 30.0f; // Hz

    UPROPERTY()
    float TimeSinceLastQuery = 0.0f;

private:
    // Internal methods
    void UpdateCharacterState(float DeltaTime);
    void UpdatePersonalityModifiers(float DeltaTime);
    void UpdateEnvironmentalFactors();
    EMotionMatchingState DetermineCurrentState();
    float CalculateOptimalBlendTime(const FMotionMatchingQuery& FromQuery, const FMotionMatchingQuery& ToQuery);
    void DrawDebugInfo();

    // Trajectory prediction
    FVector PredictFuturePosition(float TimeAhead) const;
    FRotator PredictFutureRotation(float TimeAhead) const;
    
    // Personality-specific modifiers
    float GetPersonalityMovementModifier() const;
    float GetPersonalityTensionModifier() const;
    float GetPersonalityBlendModifier() const;
};