#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

class USkeletalMeshComponent;
class UAnimSequence;

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
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    ESurvivalState SurvivalState;

    FAnim_MotionData()
        : Velocity(FVector::ZeroVector)
        , Speed(0.0f)
        , Direction(0.0f)
        , bIsInAir(false)
        , bIsCrouching(false)
        , SurvivalState(ESurvivalState::Normal)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    FAnim_MotionData MotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    float Quality;

    FAnim_MotionClip()
        : AnimSequence(nullptr)
        , StartTime(0.0f)
        , EndTime(0.0f)
        , Quality(1.0f)
    {}
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Motion matching configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TArray<FAnim_MotionClip> MotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float UpdateFrequency;

    // Current motion state
    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    FAnim_MotionClip* CurrentClip;

    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    float CurrentPlayTime;

    // Component references
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* SkeletalMeshComponent;

    // Motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionClip* FindBestMatchingClip(const FAnim_MotionData& TargetMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayMotionClip(FAnim_MotionClip* Clip);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionDistance(const FAnim_MotionData& A, const FAnim_MotionData& B);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionClip(UAnimSequence* Animation, float StartTime, float EndTime, const FAnim_MotionData& MotionData);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase();

private:
    float LastUpdateTime;
    bool bIsInitialized;

    void InitializeComponent();
    void UpdateCurrentMotion(float DeltaTime);
    FVector GetCharacterVelocity() const;
    bool IsCharacterInAir() const;
    bool IsCharacterCrouching() const;
};