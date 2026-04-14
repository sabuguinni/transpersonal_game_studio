#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "../SharedTypes.h"
#include "AnimationSystemManager.generated.h"

// Forward declarations
class UMotionMatchingDatabase;
class UAnimSequence;
class UBlendSpace;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_LocomotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    bool bIsOnGround = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    bool bIsClimbing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    bool bIsSwimming = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    EAnim_TerrainType TerrainType = EAnim_TerrainType::Grass;

    FAnim_LocomotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsOnGround = true;
        bIsClimbing = false;
        bIsSwimming = false;
        TerrainType = EAnim_TerrainType::Grass;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKFootData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float IKAlpha = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bValidHit = false;

    FAnim_IKFootData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 1.0f;
        bValidHit = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Motion Matching System
    UFUNCTION(BlueprintCallable, Category = "Animation|Motion Matching")
    void InitializeMotionMatchingDatabase();

    UFUNCTION(BlueprintCallable, Category = "Animation|Motion Matching")
    UAnimSequence* FindBestMatchingAnimation(const FAnim_LocomotionData& LocomotionData);

    UFUNCTION(BlueprintCallable, Category = "Animation|Motion Matching")
    void RegisterCharacterForMotionMatching(class APawn* Character);

    // Procedural Animation System
    UFUNCTION(BlueprintCallable, Category = "Animation|Procedural")
    FAnim_IKFootData CalculateFootIK(class APawn* Character, const FName& FootBoneName, float TraceDistance = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation|Procedural")
    void UpdateTerrainAdaptation(class APawn* Character);

    UFUNCTION(BlueprintCallable, Category = "Animation|Procedural")
    void ApplyEnvironmentalAnimations(class APawn* Character, EAnim_TerrainType TerrainType);

    // Animation Montage System
    UFUNCTION(BlueprintCallable, Category = "Animation|Montages")
    bool PlaySurvivalMontage(class APawn* Character, EAnim_SurvivalAction Action);

    UFUNCTION(BlueprintCallable, Category = "Animation|Montages")
    void StopAllMontages(class APawn* Character);

    UFUNCTION(BlueprintCallable, Category = "Animation|Montages")
    bool IsPlayingMontage(class APawn* Character, EAnim_SurvivalAction Action);

    // Blend Space Management
    UFUNCTION(BlueprintCallable, Category = "Animation|BlendSpaces")
    void UpdateLocomotionBlendSpace(class APawn* Character, const FAnim_LocomotionData& LocomotionData);

    UFUNCTION(BlueprintCallable, Category = "Animation|BlendSpaces")
    UBlendSpace* GetTerrainSpecificBlendSpace(EAnim_TerrainType TerrainType);

    // Animation Events
    UFUNCTION(BlueprintCallable, Category = "Animation|Events")
    void TriggerFootstepEvent(class APawn* Character, const FName& FootBoneName);

    UFUNCTION(BlueprintCallable, Category = "Animation|Events")
    void TriggerSurvivalActionEvent(class APawn* Character, EAnim_SurvivalAction Action);

protected:
    // Motion Matching Database
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching", meta = (AllowPrivateAccess = "true"))
    UMotionMatchingDatabase* PrehistoricLocomotionDatabase;

    // Blend Spaces for different terrains
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blend Spaces", meta = (AllowPrivateAccess = "true"))
    TMap<EAnim_TerrainType, UBlendSpace*> TerrainBlendSpaces;

    // Animation Montages for survival actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montages", meta = (AllowPrivateAccess = "true"))
    TMap<EAnim_SurvivalAction, UAnimMontage*> SurvivalMontages;

    // Registered characters for motion matching
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching", meta = (AllowPrivateAccess = "true"))
    TArray<TWeakObjectPtr<APawn>> RegisteredCharacters;

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    float FootIKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    float FootIKInterpSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    float MaxFootIKOffset = 30.0f;

private:
    // Internal helper functions
    void LoadAnimationAssets();
    void SetupTerrainBlendSpaces();
    void SetupSurvivalMontages();
    
    FVector PerformFootTrace(class APawn* Character, const FName& FootBoneName, float TraceDistance);
    FRotator CalculateFootRotationFromNormal(const FVector& SurfaceNormal);
    
    bool ValidateCharacterForAnimation(class APawn* Character);
    class USkeletalMeshComponent* GetCharacterMesh(class APawn* Character);
};