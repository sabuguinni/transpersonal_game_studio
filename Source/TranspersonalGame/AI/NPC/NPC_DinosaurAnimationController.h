#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SharedTypes.h"
#include "NPC_DinosaurAnimationController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDinosaurAnimationStateChanged, ENPC_DinosaurAnimState, OldState, ENPC_DinosaurAnimState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDinosaurMontageCompleted, UAnimMontage*, Montage);

/**
 * Dinosaur Animation Controller - Manages animation states and transitions for dinosaur NPCs
 * Integrates with Animation Agent's player animation system to provide consistent animation framework
 * Handles species-specific animations: T-Rex territorial displays, Velociraptor pack coordination, etc.
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurAnimationController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void SetAnimationState(ENPC_DinosaurAnimState NewState);

    UFUNCTION(BlueprintPure, Category = "Dinosaur Animation")
    ENPC_DinosaurAnimState GetCurrentAnimationState() const { return CurrentAnimState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void PlaySpeciesAnimation(ENPC_DinosaurSpecies Species, ENPC_DinosaurAnimState AnimState);

    // Montage Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void PlayAnimationMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void StopAnimationMontage(UAnimMontage* Montage = nullptr);

    // Behavior-Driven Animations
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void TriggerTerritorialDisplay();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void TriggerHuntingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void TriggerFeedingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void TriggerSleepingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void TriggerAlertAnimation();

    // Pack Coordination (for Velociraptors)
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void SynchronizePackAnimation(const TArray<UNPC_DinosaurAnimationController*>& PackMembers);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void PlayPackCoordinatedAnimation(ENPC_DinosaurAnimState AnimState, float SyncDelay = 0.0f);

    // Environmental Reactions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void ReactToPlayerPresence(float PlayerDistance, bool bPlayerVisible);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void ReactToWeatherChange(ENPC_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void ReactToTimeOfDay(float TimeOfDay);

    // Animation Parameters
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void UpdateMovementParameters();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void SetAnimationSpeed(float Speed);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void SetAnimationIntensity(float Intensity);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Dinosaur Animation")
    FOnDinosaurAnimationStateChanged OnAnimationStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dinosaur Animation")
    FOnDinosaurMontageCompleted OnMontageCompleted;

protected:
    // Animation State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    ENPC_DinosaurAnimState CurrentAnimState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    ENPC_DinosaurAnimState PreviousAnimState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    float StateTransitionTime;

    // Species Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Config")
    ENPC_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Config")
    float AnimationSpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Config")
    float StateTransitionDuration;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TMap<ENPC_DinosaurAnimState, UAnimMontage*> SpeciesAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* CombatBlendSpace;

    // Movement Parameters
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    float CurrentSpeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    float MovementDirection;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsInCombat;

    // Pack Coordination
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    TArray<TWeakObjectPtr<UNPC_DinosaurAnimationController>> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    bool bIsPackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    float PackSyncTimer;

    // Component References
    UPROPERTY()
    TWeakObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TWeakObjectPtr<UCharacterMovementComponent> MovementComponent;

    UPROPERTY()
    TWeakObjectPtr<UAnimInstance> AnimInstance;

private:
    // Internal Methods
    void InitializeAnimationController();
    void UpdateAnimationParameters();
    void HandleStateTransition(ENPC_DinosaurAnimState NewState);
    void UpdateBlendSpaceParameters();
    void ProcessPackSynchronization(float DeltaTime);
    
    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    // Timers
    float LastAnimationUpdate;
    float PackSyncUpdateInterval;
};