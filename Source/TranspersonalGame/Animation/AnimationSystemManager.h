#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "../SharedTypes.h"
#include "AnimationSystemManager.generated.h"

// Forward declarations
class UMotionMatchingComponent;
class UTerrainIKComponent;

UENUM(BlueprintType)
enum class EAnim_PrehistoricAction : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Hunting         UMETA(DisplayName = "Hunting"), 
    Crafting        UMETA(DisplayName = "Crafting"),
    Social          UMETA(DisplayName = "Social"),
    Survival        UMETA(DisplayName = "Survival"),
    Combat          UMETA(DisplayName = "Combat"),
    Ritual          UMETA(DisplayName = "Ritual")
};

UENUM(BlueprintType)
enum class EAnim_TerrainType : uint8
{
    Rock            UMETA(DisplayName = "Rock"),
    Dirt            UMETA(DisplayName = "Dirt"),
    Grass           UMETA(DisplayName = "Grass"),
    Mud             UMETA(DisplayName = "Mud"),
    Sand            UMETA(DisplayName = "Sand"),
    Snow            UMETA(DisplayName = "Snow")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bIsAccelerating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    EAnim_PrehistoricAction CurrentAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float ActionBlendWeight;

    FAnim_MotionMatchingData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsAccelerating = false;
        CurrentAction = EAnim_PrehistoricAction::None;
        ActionBlendWeight = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TerrainAdaptationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    float GroundSlope;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    EAnim_TerrainType SurfaceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    FVector LeftFootTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    FVector RightFootTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    float IKBlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    bool bIsOnUnevenTerrain;

    FAnim_TerrainAdaptationData()
    {
        GroundSlope = 0.0f;
        SurfaceType = EAnim_TerrainType::Dirt;
        LeftFootTarget = FVector::ZeroVector;
        RightFootTarget = FVector::ZeroVector;
        IKBlendWeight = 0.0f;
        bIsOnUnevenTerrain = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MontageConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    TSoftObjectPtr<UAnimMontage> MontageAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    FString MontageName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    EAnim_PrehistoricAction ActionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    float PlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    TArray<FString> SectionNames;

    FAnim_MontageConfig()
    {
        MontageName = TEXT("DefaultMontage");
        ActionType = EAnim_PrehistoricAction::None;
        PlayRate = 1.0f;
        bLooping = false;
    }
};

/**
 * Core Animation System Manager for Transpersonal Game
 * Handles Motion Matching, Terrain Adaptation IK, and Prehistoric Action Montages
 * Inspired by RDR2's character-specific movement and Richard Williams' principles of life in animation
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion Matching System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FAnim_MotionMatchingData MotionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UBlendSpace> LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float MotionMatchingThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime;

    // Terrain Adaptation System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    FAnim_TerrainAdaptationData TerrainData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    float IKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    bool bEnableTerrainAdaptation;

    // Prehistoric Action Montages
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Actions")
    TMap<EAnim_PrehistoricAction, FAnim_MontageConfig> ActionMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Actions")
    EAnim_PrehistoricAction CurrentPlayingAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prehistoric Actions")
    bool bIsPlayingActionMontage;

    // Character Reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    USkeletalMeshComponent* CharacterMesh;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    UAnimInstance* AnimInstance;

public:
    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatchingData(const FVector& InVelocity, float InSpeed, float InDirection);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionMatchingBlendSpace(UBlendSpace* InBlendSpace);

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FAnim_MotionMatchingData GetMotionMatchingData() const { return MotionData; }

    // Terrain Adaptation Functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    void UpdateTerrainAdaptation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    void PerformFootIKTrace(bool bLeftFoot, FVector& OutFootTarget, float& OutIKAlpha);

    UFUNCTION(BlueprintCallable, Category = "Terrain Adaptation")
    void SetTerrainAdaptationEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Terrain Adaptation")
    FAnim_TerrainAdaptationData GetTerrainAdaptationData() const { return TerrainData; }

    // Prehistoric Action Functions
    UFUNCTION(BlueprintCallable, Category = "Prehistoric Actions")
    bool PlayPrehistoricAction(EAnim_PrehistoricAction ActionType, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Actions")
    void StopCurrentAction();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Actions")
    void RegisterActionMontage(EAnim_PrehistoricAction ActionType, UAnimMontage* Montage, const FString& MontageName);

    UFUNCTION(BlueprintPure, Category = "Prehistoric Actions")
    bool IsPlayingAction() const { return bIsPlayingActionMontage; }

    UFUNCTION(BlueprintPure, Category = "Prehistoric Actions")
    EAnim_PrehistoricAction GetCurrentAction() const { return CurrentPlayingAction; }

    // System Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void InitializeAnimationSystem(ACharacter* InCharacter);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetupDefaultMontages();

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void ResetAnimationSystem();

protected:
    // Internal helper functions
    void UpdateCharacterReferences();
    void CalculateMotionMatchingParameters();
    void ProcessTerrainAdaptation(float DeltaTime);
    void HandleActionMontageEvents();

    // Terrain analysis
    EAnim_TerrainType AnalyzeSurfaceType(const FHitResult& HitResult);
    float CalculateGroundSlope(const FVector& GroundNormal);

    // Motion matching helpers
    float CalculateDirectionAngle(const FVector& Velocity, const FVector& Forward);
    bool ShouldTransitionMotion(const FAnim_MotionMatchingData& NewData);

private:
    // Internal state tracking
    float LastMotionUpdateTime;
    FVector LastValidVelocity;
    bool bSystemInitialized;
    
    // IK foot placement cache
    FVector CachedLeftFootTarget;
    FVector CachedRightFootTarget;
    float LeftFootIKAlpha;
    float RightFootIKAlpha;
};

#include "AnimationSystemManager.generated.h"