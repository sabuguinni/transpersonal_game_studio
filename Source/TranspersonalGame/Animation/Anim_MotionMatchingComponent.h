#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "TranspersonalGame/TranspersonalGame.h"
#include "Anim_MotionMatchingComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionFrame
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FRotator AngularVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    FVector RootMotionDelta;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float TimeStamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    int32 AnimationIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Data")
    float AnimationTime;

    FAnim_MotionFrame()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        AngularVelocity = FRotator::ZeroRotator;
        RootMotionDelta = FVector::ZeroVector;
        TimeStamp = 0.0f;
        AnimationIndex = -1;
        AnimationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionDatabase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    TArray<FAnim_MotionFrame> MotionFrames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    TArray<TSoftObjectPtr<UAnimSequence>> AnimSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    FString DatabaseName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    bool bIsBuilt;

    FAnim_MotionDatabase()
    {
        DatabaseName = TEXT("DefaultDatabase");
        bIsBuilt = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AccelerationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AngularVelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RootMotionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching", meta = (ClampMin = "0.01"))
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching", meta = (ClampMin = "1"))
    int32 MaxSearchFrames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Matching")
    bool bUseRootMotion;

    FAnim_MotionMatchingSettings()
    {
        VelocityWeight = 0.4f;
        AccelerationWeight = 0.3f;
        AngularVelocityWeight = 0.2f;
        RootMotionWeight = 0.1f;
        SearchRadius = 100.0f;
        MaxSearchFrames = 1000;
        BlendTime = 0.2f;
        bUseRootMotion = true;
    }
};

/**
 * Motion Matching Component for fluid character animation
 * Implements a simplified motion matching system for natural movement transitions
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    // Motion Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    FAnim_MotionDatabase MotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<TSoftObjectPtr<UAnimSequence>> SourceAnimations;

    // Matching Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_MotionMatchingSettings MatchingSettings;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Current State", meta = (AllowPrivateAccess = "true"))
    FAnim_MotionFrame CurrentFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Current State", meta = (AllowPrivateAccess = "true"))
    FAnim_MotionFrame PreviousFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Current State", meta = (AllowPrivateAccess = "true"))
    int32 CurrentBestMatch;

    UPROPERTY(BlueprintReadOnly, Category = "Current State", meta = (AllowPrivateAccess = "true"))
    float MatchScore;

    // Character References
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

    // Runtime Data
    TArray<FAnim_MotionFrame> FrameHistory;
    float LastUpdateTime;
    bool bDatabaseReady;

public:
    // Database Management
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddAnimationToDatabase(UAnimSequence* Animation);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearDatabase();

    // Motion Matching
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    int32 FindBestMotionMatch(const FAnim_MotionFrame& QueryFrame);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionDistance(const FAnim_MotionFrame& FrameA, const FAnim_MotionFrame& FrameB);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionFrame GetCurrentMotionFrame();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatching(float DeltaTime);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsDatabaseReady() const { return bDatabaseReady; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    int32 GetDatabaseSize() const { return MotionDatabase.MotionFrames.Num(); }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetCurrentMatchScore() const { return MatchScore; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    int32 GetCurrentBestMatch() const { return CurrentBestMatch; }

protected:
    // Internal Methods
    void InitializeComponent();
    void UpdateCurrentFrame(float DeltaTime);
    FAnim_MotionFrame ExtractMotionFromAnimation(UAnimSequence* Animation, float Time);
    void ProcessAnimationSequence(UAnimSequence* Animation, int32 AnimIndex);

private:
    // Helper functions
    FVector CalculateVelocityFromRootMotion(UAnimSequence* Animation, float Time, float DeltaTime);
    FVector CalculateAccelerationFromVelocity(const FVector& CurrentVel, const FVector& PrevVel, float DeltaTime);
    FRotator CalculateAngularVelocity(const FRotator& CurrentRot, const FRotator& PrevRot, float DeltaTime);
};