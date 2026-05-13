#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "Anim_PrehistoricBlendSpace.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimSequence* AnimSequence = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FVector SampleValue = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FString AnimationName = "";

    FAnim_BlendSpaceEntry()
    {
        AnimSequence = nullptr;
        SampleValue = FVector::ZeroVector;
        AnimationName = "";
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementBlendData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    float Lean = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Movement")
    bool bIsMoving = false;

    FAnim_MovementBlendData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Lean = 0.0f;
        bIsMoving = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_PrehistoricBlendSpace : public UObject
{
    GENERATED_BODY()

public:
    UAnim_PrehistoricBlendSpace();

    // Core blend space management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void InitializeBlendSpace();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateBlendSpace(const FAnim_MovementBlendData& MovementData);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    UAnimSequence* GetCurrentAnimation() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetBlendWeight(int32 AnimationIndex) const;

    // Animation management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void AddAnimationEntry(UAnimSequence* Animation, const FVector& SampleValue, const FString& Name);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void RemoveAnimationEntry(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void ClearAllAnimations();

    // Blend space configuration
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetBlendSpaceRange(float MinSpeed, float MaxSpeed, float MinDirection, float MaxDirection);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetSmoothingSettings(float BlendTime, float DirectionSmoothTime);

protected:
    // Animation entries
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TArray<FAnim_BlendSpaceEntry> AnimationEntries;

    // Current blend state
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    FAnim_MovementBlendData CurrentBlendData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    FAnim_MovementBlendData TargetBlendData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    TArray<float> BlendWeights;

    // Blend space parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MinSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MinDirection = -180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxDirection = 180.0f;

    // Smoothing settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float BlendSmoothTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DirectionSmoothTime = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SpeedSmoothTime = 0.1f;

    // Internal state
    UPROPERTY()
    bool bIsInitialized = false;

    UPROPERTY()
    float LastUpdateTime = 0.0f;

private:
    // Internal helper functions
    void CalculateBlendWeights();
    void SmoothBlendData(float DeltaTime);
    float CalculateDistance(const FVector& A, const FVector& B) const;
    void NormalizeWeights();

public:
    // Prehistoric-specific animation sets
    UFUNCTION(BlueprintCallable, Category = "Prehistoric Animations")
    void SetupHumanMovementAnimations();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Animations")
    void SetupTribalHunterAnimations();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Animations")
    void SetupSurvivalistAnimations();

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugPrintBlendState() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetBlendStateString() const;
};