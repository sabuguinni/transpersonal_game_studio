#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Engine/DataTable.h"
#include "Anim_MotionMatchingComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    class UAnimSequence* AnimSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector RootMotionVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FString MotionTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Duration;

    FAnim_MotionData()
    {
        AnimSequence = nullptr;
        RootMotionVelocity = FVector::ZeroVector;
        BlendWeight = 1.0f;
        MotionTag = TEXT("Default");
        Duration = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float VelocityWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DirectionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float AccelerationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxCandidates;

    FAnim_MotionMatchingSettings()
    {
        VelocityWeight = 1.0f;
        DirectionWeight = 0.8f;
        AccelerationWeight = 0.6f;
        BlendTime = 0.2f;
        MaxCandidates = 10;
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
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    TArray<FAnim_MotionData> MotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Database")
    class UDataTable* MotionDataTable;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FAnim_MotionMatchingSettings MatchingSettings;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FAnim_MotionData CurrentMotion;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentBlendWeight;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector DesiredVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector CurrentVelocity;

public:
    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatching(const FVector& InDesiredVelocity, const FVector& InCurrentVelocity);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionData FindBestMatch(const FVector& TargetVelocity, const FVector& CurrentVel);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void AddMotionToDatabase(const FAnim_MotionData& NewMotion);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void LoadMotionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMotionScore(const FAnim_MotionData& Motion, const FVector& TargetVel, const FVector& CurrentVel);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FAnim_MotionData GetCurrentMotion() const { return CurrentMotion; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetCurrentBlendWeight() const { return CurrentBlendWeight; }

private:
    void InitializeMotionDatabase();
    void BlendToMotion(const FAnim_MotionData& NewMotion, float BlendTime);
    
    // Character reference
    class ACharacter* OwnerCharacter;
    class USkeletalMeshComponent* SkeletalMesh;
};