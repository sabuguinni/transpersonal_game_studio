#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector FootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float DistanceFromGround;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bValidPlacement;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
        bValidPlacement = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MinFootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bDrawDebugTraces;

    FAnim_IKSettings()
    {
        TraceDistance = 150.0f;
        InterpSpeed = 15.0f;
        MaxFootOffset = 50.0f;
        MinFootOffset = -50.0f;
        bEnableIK = true;
        bDrawDebugTraces = false;
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

    // Main IK Functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData GetLeftFootIK() const { return LeftFootIK; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData GetRightFootIK() const { return RightFootIK; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    float GetHipOffset() const { return HipOffset; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "IK")
    bool IsIKEnabled() const { return IKSettings.bEnableIK; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetDebugTraces(bool bEnabled);

    // Bone name configuration
    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetFootBoneNames(const FName& LeftFootBone, const FName& RightFootBone);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKSettings(const FAnim_IKSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_IKSettings GetIKSettings() const { return IKSettings; }

    // Terrain adaptation
    UFUNCTION(BlueprintCallable, Category = "IK")
    float GetTerrainSlope() const { return TerrainSlope; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    FVector GetTerrainNormal() const { return TerrainNormal; }

protected:
    // IK Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    float HipOffset;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    // Bone Names
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName HipBoneName;

    // Terrain Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
    float TerrainSlope;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
    FVector TerrainNormal;

    // Performance optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUpdateOnlyWhenMoving;

private:
    // Cached references
    class ACharacter* OwnerCharacter;
    class USkeletalMeshComponent* MeshComponent;
    class UCharacterMovementComponent* MovementComponent;

    // Internal timing
    float TimeSinceLastUpdate;
    float LastUpdateTime;

    // Previous frame data for smoothing
    FAnim_FootIKData PreviousLeftFootIK;
    FAnim_FootIKData PreviousRightFootIK;
    float PreviousHipOffset;

    // Helper functions
    void InitializeComponent();
    void UpdateCharacterReferences();
    FAnim_FootIKData CalculateFootIK(const FName& FootBoneName, float DeltaTime);
    FVector GetFootWorldLocation(const FName& FootBoneName) const;
    bool PerformFootTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const;
    float CalculateHipAdjustment() const;
    void SmoothIKTransitions(FAnim_FootIKData& FootIK, const FAnim_FootIKData& PreviousFootIK, float DeltaTime);
    bool ShouldUpdateIK() const;
    void DrawDebugInfo() const;
};