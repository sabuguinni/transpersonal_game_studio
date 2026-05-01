#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "SurvivalFootIKSystem.generated.h"

/**
 * Foot IK data for a single foot
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FVector FootLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FRotator FootRotation;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float IKAlpha;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float DistanceFromGround;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
    }
};

/**
 * Foot IK settings for terrain adaptation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableDebugDraw;

    FAnim_FootIKSettings()
    {
        TraceDistance = 50.0f;
        InterpSpeed = 15.0f;
        IKThreshold = 5.0f;
        bEnableDebugDraw = false;
    }
};

/**
 * Survival Foot IK System - Adapts character feet to uneven terrain
 * Provides realistic foot placement for prehistoric survival gameplay
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalFootIKSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalFootIKSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core IK functionality
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData CalculateFootIK(const FName& SocketName, const FVector& RootLocation);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    bool TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKSettings(const FAnim_FootIKSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKSettings GetIKSettings() const { return IKSettings; }

    // Getters for animation blueprint
    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetLeftFootIK() const { return LeftFootIK; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetRightFootIK() const { return RightFootIK; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    float GetPelvisOffset() const { return PelvisOffset; }

protected:
    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FAnim_FootIKSettings IKSettings;

    // Current IK data
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float PelvisOffset;

    // Component references
    UPROPERTY()
    USkeletalMeshComponent* OwnerMesh;

    // Socket names for feet
    UPROPERTY(EditAnywhere, Category = "Foot IK")
    FName LeftFootSocketName;

    UPROPERTY(EditAnywhere, Category = "Foot IK")
    FName RightFootSocketName;

    // Internal state
    float TargetPelvisOffset;
    
private:
    void InitializeComponent();
    void UpdatePelvisOffset(float DeltaTime);
    void DrawDebugInfo();
};