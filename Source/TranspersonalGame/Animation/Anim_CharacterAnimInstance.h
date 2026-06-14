#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "../Core/Characters/Char_CharacterVariations.h"
#include "Anim_CharacterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Combat      UMETA(DisplayName = "Combat")
};

UENUM(BlueprintType)
enum class EAnim_WeaponType : uint8
{
    None        UMETA(DisplayName = "None"),
    Spear       UMETA(DisplayName = "Spear"),
    Club        UMETA(DisplayName = "Club"),
    Bow         UMETA(DisplayName = "Bow"),
    Axe         UMETA(DisplayName = "Axe"),
    Knife       UMETA(DisplayName = "Knife"),
    Sling       UMETA(DisplayName = "Sling")
};

USTRUCT(BlueprintType)
struct FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float JumpHeight = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Acceleration = 0.0f;
};

USTRUCT(BlueprintType)
struct FAnim_IKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LeftFootIKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector RightFootIKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootIKRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootIKRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float LeftFootIKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float RightFootIKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float HipOffset = 0.0f;
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_WeaponType CurrentWeaponType;

    UPROPERTY(BlueprintReadOnly, Category = "Movement Data")
    FAnim_MovementData MovementData;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_IKData IKData;

    UPROPERTY(BlueprintReadOnly, Category = "Character Archetype")
    EChar_CharacterArchetype CharacterArchetype;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Modifiers")
    float ArchetypeSpeedModifier = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Modifiers")
    float ArchetypeAggressionModifier = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Modifiers")
    float ArchetypePrecisionModifier = 1.0f;

private:
    void UpdateMovementData(float DeltaTime);
    void UpdateMovementState();
    void UpdateIKData(float DeltaTime);
    void UpdateArchetypeModifiers();
    
    FVector PerformFootTrace(const FVector& FootLocation, float TraceDistance = 50.0f);
    float CalculateFootIKAlpha(const FVector& IKLocation);

    UPROPERTY()
    float LastUpdateTime = 0.0f;

    UPROPERTY()
    float IKInterpSpeed = 15.0f;

    UPROPERTY()
    float FootTraceDistance = 100.0f;
};