#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Anim_TerrainAdaptation.generated.h"

// Forward declarations
class ACharacter;
class UCharacterMovementComponent;
class UCapsuleComponent;

UENUM(BlueprintType)
enum class EAnim_TerrainType : uint8
{
    Flat        UMETA(DisplayName = "Flat Ground"),
    Slope       UMETA(DisplayName = "Sloped Ground"),
    Stairs      UMETA(DisplayName = "Stairs"),
    Uneven      UMETA(DisplayName = "Uneven Terrain"),
    Soft        UMETA(DisplayName = "Soft Ground"),
    Water       UMETA(DisplayName = "Water/Mud"),
    Ice         UMETA(DisplayName = "Ice/Slippery")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector FootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bIsGrounded;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        FootOffset = 0.0f;
        bIsGrounded = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TerrainData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EAnim_TerrainType TerrainType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float SlopeAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector SurfaceNormal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Roughness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bIsStable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float PelvisOffset;

    FAnim_TerrainData()
    {
        TerrainType = EAnim_TerrainType::Flat;
        SlopeAngle = 0.0f;
        SurfaceNormal = FVector::UpVector;
        Roughness = 0.0f;
        bIsStable = true;
        PelvisOffset = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootOffset = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MinFootOffset = -30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnablePelvisOffset = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftFootBone = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName RightFootBone = TEXT("foot_r");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName PelvisBone = TEXT("pelvis");
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_TerrainAdaptation : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_TerrainAdaptation();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Main IK functions
    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    FAnim_FootIKData CalculateFootIK(bool bIsLeftFoot);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void UpdateTerrainAnalysis();

    // Terrain detection
    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    EAnim_TerrainType AnalyzeTerrainType(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    float CalculateSlopeAngle(const FVector& SurfaceNormal);

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    bool PerformFootTrace(const FVector& StartLocation, FVector& HitLocation, FVector& HitNormal);

    // Getters for Animation Blueprint
    UFUNCTION(BlueprintPure, Category = "Foot IK Data")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIKData; }

    UFUNCTION(BlueprintPure, Category = "Foot IK Data")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIKData; }

    UFUNCTION(BlueprintPure, Category = "Terrain Data")
    FAnim_TerrainData GetTerrainData() const { return CurrentTerrainData; }

    UFUNCTION(BlueprintPure, Category = "Foot IK Data")
    float GetPelvisOffset() const { return CurrentTerrainData.PelvisOffset; }

    UFUNCTION(BlueprintPure, Category = "Foot IK Data")
    bool IsFootIKEnabled() const { return IKSettings.bEnableFootIK; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "IK Configuration")
    void SetIKSettings(const FAnim_IKSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "IK Configuration")
    void EnableFootIK(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "IK Configuration")
    void SetTraceDistance(float Distance);

protected:
    // Character references
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    UPROPERTY()
    TObjectPtr<UCapsuleComponent> CapsuleComponent;

    // IK data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData LeftFootIKData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData RightFootIKData;

    // Terrain data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
    FAnim_TerrainData CurrentTerrainData;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Configuration")
    FAnim_IKSettings IKSettings;

    // Collision settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CapsuleHalfHeight = 88.0f;

private:
    // Internal helper functions
    void InitializeReferences();
    FVector GetFootWorldLocation(bool bIsLeftFoot) const;
    FVector CalculatePelvisAdjustment() const;
    void UpdatePelvisOffset(float DeltaTime);
    bool IsValidForIK() const;
    
    // Smoothing variables
    float TargetPelvisOffset = 0.0f;
    float CurrentPelvisOffset = 0.0f;
    
    // Performance optimization
    float LastUpdateTime = 0.0f;
    float UpdateInterval = 0.016f; // ~60fps
};

#include "Anim_TerrainAdaptation.generated.h"