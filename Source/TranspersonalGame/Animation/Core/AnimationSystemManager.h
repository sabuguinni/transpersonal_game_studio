#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "IKRigDefinition.h"
#include "AnimationSystemManager.generated.h"

// Forward declarations
class USkeletalMeshComponent;
class UAnimationBlueprint;
class UControlRig;

/**
 * Animation states for prehistoric character movement
 */
UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Combat      UMETA(DisplayName = "Combat")
};

/**
 * Terrain adaptation types for IK system
 */
UENUM(BlueprintType)
enum class EAnim_TerrainType : uint8
{
    Flat        UMETA(DisplayName = "Flat Ground"),
    Slope       UMETA(DisplayName = "Sloped Terrain"),
    Rocky       UMETA(DisplayName = "Rocky Surface"),
    Muddy       UMETA(DisplayName = "Muddy Ground"),
    Water       UMETA(DisplayName = "Shallow Water"),
    Sand        UMETA(DisplayName = "Sandy Surface"),
    Grass       UMETA(DisplayName = "Grassy Terrain"),
    Snow        UMETA(DisplayName = "Snowy Ground")
};

/**
 * Motion Matching configuration for different character types
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionMatchingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* MotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float PoseSearchRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bUseTrajectoryMatching = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    bool bUseBoneVelocityMatching = true;

    FAnim_MotionMatchingConfig()
    {
        MotionDatabase = nullptr;
    }
};

/**
 * IK configuration for foot placement and terrain adaptation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    class UIKRigDefinition* IKRig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootAdjustmentSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float MaxFootOffset = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnableHandIK = false;

    FAnim_IKConfig()
    {
        IKRig = nullptr;
    }
};

/**
 * Animation data for different character archetypes
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_CharacterAnimData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FAnim_MotionMatchingConfig MotionMatchingConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FAnim_IKConfig IKConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TMap<EAnim_MovementState, class UAnimMontage*> ActionMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TMap<EAnim_TerrainType, class UBlendSpace*> TerrainBlendSpaces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float TurnRate = 90.0f;

    FAnim_CharacterAnimData()
    {
        CharacterName = TEXT("Unknown");
    }
};

/**
 * Core Animation System Manager
 * Handles Motion Matching, IK, and character-specific animation logic
 * Designed for prehistoric survival game with realistic movement
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core animation data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation System")
    FAnim_CharacterAnimData CharacterAnimData;

    // Current animation state
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_TerrainType CurrentTerrainType;

    // Component references
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

public:
    // Animation state management
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetTerrainType(EAnim_TerrainType NewTerrainType);

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_TerrainType GetCurrentTerrainType() const { return CurrentTerrainType; }

    // Motion Matching
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void InitializeMotionMatching();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatchingDatabase(class UPoseSearchDatabase* NewDatabase);

    // IK System
    UFUNCTION(BlueprintCallable, Category = "IK")
    void InitializeIKSystem();

    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FVector GetFootPlacementOffset(bool bIsLeftFoot) const;

    // Montage playback
    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool PlayActionMontage(EAnim_MovementState ActionType, float PlayRate = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopCurrentMontage(float BlendOutTime = 0.25f);

    // Character setup
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void SetupCharacterAnimation(const FAnim_CharacterAnimData& NewAnimData);

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void BindToCharacterComponents();

    // Terrain analysis
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    EAnim_TerrainType AnalyzeCurrentTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetTerrainSlope() const;

private:
    // Internal state
    float CurrentSpeed;
    float CurrentDirection;
    FVector LastVelocity;
    
    // IK foot positions
    FVector LeftFootOffset;
    FVector RightFootOffset;
    
    // Terrain analysis
    float TerrainSlope;
    FVector TerrainNormal;
    
    // Helper functions
    void UpdateMovementParameters();
    void PerformTerrainTrace();
    void UpdateAnimationBlending(float DeltaTime);
    
    // Motion Matching helpers
    void ConfigureMotionMatchingParameters();
    void UpdateTrajectoryPrediction();
};