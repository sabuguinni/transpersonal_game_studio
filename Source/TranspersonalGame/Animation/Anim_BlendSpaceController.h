#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Kismet/KismetMathLibrary.h"
#include "Anim_BlendSpaceController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementBlendData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float LeanAngle;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float AimYaw;

    FAnim_MovementBlendData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        LeanAngle = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
        AimPitch = 0.0f;
        AimYaw = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Blend Space")
    TSoftObjectPtr<UBlendSpace> LocomotionBlendSpace;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Blend Space")
    TSoftObjectPtr<UBlendSpace1D> IdleBlendSpace;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Blend Space")
    TSoftObjectPtr<UBlendSpace> AimOffsetBlendSpace;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
    float SpeedInterpRate;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
    float DirectionInterpRate;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
    float LeanInterpRate;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
    float MovingThreshold;

    FAnim_BlendSpaceSettings()
    {
        SpeedInterpRate = 5.0f;
        DirectionInterpRate = 10.0f;
        LeanInterpRate = 8.0f;
        MovingThreshold = 3.0f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_BlendSpaceController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_BlendSpaceController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Blend Space Settings
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Blend Spaces")
    FAnim_BlendSpaceSettings BlendSpaceSettings;

    // Current blend data
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_MovementBlendData CurrentBlendData;

    // Smoothed blend data
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_MovementBlendData SmoothedBlendData;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementBlendData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FAnim_MovementBlendData CalculateBlendData();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetBlendSpaceSettings(const FAnim_BlendSpaceSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetNormalizedSpeed();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetMovementDirection();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetLeanAngle();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldUseLocomotion();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldUseAimOffset();

    // Getters for animation blueprint
    UFUNCTION(BlueprintPure, Category = "Animation Data")
    float GetBlendSpaceX() const { return SmoothedBlendData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    float GetBlendSpaceY() const { return SmoothedBlendData.Direction; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    float GetAimPitch() const { return SmoothedBlendData.AimPitch; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    float GetAimYaw() const { return SmoothedBlendData.AimYaw; }

private:
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    // Internal calculation functions
    float CalculateSpeed();
    float CalculateDirection();
    float CalculateLeanAngle();
    void CalculateAimRotation(float& OutPitch, float& OutYaw);
    void SmoothBlendData(float DeltaTime);

    // Previous frame data for calculations
    FVector LastVelocity;
    FRotator LastRotation;
    float LastDeltaTime;
};