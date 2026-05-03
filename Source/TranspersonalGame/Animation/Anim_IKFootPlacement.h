#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float IKAlpha = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float DistanceFromGround = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bIsValidPlacement = false;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
        bIsValidPlacement = false;
    }
};

USTRUCT(BlueprintType)
struct FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootHeight = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxIKAdjustment = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bDebugDraw = false;

    FAnim_IKSettings()
    {
        TraceDistance = 150.0f;
        IKInterpSpeed = 15.0f;
        FootHeight = 10.0f;
        MaxIKAdjustment = 50.0f;
        bEnableIK = true;
        bDebugDraw = false;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootPlacement : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKFootPlacement();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // IK Functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData CalculateFootIK(const FName& SocketName, float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "IK")
    FAnim_FootIKData GetLeftFootIK() const { return LeftFootIK; }

    UFUNCTION(BlueprintPure, Category = "IK")
    FAnim_FootIKData GetRightFootIK() const { return RightFootIK; }

    UFUNCTION(BlueprintPure, Category = "IK")
    float GetHipOffset() const { return CurrentHipOffset; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetDebugDrawEnabled(bool bEnabled);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKSettings(const FAnim_IKSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "IK")
    FAnim_IKSettings GetIKSettings() const { return IKSettings; }

protected:
    // IK Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    float CurrentHipOffset = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    float TargetHipOffset = 0.0f;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    // Socket Names
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftFootSocketName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName RightFootSocketName = TEXT("foot_r");

    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

private:
    // Internal state
    float PreviousHipOffset = 0.0f;
    FAnim_FootIKData PreviousLeftFootIK;
    FAnim_FootIKData PreviousRightFootIK;

    // Helper functions
    void CacheComponentReferences();
    FVector GetSocketWorldLocation(const FName& SocketName) const;
    bool PerformFootTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const;
    float CalculateIKAlpha(float DistanceFromGround) const;
    FRotator CalculateFootRotation(const FVector& HitNormal, const FRotator& CurrentRotation) const;
    void UpdateHipOffset(float DeltaTime);
    void DrawDebugInfo() const;
};