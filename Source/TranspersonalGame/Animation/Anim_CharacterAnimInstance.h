#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_CharacterAnimInstance.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

protected:
    // Animation state variables
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsAccelerating;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsRunning;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    class ACharacter* OwnerCharacter;

    // Movement component reference
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    class UCharacterMovementComponent* MovementComponent;

private:
    void UpdateMovementVariables();
    void UpdateStateVariables();
};