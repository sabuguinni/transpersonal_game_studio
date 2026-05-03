#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Anim_CharacterAnimationController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_AnimationState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    EDir_MovementState MovementState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    float StaminaLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    float FearLevel;

    FAnim_AnimationState()
    {
        MovementState = EDir_MovementState::Idle;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        StaminaLevel = 1.0f;
        FearLevel = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MontageSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    class UAnimMontage* Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    float PlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    float BlendInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    float BlendOutTime;

    FAnim_MontageSettings()
    {
        Montage = nullptr;
        PlayRate = 1.0f;
        BlendInTime = 0.25f;
        BlendOutTime = 0.25f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CharacterAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimationController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === ANIMATION STATE MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Animation Controller")
    void UpdateAnimationState(const FAnim_AnimationState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation Controller")
    FAnim_AnimationState GetCurrentAnimationState() const { return CurrentAnimationState; }

    UFUNCTION(BlueprintCallable, Category = "Animation Controller")
    void SetMovementState(EDir_MovementState NewMovementState);

    UFUNCTION(BlueprintCallable, Category = "Animation Controller")
    void SetSpeed(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category = "Animation Controller")
    void SetDirection(float NewDirection);

    // === MONTAGE MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Animation Controller")
    bool PlayMontage(const FAnim_MontageSettings& MontageSettings);

    UFUNCTION(BlueprintCallable, Category = "Animation Controller")
    void StopMontage(float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintCallable, Category = "Animation Controller")
    bool IsPlayingMontage() const;

    UFUNCTION(BlueprintCallable, Category = "Animation Controller")
    float GetMontagePosition() const;

    // === BLEND SPACE MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Animation Controller")
    void SetBlendSpaceInput(float X, float Y);

    UFUNCTION(BlueprintCallable, Category = "Animation Controller")
    FVector2D GetBlendSpaceInput() const { return BlendSpaceInput; }

    // === SURVIVAL ANIMATION MODIFIERS ===
    UFUNCTION(BlueprintCallable, Category = "Animation Controller")
    void ApplyStaminaModifier(float StaminaLevel);

    UFUNCTION(BlueprintCallable, Category = "Animation Controller")
    void ApplyFearModifier(float FearLevel);

    UFUNCTION(BlueprintCallable, Category = "Animation Controller")
    void ApplyInjuryModifier(float InjuryLevel);

    // === ANIMATION EVENTS ===
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnAnimationStateChanged(const FAnim_AnimationState& NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnMontageStarted(UAnimMontage* Montage);

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
    // === CURRENT STATE ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    FAnim_AnimationState CurrentAnimationState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    FAnim_AnimationState PreviousAnimationState;

    // === BLEND SPACE ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blend Space")
    FVector2D BlendSpaceInput;

    // === ANIMATION REFERENCES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TMap<EDir_MovementState, UAnimMontage*> StateMontages;

    // === SURVIVAL MODIFIERS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Modifiers")
    float StaminaSpeedModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Modifiers")
    float FearTremblingIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Modifiers")
    float InjuryLimpIntensity;

    // === CACHED REFERENCES ===
    UPROPERTY()
    class USkeletalMeshComponent* OwnerMesh;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // === INTERNAL METHODS ===
    void CacheAnimationReferences();
    void UpdateBlendSpaceFromState();
    void ApplySurvivalModifiers();
    bool ShouldTransitionState(const FAnim_AnimationState& NewState) const;
    void HandleStateTransition(const FAnim_AnimationState& OldState, const FAnim_AnimationState& NewState);
};