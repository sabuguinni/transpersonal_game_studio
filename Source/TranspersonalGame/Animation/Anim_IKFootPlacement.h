#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "../SharedTypes.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName FootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName IKBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector FootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float DistanceFromGround;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bIsGrounded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector GroundNormal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector GroundLocation;

    FAnim_FootIKData()
    {
        FootBoneName = NAME_None;
        IKBoneName = NAME_None;
        FootOffset = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
        bIsGrounded = false;
        GroundNormal = FVector::UpVector;
        GroundLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxIKAdjustment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableDebugDraw;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    TEnumAsByte<ECollisionChannel> TraceChannel;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        FootHeight = 15.0f;
        InterpSpeed = 15.0f;
        MaxIKAdjustment = 30.0f;
        bEnableFootRotation = true;
        bEnableDebugDraw = false;
        TraceChannel = ECC_WorldStatic;
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

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    // Foot IK Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    TArray<FAnim_FootIKData> FootIKData;

    // Pelvis adjustment
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pelvis IK")
    float PelvisOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pelvis IK")
    float PelvisInterpSpeed;

    // IK control functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetFootIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    bool IsFootIKEnabled() const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData GetFootIKData(FName FootBoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void AddFootIK(FName FootBoneName, FName IKBoneName);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void RemoveFootIK(FName FootBoneName);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    float GetPelvisOffset() const { return PelvisOffset; }

    // Terrain adaptation
    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    bool PerformFootTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);

    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    FRotator CalculateFootRotation(const FVector& GroundNormal, const FRotator& CurrentRotation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    void UpdatePelvisHeight(float DeltaTime);

    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "IK Events")
    void OnFootGrounded(FName FootBoneName, const FVector& GroundLocation);

    UFUNCTION(BlueprintImplementableEvent, Category = "IK Events")
    void OnFootLifted(FName FootBoneName);

    UFUNCTION(BlueprintImplementableEvent, Category = "IK Events")
    void OnPelvisAdjusted(float NewOffset);

private:
    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // Internal state
    bool bIKEnabled;
    float TargetPelvisOffset;
    TMap<FName, FAnim_FootIKData> PreviousFootData;

    // Helper functions
    void CacheComponents();
    void InitializeFootIKData();
    void UpdateFootIKData(FAnim_FootIKData& FootData, float DeltaTime);
    FVector GetFootWorldLocation(FName FootBoneName) const;
    void DrawDebugInfo() const;
    float CalculateLowestFootOffset() const;
};