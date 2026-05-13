#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimationAsset.h"
#include "Anim_TerrainAdaptationSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_TerrainType : uint8
{
    Flat            UMETA(DisplayName = "Flat Ground"),
    Slope           UMETA(DisplayName = "Slope"),
    SteepSlope      UMETA(DisplayName = "Steep Slope"),
    Rocks           UMETA(DisplayName = "Rocky Terrain"),
    Mud             UMETA(DisplayName = "Muddy Ground"),
    Sand            UMETA(DisplayName = "Sandy Ground"),
    Grass           UMETA(DisplayName = "Grassy Ground"),
    Water           UMETA(DisplayName = "Shallow Water"),
    Ice             UMETA(DisplayName = "Icy Surface"),
    Uneven          UMETA(DisplayName = "Uneven Ground")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootPlacementData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot Placement")
    FVector LeftFootPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot Placement")
    FVector RightFootPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot Placement")
    FRotator LeftFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot Placement")
    FRotator RightFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot Placement")
    float LeftFootIKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot Placement")
    float RightFootIKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot Placement")
    float HipOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot Placement")
    bool bLeftFootGrounded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot Placement")
    bool bRightFootGrounded;

    FAnim_FootPlacementData()
    {
        LeftFootPosition = FVector::ZeroVector;
        RightFootPosition = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        LeftFootIKAlpha = 0.0f;
        RightFootIKAlpha = 0.0f;
        HipOffset = 0.0f;
        bLeftFootGrounded = true;
        bRightFootGrounded = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TerrainAdaptationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    EAnim_TerrainType CurrentTerrainType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    float SlopeAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    FVector SlopeDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    float TerrainRoughness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    float MovementSpeedModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    float StabilityFactor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    bool bRequiresCarefulMovement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    FAnim_FootPlacementData FootPlacement;

    FAnim_TerrainAdaptationData()
    {
        CurrentTerrainType = EAnim_TerrainType::Flat;
        SlopeAngle = 0.0f;
        SlopeDirection = FVector::ZeroVector;
        TerrainRoughness = 0.0f;
        MovementSpeedModifier = 1.0f;
        StabilityFactor = 1.0f;
        bRequiresCarefulMovement = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_TerrainAdaptationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_TerrainAdaptationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core terrain adaptation functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    void UpdateTerrainAdaptation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    void AnalyzeTerrain();

    UFUNCTION(BlueprintPure, Category = "Terrain Adaptation")
    FAnim_TerrainAdaptationData GetTerrainAdaptationData() const { return TerrainData; }

    UFUNCTION(BlueprintPure, Category = "Terrain Adaptation")
    EAnim_TerrainType GetCurrentTerrainType() const { return TerrainData.CurrentTerrainType; }

    // Foot IK functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void CalculateFootPlacement(bool bIsLeftFoot, FVector& OutFootPosition, FRotator& OutFootRotation, float& OutIKAlpha);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void PerformFootTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal, bool& bHit);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void CalculateHipOffset();

    // Terrain analysis
    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    void DetectTerrainType();

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    void CalculateSlopeData();

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    void AssessTerrainStability();

    UFUNCTION(BlueprintCallable, Category = "Terrain Analysis")
    float GetTerrainRoughness() const;

    // Movement adaptation
    UFUNCTION(BlueprintCallable, Category = "Movement Adaptation")
    void ApplyTerrainMovementModifiers();

    UFUNCTION(BlueprintCallable, Category = "Movement Adaptation")
    float CalculateSpeedModifier() const;

    UFUNCTION(BlueprintCallable, Category = "Movement Adaptation")
    bool ShouldUseCarefulMovement() const;

protected:
    // Core terrain data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain Data")
    FAnim_TerrainAdaptationData TerrainData;

    // Character references
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    USkeletalMeshComponent* MeshComponent;

    // Foot IK settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float FootTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float FootIKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float HipOffsetInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float MaxFootIKOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float MaxHipOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    FName RightFootBoneName;

    // Terrain detection settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Detection")
    float TerrainSampleRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Detection")
    int32 TerrainSampleCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Detection")
    float SlopeAngleThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Detection")
    float SteepSlopeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Detection")
    float RoughnessThreshold;

    // Movement adaptation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Adaptation")
    float MinSpeedModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Adaptation")
    float MaxSpeedModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Adaptation")
    float StabilityThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Adaptation")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Adaptation")
    bool bEnableTerrainDetection;

    // Cached data
    UPROPERTY()
    TArray<FVector> TerrainSamplePoints;

    UPROPERTY()
    TArray<FVector> TerrainNormals;

    UPROPERTY()
    float LastTerrainAnalysisTime;

    UPROPERTY()
    float TerrainAnalysisInterval;

private:
    void InitializeComponent();
    void CacheCharacterReferences();
    void SetupTerrainSampling();
    void UpdateFootIKTargets(float DeltaTime);
    void InterpolateFootIKValues(float DeltaTime);
    FVector GetFootWorldLocation(bool bIsLeftFoot) const;
    FVector CalculateAverageTerrainNormal() const;
    float CalculateTerrainVariance() const;
    EAnim_TerrainType ClassifyTerrainFromSamples() const;
    void SmoothTerrainValues(float DeltaTime);
};