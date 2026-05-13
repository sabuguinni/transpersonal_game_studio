#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "../SharedTypes.h"
#include "Anim_BlendSpaceController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    UBlendSpace* MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    UBlendSpace1D* SpeedBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float Lean;

    FAnim_BlendSpaceData()
    {
        MovementBlendSpace = nullptr;
        SpeedBlendSpace = nullptr;
        Speed = 0.0f;
        Direction = 0.0f;
        Lean = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_LocomotionParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    float Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    float DirectionAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    float LeanAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    bool bIsRunning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    bool bIsInAir;

    FAnim_LocomotionParameters()
    {
        Velocity = 0.0f;
        DirectionAngle = 0.0f;
        LeanAmount = 0.0f;
        bIsMoving = false;
        bIsRunning = false;
        bIsInAir = false;
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

    // Core blend space management
    UFUNCTION(BlueprintCallable, Category = "Animation|Blend Space")
    void UpdateBlendSpaceParameters(float Speed, float Direction, float Lean);

    UFUNCTION(BlueprintCallable, Category = "Animation|Blend Space")
    void SetMovementBlendSpace(UBlendSpace* NewBlendSpace);

    UFUNCTION(BlueprintCallable, Category = "Animation|Blend Space")
    void SetSpeedBlendSpace(UBlendSpace1D* NewBlendSpace);

    // Locomotion parameter calculation
    UFUNCTION(BlueprintCallable, Category = "Animation|Locomotion")
    FAnim_LocomotionParameters CalculateLocomotionParameters(const FVector& Velocity, const FRotator& ActorRotation);

    UFUNCTION(BlueprintCallable, Category = "Animation|Locomotion")
    float CalculateDirectionAngle(const FVector& Velocity, const FRotator& ActorRotation);

    UFUNCTION(BlueprintCallable, Category = "Animation|Locomotion")
    float CalculateLeanAmount(const FVector& Velocity, float DeltaTime);

    // Blend space evaluation
    UFUNCTION(BlueprintCallable, Category = "Animation|Blend Space")
    void EvaluateBlendSpace(UBlendSpace* BlendSpace, float XValue, float YValue, TArray<FBlendSampleData>& OutSampleData);

    UFUNCTION(BlueprintCallable, Category = "Animation|Blend Space")
    void EvaluateBlendSpace1D(UBlendSpace1D* BlendSpace, float Value, TArray<FBlendSampleData>& OutSampleData);

    // Smoothing and interpolation
    UFUNCTION(BlueprintCallable, Category = "Animation|Smoothing")
    float SmoothParameter(float CurrentValue, float TargetValue, float SmoothingSpeed, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation|Smoothing")
    void SmoothAllParameters(float DeltaTime);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Animation|Blend Space")
    FAnim_BlendSpaceData GetBlendSpaceData() const { return BlendSpaceData; }

    UFUNCTION(BlueprintPure, Category = "Animation|Locomotion")
    FAnim_LocomotionParameters GetLocomotionParameters() const { return LocomotionParams; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space", meta = (AllowPrivateAccess = "true"))
    FAnim_BlendSpaceData BlendSpaceData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    FAnim_LocomotionParameters LocomotionParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (AllowPrivateAccess = "true"))
    float SpeedSmoothingRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (AllowPrivateAccess = "true"))
    float DirectionSmoothingRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (AllowPrivateAccess = "true"))
    float LeanSmoothingRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (AllowPrivateAccess = "true"))
    float MovementThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (AllowPrivateAccess = "true"))
    float RunningThreshold;

private:
    // Cached values for smoothing
    float PreviousSpeed;
    float PreviousDirection;
    float PreviousLean;
    
    // Reference to owner character
    class ACharacter* OwnerCharacter;
    
    void CacheOwnerCharacter();
    void UpdateLocomotionFromCharacter();
};