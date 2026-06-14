#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/DataAsset.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsFalling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsCrouching;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsFalling = false;
        bIsCrouching = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clip")
    class UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clip")
    FAnim_MotionData MotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clip")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clip")
    float PlayRate;

    FAnim_MotionClip()
    {
        AnimSequence = nullptr;
        BlendWeight = 0.0f;
        PlayRate = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingComponent();

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
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SpeedWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float BlendTime;

    // Character reference
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    // Animation instance reference
    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // Motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionClip FindBestMatch();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionScore(const FAnim_MotionClip& Clip);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BlendToClip(const FAnim_MotionClip& TargetClip);

    // Database management
    UFUNCTION(BlueprintCallable, Category = "Database")
    void AddMotionClip(UAnimSequence* Animation, const FAnim_MotionData& MotionData);

    UFUNCTION(BlueprintCallable, Category = "Database")
    void ClearDatabase();

    UFUNCTION(CallInEditor, Category = "Database")
    void InitializeDefaultDatabase();

private:
    // Internal state
    float CurrentBlendTime;
    FAnim_MotionClip PreviousClip;
    bool bIsBlending;
};

#include "Anim_MotionMatchingComponent.generated.h"