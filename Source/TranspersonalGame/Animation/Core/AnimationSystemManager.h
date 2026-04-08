#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/ActorComponent.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "AnimationSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECharacterType : uint8
{
    Player,
    SmallHerbivore,
    LargeHerbivore,
    SmallCarnivore,
    LargeCarnivore,
    AerialDinosaur
};

UENUM(BlueprintType)
enum class EMovementState : uint8
{
    Idle,
    Walking,
    Running,
    Sprinting,
    Crouching,
    Climbing,
    Swimming,
    Feeding,
    Sleeping,
    Alert,
    Aggressive,
    Fleeing,
    Hunting,
    Domesticated
};

USTRUCT(BlueprintType)
struct FCharacterAnimationProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterType CharacterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UPoseSearchDatabase> LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UPoseSearchDatabase> CombatDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UPoseSearchDatabase> InteractionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TurnRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IKIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseMotionMatching = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseLegIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseSpineIK = false;
};

/**
 * Sistema central de gestão de animação para o jogo Jurássico
 * Coordena Motion Matching, IK e sistemas especializados por tipo de personagem
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void InitializeForCharacter(ECharacterType InCharacterType, class USkeletalMeshComponent* SkeletalMesh);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateMovementState(EMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetDomesticationLevel(float Level); // 0.0 = selvagem, 1.0 = totalmente domesticado

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    FCharacterAnimationProfile GetAnimationProfile() const { return CurrentProfile; }

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void EnableTerrainAdaptation(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetIndividualVariation(float VariationSeed); // Para criar variações únicas

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Profiles")
    TMap<ECharacterType, FCharacterAnimationProfile> AnimationProfiles;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FCharacterAnimationProfile CurrentProfile;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EMovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float DomesticationLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float IndividualVariationSeed = 0.0f;

private:
    void LoadDefaultProfiles();
    void ApplyIndividualVariations();
    void UpdateIKSettings();

    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> OwnerSkeletalMesh;

    // Cache para performance
    float LastTerrainCheckTime = 0.0f;
    static constexpr float TerrainCheckInterval = 0.1f;
};