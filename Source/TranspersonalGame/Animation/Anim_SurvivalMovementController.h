#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Anim_SurvivalMovementController.generated.h"

UENUM(BlueprintType)
enum class EAnim_SurvivalMovementState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Crouching       UMETA(DisplayName = "Crouching"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Swimming        UMETA(DisplayName = "Swimming"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Defensive       UMETA(DisplayName = "Defensive"),
    Injured         UMETA(DisplayName = "Injured")
};

UENUM(BlueprintType)
enum class EAnim_TerrainType : uint8
{
    Flat            UMETA(DisplayName = "Flat Ground"),
    Uneven          UMETA(DisplayName = "Uneven Terrain"),
    Rocky           UMETA(DisplayName = "Rocky Surface"),
    Muddy           UMETA(DisplayName = "Muddy Ground"),
    Sandy           UMETA(DisplayName = "Sandy Surface"),
    Grassy          UMETA(DisplayName = "Grass"),
    Water           UMETA(DisplayName = "Water/Shallow"),
    Cliff           UMETA(DisplayName = "Cliff Edge")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsJumping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsClimbing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    EAnim_SurvivalMovementState MovementState = EAnim_SurvivalMovementState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EAnim_TerrainType TerrainType = EAnim_TerrainType::Flat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float TerrainSlope = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float StaminaLevel = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    bool bIsExhausted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    bool bIsInjured = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_SurvivalAction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    FString ActionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    TSoftObjectPtr<UAnimMontage> ActionMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    float ActionDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    bool bRequiresStamina = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    float StaminaCost = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
    bool bCanBeInterrupted = true;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_SurvivalMovementController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_SurvivalMovementController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Movement Functions
    UFUNCTION(BlueprintCallable, Category = "Survival Movement")
    void UpdateMovementState(const FAnim_MovementParameters& NewParameters);

    UFUNCTION(BlueprintCallable, Category = "Survival Movement")
    void SetMovementState(EAnim_SurvivalMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Survival Movement")
    EAnim_SurvivalMovementState GetCurrentMovementState() const { return CurrentMovementParameters.MovementState; }

    // Terrain Adaptation
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void AnalyzeTerrainBelow();

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void AdaptMovementToTerrain(EAnim_TerrainType TerrainType, float Slope);

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float CalculateTerrainDifficulty() const;

    // Survival Actions
    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    bool ExecuteSurvivalAction(const FString& ActionName);

    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    void RegisterSurvivalAction(const FAnim_SurvivalAction& NewAction);

    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    bool CanExecuteAction(const FString& ActionName) const;

    // Stamina and Condition Management
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateStaminaLevel(float NewStamina);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateFearLevel(float NewFear);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetInjuredState(bool bInjured);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsExhausted() const { return CurrentMovementParameters.bIsExhausted; }

    // Animation Blending
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void BlendToNewMovementState(EAnim_SurvivalMovementState NewState, float BlendTime = 0.3f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateAnimationBlendSpace();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAnimationSpeed(float SpeedMultiplier);

    // Environmental Reactions
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ReactToEnvironmentalThreat(const FVector& ThreatLocation, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void AdaptToWeatherConditions(float Temperature, float Humidity, bool bIsRaining);

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void DebugCurrentMovementState();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawMovementDebugInfo();

protected:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FAnim_MovementParameters CurrentMovementParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FAnim_MovementParameters PreviousMovementParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float MovementBlendTime = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float AnimationSpeedMultiplier = 1.0f;

    // Survival Actions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Actions")
    TArray<FAnim_SurvivalAction> RegisteredActions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Actions")
    FAnim_SurvivalAction CurrentAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Actions")
    bool bIsExecutingAction = false;

    // Terrain Analysis
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float TerrainCheckDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float SlopeThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TMap<EAnim_TerrainType, float> TerrainSpeedMultipliers;

    // Stamina and Condition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float StaminaRecoveryRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float ExhaustionThreshold = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float FearDecayRate = 2.0f;

    // Component References
    UPROPERTY()
    TWeakObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TWeakObjectPtr<UCharacterMovementComponent> MovementComponent;

    UPROPERTY()
    TWeakObjectPtr<USkeletalMeshComponent> MeshComponent;

    UPROPERTY()
    TWeakObjectPtr<UAnimInstance> AnimInstance;

    // Timers
    UPROPERTY()
    FTimerHandle ActionTimerHandle;

    UPROPERTY()
    FTimerHandle StaminaRecoveryTimerHandle;

private:
    // Internal Functions
    void InitializeComponent();
    void CacheComponentReferences();
    void InitializeTerrainMultipliers();
    void UpdateStaminaRecovery();
    void OnActionComplete();
    bool ValidateActionExecution(const FAnim_SurvivalAction& Action) const;
    void ApplyMovementModifiers();
    void UpdateAnimationParameters();
    FVector GetTerrainNormal() const;
    float CalculateMovementSpeed() const;
    void HandleStateTransition(EAnim_SurvivalMovementState FromState, EAnim_SurvivalMovementState ToState);
};