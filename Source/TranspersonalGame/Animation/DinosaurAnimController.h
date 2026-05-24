#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "DinosaurAnimController.generated.h"

UENUM(BlueprintType)
enum class EAnim_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Alert       UMETA(DisplayName = "Alert"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Injured     UMETA(DisplayName = "Injured"),
    Death       UMETA(DisplayName = "Death")
};

UENUM(BlueprintType)
enum class EAnim_DinosaurSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus UMETA(DisplayName = "Stegosaurus"),
    Pteranodon  UMETA(DisplayName = "Pteranodon"),
    Compsognathus UMETA(DisplayName = "Compsognathus")
};

USTRUCT(BlueprintType)
struct FAnim_DinosaurAnimationSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* AttackAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* FeedingAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* AlertAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* DeathAnimation;

    FAnim_DinosaurAnimationSet()
    {
        IdleAnimation = nullptr;
        LocomotionBlendSpace = nullptr;
        AttackAnimation = nullptr;
        FeedingAnimation = nullptr;
        AlertAnimation = nullptr;
        DeathAnimation = nullptr;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurAnimController : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurAnimController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Animation state management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void SetAnimationState(EAnim_DinosaurState NewState);

    UFUNCTION(BlueprintPure, Category = "Dinosaur Animation")
    EAnim_DinosaurState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void SetSpecies(EAnim_DinosaurSpecies NewSpecies);

    UFUNCTION(BlueprintPure, Category = "Dinosaur Animation")
    EAnim_DinosaurSpecies GetSpecies() const { return Species; }

    // Animation playback
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void PlayAttackAnimation();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void PlayFeedingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void PlayAlertAnimation();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void PlayDeathAnimation();

    // Movement parameters
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void SetMovementSpeed(float Speed);

    UFUNCTION(BlueprintPure, Category = "Dinosaur Animation")
    float GetMovementSpeed() const { return MovementSpeed; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void SetMovementDirection(float Direction);

    UFUNCTION(BlueprintPure, Category = "Dinosaur Animation")
    float GetMovementDirection() const { return MovementDirection; }

    // Animation blending
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Animation")
    void SetBlendWeight(float Weight);

    UFUNCTION(BlueprintPure, Category = "Dinosaur Animation")
    float GetBlendWeight() const { return BlendWeight; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    EAnim_DinosaurState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    EAnim_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Parameters")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Parameters")
    float MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Parameters")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Sets")
    TMap<EAnim_DinosaurSpecies, FAnim_DinosaurAnimationSet> AnimationSets;

    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

private:
    void InitializeAnimationSets();
    void UpdateAnimationParameters();
    FAnim_DinosaurAnimationSet* GetCurrentAnimationSet();
};

#include "DinosaurAnimController.generated.h"