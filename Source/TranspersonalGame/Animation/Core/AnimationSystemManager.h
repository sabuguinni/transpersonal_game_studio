#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "AnimationSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
    Idle,
    Cautious,      // Movimento cauteloso - estado padrão do protagonista
    Sneaking,      // Esgueirando-se para evitar predadores
    Running,       // Corrida de fuga
    Climbing,      // Escalada de rochas/árvores
    Swimming,      // Natação em rios
    Injured,       // Movimento ferido
    Exhausted,     // Movimento exausto
    Observing,     // Observando dinossauros escondido
    Crafting       // Construindo/criando ferramentas
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle,
    Grazing,       // Herbívoros pastando
    Hunting,       // Predadores caçando
    Drinking,      // Bebendo água
    Sleeping,      // Dormindo
    Alert,         // Estado de alerta
    Aggressive,    // Comportamento agressivo
    Fleeing,       // Fugindo de predadores maiores
    Socializing,   // Interação social (manadas)
    Territorial    // Defendendo território
};

USTRUCT(BlueprintType)
struct FCharacterAnimationProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CautiousnessFactor = 0.8f; // 0.0 = confiante, 1.0 = extremamente cauteloso

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FatigueLevel = 0.0f; // 0.0 = energético, 1.0 = exausto

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearLevel = 0.3f; // 0.0 = sem medo, 1.0 = terror absoluto

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UPoseSearchDatabase> MotionMatchingDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UAnimSequence*> IdleVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UAnimSequence*> CautiousMovements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UAnimSequence*> PanicAnimations;
};

USTRUCT(BlueprintType)
struct FDinosaurAnimationProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Size = 1.0f; // Multiplicador de tamanho

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Aggressiveness = 0.5f; // 0.0 = pacífico, 1.0 = extremamente agressivo

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHerbivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UPoseSearchDatabase> LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UPoseSearchDatabase> BehaviorDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UAnimSequence*> UniqueIdleVariations; // Para tornar cada dinossauro único

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UAnimSequence*> SocialInteractions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UAnimSequence*> FeedingAnimations;
};

/**
 * Gestor central do sistema de animação
 * Coordena Motion Matching, IK, e perfis de personagens
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAnimationSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAnimationSystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Animation")
    FCharacterAnimationProfile ProtagonistProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Animation")
    TArray<FDinosaurAnimationProfile> DinosaurProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float BlendTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootIKOffset = 50.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Animation")
    FCharacterAnimationProfile GetProtagonistProfile() const { return ProtagonistProfile; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FDinosaurAnimationProfile GetDinosaurProfile(const FString& SpeciesName) const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateCharacterFearLevel(float NewFearLevel);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateCharacterFatigue(float NewFatigueLevel);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    UPoseSearchDatabase* GetMotionMatchingDatabase(ECharacterMovementState MovementState) const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    UPoseSearchDatabase* GetDinosaurDatabase(const FString& SpeciesName, EDinosaurBehaviorState BehaviorState) const;
};