#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Animation/AnimSequence.h"
#include "Anim_BlendSpaceManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    UBlendSpace* BlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float XValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float YValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    bool bIsActive;

    FAnim_BlendSpaceData()
    {
        BlendSpace = nullptr;
        XValue = 0.0f;
        YValue = 0.0f;
        BlendWeight = 1.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpace1DData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space 1D")
    UBlendSpace1D* BlendSpace1D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space 1D")
    float Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space 1D")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space 1D")
    bool bIsActive;

    FAnim_BlendSpace1DData()
    {
        BlendSpace1D = nullptr;
        Value = 0.0f;
        BlendWeight = 1.0f;
        bIsActive = true;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_BlendSpaceManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_BlendSpaceManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Locomotion Blend Spaces
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    FAnim_BlendSpaceData LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    FAnim_BlendSpace1DData IdleBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    FAnim_BlendSpace1DData WalkBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    FAnim_BlendSpace1DData RunBlendSpace;

    // Combat Blend Spaces
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FAnim_BlendSpaceData CombatBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FAnim_BlendSpace1DData AttackBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FAnim_BlendSpace1DData BlockBlendSpace;

    // Interaction Blend Spaces
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FAnim_BlendSpace1DData CraftingBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FAnim_BlendSpace1DData GatheringBlendSpace;

    // Blend Space Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float BlendSpaceInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoUpdateBlendSpaces;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void UpdateLocomotionBlendSpace(float Speed, float Direction);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void UpdateCombatBlendSpace(float Intensity, float Direction);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void SetBlendSpaceValue(FAnim_BlendSpaceData& BlendSpaceData, float X, float Y);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void SetBlendSpace1DValue(FAnim_BlendSpace1DData& BlendSpace1DData, float Value);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void SetBlendSpaceActive(FAnim_BlendSpaceData& BlendSpaceData, bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void SetBlendSpace1DActive(FAnim_BlendSpace1DData& BlendSpace1DData, bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    float GetBlendSpaceXValue(const FAnim_BlendSpaceData& BlendSpaceData);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    float GetBlendSpaceYValue(const FAnim_BlendSpaceData& BlendSpaceData);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    float GetBlendSpace1DValue(const FAnim_BlendSpace1DData& BlendSpace1DData);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void InterpolateBlendSpaceValues(float DeltaTime);

private:
    void InterpolateBlendSpace(FAnim_BlendSpaceData& BlendSpaceData, float TargetX, float TargetY, float DeltaTime);
    void InterpolateBlendSpace1D(FAnim_BlendSpace1DData& BlendSpace1DData, float TargetValue, float DeltaTime);
    
    // Target values for interpolation
    float TargetLocomotionX;
    float TargetLocomotionY;
    float TargetCombatX;
    float TargetCombatY;
};