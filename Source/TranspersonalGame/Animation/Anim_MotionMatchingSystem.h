#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Anim_MotionMatchingSystem.generated.h"

// Motion matching pose data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionPoseData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    FVector Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    float GroundDistance;

    FAnim_MotionPoseData()
    {
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        GroundDistance = 0.0f;
    }
};

// Motion matching database entry
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionDatabaseEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Database")
    class UAnimSequence* AnimationSequence;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Database")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Database")
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Database")
    FAnim_MotionPoseData PoseData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Database")
    float Quality;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Database")
    TArray<FString> Tags;

    FAnim_MotionDatabaseEntry()
    {
        AnimationSequence = nullptr;
        StartTime = 0.0f;
        EndTime = 0.0f;
        Quality = 1.0f;
    }
};

/**
 * Advanced Motion Matching System for realistic character movement
 * Provides fluid transitions between locomotion states based on character velocity and context
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Motion matching database
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Database")
    TArray<FAnim_MotionDatabaseEntry> MotionDatabase;

    // Current pose data
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionPoseData CurrentPose;

    // Motion matching settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float AccelerationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float GroundDistanceWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    // Current best match
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    int32 CurrentBestMatchIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float CurrentMatchScore;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    class ACharacter* OwnerCharacter;

    // Animation instance reference
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    class UAnimInstance* AnimInstance;

public:
    // Motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateCurrentPose();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    int32 FindBestMotionMatch();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionScore(const FAnim_MotionDatabaseEntry& Entry);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ApplyMotionMatch(int32 MatchIndex);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionToDatabase(UAnimSequence* Animation, float StartTime, float EndTime, const TArray<FString>& Tags);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BuildMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ClearMotionDatabase();

    // Utility functions
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsValidMotionMatch(int32 Index) const;

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FString GetCurrentMotionInfo() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionMatchingEnabled(bool bEnabled);

private:
    // Internal state
    bool bMotionMatchingEnabled;
    float LastUpdateTime;
    FVector LastVelocity;
    FVector LastAcceleration;

    // Helper functions
    void InitializeMotionDatabase();
    void UpdateCharacterReferences();
    float CalculateVelocityScore(const FVector& TargetVelocity, const FVector& CurrentVelocity) const;
    float CalculateDirectionScore(float TargetDirection, float CurrentDirection) const;
};