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
#include "Anim_StateManager.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_ActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Combat          UMETA(DisplayName = "Combat"),
    Interaction     UMETA(DisplayName = "Interaction"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Sleeping        UMETA(DisplayName = "Sleeping")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_StateData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState MovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_ActionState ActionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsCrouched;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float AimYaw;

    FAnim_StateData()
    {
        MovementState = EAnim_MovementState::Idle;
        ActionState = EAnim_ActionState::None;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouched = false;
        AimPitch = 0.0f;
        AimYaw = 0.0f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_StateManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_StateManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // State Management
    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation State")
    void SetActionState(EAnim_ActionState NewActionState);

    UFUNCTION(BlueprintPure, Category = "Animation State")
    FAnim_StateData GetCurrentStateData() const { return CurrentStateData; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    EAnim_MovementState GetMovementState() const { return CurrentStateData.MovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    EAnim_ActionState GetActionState() const { return CurrentStateData.ActionState; }

    // Animation Playback
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayActionMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopActionMontage(float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsPlayingActionMontage() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    FAnim_StateData CurrentStateData;

    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    UAnimInstance* AnimInstance;

    // State transition thresholds
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float WalkThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float RunThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float DirectionSmoothingSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float AimSmoothingSpeed = 15.0f;

private:
    void UpdateMovementData();
    void UpdateAimData();
    EAnim_MovementState CalculateMovementState() const;
};