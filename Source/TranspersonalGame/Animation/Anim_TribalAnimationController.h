#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "Anim_TribalAnimationController.generated.h"

UENUM(BlueprintType)
enum class EAnim_TribalMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming")
};

UENUM(BlueprintType)
enum class EAnim_SurvivalAction : uint8
{
    None        UMETA(DisplayName = "None"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Building    UMETA(DisplayName = "Building"),
    Eating      UMETA(DisplayName = "Eating"),
    Drinking    UMETA(DisplayName = "Drinking")
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
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    EAnim_TribalMovementState MovementState;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_TribalMovementState::Idle;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_TribalAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_TribalAnimationController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Movement animation control
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementData(float Speed, float Direction, bool bInAir, bool bCrouching);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_TribalMovementState NewState);

    // Survival action animations
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlaySurvivalAction(EAnim_SurvivalAction Action);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopCurrentAction();

    // Montage management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayAnimationMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAnimationMontage(UAnimMontage* Montage);

    // IK and procedural animation
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void EnableFootIK(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateFootIK(float DeltaTime);

protected:
    // Movement data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    // Current survival action
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    EAnim_SurvivalAction CurrentAction;

    // Animation montages
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* HuntingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* BuildingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* EatingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* DrinkingMontage;

    // IK settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootIKInterpSpeed;

    // Character references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

private:
    // Internal animation state
    float LastUpdateTime;
    bool bIsPlayingAction;
    
    // IK data
    float LeftFootOffset;
    float RightFootOffset;
    FVector HipOffset;

    // Helper functions
    void InitializeAnimationReferences();
    void UpdateAnimationState(float DeltaTime);
    float CalculateFootOffset(const FVector& FootLocation, const FVector& TraceStart);
    void ApplyFootIK();
};