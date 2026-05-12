#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionFrame
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Acceleration;

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
    float TimeStamp;

    FAnim_MotionFrame()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsFalling = false;
        bIsJumping = false;
        TimeStamp = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    UAnimSequence* AnimationSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    TArray<FAnim_MotionFrame> MotionFrames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    float ClipDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    FString ClipName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Clip")
    ECharacterMovementState MovementState;

    FAnim_MotionClip()
    {
        AnimationSequence = nullptr;
        ClipDuration = 0.0f;
        ClipName = TEXT("Default");
        MovementState = ECharacterMovementState::Idle;
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

    // Motion matching core functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void InitializeMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionClip FindBestMatchingClip(const FAnim_MotionFrame& CurrentFrame);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateCurrentMotionFrame();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionDistance(const FAnim_MotionFrame& FrameA, const FAnim_MotionFrame& FrameB);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionClipToDatabase(UAnimSequence* AnimSequence, ECharacterMovementState State);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayMatchedAnimation(const FAnim_MotionClip& MotionClip);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetCharacterReference(ACharacter* InCharacter);

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching Debug")
    void DebugDrawMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching Debug")
    FString GetCurrentMotionInfo();

protected:
    // Motion database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FAnim_MotionClip> MotionDatabase;

    // Current motion state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current Motion")
    FAnim_MotionFrame CurrentMotionFrame;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current Motion")
    FAnim_MotionClip CurrentPlayingClip;

    // Character references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
    ACharacter* OwnerCharacter;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
    UCharacterMovementComponent* MovementComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
    USkeletalMeshComponent* MeshComponent;

    // Motion matching settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float AccelerationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float SpeedWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float MatchingThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float BlendTime;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDatabaseSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

private:
    float LastUpdateTime;
    int32 CurrentClipIndex;
    bool bIsInitialized;

    // Helper functions
    void ExtractMotionDataFromAnimation(UAnimSequence* AnimSequence, FAnim_MotionClip& OutClip);
    void ValidateMotionDatabase();
    void CacheCharacterReferences();
};