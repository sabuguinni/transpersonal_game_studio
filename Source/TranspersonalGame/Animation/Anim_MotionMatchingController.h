#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "../Core/Characters/Char_CharacterVariations.h"
#include "Anim_MotionMatchingController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MotionMatchingState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Locomotion      UMETA(DisplayName = "Locomotion"),
    Jump            UMETA(DisplayName = "Jump"),
    Fall            UMETA(DisplayName = "Fall"),
    Land            UMETA(DisplayName = "Land"),
    Combat          UMETA(DisplayName = "Combat"),
    Interaction     UMETA(DisplayName = "Interaction"),
    Death           UMETA(DisplayName = "Death")
};

USTRUCT(BlueprintType)
struct FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Acceleration = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float TimeSinceLastStep = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector PredictedPosition = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FAnim_MotionMatchingDatabase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    TArray<class UAnimSequence*> IdleAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    TArray<class UAnimSequence*> WalkAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    TArray<class UAnimSequence*> RunAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    TArray<class UAnimSequence*> JumpAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    TArray<class UAnimSequence*> FallAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    TArray<class UAnimSequence*> LandAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    TArray<class UAnimSequence*> CombatAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    EChar_CharacterArchetype ArchetypeFilter = EChar_CharacterArchetype::TribalWarrior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    float DatabaseWeight = 1.0f;
};

USTRUCT(BlueprintType)
struct FAnim_MotionMatchingResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Result")
    class UAnimSequence* SelectedAnimation = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Result")
    float StartTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Result")
    float BlendWeight = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Result")
    float MatchingScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Result")
    EAnim_MotionMatchingState MatchedState = EAnim_MotionMatchingState::Idle;
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_MotionMatchingController : public UObject
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingController();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionMatchingResult FindBestMotionMatch(const FAnim_MotionData& CurrentMotion, EAnim_MotionMatchingState DesiredState);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetCharacterArchetype(EChar_CharacterArchetype NewArchetype);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void RegisterAnimationDatabase(const FAnim_MotionMatchingDatabase& Database);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData(const FVector& Velocity, const FVector& Acceleration, bool bInAir);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    TArray<UAnimSequence*> GetAnimationsForState(EAnim_MotionMatchingState State) const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionMatchingScore(const FAnim_MotionData& MotionA, const FAnim_MotionData& MotionB) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FAnim_MotionMatchingDatabase> AnimationDatabases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    EChar_CharacterArchetype CurrentArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float VelocityWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float DirectionWeight = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float AccelerationWeight = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float PredictionWeight = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ArchetypeMatchingBonus = 0.2f;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    FAnim_MotionData LastMotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    EAnim_MotionMatchingState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float StateTransitionTime = 0.0f;

private:
    float CalculateVelocityScore(const FVector& VelA, const FVector& VelB) const;
    float CalculateDirectionScore(float DirA, float DirB) const;
    float CalculateAccelerationScore(const FVector& AccelA, const FVector& AccelB) const;
    
    TArray<UAnimSequence*> FilterAnimationsByArchetype(const TArray<UAnimSequence*>& Animations) const;
    
    EAnim_MotionMatchingState DetermineStateFromMotion(const FAnim_MotionData& Motion) const;
    
    void PredictFutureMotion(FAnim_MotionData& Motion, float PredictionTime = 0.5f) const;
};