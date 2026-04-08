#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "AnimationSystemManager.generated.h"

USTRUCT(BlueprintType)
struct FCharacterAnimationProfile : public FTableRowBase
{
    GENERATED_BODY()

    // Character archetype for animation selection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString ArchetypeID;

    // Motion Matching Database for this character type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchDatabase> LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchDatabase> CombatDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TSoftObjectPtr<UPoseSearchDatabase> InteractionDatabase;

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootIKIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float HandIKIntensity = 0.8f;

    // Fear Response Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear System")
    float FearMovementModifier = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear System")
    float CautiousWalkThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear System")
    float PanicRunThreshold = 0.8f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAnimationSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAnimationSystemManager();

protected:
    virtual void BeginPlay() override;

    // Character Animation Profiles Data Table
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation System")
    UDataTable* CharacterAnimationProfiles;

    // Global Animation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalAnimationSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    bool bEnableMotionMatching = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    bool bEnableFearSystem = true;

public:
    // Get animation profile for character archetype
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    FCharacterAnimationProfile GetAnimationProfile(const FString& ArchetypeID);

    // Register character with animation system
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void RegisterCharacter(AActor* Character, const FString& ArchetypeID);

    // Update fear level for character (affects animation selection)
    UFUNCTION(BlueprintCallable, Category = "Fear System")
    void UpdateCharacterFearLevel(AActor* Character, float FearLevel);

private:
    // Registered characters and their profiles
    UPROPERTY()
    TMap<AActor*, FString> RegisteredCharacters;

    UPROPERTY()
    TMap<AActor*, float> CharacterFearLevels;
};