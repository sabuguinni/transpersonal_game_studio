#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Anim_SwimmingAnimationController.generated.h"

UENUM(BlueprintType)
enum class EAnim_SwimmingAnimState : uint8
{
    NotSwimming     UMETA(DisplayName = "Not Swimming"),
    SurfaceSwim     UMETA(DisplayName = "Surface Swimming"),
    Underwater      UMETA(DisplayName = "Underwater"),
    Diving          UMETA(DisplayName = "Diving Down"),
    Surfacing       UMETA(DisplayName = "Surfacing Up"),
    BreathHolding   UMETA(DisplayName = "Breath Holding"),
    Treading        UMETA(DisplayName = "Treading Water")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_SwimmingAnimData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Animation")
    EAnim_SwimmingAnimState CurrentSwimState = EAnim_SwimmingAnimState::NotSwimming;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Animation")
    float SwimSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Animation")
    float WaterDepth = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Animation")
    float BreathPercentage = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Animation")
    bool bIsUnderwater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Animation")
    bool bIsDiving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Animation")
    bool bIsSurfacing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Animation")
    FVector SwimDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Animation")
    float StrokeIntensity = 1.0f;

    FAnim_SwimmingAnimData()
    {
        CurrentSwimState = EAnim_SwimmingAnimState::NotSwimming;
        SwimSpeed = 0.0f;
        WaterDepth = 0.0f;
        BreathPercentage = 100.0f;
        bIsUnderwater = false;
        bIsDiving = false;
        bIsSurfacing = false;
        SwimDirection = FVector::ZeroVector;
        StrokeIntensity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_SwimmingMontageSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Montages")
    class UAnimMontage* SurfaceSwimMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Montages")
    class UAnimMontage* UnderwaterSwimMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Montages")
    class UAnimMontage* DivingMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Montages")
    class UAnimMontage* SurfacingMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Montages")
    class UAnimMontage* BreathHoldMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Montages")
    class UAnimMontage* TreadingWaterMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Montages")
    float MontagePlayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Montages")
    float BlendInTime = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Montages")
    float BlendOutTime = 0.25f;

    FAnim_SwimmingMontageSettings()
    {
        SurfaceSwimMontage = nullptr;
        UnderwaterSwimMontage = nullptr;
        DivingMontage = nullptr;
        SurfacingMontage = nullptr;
        BreathHoldMontage = nullptr;
        TreadingWaterMontage = nullptr;
        MontagePlayRate = 1.0f;
        BlendInTime = 0.25f;
        BlendOutTime = 0.25f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation))
class TRANSPERSONALGAME_API UAnim_SwimmingAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_SwimmingAnimationController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Swimming animation data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Animation")
    FAnim_SwimmingAnimData SwimmingData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming Animation")
    FAnim_SwimmingMontageSettings MontageSettings;

    // Animation control functions
    UFUNCTION(BlueprintCallable, Category = "Swimming Animation")
    void UpdateSwimmingAnimation(EAnim_SwimmingAnimState NewState, float SwimSpeed, float WaterDepth);

    UFUNCTION(BlueprintCallable, Category = "Swimming Animation")
    void PlaySwimmingMontage(EAnim_SwimmingAnimState SwimState);

    UFUNCTION(BlueprintCallable, Category = "Swimming Animation")
    void StopCurrentSwimmingMontage();

    UFUNCTION(BlueprintCallable, Category = "Swimming Animation")
    void UpdateBreathHoldingAnimation(float BreathPercentage);

    UFUNCTION(BlueprintCallable, Category = "Swimming Animation")
    void SetSwimmingDirection(FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "Swimming Animation")
    void SetStrokeIntensity(float Intensity);

    // State queries
    UFUNCTION(BlueprintPure, Category = "Swimming Animation")
    EAnim_SwimmingAnimState GetCurrentSwimState() const { return SwimmingData.CurrentSwimState; }

    UFUNCTION(BlueprintPure, Category = "Swimming Animation")
    bool IsSwimming() const { return SwimmingData.CurrentSwimState != EAnim_SwimmingAnimState::NotSwimming; }

    UFUNCTION(BlueprintPure, Category = "Swimming Animation")
    bool IsUnderwater() const { return SwimmingData.bIsUnderwater; }

    UFUNCTION(BlueprintPure, Category = "Swimming Animation")
    float GetSwimSpeed() const { return SwimmingData.SwimSpeed; }

    UFUNCTION(BlueprintPure, Category = "Swimming Animation")
    float GetBreathPercentage() const { return SwimmingData.BreathPercentage; }

private:
    // Internal animation management
    void TransitionToSwimState(EAnim_SwimmingAnimState NewState);
    void UpdateSwimSpeedBasedAnimation();
    void HandleBreathHoldingEffects();
    
    // Animation instance reference
    UPROPERTY()
    class UAnimInstance* CachedAnimInstance = nullptr;

    // Current montage tracking
    UPROPERTY()
    class UAnimMontage* CurrentPlayingMontage = nullptr;

    // Timing and blending
    float StateTransitionTimer = 0.0f;
    float LastSwimSpeedUpdate = 0.0f;
    bool bIsTransitioning = false;
};

#include "Anim_SwimmingAnimationController.generated.h"