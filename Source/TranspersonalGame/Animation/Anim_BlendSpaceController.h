#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Engine/Engine.h"
#include "Anim_BlendSpaceController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float Lean = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float Turn = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float Slope = 0.0f;

    FAnim_BlendSpaceData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Lean = 0.0f;
        Turn = 0.0f;
        Slope = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float SpeedSmoothingRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float DirectionSmoothingRate = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float LeanSmoothingRate = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float TurnSmoothingRate = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float MaxLeanAngle = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float MaxTurnRate = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    float SlopeDetectionDistance = 200.0f;

    FAnim_BlendSpaceSettings()
    {
        SpeedSmoothingRate = 5.0f;
        DirectionSmoothingRate = 8.0f;
        LeanSmoothingRate = 6.0f;
        TurnSmoothingRate = 4.0f;
        MaxLeanAngle = 15.0f;
        MaxTurnRate = 180.0f;
        SlopeDetectionDistance = 200.0f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_BlendSpaceController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_BlendSpaceController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Blend space data access
    UFUNCTION(BlueprintPure, Category = "Blend Space")
    FAnim_BlendSpaceData GetBlendSpaceData() const { return CurrentBlendData; }

    UFUNCTION(BlueprintPure, Category = "Blend Space")
    float GetSpeed() const { return CurrentBlendData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Blend Space")
    float GetDirection() const { return CurrentBlendData.Direction; }

    UFUNCTION(BlueprintPure, Category = "Blend Space")
    float GetLean() const { return CurrentBlendData.Lean; }

    UFUNCTION(BlueprintPure, Category = "Blend Space")
    float GetTurn() const { return CurrentBlendData.Turn; }

    UFUNCTION(BlueprintPure, Category = "Blend Space")
    float GetSlope() const { return CurrentBlendData.Slope; }

    // Manual overrides
    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void SetSpeedOverride(float Speed, float Duration = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void SetDirectionOverride(float Direction, float Duration = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void ClearOverrides();

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void SetBlendSpaceSettings(const FAnim_BlendSpaceSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Blend Space")
    FAnim_BlendSpaceSettings GetBlendSpaceSettings() const { return Settings; }

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void ResetBlendData();

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void SetSmoothingEnabled(bool bEnabled);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blend Space")
    FAnim_BlendSpaceData CurrentBlendData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blend Space")
    FAnim_BlendSpaceData TargetBlendData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    FAnim_BlendSpaceSettings Settings;

    // Component references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class UAnim_StateManager* StateManager;

    // Override system
    UPROPERTY()
    bool bSpeedOverride = false;

    UPROPERTY()
    bool bDirectionOverride = false;

    UPROPERTY()
    float SpeedOverrideValue = 0.0f;

    UPROPERTY()
    float DirectionOverrideValue = 0.0f;

    UPROPERTY()
    float SpeedOverrideEndTime = 0.0f;

    UPROPERTY()
    float DirectionOverrideEndTime = 0.0f;

    // Smoothing control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space Settings")
    bool bSmoothingEnabled = true;

    // Previous frame data for calculations
    UPROPERTY()
    FVector PreviousVelocity;

    UPROPERTY()
    FRotator PreviousRotation;

    UPROPERTY()
    float PreviousSpeed = 0.0f;

private:
    void CacheComponentReferences();
    void UpdateTargetBlendData();
    void SmoothBlendData(float DeltaTime);
    void UpdateSpeedAndDirection();
    void UpdateLeanAndTurn(float DeltaTime);
    void UpdateSlope();
    void ProcessOverrides();
    float CalculateDirectionFromVelocity(const FVector& Velocity, const FRotator& ActorRotation) const;
    float CalculateLeanFromAcceleration(const FVector& Acceleration) const;
    float CalculateTurnRate(const FRotator& CurrentRotation, const FRotator& PreviousRotation, float DeltaTime) const;
    float SmoothDamp(float Current, float Target, float SmoothTime, float DeltaTime) const;
};