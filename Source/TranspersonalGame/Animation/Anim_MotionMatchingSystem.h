#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_MotionMatchingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionFrame
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion")
    FVector Velocity;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion")
    FVector Acceleration;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion")
    float Speed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion")
    float Direction;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion")
    bool bIsInAir;

    FAnim_MotionFrame()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionClip
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion Clip")
    UAnimSequence* Animation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion Clip")
    TArray<FAnim_MotionFrame> MotionData;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion Clip")
    float StartTime;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion Clip")
    float EndTime;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion Clip")
    FString ClipName;

    FAnim_MotionClip()
    {
        Animation = nullptr;
        StartTime = 0.0f;
        EndTime = 0.0f;
        ClipName = TEXT("Default");
    }
};

UENUM(BlueprintType)
enum class EAnim_MotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Crawling    UMETA(DisplayName = "Crawling")
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
    // Motion Database
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion Database")
    TArray<FAnim_MotionClip> MotionDatabase;

    // Current Motion State
    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    FAnim_MotionFrame CurrentMotion;

    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    EAnim_MotionState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    FAnim_MotionClip* CurrentClip;

    UPROPERTY(BlueprintReadOnly, Category = "Motion State")
    float CurrentPlayTime;

    // Motion Matching Settings
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion Matching")
    float VelocityWeight;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion Matching")
    float AccelerationWeight;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion Matching")
    float DirectionWeight;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion Matching")
    float SearchRadius;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Motion Matching")
    float BlendTime;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionClip* FindBestMatch();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionCost(const FAnim_MotionFrame& TargetMotion, const FAnim_MotionFrame& ClipMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void TransitionToClip(FAnim_MotionClip* NewClip, float StartTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_MotionState DetermineMotionState();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionClip(UAnimSequence* Animation, const FString& ClipName);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PreprocessMotionDatabase();

private:
    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    ACharacter* OwnerCharacter;

    FVector PreviousVelocity;
    float TransitionTimer;
    bool bIsTransitioning;
};