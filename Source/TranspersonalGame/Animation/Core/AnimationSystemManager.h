#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataTable.h"
#include "AnimationSystemManager.generated.h"

class UMotionMatchingComponent;
class UIKRigComponent;
class UCharacterMovementComponent;

/**
 * Manages the core animation system for all characters and dinosaurs
 * Implements Motion Matching with IK foot placement and procedural variations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

    // Initialize the animation system
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void InitializeAnimationSystem();

    // Register a character with the animation system
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void RegisterCharacter(AActor* Character, ECharacterType CharacterType);

    // Update animation system (called per frame)
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateAnimationSystem(float DeltaTime);

    // Get motion matching database for character type
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    class UPoseSearchDatabase* GetMotionMatchingDatabase(ECharacterType CharacterType);

protected:
    // Motion Matching databases for different character types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<ECharacterType, class UPoseSearchDatabase*> MotionMatchingDatabases;

    // IK Rig assets for different character types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    TMap<ECharacterType, class UIKRigDefinition*> IKRigAssets;

    // Animation Blueprint classes for different character types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Blueprints")
    TMap<ECharacterType, TSubclassOf<UAnimInstance>> AnimBPClasses;

    // Registered characters
    UPROPERTY()
    TArray<AActor*> RegisteredCharacters;

private:
    // Performance monitoring
    float LastUpdateTime;
    int32 ActiveAnimationCount;
    
    // Update motion matching for character
    void UpdateMotionMatching(AActor* Character, float DeltaTime);
    
    // Update IK system for character
    void UpdateIKSystem(AActor* Character, float DeltaTime);
    
    // Apply procedural variations
    void ApplyProceduralVariations(AActor* Character, float DeltaTime);
};

// Character types for animation system
UENUM(BlueprintType)
enum class ECharacterType : uint8
{
    Player          UMETA(DisplayName = "Player Character"),
    SmallHerbivore  UMETA(DisplayName = "Small Herbivore"),
    LargeHerbivore  UMETA(DisplayName = "Large Herbivore"),
    SmallCarnivore  UMETA(DisplayName = "Small Carnivore"),
    LargeCarnivore  UMETA(DisplayName = "Large Carnivore"),
    FlyingCreature  UMETA(DisplayName = "Flying Creature"),
    AquaticCreature UMETA(DisplayName = "Aquatic Creature")
};

// Animation state for motion matching
UENUM(BlueprintType)
enum class EAnimationState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Sneaking        UMETA(DisplayName = "Sneaking"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Swimming        UMETA(DisplayName = "Swimming"),
    Flying          UMETA(DisplayName = "Flying"),
    Feeding         UMETA(DisplayName = "Feeding"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Alert           UMETA(DisplayName = "Alert"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Interacting     UMETA(DisplayName = "Interacting")
};

// Animation quality settings
UENUM(BlueprintType)
enum class EAnimationQuality : uint8
{
    Low             UMETA(DisplayName = "Low Quality"),
    Medium          UMETA(DisplayName = "Medium Quality"),
    High            UMETA(DisplayName = "High Quality"),
    Ultra           UMETA(DisplayName = "Ultra Quality")
};