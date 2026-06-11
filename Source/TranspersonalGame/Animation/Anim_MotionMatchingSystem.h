#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsOnGround;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float TurnRate;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = FVector::ForwardVector;
        bIsOnGround = true;
        bIsCrouching = false;
        TurnRate = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionData> MotionFrames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float ClipDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FString ClipName;

    FAnim_MotionClip()
    {
        AnimSequence = nullptr;
        ClipDuration = 0.0f;
        ClipName = TEXT("DefaultClip");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion Matching Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionClip> MotionDatabase;

    // Current Motion State
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionData CurrentMotionData;

    // Best Match Results
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    class UAnimSequence* BestMatchAnimation;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float BestMatchTime;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float MatchScore;

    // Motion Matching Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float SpeedWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    int32 MaxSearchFrames;

    // Character Reference
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    class ACharacter* OwnerCharacter;

    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void FindBestMotionMatch();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionScore(const FAnim_MotionData& TargetMotion, const FAnim_MotionData& ClipMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionClip(UAnimSequence* Animation, const FString& ClipName);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayBestMatch();

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching Debug")
    void DebugDrawMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching Debug")
    void LogMotionMatchingInfo();

private:
    // Internal motion matching logic
    void ExtractMotionDataFromAnimation(UAnimSequence* Animation, TArray<FAnim_MotionData>& OutMotionFrames);
    
    // Performance optimization
    float LastUpdateTime;
    float UpdateFrequency;
};