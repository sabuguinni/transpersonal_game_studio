#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Anim_MotionMatchingComponent.generated.h"

USTRUCT(BlueprintType)
struct FAnim_MotionFrame
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Time;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AnimationName;

    FAnim_MotionFrame()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Time = 0.0f;
        AnimationName = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FAnim_MotionDatabase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FAnim_MotionFrame> Frames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UAnimSequence*> Animations;

    FAnim_MotionDatabase()
    {
        Frames.Empty();
        Animations.Empty();
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
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
    FAnim_MotionDatabase MotionDatabase;

    // Current motion state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionFrame CurrentFrame;

    // Search parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float PositionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float RotationWeight;

    // Motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionFrame FindBestMatch(const FAnim_MotionFrame& TargetFrame);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateCurrentFrame();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddFrameToDatabase(const FAnim_MotionFrame& Frame);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildDatabaseFromAnimations();

private:
    float CalculateFrameDistance(const FAnim_MotionFrame& Frame1, const FAnim_MotionFrame& Frame2);
    
    UPROPERTY()
    class ACharacter* OwningCharacter;
};