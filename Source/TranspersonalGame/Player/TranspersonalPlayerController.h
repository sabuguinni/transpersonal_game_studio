#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Core/ConsciousnessSystem.h"
#include "TranspersonalPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UConsciousnessSystem;

UENUM(BlueprintType)
enum class EInteractionMode : uint8
{
    Exploration     UMETA(DisplayName = "Exploration Mode"),
    Meditation      UMETA(DisplayName = "Meditation Mode"),
    Observation     UMETA(DisplayName = "Observation Mode"),
    Transformation  UMETA(DisplayName = "Transformation Mode")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMeditationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float BreathingCycleDuration = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float AwarenessGainRate = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float ClarityGainRate = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    bool bRequireStillness = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float MaxMovementThreshold = 10.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionModeChanged, EInteractionMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMeditationStateChanged, bool, bIsMeditating, float, SessionDuration);

/**
 * Enhanced player controller for transpersonal gameplay mechanics
 * Handles consciousness-based interactions, meditation systems, and reality shifting
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ATranspersonalPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Input Actions
    UFUNCTION(BlueprintCallable, Category = "Input")
    void SetupEnhancedInput();

    // Interaction Modes
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetInteractionMode(EInteractionMode NewMode);

    UFUNCTION(BlueprintPure, Category = "Interaction")
    EInteractionMode GetCurrentInteractionMode() const { return CurrentInteractionMode; }

    // Meditation System
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void StartMeditation();

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void StopMeditation();

    UFUNCTION(BlueprintPure, Category = "Meditation")
    bool IsMeditating() const { return bIsMeditating; }

    UFUNCTION(BlueprintPure, Category = "Meditation")
    float GetMeditationDuration() const { return MeditationDuration; }

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void SetMeditationSettings(const FMeditationSettings& NewSettings);

    // Consciousness Interaction
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void AttemptConsciousnessStateTransition(EConsciousnessState TargetState);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void AttemptRealityLayerShift(ERealityLayer TargetLayer);

    // Observation System
    UFUNCTION(BlueprintCallable, Category = "Observation")
    void StartObservation(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Observation")
    void StopObservation();

    UFUNCTION(BlueprintPure, Category = "Observation")
    bool IsObserving() const { return bIsObserving; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnInteractionModeChanged OnInteractionModeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnMeditationStateChanged OnMeditationStateChanged;

protected:
    // Input Mapping Context
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    // Input Actions
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> MeditateAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> ObserveAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> InteractAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> TransformAction;

    // Meditation Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    FMeditationSettings MeditationSettings;

    // State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EInteractionMode CurrentInteractionMode;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsMeditating = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float MeditationDuration = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsObserving = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TObjectPtr<AActor> ObservationTarget;

private:
    // Input Handlers
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void ToggleMeditation();
    void ToggleObservation();
    void Interact();
    void Transform();

    // Consciousness System Reference
    UPROPERTY()
    TObjectPtr<UConsciousnessSystem> ConsciousnessSystem;

    // Meditation System
    void UpdateMeditation(float DeltaTime);
    void ProcessMeditationGains(float DeltaTime);
    bool CheckMeditationConditions() const;

    // Observation System
    void UpdateObservation(float DeltaTime);
    void ProcessObservationGains(float DeltaTime);

    // Movement Tracking for Meditation
    FVector LastFrameLocation;
    float AccumulatedMovement = 0.0f;
    float MovementResetTime = 0.0f;

    // Breathing Cycle for Meditation
    float BreathingCycleTime = 0.0f;
    bool bInhaling = true;

    // Observation Data
    float ObservationDuration = 0.0f;
    float LastObservationGainTime = 0.0f;
};