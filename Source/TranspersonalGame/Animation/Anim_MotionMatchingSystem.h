#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsFalling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsJumping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float Acceleration;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsFalling = false;
        bIsJumping = false;
        Acceleration = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    FAnim_MotionData MotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    float Weight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    float BlendTime;

    FAnim_MotionClip()
    {
        AnimSequence = nullptr;
        Weight = 1.0f;
        BlendTime = 0.2f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion matching database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionClip> MotionDatabase;

    // Current motion state
    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    FAnim_MotionData CurrentMotionData;

    // Best matching clip
    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    FAnim_MotionClip BestMatchClip;

    // Motion matching settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float VelocityWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DirectionWeight = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float AccelerationWeight = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MinBlendTime = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxBlendTime = 0.5f;

    // Public functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(const FVector& InVelocity, bool bInIsMoving, bool bInIsFalling, bool bInIsJumping);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionClip FindBestMatchingClip(const FAnim_MotionData& TargetMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionScore(const FAnim_MotionData& MotionA, const FAnim_MotionData& MotionB);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionClip(UAnimSequence* AnimSeq, const FAnim_MotionData& MotionData);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    UAnimSequence* GetCurrentBestAnimation() const;

protected:
    // Internal functions
    void AnalyzeMotionData();
    void UpdateBestMatch();
    float NormalizeAngle(float Angle);

private:
    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // Internal state
    FVector PreviousVelocity;
    float LastUpdateTime;
    bool bIsInitialized;
};