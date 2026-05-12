#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Anim_EnhancedCharacterController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Crawling    UMETA(DisplayName = "Crawling")
};

UENUM(BlueprintType)
enum class EAnim_ActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Combat          UMETA(DisplayName = "Combat"),
    Interaction     UMETA(DisplayName = "Interaction"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Swimming        UMETA(DisplayName = "Swimming")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float AimYaw;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float AimPitch;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        AimYaw = 0.0f;
        AimPitch = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* IdleBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* WalkBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* RunBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* CombatBlendSpace;

    FAnim_BlendSpaceData()
    {
        IdleBlendSpace = nullptr;
        WalkBlendSpace = nullptr;
        RunBlendSpace = nullptr;
        CombatBlendSpace = nullptr;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_EnhancedCharacterController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_EnhancedCharacterController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Animation Functions
    UFUNCTION(BlueprintCallable, Category = "Enhanced Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Enhanced Animation")
    void UpdateActionState(EAnim_ActionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Animation")
    void PlayActionMontage(class UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Animation")
    void StopActionMontage(float BlendOutTime = 0.25f);

    // Movement Animation Functions
    UFUNCTION(BlueprintCallable, Category = "Enhanced Animation")
    void SetMovementBlendSpace(EAnim_MovementState State, class UBlendSpace* BlendSpace);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Animation")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintCallable, Category = "Enhanced Animation")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintCallable, Category = "Enhanced Animation")
    EAnim_ActionState GetCurrentActionState() const { return CurrentActionState; }

    // Enhanced Features
    UFUNCTION(BlueprintCallable, Category = "Enhanced Animation")
    void EnableFootIK(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Animation")
    void SetLookAtTarget(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Enhanced Animation")
    void ClearLookAtTarget();

    UFUNCTION(BlueprintCallable, Category = "Enhanced Animation")
    void SetEmotionalState(float Happiness, float Fear, float Anger, float Calm);

protected:
    // Core References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enhanced Animation")
    class ACharacter* OwnerCharacter;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enhanced Animation")
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enhanced Animation")
    class UAnimInstance* AnimInstance;

    // Animation States
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enhanced Animation")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enhanced Animation")
    EAnim_ActionState CurrentActionState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enhanced Animation")
    FAnim_MovementData MovementData;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Animation")
    FAnim_BlendSpaceData BlendSpaces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Animation")
    TMap<EAnim_ActionState, class UAnimMontage*> ActionMontages;

    // Enhanced Features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Animation")
    bool bFootIKEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Animation")
    bool bLookAtEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Animation")
    FVector LookAtTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Animation")
    float LookAtAlpha;

    // Emotional State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Animation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EmotionalHappiness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Animation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EmotionalFear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Animation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EmotionalAnger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Animation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EmotionalCalm;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Animation")
    float UpdateFrequency;

    UPROPERTY()
    float LastUpdateTime;

private:
    void InitializeComponent();
    void UpdateMovementData();
    void UpdateLookAt(float DeltaTime);
    EAnim_MovementState CalculateMovementState();
};